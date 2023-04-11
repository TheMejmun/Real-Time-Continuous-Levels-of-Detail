//
// Created by Sam on 2023-03-28.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDERER_H
#define REALTIME_CELL_COLLAPSE_RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <set>
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include "renderer.h"
#include "importer.h"
#include "vertex.h"
#include "triangle.h"
#include "timer.h"
#include "printer.h"

//#define WIREFRAME_MODE

const int MAX_FRAMES_IN_FLIGHT = 2;
// TODO https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight

const std::vector<const char *> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,

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

class Renderer {
public:
    void create(const std::string &title, GLFWwindow *window);

    sec draw();

    void destroy();

private:
    Triangle triangle{}; // TODO

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

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void createLogicalDevice();

    bool shouldRecreateSwapchain();

    bool createSwapchain();

    bool recreateSwapchain();

    void destroySwapchain();

    void createImageViews();

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void initVulkan();

    VkShaderModule createShaderModule(const std::vector<char> &code);

    void createCommandPool();

    void createVertexBuffer();

    void createIndexBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createCommandBuffer();

    void createSyncObjects();

    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);

    chrono_sec_point lastTimestamp = Timer::now();
    bool needsNewSwapchain = false;

    std::string title;
    GLFWwindow *window = nullptr;
    uint32_t framebufferWidth = 0, framebufferHeight = 0;

    // Vulkan
    VkInstance instance = nullptr;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice logicalDevice = nullptr;
    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue = nullptr;
    VkSurfaceKHR surface = nullptr;
    VkSwapchainKHR swapchain = nullptr;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat{};
    VkExtent2D swapchainExtent{};
    std::vector<VkImageView> swapchainImageViews;
    VkRenderPass renderPass = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline graphicsPipeline = nullptr;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;

    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkSemaphore imageAvailableSemaphore = nullptr;
    VkSemaphore renderFinishedSemaphore = nullptr;
    VkFence inFlightFence = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_RENDERER_H
