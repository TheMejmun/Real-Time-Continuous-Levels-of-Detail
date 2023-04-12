//
// Created by Sam on 2023-04-12.
//

#include "vbuffer_manager.h"
#include "printer.h"
#include "queue_family_indices.h"

void VBufferManager::create(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilyIndices indices) {
    INF "VBufferManager Renderer" ENDL;

    this->logicalDevice = device;
    this->queueFamilyIndices = indices;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    this->maxAllocations = deviceProperties.limits.maxMemoryAllocationCount;
    DBG "Maximum memory allocation count: " << this->maxAllocations ENDL;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &this->memProperties);

    createTransferCommandPool();
}

void VBufferManager::destroy() {
    INF "VBufferManager Renderer" ENDL;

    vkFreeMemory(this->logicalDevice, this->vertexBufferMemory, nullptr);
    vkDestroyBuffer(this->logicalDevice, this->vertexBuffer, nullptr);
    vkDestroyCommandPool(this->logicalDevice, this->transferCommandPool, nullptr);
}

void VBufferManager::createVertexBuffer(Triangle triangle) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * triangle.renderable.vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // Can be and-ed with other use cases
//    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Like swap chain images
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; // TODO Switch to memory barriers
    bufferInfo.queueFamilyIndexCount = 2;
    uint32_t queueIndices[] = {this->queueFamilyIndices.graphicsFamily.value(),
                               this->queueFamilyIndices.transferFamily.value()};
    bufferInfo.pQueueFamilyIndices = queueIndices;

    if (vkCreateBuffer(this->logicalDevice, &bufferInfo, nullptr, &this->vertexBuffer) != VK_SUCCESS) {
        THROW("Failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(this->logicalDevice, this->vertexBuffer, &memRequirements);

    // Malloc
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Is visible and coherent when viewing from host
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &this->vertexBufferMemory) != VK_SUCCESS) {
        THROW("Failed to allocate vertex buffer memory!");
    }

    // offset % memRequirements.alignment == 0
    vkBindBufferMemory(this->logicalDevice, this->vertexBuffer, this->vertexBufferMemory, 0);

    void *data;
    // Size can also be VK_WHOLE_SIZE -> Entire buffer past the offset
    vkMapMemory(this->logicalDevice, this->vertexBufferMemory, 0, bufferInfo.size, 0, &data);

    memcpy(data, triangle.renderable.vertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(this->logicalDevice, this->vertexBufferMemory);
}

uint32_t VBufferManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

    for (uint32_t i = 0; i < this->memProperties.memoryTypeCount; i++) {
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
        // Look for first memory of type with required properties (-> Fastest)
        const bool isRequiredMemoryType = (typeFilter & (1 << i));
        const bool hasRequiredProperties =
                (this->memProperties.memoryTypes[i].propertyFlags & properties) == properties;
        if (isRequiredMemoryType && hasRequiredProperties) {
            return i;
        }
    }

    THROW("Failed to find suitable memory type!");
}

void VBufferManager::createCommandBuffer(VkCommandPool commandPool) {
    createCommandBuffer(commandPool, &this->commandBuffer);
}

void VBufferManager::createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *buffer) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandPoolCreateFlagBits.html
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted for execution, but not called from other command buffers
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY can not be submitted, but called from other command buffers
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, buffer) != VK_SUCCESS) {
        THROW("Failed to allocate command buffers!");
    }
}

void VBufferManager::createTransferCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->transferCommandPool) != VK_SUCCESS) {
        THROW("Failed to create command pool!");
    }

    createCommandBuffer(this->transferCommandPool, &this->transferCommandBuffer);
}

void VBufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkBuffer &buffer, VkDeviceMemory &bufferMemory) {

}