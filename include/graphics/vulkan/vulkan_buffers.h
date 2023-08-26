//
// Created by Sam on 2023-04-12.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H
#define REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H

#include <vulkan/vulkan.h>
#include "graphics/triangle.h"
#include "util/byte_size.h"
#include "vulkan_devices.h"

namespace VulkanBuffers {
    extern uint32_t maxAllocations, currentAllocations;

    extern VkCommandBuffer commandBuffer; // Cleaned automatically by command pool clean.
    extern VkBuffer vertexBuffer;
    extern uint32_t vertexCount;
    extern VkBuffer indexBuffer;
    extern uint32_t indexCount;

    extern const uint32_t UBO_BUFFER_COUNT;
    extern const uint32_t DEFAULT_ALLOCATION_SIZE;

    extern uint32_t uniformBufferIndex;

    extern VkPhysicalDeviceMemoryProperties memProperties;

    extern VkDeviceMemory vertexBufferMemory;
    extern VkDeviceMemory indexBufferMemory;
    extern std::vector<VkBuffer> uniformBuffers;
    extern std::vector<VkDeviceMemory> uniformBuffersMemory;
    extern std::vector<void *> uniformBuffersMapped;

    extern VkQueue transferQueue;
    extern VkCommandPool transferCommandPool;
    extern VkCommandBuffer transferCommandBuffer; // Cleaned automatically by command pool clean.

    void create();

    void destroy();

    void createCommandBuffer(VkCommandPool commandPool);

    void destroyCommandBuffer(VkCommandPool commandPool);

    void nextUniformBuffer();

    void *getCurrentUniformBufferMapping();

    void uploadVertices(const std::vector<Vertex> &vertices);

    void uploadIndices(const std::vector<uint32_t> &indices);

    VkBuffer getCurrentUniformBuffer();

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffers();

    void createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer);

    void createTransferCommandPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer,
                      VkDeviceMemory *pBufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H
