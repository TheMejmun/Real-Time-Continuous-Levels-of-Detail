//
// Created by Saman on 24.08.23.
//

#define STB_IMAGE_IMPLEMENTATION // Only add this to one cpp file to create the implementation

#include <vulkan/vulkan.h>
#include <stb_image.h>
#include "graphics/vulkan/vulkan_images.h"
#include "io/printer.h"

void VulkanImages::createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("resources/textures/planet-albedo.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        THROW("Failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // TODO https://vulkan-tutorial.com/Texture_mapping/Images

    stbi_image_free(pixels);
}