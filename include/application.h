//
// Created by Sam on 2023-03-28.
//

#ifndef REALTIME_CELL_COLLAPSE_APPLICATION_H
#define REALTIME_CELL_COLLAPSE_APPLICATION_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>
#include <utility>
#include <vector>

const int32_t DEFAULT_WIDTH = 1280;
const int32_t DEFAULT_HEIGHT = 720;

const std::vector<const char *> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

class Application {
public:
    explicit Application(std::string title) : title(std::move(title)) {}

    void run();

    void run(const int32_t &w, const int32_t &h);

private:
    void initWindow();

    void createInstance();

    static void printAvailableVkExtensions();

    static bool checkValidationLayerSupport();

    void pickPhysicalDevice();

    void initVulkan();

    void mainLoop();

    void cleanup();

    std::string title;
    GLFWwindow *window = nullptr;
    int32_t width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;
    VkInstance instance = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_APPLICATION_H
