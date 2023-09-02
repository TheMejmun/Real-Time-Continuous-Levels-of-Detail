//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
#define REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H

#include "preprocessor.h"
#include "graphics/render_state.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>

//const int MAX_FRAMES_IN_FLIGHT = 2;

namespace VulkanSwapchain {
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    extern VkSurfaceKHR surface;
    extern uint32_t framebufferWidth, framebufferHeight;
    extern float aspectRatio;
    extern VkSwapchainKHR swapchain;
    extern VkPresentModeKHR presentMode;
    extern uint32_t minImageCount;
    extern uint32_t imageCount;
    extern VkFormat imageFormat;
    extern VkExtent2D extent;
    extern std::vector<VkImage> images;
    extern std::vector<VkImageView> imageViews;
    extern std::vector<VkFramebuffer> framebuffers;
    extern bool needsNewSwapchain;

    extern VkImage depthImage;
    extern VkDeviceMemory depthImageMemory;
    extern VkImageView depthImageView;

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);

    void createSurface(GLFWwindow *window);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    bool shouldRecreateSwapchain();

    bool createSwapchain();

    bool recreateSwapchain(RenderState &state);

    void destroySwapchain();

    void createImageViews();

    void createFramebuffers();

    void createDepthResources();

    VkFormat
    findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
