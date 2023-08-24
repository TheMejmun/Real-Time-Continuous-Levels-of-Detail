//
// Created by Saman on 24.08.23.
//

#include "graphics/vulkan/vulkan_swapchain.h"

VkSurfaceKHR VulkanSwapchain::surface = nullptr;

VulkanSwapchain::SwapchainSupportDetails VulkanSwapchain::querySwapchainSupport(VkPhysicalDevice device) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VulkanSwapchain::surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanSwapchain::surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanSwapchain::surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanSwapchain::surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanSwapchain::surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}