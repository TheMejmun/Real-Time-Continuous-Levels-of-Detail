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

const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const;

    [[nodiscard]] bool isUnifiedGraphicsPresentQueue() const;
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Engine {
public:
    explicit Engine(std::string title) : title(std::move(title)) {}

    void run();

    void run(const int32_t &w, const int32_t &h);

private:
    void initWindow();

    void createInstance();

    static void printAvailableInstanceExtensions();

    static bool checkValidationLayerSupport();

    void createSurface();

    void pickPhysicalDevice();

    void printAvailablePhysicalDevices();

    bool isDeviceSuitable(VkPhysicalDevice device, bool strictMode);

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void createLogicalDevice();

    void createSwapchain();

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
    VkQueue presentQueue = nullptr;
    VkSurfaceKHR surface = nullptr;
    VkSwapchainKHR swapchain = nullptr;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
};

#endif //REALTIME_CELL_COLLAPSE_ENGINE_H
