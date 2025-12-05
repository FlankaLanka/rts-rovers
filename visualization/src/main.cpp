#include "core/Application.h"
#include <iostream>

int main() {
    std::cout << "=================================\n";
    std::cout << " Terrafirma Rovers Command Center\n";
    std::cout << "=================================\n\n";

    terrafirma::Application app;
    
    if (!app.init()) {
        std::cerr << "Failed to initialize application\n";
        return 1;
    }

    std::cout << "\nControls:\n";
    std::cout << "  Right-click + drag: Look around\n";
    std::cout << "  WASD: Move camera\n";
    std::cout << "  Space/Shift: Move up/down\n";
    std::cout << "  1-5: Select rover\n";
    std::cout << "  F: Toggle follow mode\n";
    std::cout << "  F11: Toggle fullscreen\n";
    std::cout << "  ESC: Release mouse / Exit\n\n";

    app.run();
    
    return 0;
}

