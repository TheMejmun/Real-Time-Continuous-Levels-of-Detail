//
// Created by Sam on 2023-04-11.
//

#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_swapchain.h"

void Renderer::create(const std::string &t, GLFWwindow *w) {
    INF "Creating Renderer" ENDL;

    this->window = w;
    this->title = t;
    this->initVulkan();
}

void Renderer::initVulkan() {
    VulkanInstance::create(this->title);
    VulkanSwapchain::createSurface(this->window);
    VulkanDevices::create();
    VulkanSwapchain::createSwapchain();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    this->bufferManager.create();
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    VulkanSwapchain::createDepthResources();
    // TODO createTextureImage();
    createSyncObjects();
}

void Renderer::destroy() {
    INF "Destroying Renderer" ENDL;

    // Wait until resources are not actively being used anymore
    vkDeviceWaitIdle(VulkanDevices::logical);

    vkDestroySemaphore(VulkanDevices::logical, this->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(VulkanDevices::logical, this->renderFinishedSemaphore, nullptr);
    vkDestroyFence(VulkanDevices::logical, this->inFlightFence, nullptr);

    this->bufferManager.destroy();

//    this->bufferManager.destroyCommandBuffer(this->commandPool);
    vkDestroyCommandPool(VulkanDevices::logical, this->commandPool, nullptr);
    vkDestroyDescriptorPool(VulkanDevices::logical, this->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(VulkanDevices::logical, this->descriptorSetLayout, nullptr);
    vkDestroyPipeline(VulkanDevices::logical, this->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(VulkanDevices::logical, this->pipelineLayout, nullptr);
    VulkanSwapchain::destroySwapchain();
    VulkanDevices::destroy();
    vkDestroySurfaceKHR(VulkanInstance::instance, VulkanSwapchain::surface, nullptr);

    VulkanInstance::destroy();
}
