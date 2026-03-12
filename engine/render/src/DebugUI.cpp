#include <freak/render/DebugUI.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#ifdef FREAK_IMGUI_ENABLED

// ImGui + bgfx integration
// NOTE: bgfx ships with an imgui integration in examples/common/imgui.
// For M0, we use a minimal setup. The full imgui/bgfx backend will be
// wired up once we confirm the dependency chain works cleanly.
// This file is a placeholder that compiles and links — the actual
// ImGui rendering via bgfx will be completed as the first task of M0
// integration week.

#include <imgui.h>
#include <SDL3/SDL_events.h>

namespace freak::DebugUI {

static bool s_initialized = false;

Status Init([[maybe_unused]] f32 fontSize) {
    FREAK_PROFILE_SCOPE;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Use dark theme as base, then customize for our moody aesthetic
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.Alpha = 0.92f;

    // Font will be loaded here once we have the imgui/bgfx backend wired
    // For now ImGui uses its built-in font.
    // We MUST build the font atlas before the first NewFrame() call,
    // otherwise ImGui will assert/crash.
    unsigned char* pixels = nullptr;
    int texW = 0, texH = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &texW, &texH);
    // Set a dummy font texture ID — rendering won't use it yet,
    // but ImGui needs a non-null value to proceed.
    io.Fonts->SetTexID(ImTextureID(1));

    s_initialized = true;
    FREAK_LOG_INFO("DebugUI", "ImGui initialized (v{})", IMGUI_VERSION);
    return {};
}

void Shutdown() {
    if (s_initialized) {
        ImGui::DestroyContext();
        s_initialized = false;
        FREAK_LOG_INFO("DebugUI", "ImGui shutdown");
    }
}

void BeginFrame(i32 width, i32 height, f64 deltaTime) {
    FREAK_PROFILE_SCOPE_N("DebugUI::BeginFrame");

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<f32>(width), static_cast<f32>(height));
    io.DeltaTime = static_cast<f32>(deltaTime);

    ImGui::NewFrame();
}

void EndFrame() {
    FREAK_PROFILE_SCOPE_N("DebugUI::EndFrame");

    ImGui::Render();

    // TODO: Submit ImGui draw data to bgfx here.
    // This requires the imgui bgfx rendering backend.
    // For M0, ImGui windows are created but not yet rendered to screen.
    // The bgfx imgui backend integration is the first post-bootstrap task.
}

bool ProcessEvent(const SDL_Event& event) {
    // TODO: Forward SDL events to ImGui (keyboard, mouse, text input)
    // This will be implemented alongside the full imgui backend.
    (void)event;
    return false;
}

bool WantsCaptureKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool WantsCaptureMouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

} // namespace freak::DebugUI

#endif // FREAK_IMGUI_ENABLED
