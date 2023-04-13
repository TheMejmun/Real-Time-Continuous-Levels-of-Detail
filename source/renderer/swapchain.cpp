//
// Created by Sam on 2023-04-11.
//

#include "renderer.h"

void Renderer::createSurface() {
    if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
        THROW("Failed to create window surface!");
    }
}

SwapchainSupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    VkSurfaceFormatKHR out = availableFormats[0];

    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            out = availableFormat;
        }
    }

    DBG "Picked Swapchain Surface Format: " ENDL;
    DBG "\tFormat: " << out.format ENDL;
    DBG "\tColor Space: " << out.colorSpace ENDL;

    return out;
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
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

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
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

    this->framebufferWidth = out.width;
    this->framebufferHeight = out.height;

    DBG "Swapchain extents set to: " << out.width << " * " << out.height ENDL;
    return out;
}

bool Renderer::recreateSwapchain() {
    DBG "Recreate Swapchain" ENDL;

    // May need to recreate render pass here if e.g. window moves to HDR monitor

    vkDeviceWaitIdle(this->logicalDevice);

    destroySwapchain();

    return createSwapchain();
}

bool Renderer::createSwapchain() {
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    if (extent.width < 1 || extent.height < 1) {
        DBG "Invalid swapchain extents. Retry later!" ENDL;
        this->needsNewSwapchain = true;
        return false;
    }

    // One more image than the minimum to avoid stalling if the driver is still working on the image
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }
    DBG "Creating the Swapchain with at least " << imageCount << " images!" ENDL;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // Can be 2 for 3D, etc.
    // TODO switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing, instead of directly rendering to the SC
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueIndices[] = {this->queueFamilyIndices.graphicsFamily.value(),
                               this->queueFamilyIndices.presentFamily.value()};

    if (!this->queueFamilyIndices.isUnifiedGraphicsPresentQueue()) {
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

    createInfo.presentMode = presentMode;

    // Clip pixels if obscured by other window -> Perf+
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr; // Put previous swapchain here if overridden, e.g. if window size changed

    if (vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
        THROW("Failed to create swap chain!");
    }

    // imageCount only specified a minimum!
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, nullptr);
    this->swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, this->swapchainImages.data());
    this->swapchainImageFormat = surfaceFormat.format;
    this->swapchainExtent = extent;

    createImageViews();
    createRenderPass();
    createFramebuffers();

    return true;
}

void Renderer::destroySwapchain() {
    for (auto &swapchainFramebuffer: this->swapchainFramebuffers) {
        vkDestroyFramebuffer(this->logicalDevice, swapchainFramebuffer, nullptr);
    }

    vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);

    for (auto &swapchainImageView: this->swapchainImageViews) {
        vkDestroyImageView(this->logicalDevice, swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(this->logicalDevice, this->swapchain, nullptr);
}

void Renderer::createImageViews() {
    this->swapchainImageViews.resize(this->swapchainImages.size());

    for (size_t i = 0; i < this->swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 1D - 3D or Cube maps
        createInfo.format = this->swapchainImageFormat;

        // Can swizzle all components to be mapped to a single channel, or map to constants, etc.
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

        if (vkCreateImageView(this->logicalDevice, &createInfo, nullptr, &this->swapchainImageViews[i]) != VK_SUCCESS) {
            THROW("Failed to create image views!");
        }
    }
}

void Renderer::createFramebuffers() {
    this->swapchainFramebuffers.resize(this->swapchainImageViews.size());

    for (size_t i = 0; i < this->swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
                this->swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = this->swapchainExtent.width;
        framebufferInfo.height = this->swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(this->logicalDevice, &framebufferInfo, nullptr, &this->swapchainFramebuffers[i]) !=
            VK_SUCCESS) {
            THROW("Failed to create framebuffer!");
        }
    }
}

bool Renderer::shouldRecreateSwapchain() {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    bool framebufferChanged = w != this->framebufferWidth || h != this->framebufferHeight;

    return this->needsNewSwapchain || framebufferChanged;
}