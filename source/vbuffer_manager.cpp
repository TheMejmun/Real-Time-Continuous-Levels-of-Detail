//
// Created by Sam on 2023-04-12.
//

#include "vbuffer_manager.h"
#include "printer.h"
#include "queue_family_indices.h"

void VBufferManager::create(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilyIndices indices) {
    INF "Creating VBufferManager" ENDL;

    this->logicalDevice = device;
    this->queueFamilyIndices = indices;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // TODO get max allocation size VkPhysicalDeviceMaintenance3Properties

    this->maxAllocations = deviceProperties.limits.maxMemoryAllocationCount;
    DBG "Maximum memory allocation count: " << this->maxAllocations ENDL;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &this->memProperties);

    vkGetDeviceQueue(this->logicalDevice, indices.transferFamily.value(), 0, &this->transferQueue);

    createTransferCommandPool();
}

void VBufferManager::destroy() {
    INF "Destroying VBufferManager" ENDL;

    vkFreeMemory(this->logicalDevice, this->vertexBufferMemory, nullptr);
    vkDestroyBuffer(this->logicalDevice, this->vertexBuffer, nullptr);
    vkDestroyCommandPool(this->logicalDevice, this->transferCommandPool, nullptr);
}

void VBufferManager::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * triangle.renderable.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                 &stagingBufferMemory);

    void *data;
    vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, triangle.renderable.vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->vertexBuffer, &this->vertexBufferMemory);

    copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

    // Cleanup
    vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);
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

void VBufferManager::createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandPoolCreateFlagBits.html
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted for execution, but not called from other command buffers
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY can not be submitted, but called from other command buffers
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, pBuffer) != VK_SUCCESS) {
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
                                  VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage; // Can be and-ed with other use cases
//    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Like swap chain images
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; // TODO Switch to memory barriers
    bufferInfo.queueFamilyIndexCount = 2;
    uint32_t queueIndices[] = {this->queueFamilyIndices.graphicsFamily.value(),
                               this->queueFamilyIndices.transferFamily.value()};
    bufferInfo.pQueueFamilyIndices = queueIndices;

    if (vkCreateBuffer(this->logicalDevice, &bufferInfo, nullptr, pBuffer) != VK_SUCCESS) {
        THROW("Failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(this->logicalDevice, *pBuffer, &memRequirements);

    // Malloc
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Is visible and coherent when viewing from host
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, pBufferMemory) != VK_SUCCESS) {
        THROW("Failed to allocate vertex buffer memory!");
    }

    // offset % memRequirements.alignment == 0
    vkBindBufferMemory(this->logicalDevice, *pBuffer, *pBufferMemory, 0);
}

void VBufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->transferCommandPool;
    allocInfo.commandBufferCount = 1;

//    VkCommandBuffer transferBuffer;
//    vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &transferBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(this->transferCommandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size; // VK_WHOLE_SIZE  not allowed here!
    vkCmdCopyBuffer(this->transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(this->transferCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->transferCommandBuffer;

    vkQueueSubmit(this->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->transferQueue); // TODO replace with fence

//    vkFreeCommandBuffers(this->logicalDevice, this->transferCommandPool, 1, &this->transferCommandBuffer);
}