
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include "application.h"
#include "colors.h"

int main() {
//    auto color = Color::fromRGB({0.0f, 1.0f, 0.0f}).setLumaLab(0.5);
//    std::cout << color.toString() << std::endl;
//    return 0;

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