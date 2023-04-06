//
// Created by Sam on 2023-03-28.
//

#ifndef REALTIME_CELL_COLLAPSE_ENGINE_H
#define REALTIME_CELL_COLLAPSE_ENGINE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>
#include <utility>
#include <vector>
#include <optional>

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

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    [[nodiscard]] bool isComplete() const;
};

class Engine {
public:
    explicit Engine(std::string title) : title(std::move(title)) {}

    void run();

    void run(const int32_t &w, const int32_t &h);

private:
    void initWindow();

    void createInstance();

    static void printAvailableVkExtensions();

    static bool checkValidationLayerSupport();

    void pickPhysicalDevice();

    void printAvailablePhysicalDevices();

    static bool isDeviceSuitable(VkPhysicalDevice device, bool strictMode);

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    void createSurface();

    void initVulkan();

    void mainLoop();

    void cleanup();

    std::string title;
    int32_t width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;
    GLFWwindow *window = nullptr;

    // Vulkan
    VkInstance instance = nullptr;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice logicalDevice = nullptr;
    VkQueue graphicsQueue = nullptr;
    VkSurfaceKHR surface = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_ENGINE_H
