
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include "application.h"
#include "colors.h"
#include <sstream>
#include <iomanip>

const glm::mat3 RGB_TO_XYZ_MAT = glm::mat3{
        0.4124564, 0.2126729, 0.0193339,
        0.3575761, 0.7151522, 0.1191920,
        0.1804375, 0.0721750, 0.9503041,
};

const glm::mat3 XYZ_TO_RGB_MAT = glm::mat3{
        3.2404549, -0.9692665, 0.0556434,
        -1.5371389, 1.8760110, -0.2040258,
        -0.4985316, 0.0415561, 1.0572252,
};

int main() {
//    std::stringstream stream;
//    stream << std::fixed << std::setprecision(7);
//    stream << RGB_TO_XYZ_MAT[0][0] << ", " << RGB_TO_XYZ_MAT[0][1] << ", " << RGB_TO_XYZ_MAT[0][2] << ", " << std::endl;
//    stream << RGB_TO_XYZ_MAT[1][0] << ", " << RGB_TO_XYZ_MAT[1][1] << ", " << RGB_TO_XYZ_MAT[1][2] << ", " << std::endl;
//    stream << RGB_TO_XYZ_MAT[2][0] << ", " << RGB_TO_XYZ_MAT[2][1] << ", " << RGB_TO_XYZ_MAT[2][2] << ", " << std::endl;
//    stream << std::endl;
//    stream << XYZ_TO_RGB_MAT[0][0] << ", " << XYZ_TO_RGB_MAT[0][1] << ", " << XYZ_TO_RGB_MAT[0][2] << ", " << std::endl;
//    stream << XYZ_TO_RGB_MAT[1][0] << ", " << XYZ_TO_RGB_MAT[1][1] << ", " << XYZ_TO_RGB_MAT[1][2] << ", " << std::endl;
//    stream << XYZ_TO_RGB_MAT[2][0] << ", " << XYZ_TO_RGB_MAT[2][1] << ", " << XYZ_TO_RGB_MAT[2][2] << ", " << std::endl;
//    std::cout << stream.str() << std::endl;
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