//
// Created by Sam on 2023-04-11.
//

#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_validation.h"

void Renderer::printAvailablePhysicalDevices() {
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

void Renderer::pickPhysicalDevice() {
    printAvailablePhysicalDevices();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        THROW("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(VulkanInstance::instance, &deviceCount, devices.data());

    for (const auto &device: devices) {
        if (isDeviceSuitable(device, true)) {
            VulkanDevices::physicalDevice = device;
            break;
        } else if (isDeviceSuitable(device, false)) {
            VulkanDevices::physicalDevice = device;
            // Keep looking for a better one
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(VulkanDevices::physicalDevice, &deviceProperties);
    INF "Picked physical device: " << deviceProperties.deviceName ENDL;

    if (VulkanDevices::physicalDevice == VK_NULL_HANDLE) {
        THROW("Failed to find a suitable GPU!");
    }

    this->queueFamilyIndices = Renderer::findQueueFamilies(VulkanDevices::physicalDevice);
    this->queueFamilyIndices.print();

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(VulkanDevices::physicalDevice, &deviceFeatures);
    this->optionalFeatures.supportsWireframeMode = deviceFeatures.fillModeNonSolid;
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device, bool strictMode) {
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
    QueueFamilyIndices indices = Renderer::findQueueFamilies(device);
    suitable = suitable && indices.isComplete();

    // Supports required extensions
    suitable = suitable && checkDeviceExtensionSupport(device);

    // Supports required swapchain features
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    bool swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    suitable = suitable && swapchainAdequate;

    return suitable;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end());

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VRB "Available device extensions for " << deviceProperties.deviceName << ":" ENDL;

    for (const auto &extension: availableExtensions) {
        VRB '\t' << extension.extensionName ENDL;
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Renderer::checkDevicePortabilityMode(VkPhysicalDevice device) {
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

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);

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

void Renderer::createLogicalDevice() {
    auto indices = this->queueFamilyIndices;

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

    // Define the features we will use as queried in isDeviceSuitable
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = this->optionalFeatures.supportsWireframeMode;

    VkDeviceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char *> requiredExtensions = REQUIRED_DEVICE_EXTENSIONS;
    if (checkDevicePortabilityMode(VulkanDevices::physicalDevice)) {
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

    if (vkCreateDevice(VulkanDevices::physicalDevice, &createInfo, nullptr, &VulkanDevices::logicalDevice) !=
        VK_SUCCESS) {
        THROW("Failed to create logical device!");
    }

    // Get each queue
    vkGetDeviceQueue(VulkanDevices::logicalDevice, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(VulkanDevices::logicalDevice, indices.presentFamily.value(), 0, &this->presentQueue);
}