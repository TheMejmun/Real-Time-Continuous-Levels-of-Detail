//
// Created by Sam on 2023-04-11.
//

#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_swapchain.h"
#include "graphics/vulkan/vulkan_images.h"
#include "graphics/vulkan/vulkan_imgui.h"

void Renderer::create(const std::string &title, GLFWwindow *window) {
    INF "Creating Renderer" ENDL;

    this->state.title = title;
    this->state.window = window;

    this->state.uiState.title = title;
    this->state.uiState.window = window;

    this->initVulkan();
    VulkanImgui::create(this->state);
}

void Renderer::initVulkan() {
    VulkanInstance::create(this->state.title);
    VulkanSwapchain::createSurface(this->state.window);
    VulkanDevices::create();
    VulkanSwapchain::createSwapchain();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    VulkanBuffers::create();
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    VulkanSwapchain::createDepthResources();
    VulkanImages::createTextureImage();
    createSyncObjects();
}

void Renderer::destroyVulkan() {
    vkDestroySemaphore(VulkanDevices::logical, this->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(VulkanDevices::logical, this->renderFinishedSemaphore, nullptr);
    vkDestroyFence(VulkanDevices::logical, this->inFlightFence, nullptr);

    VulkanBuffers::destroy();

    // VulkanBuffers::destroyCommandBuffer(this->commandPool);
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

void Renderer::destroy() {
    INF "Destroying Renderer" ENDL;

    if (this->simplifiedMeshAllocationThreadRunning)
        this->simplifiedMeshAllocationThread.join();

    // Wait until resources are not actively being used anymore
    vkDeviceWaitIdle(VulkanDevices::logical);

    VulkanImgui::destroy();

    this->destroyVulkan();
}
