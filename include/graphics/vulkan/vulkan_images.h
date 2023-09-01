//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_IMAGES_H
#define REALTIME_CELL_COLLAPSE_VULKAN_IMAGES_H

#include "preprocessor.h"

#include <vulkan/vulkan.h>

namespace VulkanImages {
    void createTextureImage();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

    VkImageView
    createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_IMAGES_H
