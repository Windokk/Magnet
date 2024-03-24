#include "App.h"


int main() {

    Magnet::AppBase app{};

    //Initialization
    app.init();

    //Main App Loop
    while (not app.shouldClose()) {
        app.run();
    }

    app.waitIdle();

    return 0;
}