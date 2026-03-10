// ─────────────────────────────────────────────
// Freakland - Main Entry Point
// M1: Load a scene, render 3D geometry with lighting
// and fog, navigate with a free-fly debug camera.
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
#include <freak/render/MeshCache.h>
#include <freak/render/ForwardPass.h>
#include <freak/scene/Scene.h>
#include <freak/scene/SceneLoader.h>
#include <freak/scene/SceneLighting.h>

#ifdef FREAK_IMGUI_ENABLED
#include <imgui.h>
#endif

#include <SDL3/SDL_main.h>

using namespace freak;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // ── Initialize core systems ──────────────────

    log::Init();
    FREAK_LOG_INFO("App", "Freakland v0.1.0 — M1");

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
        // This runs at load time only — the hot render path uses MeshID directly.
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

    // ── Initialize camera ────────────────────────

    DebugCamera camera;
    {
        CameraState startState;
        startState.position = {0.0f, 3.0f, 8.0f};
        startState.yaw = -90.0f;
        startState.pitch = -10.0f;
        camera.Init(startState);
        camera.SetAspect(
            static_cast<f32>(window.Width()) / static_cast<f32>(window.Height())
        );
    }

    // ── Initialize frame timer ───────────────────

    FrameTimer timer;
    timer.Init();

    Input input;
    bool mouseCaptured = false;

    FREAK_LOG_INFO("App", "All systems initialized. Entering main loop.");
    FREAK_LOG_INFO("App", "Controls: WASD=move, Mouse=look, Shift=fast, E/Q=up/down");
    FREAK_LOG_INFO("App", "          Right-click=capture mouse, ESC=release/quit");
    FREAK_LOG_INFO("App", "          F1=bgfx stats, +/-=camera speed");

    // ── Main loop ────────────────────────────────

    bool running = true;
    while (running) {
        FREAK_FRAME_MARK;
        FREAK_PROFILE_SCOPE_N("MainLoop");

        timer.BeginFrame();

        // ── Input ────────────────────────────────
        {
            FREAK_PROFILE_SCOPE_N("Input");
            running = input.PollEvents();
        }

        // Handle window resize
        if (input.WindowResized()) {
            i32 w = input.WindowWidth();
            i32 h = input.WindowHeight();
            if (w > 0 && h > 0) {
                window.OnResize(w, h);
                renderer.OnResize(w, h);
                camera.SetAspect(static_cast<f32>(w) / static_cast<f32>(h));
            }
        }

        // Mouse capture toggle: right-click to capture, ESC to release
        if (input.IsMouseButtonDown(3) && !mouseCaptured) { // SDL button 3 = right
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

        // ── Camera update ────────────────────────
        {
            FREAK_PROFILE_SCOPE_N("CameraUpdate");
            camera.Update(input, timer.DeltaTime());
        }

        // ── Debug UI ─────────────────────────────
        {
            FREAK_PROFILE_SCOPE_N("DebugUI");
            DebugUI::BeginFrame(
                renderer.Width(),
                renderer.Height(),
                timer.DeltaTime()
            );

            #ifdef FREAK_IMGUI_ENABLED
            {
                ImGui::Begin("Freak Engine");
                ImGui::Text("Freakland v0.1.0 (M1)");
                ImGui::Separator();

                // Frame stats
                ImGui::Text("Frame: %llu", timer.FrameCount());
                ImGui::Text("FPS:   %.1f", timer.FPS());
                ImGui::Text("dt:    %.3f ms", timer.DeltaTime() * 1000.0);
                ImGui::Separator();

                // Render stats
                ImGui::Text("Resolution: %dx%d", renderer.Width(), renderer.Height());
                ImGui::Text("Draw calls: %u", forwardPass.DrawCallCount());
                ImGui::Text("Triangles:  %u", forwardPass.TriangleCount());
                ImGui::Separator();

                // Camera state
                auto pos = camera.Position();
                ImGui::Text("Camera pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
                ImGui::Text("Yaw: %.1f  Pitch: %.1f", camera.Yaw(), camera.Pitch());
                ImGui::Text("Speed: %.1f u/s", camera.MoveSpeed());
                ImGui::Text("Mouse: %s", mouseCaptured ? "CAPTURED" : "free");
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

                ImGui::Text("F1: bgfx stats | ESC: release mouse / quit");
                ImGui::Text("Right-click: capture mouse");
                ImGui::End();
            }
            #endif

            DebugUI::EndFrame();
        }

        // ── Render ───────────────────────────────
        {
            FREAK_PROFILE_SCOPE_N("Render");
            renderer.BeginFrame();

            forwardPass.Execute(
                camera,
                scene.Registry(),
                meshCache,
                sceneLighting
            );

            renderer.EndFrame();
        }

        // ── Tracy plots ──────────────────────────
        FREAK_PROFILE_PLOT("FPS", timer.FPS());
        FREAK_PROFILE_PLOT("Frame ms", timer.DeltaTime() * 1000.0);
    }

    // ── Shutdown (reverse init order) ────────────

    FREAK_LOG_INFO("App", "Shutting down...");

    DebugUI::Shutdown();
    forwardPass.Shutdown();
    meshCache.Shutdown();
    renderer.Shutdown();
    window.Destroy();
    AppShutdown();
    log::Shutdown();

    return 0;
}
