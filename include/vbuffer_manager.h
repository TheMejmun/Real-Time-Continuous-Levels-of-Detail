//
// Created by Sam on 2023-04-12.
//

#ifndef REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H
#define REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include "triangle.h"

class VBufferManager {
public:
    void create(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

    void destroy();

    void createCommandBuffer(VkCommandPool commandPool);

    void createVertexBuffer(Triangle triangle);

    void createIndexBuffer();

    VkDevice logicalDevice = nullptr;
    uint32_t maxAllocations = 0, currentAllocations = 0;
    VkPhysicalDeviceMemoryProperties memProperties{};

    VkCommandBuffer commandBuffer = nullptr; // Cleaned automatically by command pool clean.
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif //REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H
