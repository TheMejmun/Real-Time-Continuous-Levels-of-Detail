
//#define GLM_FORCE_RADIANS 1
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
//#define GLM_FORCE_LEFT_HANDED 1
//#define GLM_FORCE_MESSAGES 1

#include <iostream>
#include "application.h"

int main() {
    Application app{};
    app.title = "Hello World!";

    glm::vec4 test{0};
    DBG test.x ENDL;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}