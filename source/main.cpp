
#include "application.h"

#include <iostream>

int main() {
    Application app{};
    app.title = "Hello World!";

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}