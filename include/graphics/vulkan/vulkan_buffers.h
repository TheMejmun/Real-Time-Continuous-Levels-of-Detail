//
// Created by Sam on 2023-04-12.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H
#define REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H

#include "preprocessor.h"
#include "graphics/triangle.h"
#include "util/byte_size.h"
#include "vulkan_devices.h"

#include <vulkan/vulkan.h>

namespace VulkanBuffers {
    extern const uint32_t UBO_BUFFER_COUNT;
    extern const uint32_t DEFAULT_ALLOCATION_SIZE;
    extern uint32_t maxAllocations, currentAllocations;

    extern VkCommandBuffer commandBuffer; // Cleaned automatically by command pool clean.
    extern VkBuffer vertexBuffer[];
    extern uint32_t vertexCount[];
    extern VkBuffer indexBuffer[];
    extern uint32_t indexCount[];
    extern uint32_t meshBufferToUse;
    extern uint32_t uniformBufferIndex;

    extern VkPhysicalDeviceMemoryProperties memProperties;

    extern VkDeviceMemory vertexBufferMemory[];
    extern VkDeviceMemory indexBufferMemory[];
    extern std::vector<VkBuffer> uniformBuffers;
    extern std::vector<VkDeviceMemory> uniformBuffersMemory;
    extern std::vector<void *> uniformBuffersMapped;

    extern VkQueue transferQueue;
    extern VkCommandPool transferCommandPool;
    extern VkCommandBuffer transferCommandBuffer; // Cleaned automatically by command pool clean.
    extern bool waitingForFence;
    extern VkFence uploadFence;

    void create();

    void destroy();

    void createCommandBuffer(VkCommandPool commandPool);

    void destroyCommandBuffer(VkCommandPool commandPool);

    void nextUniformBuffer();

    void *getCurrentUniformBufferMapping();

    void uploadVertices(const std::vector<Vertex> &vertices, uint32_t bufferIndex = 0);

    void uploadIndices(const std::vector<uint32_t> &indices, uint32_t bufferIndex = 0);

    void uploadMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                    bool parallel = false, uint32_t bufferIndex = 0);

    VkBuffer getCurrentUniformBuffer();

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffers();

    void createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer);

    void createTransferCommandPool();

    void createUploadFence();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer,
                      VkDeviceMemory *pBufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, bool parallel = false);

    bool isTransferQueueReady();

    void finishTransfer();

    void waitForTransfer();

    void resetMeshBufferToUse();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H
