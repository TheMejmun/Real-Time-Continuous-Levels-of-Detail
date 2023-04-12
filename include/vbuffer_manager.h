//
// Created by Sam on 2023-04-12.
//

#ifndef REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H
#define REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include "triangle.h"
#include "queue_family_indices.h"

class VBufferManager {
public:
    void create(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, QueueFamilyIndices indices);

    void destroy();

    void createCommandBuffer(VkCommandPool commandPool);

    void createVertexBuffer();

    void createIndexBuffer();

    uint32_t maxAllocations = 0, currentAllocations = 0;
    VkCommandBuffer commandBuffer = nullptr; // Cleaned automatically by command pool clean.
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;

private:
    void createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer);

    void createTransferCommandPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer,
                      VkDeviceMemory *pBufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDevice logicalDevice = nullptr;
    QueueFamilyIndices queueFamilyIndices{};
    VkPhysicalDeviceMemoryProperties memProperties{};
    VkQueue transferQueue = nullptr;
    VkCommandPool transferCommandPool = nullptr;
    VkCommandBuffer transferCommandBuffer = nullptr; // Cleaned automatically by command pool clean.

    Triangle triangle{}; // TODO
};

#endif //REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H
