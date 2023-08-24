//
// Created by Sam on 2023-04-11.
//

#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_instance.h"

void Renderer::create(const std::string &t, GLFWwindow *w) {
    INF "Creating Renderer" ENDL;

    this->window = w;
    this->title = t;
    this->initVulkan();
}

void Renderer::initVulkan() {
    VulkanInstance::createInstance(this->title);
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    this->bufferManager.create(VulkanDevices::physicalDevice, VulkanDevices::logicalDevice, this->queueFamilyIndices);
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    createDepthResources();
    // TODO createTextureImage();
    createSyncObjects();
}

void Renderer::destroy() {
    INF "Destroying Renderer" ENDL;

    // Wait until resources are not actively being used anymore
    vkDeviceWaitIdle(VulkanDevices::logicalDevice);

    vkDestroySemaphore(VulkanDevices::logicalDevice, this->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(VulkanDevices::logicalDevice, this->renderFinishedSemaphore, nullptr);
    vkDestroyFence(VulkanDevices::logicalDevice, this->inFlightFence, nullptr);

    this->bufferManager.destroy();

//    this->bufferManager.destroyCommandBuffer(this->commandPool);
    vkDestroyCommandPool(VulkanDevices::logicalDevice, this->commandPool, nullptr);
    vkDestroyDescriptorPool(VulkanDevices::logicalDevice, this->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(VulkanDevices::logicalDevice, this->descriptorSetLayout, nullptr);
    vkDestroyPipeline(VulkanDevices::logicalDevice, this->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(VulkanDevices::logicalDevice, this->pipelineLayout, nullptr);
    destroySwapchain();
    vkDestroyDevice(VulkanDevices::logicalDevice, nullptr);
    vkDestroySurfaceKHR(VulkanInstance::instance, this->surface, nullptr);

    VulkanInstance::destroyInstance();
}
