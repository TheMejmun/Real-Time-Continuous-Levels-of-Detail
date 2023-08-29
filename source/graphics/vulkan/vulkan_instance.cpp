//
// Created by Saman on 24.08.23.
//

#include "graphics/vulkan/vulkan_instance.h"
#include "io/printer.h"
#include "graphics/vulkan/vulkan_validation.h"
#include "util/glfw_include.h"

#include <vulkan/vulkan.h>
#include <vector>

// Global
VkInstance VulkanInstance::instance = nullptr;

void VulkanInstance::create(const std::string &title) {
    INF "Creating VulkanInstance" ENDL;

    // App Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title.c_str();
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
    VulkanInstance::printAvailableExtensions();
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation layers
    if (VulkanValidation::ENABLE_VALIDATION_LAYERS) {
        if (!VulkanValidation::checkValidationLayerSupport()) {
            THROW("Validation layers not available!");
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanValidation::VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VulkanValidation::VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // Done
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        THROW("Failed to create instance!");
    }
}

void VulkanInstance::printAvailableExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    VRB "Available instance extensions:" ENDL;
    for (const auto &extension: extensions) {
        VRB '\t' << extension.extensionName ENDL;
    }
}

void VulkanInstance::destroy() {
    INF "Destroying VulkanInstance" ENDL;

    vkDestroyInstance(instance, nullptr);
}