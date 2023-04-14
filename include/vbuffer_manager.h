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

    void destroyCommandBuffer(VkCommandPool commandPool);

    void nextUniformBuffer();

    void *getCurrentUniformBufferMapping();

    VkBuffer getCurrentUniformBuffer();

    VkBuffer getUniformBuffer(uint32_t i);

    uint32_t maxAllocations = 0, currentAllocations = 0;

    VkCommandBuffer commandBuffer = nullptr; // Cleaned automatically by command pool clean.
    VkBuffer vertexBuffer = nullptr;
    VkBuffer indexBuffer = nullptr;

    uint32_t uniformBufferIndex = UBO_BUFFER_COUNT;

    static constexpr uint32_t UBO_BUFFER_COUNT = 2;
private:
    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffers();

    void createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer);

    void createTransferCommandPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer,
                      VkDeviceMemory *pBufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDevice logicalDevice = nullptr;
    QueueFamilyIndices queueFamilyIndices{};
    VkPhysicalDeviceMemoryProperties memProperties{};

    VkDeviceMemory vertexBufferMemory = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};
    std::vector<void *> uniformBuffersMapped{};

    VkQueue transferQueue = nullptr;
    VkCommandPool transferCommandPool = nullptr;
    VkCommandBuffer transferCommandBuffer = nullptr; // Cleaned automatically by command pool clean.

    Triangle triangle{}; // TODO
};

#endif //REALTIME_CELL_COLLAPSE_VBUFFER_MANAGER_H
