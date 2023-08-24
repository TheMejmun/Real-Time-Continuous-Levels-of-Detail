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
 // TODO
}

class VBufferManager {
public:
    void create(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VulkanDevices::QueueFamilyIndices indices);

    void destroy();

    void createCommandBuffer(VkCommandPool commandPool);

    void destroyCommandBuffer(VkCommandPool commandPool);

    void nextUniformBuffer();

    void *getCurrentUniformBufferMapping();

    void uploadVertices(const std::vector<Vertex> &vertices);

    void uploadIndices(const std::vector<uint32_t> &indices);

    VkBuffer getCurrentUniformBuffer();

    VkBuffer getUniformBuffer(uint32_t i);

    uint32_t maxAllocations = 0, currentAllocations = 0;

    VkCommandBuffer commandBuffer = nullptr; // Cleaned automatically by command pool clean.
    VkBuffer vertexBuffer = nullptr;
    uint32_t vertexCount = 0;
    VkBuffer indexBuffer = nullptr;
    uint32_t indexCount = 0;

    uint32_t uniformBufferIndex = UBO_BUFFER_COUNT;

    static constexpr uint32_t UBO_BUFFER_COUNT = 2;
    static constexpr uint32_t DEFAULT_ALLOCATION_SIZE = FROM_MB(256);
private:
    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffers();

    void createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer);

    void createTransferCommandPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer,
                      VkDeviceMemory *pBufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDevice logicalDevice = nullptr;
    VulkanDevices::QueueFamilyIndices queueFamilyIndices{};
    VkPhysicalDeviceMemoryProperties memProperties{};

    VkDeviceMemory vertexBufferMemory = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};
    std::vector<void *> uniformBuffersMapped{};

    VkQueue transferQueue = nullptr;
    VkCommandPool transferCommandPool = nullptr;
    VkCommandBuffer transferCommandBuffer = nullptr; // Cleaned automatically by command pool clean.
};

#endif //REALTIME_CELL_COLLAPSE_VULKAN_BUFFERS_H
