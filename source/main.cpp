

#include <iostream>
#include "application.h"
#include "imgui_example.h"

//#define IMGUI_EXAMPLE

int main() {
#ifdef IMGUI_EXAMPLE
    return imguiExample();
#else

    Application app{};
    app.title = "Hello World!";

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

#endif
}