// ─────────────────────────────────────────────
// Freakland - Main Entry Point
// M2A: First-person gameplay with AABB collision.
// Walk around, jump, collide with geometry.
// F2 toggles between gameplay and debug camera.
// ─────────────────────────────────────────────

#include <freak/core/Log.h>
#include <freak/core/Assert.h>
#include <freak/core/Types.h>
#include <freak/core/Timer.h>
#include <freak/core/Profiler.h>
#include <freak/platform/App.h>
#include <freak/platform/Window.h>
#include <freak/platform/Input.h>
#include <freak/render/RenderDevice.h>
#include <freak/render/DebugUI.h>
#include <freak/render/Camera.h>
#include <freak/render/CameraData.h>
#include <freak/render/MeshCache.h>
#include <freak/render/ForwardPass.h>
#include <freak/scene/Scene.h>
#include <freak/scene/SceneLoader.h>
#include <freak/scene/SceneLighting.h>
#include <freak/scene/Components.h>
#include <freak/physics/CollisionWorld.h>
#include <freak/framework/InputActions.h>
#include <freak/framework/FPSController.h>
#include <freak/framework/GameplayCamera.h>

#ifdef FREAK_IMGUI_ENABLED
#include <imgui.h>
#endif

#include <SDL3/SDL_main.h>

#include <vector>
#include <cmath>

using namespace freak;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // ── Initialize core systems ──────────────────

    log::Init();
    FREAK_LOG_INFO("App", "Freakland v0.2.0 — M2A");

    auto appResult = AppInit();
    if (!appResult) {
        FREAK_LOG_FATAL("App", "Platform init failed");
        return 1;
    }

    // ── Create window ────────────────────────────

    Window window;
    {
        WindowConfig windowCfg;
        windowCfg.title  = "Freakland";
        windowCfg.width  = 1280;
        windowCfg.height = 720;

        auto result = window.Create(windowCfg);
        if (!result) {
            FREAK_LOG_FATAL("App", "Window creation failed");
            AppShutdown();
            return 1;
        }
    }

    // ── Initialize renderer ──────────────────────

    RenderDevice renderer;
    {
        RenderConfig renderCfg;
        renderCfg.width  = window.Width();
        renderCfg.height = window.Height();
        renderCfg.vsync  = true;

        auto result = renderer.Init(window, renderCfg);
        if (!result) {
            FREAK_LOG_FATAL("App", "Renderer init failed");
            window.Destroy();
            AppShutdown();
            return 1;
        }
    }

    // ── Initialize mesh cache (procedural geometry) ──

    MeshCache meshCache;
    {
        auto result = meshCache.Init();
        if (!result) {
            FREAK_LOG_FATAL("App", "MeshCache init failed");
            renderer.Shutdown();
            window.Destroy();
            AppShutdown();
            return 1;
        }
    }

    // ── Initialize forward render pass ───────────

    ForwardPass forwardPass;
    {
        auto result = forwardPass.Init();
        if (!result) {
            FREAK_LOG_FATAL("App", "ForwardPass init failed");
            meshCache.Shutdown();
            renderer.Shutdown();
            window.Destroy();
            AppShutdown();
            return 1;
        }
    }

    // ── Initialize debug UI ──────────────────────

    {
        auto result = DebugUI::Init();
        if (!result) {
            FREAK_LOG_WARN("App", "DebugUI init failed (non-fatal)");
        }
    }

    // ── Load scene ───────────────────────────────

    Scene scene;
    SceneLighting sceneLighting;
    {
        // MeshResolver lambda: bridges Scene (no bgfx knowledge) to Render's MeshCache.
        MeshResolver resolver = [&meshCache](StringView name) -> MeshID {
            return meshCache.FindByName(name);
        };

        auto result = LoadScene(
            "content/raw/scenes/test_scene.json",
            scene.Registry(),
            resolver
        );
        if (result) {
            sceneLighting = result->lighting;
            FREAK_LOG_INFO("App", "Scene loaded: {} entities", result->entityCount);
        } else {
            FREAK_LOG_WARN("App", "Failed to load scene — running with empty world");
        }
    }

    // ── Initialize collision world from scene ────

    CollisionWorld collisionWorld;
    // Maps collision body index → ECS entity (for interaction lookup)
    std::vector<entt::entity> bodyToEntity;
    {
        auto result = collisionWorld.Init();
        if (!result) {
            FREAK_LOG_FATAL("App", "CollisionWorld init failed");
            DebugUI::Shutdown();
            forwardPass.Shutdown();
            meshCache.Shutdown();
            renderer.Shutdown();
            window.Destroy();
            AppShutdown();
            return 1;
        }

        // Populate collision bodies from scene entities
        auto view = scene.Registry().view<TransformComponent, ColliderComponent>();
        u32 colliderCount = 0;
        for (auto entity : view) {
            const auto& transform = view.get<TransformComponent>(entity);
            const auto& collider  = view.get<ColliderComponent>(entity);
            collisionWorld.AddStaticBox(transform.position, collider.halfExtents);
            bodyToEntity.push_back(entity);
            ++colliderCount;
        }
        FREAK_LOG_INFO("App", "Collision world: {} static bodies", colliderCount);
    }

    // ── Initialize cameras ───────────────────────

    f32 aspect = static_cast<f32>(window.Width()) / static_cast<f32>(window.Height());

    // Debug camera (free-fly, kept for development)
    DebugCamera debugCamera;
    {
        CameraState startState;
        startState.position = {0.0f, 3.0f, 8.0f};
        startState.yaw = -90.0f;
        startState.pitch = -10.0f;
        debugCamera.Init(startState);
        debugCamera.SetAspect(aspect);
    }

    // Gameplay camera (first-person, locked to controller)
    GameplayCamera gameplayCamera;
    gameplayCamera.Init(-90.0f, 0.0f);
    gameplayCamera.SetAspect(aspect);

    bool useDebugCamera = false; // Start in gameplay mode

    // ── Initialize FPS controller ────────────────

    FPSController fpsController;
    {
        FPSControllerConfig fpsConfig;
        // Default values are fine for M2A
        glm::vec3 spawnPos{0.0f, 0.0f, 8.0f}; // Feet position (Y=0 = ground)
        fpsController.Init(spawnPos, fpsConfig);
        FREAK_LOG_INFO("App", "FPS controller spawned at (0, 0, 8)");
    }

    // ── Initialize frame timer ───────────────────

    FrameTimer timer;
    timer.Init();

    Input input;
    InputActions actions;
    bool mouseCaptured = false;

    // Interaction state (updated each frame in gameplay mode)
    constexpr f32 interactRange = 3.5f; // Max interaction distance in units
    const char* interactLabel = nullptr; // Current frame's interactable label (or null)

    FREAK_LOG_INFO("App", "All systems initialized. Entering main loop.");
    FREAK_LOG_INFO("App", "Controls: WASD=move, Mouse=look, Shift=sprint, Space=jump, Ctrl=crouch");
    FREAK_LOG_INFO("App", "          E=interact, F2=toggle debug camera");
    FREAK_LOG_INFO("App", "          Right-click=capture mouse, ESC=release/quit");
    FREAK_LOG_INFO("App", "          F1=bgfx stats");

    // ── Main loop ────────────────────────────────

    bool running = true;
    bool firstFrame = true;
    while (running) {
        FREAK_FRAME_MARK;
        FREAK_PROFILE_SCOPE_N("MainLoop");

        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] Loop start");

        timer.BeginFrame();
        f32 dt = static_cast<f32>(timer.DeltaTime());

        // Clamp delta time to prevent physics explosion on window drag / breakpoint
        if (dt > 0.1f) dt = 0.1f;

        // ── Input ────────────────────────────────
        {
            FREAK_PROFILE_SCOPE_N("Input");
            running = input.PollEvents();
        }
        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] After input (running={})", running);

        // Map raw input to gameplay actions
        UpdateInputActions(actions, input);

        // Handle window resize
        if (input.WindowResized()) {
            i32 w = input.WindowWidth();
            i32 h = input.WindowHeight();
            if (w > 0 && h > 0) {
                window.OnResize(w, h);
                renderer.OnResize(w, h);
                f32 newAspect = static_cast<f32>(w) / static_cast<f32>(h);
                debugCamera.SetAspect(newAspect);
                gameplayCamera.SetAspect(newAspect);
            }
        }

        // Mouse capture toggle: right-click to capture, ESC to release
        if (input.IsMouseButtonDown(3) && !mouseCaptured) {
            mouseCaptured = true;
            input.SetMouseCaptured(true);
        }
        if (input.IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
            if (mouseCaptured) {
                mouseCaptured = false;
                input.SetMouseCaptured(false);
            } else {
                running = false;
            }
        }

        // F1 to toggle bgfx stats
        if (input.IsKeyPressed(SDL_SCANCODE_F1)) {
            renderer.ToggleStats();
        }

        // F2 to toggle between gameplay and debug camera
        if (actions.toggleDebugCam) {
            useDebugCamera = !useDebugCamera;
            if (useDebugCamera) {
                // Switching TO debug camera: copy gameplay camera position
                CameraState debugState;
                debugState.position = fpsController.EyePosition();
                debugState.yaw = gameplayCamera.Yaw();
                debugState.pitch = gameplayCamera.Pitch();
                debugCamera.Init(debugState);
                debugCamera.SetAspect(
                    static_cast<f32>(renderer.Width()) / static_cast<f32>(renderer.Height())
                );
                FREAK_LOG_INFO("App", "Switched to DEBUG camera");
            } else {
                FREAK_LOG_INFO("App", "Switched to GAMEPLAY camera");
            }
        }

        // ── Gameplay update ──────────────────────
        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] Before gameplay update");
        CameraData activeCameraData;

        if (useDebugCamera) {
            // Debug camera: free-fly, controlled by raw input
            FREAK_PROFILE_SCOPE_N("DebugCameraUpdate");
            debugCamera.Update(input, dt);
            activeCameraData = debugCamera.GetCameraData();
        } else {
            // Gameplay mode: controller + camera
            {
                FREAK_PROFILE_SCOPE_N("GameplayUpdate");

                // Update gameplay camera look (mouse)
                if (mouseCaptured) {
                    gameplayCamera.Update(
                        fpsController.EyePosition(),
                        actions.lookDelta.x,
                        actions.lookDelta.y
                    );
                } else {
                    // Still update position even if mouse isn't captured
                    // (but don't apply mouse look)
                    gameplayCamera.Update(
                        fpsController.EyePosition(),
                        0.0f,
                        0.0f
                    );
                }

                // Update FPS controller (movement + collision)
                fpsController.Update(
                    actions,
                    collisionWorld,
                    gameplayCamera.Yaw(),
                    dt
                );
            }

            activeCameraData = gameplayCamera.GetCameraData();
        }

        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] After gameplay update");

        // ── Interaction trace ────────────────────
        interactLabel = nullptr;
        if (!useDebugCamera) {
            FREAK_PROFILE_SCOPE_N("InteractionTrace");

            RayHit hit = collisionWorld.Raycast(
                fpsController.EyePosition(),
                gameplayCamera.Forward(),
                interactRange
            );

            if (hit.hit && hit.bodyIndex < static_cast<u32>(bodyToEntity.size())) {
                entt::entity hitEntity = bodyToEntity[hit.bodyIndex];
                auto& reg = scene.Registry();

                if (reg.valid(hitEntity) && reg.all_of<InteractableComponent>(hitEntity)) {
                    const auto& interact = reg.get<InteractableComponent>(hitEntity);
                    interactLabel = interact.label;

                    // E key = interact
                    if (actions.interact) {
                        // Get entity name if available
                        if (reg.all_of<NameComponent>(hitEntity)) {
                            const auto& name = reg.get<NameComponent>(hitEntity);
                            FREAK_LOG_INFO("Interact", "[{}] {} (dist: {:.1f})",
                                interact.label, name.name, hit.distance);
                        } else {
                            FREAK_LOG_INFO("Interact", "[{}] (dist: {:.1f})",
                                interact.label, hit.distance);
                        }
                    }
                }
            }
        }

        // ── Debug UI ─────────────────────────────
        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] Before DebugUI");
        {
            FREAK_PROFILE_SCOPE_N("DebugUI");
            DebugUI::BeginFrame(
                renderer.Width(),
                renderer.Height(),
                dt
            );

            #ifdef FREAK_IMGUI_ENABLED
            {
                ImGui::Begin("Freak Engine");
                ImGui::Text("Freakland v0.2.0 (M2A)");
                ImGui::Separator();

                // Frame stats
                ImGui::Text("Frame: %llu", timer.FrameCount());
                ImGui::Text("FPS:   %.1f", timer.FPS());
                ImGui::Text("dt:    %.3f ms", dt * 1000.0f);
                ImGui::Separator();

                // Render stats
                ImGui::Text("Resolution: %dx%d", renderer.Width(), renderer.Height());
                ImGui::Text("Draw calls: %u", forwardPass.DrawCallCount());
                ImGui::Text("Triangles:  %u", forwardPass.TriangleCount());
                ImGui::Separator();

                // Camera mode
                ImGui::Text("Camera: %s", useDebugCamera ? "DEBUG (free-fly)" : "GAMEPLAY (FPS)");
                if (useDebugCamera) {
                    auto pos = debugCamera.Position();
                    ImGui::Text("Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
                    ImGui::Text("Yaw: %.1f  Pitch: %.1f", debugCamera.Yaw(), debugCamera.Pitch());
                    ImGui::Text("Speed: %.1f u/s", debugCamera.MoveSpeed());
                } else {
                    auto pos = fpsController.Position();
                    auto vel = fpsController.Velocity();
                    auto eye = fpsController.EyePosition();
                    ImGui::Text("Body pos:  %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
                    ImGui::Text("Eye pos:   %.2f, %.2f, %.2f", eye.x, eye.y, eye.z);
                    ImGui::Text("Velocity:  %.2f, %.2f, %.2f", vel.x, vel.y, vel.z);
                    f32 hSpeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
                    ImGui::Text("H-Speed:   %.2f u/s", hSpeed);
                    ImGui::Text("Grounded:  %s", fpsController.IsGrounded() ? "YES" : "NO");
                    ImGui::Text("Crouching: %s", fpsController.IsCrouching() ? "YES" : "NO");
                    ImGui::Text("Yaw: %.1f  Pitch: %.1f", gameplayCamera.Yaw(), gameplayCamera.Pitch());
                }
                ImGui::Text("Mouse: %s", mouseCaptured ? "CAPTURED" : "free");
                ImGui::Separator();

                // Interaction
                if (interactLabel) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f),
                        "[E] %s", interactLabel);
                } else if (!useDebugCamera) {
                    ImGui::TextDisabled("(no interactable in range)");
                }
                ImGui::Separator();

                // Collision stats
                ImGui::Text("Collision bodies: %u", collisionWorld.BodyCount());
                ImGui::Separator();

                // Fog controls (live tweaking)
                ImGui::Text("Fog");
                ImGui::SliderFloat("Density", &sceneLighting.fogDensity, 0.0f, 0.2f);
                ImGui::SliderFloat("Start",   &sceneLighting.fogStart,   0.0f, 50.0f);
                ImGui::SliderFloat("End",     &sceneLighting.fogEnd,     10.0f, 200.0f);
                ImGui::ColorEdit3("Fog Color", &sceneLighting.fogColor.x);
                ImGui::Separator();

                // Light controls
                ImGui::Text("Moonlight");
                ImGui::SliderFloat("Intensity", &sceneLighting.lightIntensity, 0.0f, 2.0f);
                ImGui::ColorEdit3("Light Color", &sceneLighting.lightColor.x);
                ImGui::ColorEdit3("Ambient", &sceneLighting.ambientColor.x);
                ImGui::Separator();

                ImGui::Text("F1: bgfx stats | F2: toggle camera | Ctrl: crouch");
                ImGui::Text("Right-click: capture mouse | ESC: release/quit");
                ImGui::End();

                // ── Crosshair (gameplay mode only) ──
                if (!useDebugCamera) {
                    ImDrawList* drawList = ImGui::GetForegroundDrawList();
                    f32 cx = static_cast<f32>(renderer.Width()) * 0.5f;
                    f32 cy = static_cast<f32>(renderer.Height()) * 0.5f;
                    constexpr f32 size = 8.0f;
                    constexpr f32 gap  = 3.0f;
                    ImU32 color = interactLabel
                        ? IM_COL32(255, 255, 80, 220)   // Yellow when interactable
                        : IM_COL32(200, 200, 200, 180);  // Grey default
                    // Horizontal lines
                    drawList->AddLine({cx - size, cy}, {cx - gap, cy}, color, 1.5f);
                    drawList->AddLine({cx + gap, cy}, {cx + size, cy}, color, 1.5f);
                    // Vertical lines
                    drawList->AddLine({cx, cy - size}, {cx, cy - gap}, color, 1.5f);
                    drawList->AddLine({cx, cy + gap}, {cx, cy + size}, color, 1.5f);
                    // Center dot
                    drawList->AddCircleFilled({cx, cy}, 1.5f, color);
                }
            }
            #endif

            DebugUI::EndFrame();
        }

        // ── Render ───────────────────────────────
        if (firstFrame) FREAK_LOG_INFO("App", "[DBG] Before render");
        {
            FREAK_PROFILE_SCOPE_N("Render");
            renderer.BeginFrame();
            if (firstFrame) FREAK_LOG_INFO("App", "[DBG] After BeginFrame");

            forwardPass.Execute(
                activeCameraData,
                scene.Registry(),
                meshCache,
                sceneLighting
            );
            if (firstFrame) FREAK_LOG_INFO("App", "[DBG] After ForwardPass");

            renderer.EndFrame();
            if (firstFrame) FREAK_LOG_INFO("App", "[DBG] After EndFrame");
        }

        // ── Tracy plots ──────────────────────────
        FREAK_PROFILE_PLOT("FPS", timer.FPS());
        FREAK_PROFILE_PLOT("Frame ms", static_cast<double>(dt) * 1000.0);
        if (firstFrame) {
            FREAK_LOG_INFO("App", "[DBG] Frame 1 complete!");
            firstFrame = false;
        }
    }

    // ── Shutdown (reverse init order) ────────────

    FREAK_LOG_INFO("App", "Shutting down...");

    collisionWorld.Shutdown();
    DebugUI::Shutdown();
    forwardPass.Shutdown();
    meshCache.Shutdown();
    renderer.Shutdown();
    window.Destroy();
    AppShutdown();
    log::Shutdown();

    return 0;
}
