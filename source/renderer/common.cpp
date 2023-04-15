//
// Created by Sam on 2023-04-11.
//

#include "renderer.h"

void Renderer::create(const std::string &t, GLFWwindow *w) {
    INF "Creating Renderer" ENDL;

    camera.view.translate(glm::vec3(0, 0, -5));

    this->window = w;
    this->title = t;
    this->initVulkan();
}

void Renderer::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    this->bufferManager.create(this->physicalDevice, this->logicalDevice, this->queueFamilyIndices);
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    createSyncObjects();
}

void Renderer::destroy() {
    INF "Destroying Renderer" ENDL;

    // Wait until resources are not actively being used anymore
    vkDeviceWaitIdle(this->logicalDevice);

    vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphore, nullptr);
    vkDestroyFence(this->logicalDevice, this->inFlightFence, nullptr);

    this->bufferManager.destroy();

//    this->bufferManager.destroyCommandBuffer(this->commandPool);
    vkDestroyCommandPool(this->logicalDevice, this->commandPool, nullptr);
    vkDestroyDescriptorPool(this->logicalDevice, this->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(this->logicalDevice, this->descriptorSetLayout, nullptr);
    vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);
    destroySwapchain();
    vkDestroyDevice(this->logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
}

void Renderer::createInstance() {
    // App Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Info on which extensions and features we need
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    std::vector<const char *> requiredExtensions;

    // GLFW extensions
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }

    // MacOS compatibility
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // Extensions final
    Renderer::printAvailableInstanceExtensions();
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation layers
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
    if (ENABLE_VALIDATION_LAYERS) {
        if (!checkValidationLayerSupport()) {
            THROW("Validation layers not available!");
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
#pragma clang diagnostic pop

    // Done
    VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
    if (result != VK_SUCCESS) {
        THROW("Failed to create instance!");
    }
}