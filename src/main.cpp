#include "Application.h"
#include <iostream>

int main() {
    Application app(1920, 1080, "Portal Game");

    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }

    app.run();
    app.shutdown();

    return 0;
}

