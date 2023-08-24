//
// Created by Sam on 2023-04-12.
//

#include "graphics/vulkan/vulkan_buffers.h"
#include "io/printer.h"
#include "graphics/uniform_buffer_object.h"
#include "graphics/vulkan/vulkan_memory.h"
#include "graphics/vulkan/vulkan_devices.h"

void VBufferManager::create() {
    INF "Creating VBufferManager" ENDL;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(VulkanDevices::physical, &deviceProperties);

    this->maxAllocations = deviceProperties.limits.maxMemoryAllocationCount;
    VRB "Maximum memory allocation count: " << this->maxAllocations ENDL;

    vkGetPhysicalDeviceMemoryProperties(VulkanDevices::physical, &this->memProperties);

    vkGetDeviceQueue(VulkanDevices::logical, VulkanDevices::queueFamilyIndices.transferFamily.value(), 0, &this->transferQueue);

    createTransferCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
}

void VBufferManager::destroy() {
    INF "Destroying VBufferManager" ENDL;

    for (size_t i = 0; i < UBO_BUFFER_COUNT; i++) {
        vkDestroyBuffer(VulkanDevices::logical, this->uniformBuffers[i], nullptr);
        vkFreeMemory(VulkanDevices::logical, this->uniformBuffersMemory[i], nullptr);
    }

    vkDestroyBuffer(VulkanDevices::logical, this->vertexBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, this->vertexBufferMemory, nullptr);

    vkDestroyBuffer(VulkanDevices::logical, this->indexBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, this->indexBufferMemory, nullptr);

    vkDestroyCommandPool(VulkanDevices::logical, this->transferCommandPool, nullptr);
//    vkFreeCommandBuffers(VulkanDevices::logical, this->transferCommandPool, 1, &this->transferCommandBuffer);
}

void VBufferManager::destroyCommandBuffer(VkCommandPool commandPool) {
    vkFreeCommandBuffers(VulkanDevices::logical, commandPool, 1, &this->commandBuffer);
}

void VBufferManager::nextUniformBuffer() {
    this->uniformBufferIndex = (this->uniformBufferIndex + 1) % UBO_BUFFER_COUNT;
}

void *VBufferManager::getCurrentUniformBufferMapping() {
    return this->uniformBuffersMapped[this->uniformBufferIndex];
}

VkBuffer VBufferManager::getCurrentUniformBuffer() {
    return this->uniformBuffers[this->uniformBufferIndex];
}

VkBuffer VBufferManager::getUniformBuffer(uint32_t i) {
    return this->uniformBuffers[i];
}

void VBufferManager::uploadVertices(const std::vector<Vertex> &vertices) {
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                 &stagingBufferMemory);

    void *data;
    vkMapMemory(VulkanDevices::logical, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(VulkanDevices::logical, stagingBufferMemory);

    copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

    // Cleanup
    vkDestroyBuffer(VulkanDevices::logical, stagingBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, stagingBufferMemory, nullptr);

    this->vertexCount += vertices.size();
}

void VBufferManager::createVertexBuffer() {
//    VkDeviceSize bufferSize = sizeof(Vertex) *  this->world.entities.renderable->vertices.size();
    VkDeviceSize bufferSize = DEFAULT_ALLOCATION_SIZE;

//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
//                 &stagingBufferMemory);
//
//    void *data;
//    vkMapMemory(logical, stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, this->world.entities.renderable->vertices.data(), (size_t) bufferSize);
//    vkUnmapMemory(logical, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->vertexBuffer, &this->vertexBufferMemory);

//    copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);
//
//    // Cleanup
//    vkDestroyBuffer(logical, stagingBuffer, nullptr);
//    vkFreeMemory(logical, stagingBufferMemory, nullptr);
}

void VBufferManager::uploadIndices(const std::vector<uint32_t> &indices) {
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                 &stagingBufferMemory);

    void *data;
    vkMapMemory(VulkanDevices::logical, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(VulkanDevices::logical, stagingBufferMemory);

    copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

    vkDestroyBuffer(VulkanDevices::logical, stagingBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, stagingBufferMemory, nullptr);

    this->indexCount += indices.size();
}

void VBufferManager::createIndexBuffer() {
//    VkDeviceSize bufferSize = sizeof(uint32_t) *  this->world.entities.renderable->indices.size();
    VkDeviceSize bufferSize = DEFAULT_ALLOCATION_SIZE;

//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
//                 &stagingBufferMemory);
//
//    void *data;
//    vkMapMemory(logical, stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, this->world.entities.renderable->indices.data(), (size_t) bufferSize);
//    vkUnmapMemory(logical, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->indexBuffer, &this->indexBufferMemory);

//    copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);
//
//    vkDestroyBuffer(logical, stagingBuffer, nullptr);
//    vkFreeMemory(logical, stagingBufferMemory, nullptr);
}

void VBufferManager::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // One to be written, one to be read!
    this->uniformBuffers.resize(UBO_BUFFER_COUNT);
    this->uniformBuffersMemory.resize(UBO_BUFFER_COUNT);
    this->uniformBuffersMapped.resize(UBO_BUFFER_COUNT);

    for (size_t i = 0; i < UBO_BUFFER_COUNT; i++) {
        // Instead of memcopy, because it is written to every frame!
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &this->uniformBuffers[i], &this->uniformBuffersMemory[i]);

        // Persistent mapping:
        vkMapMemory(VulkanDevices::logical, this->uniformBuffersMemory[i], 0, bufferSize, 0,
                    &this->uniformBuffersMapped[i]);
    }
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

    if (vkAllocateCommandBuffers(VulkanDevices::logical, &allocInfo, pBuffer) != VK_SUCCESS) {
        THROW("Failed to allocate command buffers!");
    }
}

void VBufferManager::createTransferCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = VulkanDevices::queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(VulkanDevices::logical, &poolInfo, nullptr, &this->transferCommandPool) !=
        VK_SUCCESS) {
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
    if (VulkanDevices::queueFamilyIndices.hasUniqueTransferQueue()) {
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; // TODO Switch to memory barriers
        bufferInfo.queueFamilyIndexCount = 2;
        uint32_t queueIndices[] = {VulkanDevices::queueFamilyIndices.graphicsFamily.value(),
                                   VulkanDevices::queueFamilyIndices.transferFamily.value()};
        bufferInfo.pQueueFamilyIndices = queueIndices;
    } else {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Like swap chain images
    }

    if (vkCreateBuffer(VulkanDevices::logical, &bufferInfo, nullptr, pBuffer) != VK_SUCCESS) {
        THROW("Failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VulkanDevices::logical, *pBuffer, &memRequirements);

    // Malloc
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Is visible and coherent when viewing from host
    allocInfo.memoryTypeIndex = VulkanMemory::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(VulkanDevices::logical, &allocInfo, nullptr, pBufferMemory) != VK_SUCCESS) {
        THROW("Failed to allocate vertex buffer memory!");
    }

    // offset % memRequirements.alignment == 0
    vkBindBufferMemory(VulkanDevices::logical, *pBuffer, *pBufferMemory, 0);
}

void VBufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->transferCommandPool;
    allocInfo.commandBufferCount = 1;

//    VkCommandBuffer transferBuffer;
//    vkAllocateCommandBuffers(logical, &allocInfo, &transferBuffer);

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

//    vkFreeCommandBuffers(logical, this->transferCommandPool, 1, &this->transferCommandBuffer);
}