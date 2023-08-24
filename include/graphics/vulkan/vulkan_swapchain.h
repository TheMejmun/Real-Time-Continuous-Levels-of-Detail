//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
#define REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vector>

namespace VulkanSwapchain {
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    extern VkSurfaceKHR surface;

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_SWAPCHAIN_H
