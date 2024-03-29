//
// Created by Sam on 2023-04-12.
//

#include "graphics/vulkan/vulkan_buffers.h"
#include "io/printer.h"
#include "graphics/uniform_buffer_object.h"
#include "graphics/vulkan/vulkan_memory.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "util/timer.h"

uint32_t VulkanBuffers::maxAllocations = 0, VulkanBuffers::currentAllocations = 0;

VkCommandBuffer VulkanBuffers::commandBuffer = nullptr; // Cleaned automatically by command pool clean.
VkBuffer VulkanBuffers::vertexBuffer[] = {nullptr, nullptr, nullptr};
uint32_t VulkanBuffers::vertexCount[] = {0, 0, 0};
VkBuffer VulkanBuffers::indexBuffer[] = {nullptr, nullptr, nullptr};
uint32_t VulkanBuffers::indexCount[] = {0, 0, 0};
uint32_t VulkanBuffers::meshBufferToUse = 0;

extern const uint32_t VulkanBuffers::UBO_BUFFER_COUNT = 2;
extern const uint32_t VulkanBuffers::DEFAULT_ALLOCATION_SIZE = FROM_MB(256); // 128MB is not enough

uint32_t VulkanBuffers::uniformBufferIndex = UBO_BUFFER_COUNT;

VkPhysicalDeviceMemoryProperties VulkanBuffers::memProperties{};

VkDeviceMemory VulkanBuffers::vertexBufferMemory[] = {nullptr, nullptr, nullptr};
VkDeviceMemory VulkanBuffers::indexBufferMemory[] = {nullptr, nullptr, nullptr};
std::vector<VkBuffer> VulkanBuffers::uniformBuffers{};
std::vector<VkDeviceMemory> VulkanBuffers::uniformBuffersMemory{};
std::vector<void *> VulkanBuffers::uniformBuffersMapped{};

VkQueue VulkanBuffers::transferQueue = nullptr;
VkCommandPool VulkanBuffers::transferCommandPool = nullptr;
VkCommandBuffer VulkanBuffers::transferCommandBuffer = nullptr; // Cleaned automatically by command pool clean.

VkFence VulkanBuffers::uploadFence = nullptr;
bool VulkanBuffers::waitingForFence = false;
std::vector<VkBuffer> stagingBuffersToDestroy{};
std::vector<VkDeviceMemory> stagingBufferMemoriesToDestroy{};
uint32_t indexCountToSet, vertexCountToSet;
uint32_t meshBufferIndexToSet;

void VulkanBuffers::create() {
    INF "Creating VulkanBuffers" ENDL;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(VulkanDevices::physical, &deviceProperties);

    VulkanBuffers::maxAllocations = deviceProperties.limits.maxMemoryAllocationCount;
    VRB "Maximum memory allocation count: " << VulkanBuffers::maxAllocations ENDL;

    vkGetPhysicalDeviceMemoryProperties(VulkanDevices::physical, &VulkanBuffers::memProperties);

    vkGetDeviceQueue(VulkanDevices::logical, VulkanDevices::queueFamilyIndices.transferFamily.value(), 0,
                     &VulkanBuffers::transferQueue);

    createTransferCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createUploadFence();
}

void destroyStagingBuffers() {
    for (auto stagingBuffer: stagingBuffersToDestroy) {
        vkDestroyBuffer(VulkanDevices::logical, stagingBuffer, nullptr);
    }
    stagingBuffersToDestroy.clear();

    for (auto stagingBufferMemory: stagingBufferMemoriesToDestroy) {
        vkFreeMemory(VulkanDevices::logical, stagingBufferMemory, nullptr);
    }
    stagingBufferMemoriesToDestroy.clear();
}

void VulkanBuffers::destroy() {
    INF "Destroying VulkanBuffers" ENDL;

    vkQueueWaitIdle(VulkanBuffers::transferQueue); // In case we are still uploading
    destroyStagingBuffers();

    vkDestroyFence(VulkanDevices::logical, VulkanBuffers::uploadFence, nullptr);

    for (size_t i = 0; i < UBO_BUFFER_COUNT; i++) {
        vkDestroyBuffer(VulkanDevices::logical, VulkanBuffers::uniformBuffers[i], nullptr);
        vkFreeMemory(VulkanDevices::logical, VulkanBuffers::uniformBuffersMemory[i], nullptr);
    }

    for (auto buffer: VulkanBuffers::vertexBuffer)
        vkDestroyBuffer(VulkanDevices::logical, buffer, nullptr);
    for (auto bufferMemory: VulkanBuffers::vertexBufferMemory)
        vkFreeMemory(VulkanDevices::logical, bufferMemory, nullptr);

    for (auto buffer: VulkanBuffers::indexBuffer)
        vkDestroyBuffer(VulkanDevices::logical, buffer, nullptr);
    for (auto bufferMemory: VulkanBuffers::indexBufferMemory)
        vkFreeMemory(VulkanDevices::logical, bufferMemory, nullptr);

    vkDestroyCommandPool(VulkanDevices::logical, VulkanBuffers::transferCommandPool, nullptr);
//    vkFreeCommandBuffers(VulkanDevices::logical, VulkanBuffers::transferCommandPool, 1, &VulkanBuffers::transferCommandBuffer);
}

void VulkanBuffers::uploadVertices(const std::vector<Vertex> &vertices, uint32_t bufferIndex) {
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

    // +1 because -1 means that we are accessing a non-simplified buffer
    auto dstBuffer = VulkanBuffers::vertexBuffer[bufferIndex];
    copyBuffer(stagingBuffer, dstBuffer, bufferSize);

    // Cleanup
    vkDestroyBuffer(VulkanDevices::logical, stagingBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, stagingBufferMemory, nullptr);

    // TODO += -> = revert
    VulkanBuffers::vertexCount[bufferIndex] = vertices.size();

    // TODO for now assume that initial buffer is only copied to once
//    VulkanBuffers::simplifiedMeshBuffersIndex = simplifiedIndex; TODO
}

void VulkanBuffers::uploadIndices(const std::vector<uint32_t> &indices, uint32_t bufferIndex) {
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

    // +1 because -1 means that we are accessing a non-simplified buffer
    copyBuffer(stagingBuffer, VulkanBuffers::indexBuffer[bufferIndex], bufferSize);

    vkDestroyBuffer(VulkanDevices::logical, stagingBuffer, nullptr);
    vkFreeMemory(VulkanDevices::logical, stagingBufferMemory, nullptr);

    // TODO += -> = revert
    VulkanBuffers::indexCount[bufferIndex] = indices.size();

    // TODO for now assume that initial buffer is only copied to once
//    VulkanBuffers::simplifiedMeshBuffersIndex = simplifiedIndex; TODO
}

void VulkanBuffers::uploadMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                               bool parallel, uint32_t bufferIndex) {

    size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
    size_t indexBufferSize = sizeof(uint32_t) * indices.size();
    VkDeviceSize bufferSize = vertexBufferSize + indexBufferSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                 &stagingBufferMemory);

    // To staging buffer

    void *data;
    // Upload vertices
    vkMapMemory(VulkanDevices::logical, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) vertexBufferSize);
    vkUnmapMemory(VulkanDevices::logical, stagingBufferMemory);
    // Upload indices
    vkMapMemory(VulkanDevices::logical, stagingBufferMemory, vertexBufferSize, indexBufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) indexBufferSize);
    vkUnmapMemory(VulkanDevices::logical, stagingBufferMemory);

    // To final buffer

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(VulkanBuffers::transferCommandBuffer, &beginInfo);

    // Upload vertices
    auto dstBufferVertices = VulkanBuffers::vertexBuffer[bufferIndex];
    VkBufferCopy copyRegionVertices{};
    copyRegionVertices.srcOffset = 0; // Optional
    copyRegionVertices.dstOffset = 0; // Optional
    copyRegionVertices.size = vertexBufferSize; // VK_WHOLE_SIZE  not allowed here!
    vkCmdCopyBuffer(VulkanBuffers::transferCommandBuffer, stagingBuffer, dstBufferVertices, 1, &copyRegionVertices);

    // Upload indices
    auto dstBufferIndices = VulkanBuffers::indexBuffer[bufferIndex];
    VkBufferCopy copyRegionIndices{};
    copyRegionIndices.srcOffset = vertexBufferSize; // Optional
    copyRegionIndices.dstOffset = 0; // Optional
    copyRegionIndices.size = indexBufferSize; // VK_WHOLE_SIZE  not allowed here!
    vkCmdCopyBuffer(VulkanBuffers::transferCommandBuffer, stagingBuffer, dstBufferIndices, 1, &copyRegionIndices);

    vkEndCommandBuffer(VulkanBuffers::transferCommandBuffer);

    // Submit

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VulkanBuffers::transferCommandBuffer;

    START_TRACE
    vkQueueSubmit(VulkanBuffers::transferQueue, 1, &submitInfo, uploadFence);
    END_TRACE("Queue submit")

    // End
    stagingBuffersToDestroy.push_back(stagingBuffer);
    stagingBufferMemoriesToDestroy.push_back(stagingBufferMemory);
    waitingForFence = true;

    meshBufferIndexToSet = bufferIndex;
    indexCountToSet = indices.size();
    vertexCountToSet = vertices.size();
}

void VulkanBuffers::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, bool parallel) {
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandPool = VulkanBuffers::transferCommandPool;
//    allocInfo.commandBufferCount = 1;
//    VkCommandBuffer transferBuffer;
//    vkAllocateCommandBuffers(logical, &allocInfo, &transferBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(VulkanBuffers::transferCommandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size; // VK_WHOLE_SIZE  not allowed here!
    vkCmdCopyBuffer(VulkanBuffers::transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(VulkanBuffers::transferCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VulkanBuffers::transferCommandBuffer;

    START_TRACE
    vkQueueSubmit(VulkanBuffers::transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VulkanBuffers::transferQueue);
    END_TRACE("Queue submit & wait idle")

//    vkFreeCommandBuffers(logical, VulkanBuffers::transferCommandPool, 1, &VulkanBuffers::transferCommandBuffer);
}

void VulkanBuffers::waitForTransfer() {
    VkResult result = vkWaitForFences(VulkanDevices::logical, 1, &VulkanBuffers::uploadFence, true,
                                      30'000'000'000); // Wait for 30s max
    if (result != VK_SUCCESS) {
        THROW("Waiting for the upload fence was unsuccessful");
    }
    finishTransfer();
}

void VulkanBuffers::finishTransfer() {
    vkResetFences(VulkanDevices::logical, 1, &VulkanBuffers::uploadFence);
    destroyStagingBuffers();
    waitingForFence = false;
    VulkanBuffers::meshBufferToUse = meshBufferIndexToSet;
    VulkanBuffers::indexCount[meshBufferIndexToSet] = indexCountToSet;
    VulkanBuffers::vertexCount[meshBufferIndexToSet] = vertexCountToSet;
}

bool VulkanBuffers::isTransferQueueReady() {
    if (!waitingForFence) return true;

    VkResult result = vkGetFenceStatus(VulkanDevices::logical, VulkanBuffers::uploadFence);
    if (result == VK_SUCCESS) {
        return true;
    } else if (result == VK_ERROR_DEVICE_LOST) {
        THROW("Device lost when checking fence");
    } else {
        return false;
    }
}

void VulkanBuffers::createUploadFence() {
    VkFenceCreateInfo createInfo{
            .sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags=0
    };
    if (vkCreateFence(VulkanDevices::logical, &createInfo, nullptr, &VulkanBuffers::uploadFence) != VK_SUCCESS) {
        THROW("Failed to create upload fence");
    }
}

void VulkanBuffers::destroyCommandBuffer(VkCommandPool commandPool) {
    vkFreeCommandBuffers(VulkanDevices::logical, commandPool, 1, &VulkanBuffers::commandBuffer);
}

void VulkanBuffers::nextUniformBuffer() {
    VulkanBuffers::uniformBufferIndex = (VulkanBuffers::uniformBufferIndex + 1) % UBO_BUFFER_COUNT;
}

void *VulkanBuffers::getCurrentUniformBufferMapping() {
    return VulkanBuffers::uniformBuffersMapped[VulkanBuffers::uniformBufferIndex];
}

VkBuffer VulkanBuffers::getCurrentUniformBuffer() {
    return VulkanBuffers::uniformBuffers[VulkanBuffers::uniformBufferIndex];
}

void VulkanBuffers::createVertexBuffer() {
    VkDeviceSize bufferSize = DEFAULT_ALLOCATION_SIZE;

    for (int i = 0; i < 3; ++i)
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VulkanBuffers::vertexBuffer + i,
                     VulkanBuffers::vertexBufferMemory + i);
}

void VulkanBuffers::createIndexBuffer() {
    VkDeviceSize bufferSize = DEFAULT_ALLOCATION_SIZE;

    for (int i = 0; i < 3; ++i)
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VulkanBuffers::indexBuffer + i,
                     VulkanBuffers::indexBufferMemory + i);
}

void VulkanBuffers::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // One to be written, one to be read!
    VulkanBuffers::uniformBuffers.resize(UBO_BUFFER_COUNT);
    VulkanBuffers::uniformBuffersMemory.resize(UBO_BUFFER_COUNT);
    VulkanBuffers::uniformBuffersMapped.resize(UBO_BUFFER_COUNT);

    for (size_t i = 0; i < UBO_BUFFER_COUNT; i++) {
        // Instead of memcopy, because it is written to every frame!
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &VulkanBuffers::uniformBuffers[i], &VulkanBuffers::uniformBuffersMemory[i]);

        // Persistent mapping:
        vkMapMemory(VulkanDevices::logical, VulkanBuffers::uniformBuffersMemory[i], 0, bufferSize, 0,
                    &VulkanBuffers::uniformBuffersMapped[i]);
    }
}

void VulkanBuffers::createCommandBuffer(VkCommandPool commandPool) {
    createCommandBuffer(commandPool, &VulkanBuffers::commandBuffer);
}

void VulkanBuffers::createCommandBuffer(VkCommandPool commandPool, VkCommandBuffer *pBuffer) {
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

void VulkanBuffers::createTransferCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = VulkanDevices::queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(VulkanDevices::logical, &poolInfo, nullptr, &VulkanBuffers::transferCommandPool) !=
        VK_SUCCESS) {
        THROW("Failed to create command pool!");
    }

    createCommandBuffer(VulkanBuffers::transferCommandPool, &VulkanBuffers::transferCommandBuffer);
}

void VulkanBuffers::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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

void VulkanBuffers::resetMeshBufferToUse() {
    VulkanBuffers::meshBufferToUse = 0;
}