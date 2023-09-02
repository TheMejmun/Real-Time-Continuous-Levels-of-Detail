//
// Created by Saman on 24.08.23.
//

#define STB_IMAGE_IMPLEMENTATION // Only add this to one cpp file to create the implementation

#include "graphics/vulkan/vulkan_images.h"
#include "io/printer.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "graphics/vulkan/vulkan_memory.h"

#include <vulkan/vulkan.h>
#include <stb_image.h>


void VulkanImages::createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("resources/textures/planet-albedo.png", &texWidth, &texHeight, &texChannels,
                                STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        THROW("Failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // TODO https://vulkan-tutorial.com/Texture_mapping/Images

    stbi_image_free(pixels);
}

void VulkanImages::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
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

VkImageView VulkanImages::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;

    // Can swizzle all entities to be mapped to a single channel, or map to constants, etc.
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(VulkanDevices::logical, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        THROW("Failed to create texture image view!");
    }

    return imageView;
}