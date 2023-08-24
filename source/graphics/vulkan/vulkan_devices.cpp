//
// Created by Saman on 24.08.23.
//

#include "io/printer.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "graphics/vulkan/vulkan_validation.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_swapchain.h"
#include <set>
#include <cstdint>

// Constant
extern const std::vector<const char *> VulkanDevices::REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
extern const std::string VulkanDevices::PORTABILITY_EXTENSION = "VK_KHR_portability_subset";

// Global
VkPhysicalDevice VulkanDevices::physical = nullptr;
VkDevice VulkanDevices::logical = nullptr;
VkQueue VulkanDevices::graphicsQueue = nullptr;
VkQueue VulkanDevices::presentQueue = nullptr;
VulkanDevices::QueueFamilyIndices VulkanDevices::queueFamilyIndices{};
VulkanDevices::OptionalFeatures  VulkanDevices::optionalFeatures{};

void VulkanDevices::create() {
    INF "Creating VulkanDevices" ENDL;

    VulkanDevices::pickPhysical();
    VulkanDevices::createLogical();
}

void VulkanDevices::printAvailablePhysicalDevices() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, devices.data());

    VRB "Available physical devices:" ENDL;

    for (const auto &device: devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VRB "\t" << deviceProperties.deviceName ENDL;
    }
}

void VulkanDevices::pickPhysical() {
    printAvailablePhysicalDevices();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        THROW("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, devices.data());

    for (const auto &device: devices) {
        if (isPhysicalDeviceSuitable(device, true)) {
            VulkanDevices::physical = device;
            break;
        } else if (isPhysicalDeviceSuitable(device, false)) {
            VulkanDevices::physical = device;
            // Keep looking for a better one
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(VulkanDevices::physical, &deviceProperties);
    INF "Picked physical device: " << deviceProperties.deviceName ENDL;

    if (VulkanDevices::physical == VK_NULL_HANDLE) {
        THROW("Failed to find a suitable GPU!");
    }

    VulkanDevices::queueFamilyIndices = VulkanDevices::findQueueFamilies(VulkanDevices::physical);
    VulkanDevices::queueFamilyIndices.print();

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(VulkanDevices::physical, &deviceFeatures);
    VulkanDevices::optionalFeatures.supportsWireframeMode = deviceFeatures.fillModeNonSolid;
}

bool VulkanDevices::isPhysicalDeviceSuitable(VkPhysicalDevice device, bool strictMode) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Is discrete GPU
    bool suitable = true;
    if (strictMode) {
        suitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    // Supports required queues
    QueueFamilyIndices indices = VulkanDevices::findQueueFamilies(device);
    suitable = suitable && indices.isComplete();

    // Supports required extensions
    suitable = suitable && checkExtensionSupport(device);

    // Supports required swapchain features
    auto swapchainSupport = VulkanSwapchain::querySwapchainSupport(device);
    bool swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    suitable = suitable && swapchainAdequate;

    return suitable;
}

bool VulkanDevices::checkExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(VulkanDevices::REQUIRED_DEVICE_EXTENSIONS.begin(),
                                             VulkanDevices::REQUIRED_DEVICE_EXTENSIONS.end());

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VRB "Available device extensions for " << deviceProperties.deviceName << ":" ENDL;

    for (const auto &extension: availableExtensions) {
        VRB '\t' << extension.extensionName ENDL;
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanDevices::checkPortabilityMode(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    for (const auto &extension: availableExtensions) {
        if (PORTABILITY_EXTENSION == extension.extensionName) {
            return true;
        }
    }

    return false;
}

VulkanDevices::QueueFamilyIndices VulkanDevices::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VulkanSwapchain::surface, &presentSupport);

        // Look for transfer queue that is not a graphics queue
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
            (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) || !indices.transferFamily.has_value())) {
            indices.transferFamily = i;
        }

        // Better performance if a queue supports all features together
        // Do not execute if a unified family has already been found
        if (!indices.isUnifiedGraphicsPresentQueue()) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (presentSupport) {
                indices.presentFamily = i;
            }
        }

        ++i;
    }

    return indices;
}

void VulkanDevices::createLogical() {
    auto indices = VulkanDevices::queueFamilyIndices;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // If the indices are the same, the set will merge them -> Only one single queue creation.
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(),
                                              indices.transferFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Define the features we will use as queried in isPhysicalDeviceSuitable
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = VulkanDevices::optionalFeatures.supportsWireframeMode;

    VkDeviceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char *> requiredExtensions = REQUIRED_DEVICE_EXTENSIONS;
    if (checkPortabilityMode(VulkanDevices::physical)) {
        requiredExtensions.push_back(PORTABILITY_EXTENSION.c_str());
    }

    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (VulkanValidation::ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanValidation::VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VulkanValidation::VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(VulkanDevices::physical, &createInfo, nullptr, &VulkanDevices::logical) !=
        VK_SUCCESS) {
        THROW("Failed to create logical device!");
    }

    // Get each queue
    vkGetDeviceQueue(VulkanDevices::logical, indices.graphicsFamily.value(), 0, &VulkanDevices::graphicsQueue);
    vkGetDeviceQueue(VulkanDevices::logical, indices.presentFamily.value(), 0, &VulkanDevices::presentQueue);
}


bool VulkanDevices::QueueFamilyIndices::isComplete() const {
    return this->graphicsFamily.has_value() &&
           this->presentFamily.has_value() &&
           this->transferFamily.has_value();
}

bool VulkanDevices::QueueFamilyIndices::isUnifiedGraphicsPresentQueue() const {
    if (!this->graphicsFamily.has_value() ||
        !this->presentFamily.has_value())
        return false;
    return this->graphicsFamily.value() == this->presentFamily.value();
}

bool VulkanDevices::QueueFamilyIndices::hasUniqueTransferQueue() const {
    if (!this->graphicsFamily.has_value() ||
        !this->presentFamily.has_value())
        return false;
    return !(this->graphicsFamily.value() == this->transferFamily.value());
}

void VulkanDevices::QueueFamilyIndices::print() {
    VRB
        "QueueFamilyIndices:"
                << " Graphics: " << this->graphicsFamily.value()
                << " Present: " << this->presentFamily.value()
                << " Transfer: " << this->transferFamily.value()
                ENDL;
}

void VulkanDevices::destroy() {
    INF "Destroying VulkanDevices" ENDL;

    vkDestroyDevice(VulkanDevices::logical, nullptr);
}