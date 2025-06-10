#include "window/window.hpp"
#include "game/game.h"
#include <thread>

int main() {
    Overlay overlay;
    overlay.SetupOverlay("FiveM Cheat");
    FiveM::Setup();

    std::thread updateThread(FiveM::Update);

    while (overlay.shouldRun) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        overlay.StartRender();

        if (overlay.drawESP) {
            FiveM::ESP::RunESP(overlay);
        }
        if (overlay.RenderMenu)
            overlay.Render();
        overlay.EndRender();
    }

    updateThread.join();

    return 0;
}
