#include "Engine.h"


int main() {

    Magnet::Engine app{};

    //Initialization
    app.init();

    //Main App Loop
    while (not app.shouldClose()) {
        app.run();
    }

    app.waitIdle();

    return 0;
}