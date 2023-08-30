//
// Created by Saman on 24.08.23.
//

#include "io/printer.h"
#include "graphics/vulkan/vulkan_swapchain.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "graphics/vulkan/vulkan_renderpasses.h"
#include "graphics/vulkan/vulkan_memory.h"
#include "graphics/vulkan/vulkan_imgui.h"

#include <GLFW/glfw3.h>
#include <algorithm>

// Global
VkSurfaceKHR VulkanSwapchain::surface = nullptr;
uint32_t VulkanSwapchain::framebufferWidth = 0, VulkanSwapchain::framebufferHeight = 0;
float VulkanSwapchain::aspectRatio = 1.0f;
VkSwapchainKHR VulkanSwapchain::swapchain = nullptr;
VkFormat VulkanSwapchain::imageFormat{};
VkPresentModeKHR VulkanSwapchain::presentMode;
VkExtent2D VulkanSwapchain::extent{};
std::vector<VkImage> VulkanSwapchain::images{};
std::vector<VkImageView> VulkanSwapchain::imageViews{};
std::vector<VkFramebuffer> VulkanSwapchain::framebuffers{};
bool VulkanSwapchain::needsNewSwapchain = false;
uint32_t VulkanSwapchain::minImageCount = 2;
uint32_t VulkanSwapchain::imageCount = 2;

// Local
GLFWwindow *window = nullptr;

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

void VulkanSwapchain::createSurface(GLFWwindow *w) {
    window = w;

    if (glfwCreateWindowSurface(VulkanInstance::instance, window, nullptr, &VulkanSwapchain::surface) != VK_SUCCESS) {
        THROW("Failed to create window surface!");
    }
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    VkSurfaceFormatKHR out = availableFormats[0];

    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            out = availableFormat;
        }
    }

    VRB "Picked Swapchain Surface Format: " ENDL;
    VRB "\tFormat: " << out.format ENDL;
    VRB "\tColor Space: " << out.colorSpace ENDL;

    return out;
}

VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    const std::vector<VkPresentModeKHR> presentModePreferences = {
            VK_PRESENT_MODE_FIFO_KHR,
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_MAILBOX_KHR
    };
    const std::vector<std::string> presentModeNames = {
            "V-Sync",
            "Uncapped",
            "Triple-Buffering"
    };
    uint32_t currentIndex = 0;

    for (const auto &availablePresentMode: availablePresentModes) {
        for (uint32_t i = 0; i < presentModePreferences.size(); ++i) {
            if (availablePresentMode == presentModePreferences[i] && i > currentIndex) {
                currentIndex = i;
            }
        }
    }

    INF "Picked Swapchain Present Mode: " << presentModeNames[currentIndex] ENDL;
    return presentModePreferences[currentIndex];
}

VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    VkExtent2D out;
    // If the current extents are set to the maximum values,
    // the window manager is trying to tell us to set it manually.
    // Otherwise, return the current value.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        out = capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        out = {
                static_cast<uint32_t>(w),
                static_cast<uint32_t>(h)
        };

        out.width = std::clamp(out.width,
                               capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width);
        out.height = std::clamp(out.height,
                                capabilities.minImageExtent.height,
                                capabilities.maxImageExtent.height);
    }

    VulkanSwapchain::framebufferWidth = out.width;
    VulkanSwapchain::framebufferHeight = out.height;
    VulkanSwapchain::aspectRatio = static_cast<float>(out.width) / static_cast<float>(out.height);

    INF "Swapchain extents set to: " << out.width << " * " << out.height ENDL;
    return out;
}

bool VulkanSwapchain::recreateSwapchain(RenderState &state) {
    VRB "Recreating Swapchain" ENDL;

    // May need to recreate render pass here if e.g. window moves to HDR monitor

    vkDeviceWaitIdle(VulkanDevices::logical);

    destroySwapchain();

    auto success = createSwapchain();

    if(success){
        VulkanImgui::recalculateScale(state);
    }

    return success;
}

bool VulkanSwapchain::createSwapchain() {
    INF "Creating VulkanSwapchain" ENDL;

    VulkanSwapchain::SwapchainSupportDetails swapchainSupport = VulkanSwapchain::querySwapchainSupport(
            VulkanDevices::physical);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentModeTemp = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extentTemp = chooseSwapExtent(swapchainSupport.capabilities);

    if (extentTemp.width < 1 || extentTemp.height < 1) {
        VRB "Invalid swapchain extents. Retry later!" ENDL;
        VulkanSwapchain::needsNewSwapchain = true;
        return false;
    }

    // One more image than the minimum to avoid stalling if the driver is still working on the image
    VulkanSwapchain::minImageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        VulkanSwapchain::minImageCount > swapchainSupport.capabilities.maxImageCount) {
        VulkanSwapchain::minImageCount = swapchainSupport.capabilities.maxImageCount;
    }
    VRB "Creating the swapchain with at least " << VulkanSwapchain::minImageCount << " images!" ENDL;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = VulkanSwapchain::surface;
    createInfo.minImageCount = VulkanSwapchain::minImageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extentTemp;
    createInfo.imageArrayLayers = 1; // Can be 2 for 3D, etc.
    // TODO switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing, instead of directly rendering to the SC
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueIndices[] = {VulkanDevices::queueFamilyIndices.graphicsFamily.value(),
                               VulkanDevices::queueFamilyIndices.presentFamily.value()};

    if (!VulkanDevices::queueFamilyIndices.isUnifiedGraphicsPresentQueue()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Image is shared between queues -> no transfers!
        createInfo.queueFamilyIndexCount = 2; // Concurrent mode requires at least two indices
        createInfo.pQueueFamilyIndices = queueIndices; // Share image between these queues
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Image is owned by one queue at a time -> Perf+
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // Do not add any swapchain transforms beyond the default
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;

    // Do not blend with other windows
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentModeTemp;

    // Clip pixels if obscured by other window -> Perf+
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr; // Put previous swapchain here if overridden, e.g. if window size changed

    if (vkCreateSwapchainKHR(VulkanDevices::logical, &createInfo, nullptr, &VulkanSwapchain::swapchain) != VK_SUCCESS) {
        THROW("Failed to create swapchain!");
    }

    // imageCount only specified a minimum!
    vkGetSwapchainImagesKHR(VulkanDevices::logical, VulkanSwapchain::swapchain, &VulkanSwapchain::imageCount, nullptr);
    VulkanSwapchain::images.resize(VulkanSwapchain::imageCount);
    vkGetSwapchainImagesKHR(VulkanDevices::logical, VulkanSwapchain::swapchain, &VulkanSwapchain::imageCount,
                            VulkanSwapchain::images.data());
    VulkanSwapchain::imageFormat = surfaceFormat.format;
    VulkanSwapchain::extent = extentTemp;
    VulkanSwapchain::presentMode = presentModeTemp;

    createImageViews();
    VulkanRenderPasses::create();
    createFramebuffers();

    return true;
}

void VulkanSwapchain::destroySwapchain() {
    INF "Destroying VulkanSwapchain" ENDL;

    for (auto &swapchainFramebuffer: VulkanSwapchain::framebuffers) {
        vkDestroyFramebuffer(VulkanDevices::logical, swapchainFramebuffer, nullptr);
    }

    VulkanRenderPasses::destroy();

    for (auto &swapchainImageView: VulkanSwapchain::imageViews) {
        vkDestroyImageView(VulkanDevices::logical, swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanDevices::logical, VulkanSwapchain::swapchain, nullptr);
}

void VulkanSwapchain::createImageViews() {
    VulkanSwapchain::imageViews.resize(VulkanSwapchain::images.size());

    for (size_t i = 0; i < VulkanSwapchain::images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = VulkanSwapchain::images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 1D - 3D or Cube maps
        createInfo.format = VulkanSwapchain::imageFormat;

        // Can swizzle all entities to be mapped to a single channel, or map to constants, etc.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Color, no mipmapping, single layer
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1; // No 3D

        if (vkCreateImageView(VulkanDevices::logical, &createInfo, nullptr, &VulkanSwapchain::imageViews[i]) !=
            VK_SUCCESS) {
            THROW("Failed to create image views!");
        }
    }
}

void VulkanSwapchain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

//    createImage(VulkanSwapchain::extent.width,
//                VulkanSwapchain::extent.height,
//                depthFormat,
//                VK_IMAGE_TILING_OPTIMAL,
//                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                VulkanSwapchain::depthImage,
//                VulkanSwapchain::depthImageMemory);
//    VulkanSwapchain::depthImageView = createImageView(depthImage, depthFormat);
}

VkFormat VulkanSwapchain::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                              VkFormatFeatureFlags features) {
    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(VulkanDevices::physical, format, &props);

        if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) ||
            (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }

    THROW("Failed to find supported format!");
}

VkFormat VulkanSwapchain::findDepthFormat() {
    return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, // -> More efficient https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageTiling.html
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT // -> Can be used as depth/stencil attachment & input attachment
    );
}

void VulkanSwapchain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                  VkImageUsageFlags usage,
                                  VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(VulkanDevices::logical, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        THROW("Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(VulkanDevices::logical, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanMemory::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(VulkanDevices::logical, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        THROW("Failed to allocate image memory!");
    }

    vkBindImageMemory(VulkanDevices::logical, image, imageMemory, 0);
}

void VulkanSwapchain::createFramebuffers() {
    VulkanSwapchain::framebuffers.resize(VulkanSwapchain::imageViews.size());

    for (size_t i = 0; i < VulkanSwapchain::imageViews.size(); i++) {
        VkImageView attachments[] = {
                VulkanSwapchain::imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = VulkanRenderPasses::renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = VulkanSwapchain::extent.width;
        framebufferInfo.height = VulkanSwapchain::extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(VulkanDevices::logical, &framebufferInfo, nullptr,
                                &VulkanSwapchain::framebuffers[i]) !=
            VK_SUCCESS) {
            THROW("Failed to create framebuffer!");
        }
    }
}

bool VulkanSwapchain::shouldRecreateSwapchain() {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    bool framebufferChanged = w != VulkanSwapchain::framebufferWidth || h != VulkanSwapchain::framebufferHeight;

    return VulkanSwapchain::needsNewSwapchain || framebufferChanged;
}