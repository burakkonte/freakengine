// ─────────────────────────────────────────────
// Freak Engine - Asset Baker
// Offline tool for converting raw assets to cooked format.
// Implementation: M1
// ─────────────────────────────────────────────

#include <freak/core/Log.h>
#include <cstdio>

int main(int argc, char* argv[]) {
    freak::log::Init();
    FREAK_LOG_INFO("Baker", "Asset baker placeholder — implementation in M1");

    if (argc < 2) {
        std::fprintf(stderr, "Usage: asset_baker <command> [options]\n");
        std::fprintf(stderr, "Commands:\n");
        std::fprintf(stderr, "  mesh   <input.gltf> <output.fmesh>\n");
        std::fprintf(stderr, "  tex    <input.png>  <output.ftex>\n");
        std::fprintf(stderr, "  scene  <input.json> <output.fscene>\n");
        return 1;
    }

    FREAK_LOG_WARN("Baker", "Not yet implemented. Exiting.");
    freak::log::Shutdown();
    return 0;
}
