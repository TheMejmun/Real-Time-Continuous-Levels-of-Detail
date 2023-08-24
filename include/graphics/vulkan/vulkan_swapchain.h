//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
#define REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vector>
#include "util/glfw_include.h"

namespace VulkanSwapchain {
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    extern VkSurfaceKHR surface;
    extern uint32_t framebufferWidth, framebufferHeight;
    extern VkSwapchainKHR swapchain;
    extern VkFormat imageFormat;
    extern VkExtent2D extent;
    extern std::vector<VkImage> images;
    extern std::vector<VkImageView> imageViews;
    extern std::vector<VkFramebuffer> framebuffers;
    extern bool needsNewSwapchain;

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);

    void createSurface(GLFWwindow *window);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    bool shouldRecreateSwapchain();

    bool createSwapchain();

    bool recreateSwapchain();

    void destroySwapchain();

    void createImageViews();

    void createFramebuffers();

    void createDepthResources();

    VkFormat
    findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
