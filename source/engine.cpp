//
// Created by Sam on 2023-03-28.
//

#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <set>
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include "engine.h"

bool QueueFamilyIndices::isComplete() const {
    return this->graphicsFamily.has_value() &&
           this->presentFamily.has_value();
}

bool QueueFamilyIndices::isUnifiedGraphicsPresentQueue() const {
    if (!this->isComplete()) return false;
    return this->graphicsFamily.value() == this->presentFamily.value();
}

void Engine::run() {
    this->initWindow();
    this->initVulkan();
    this->mainLoop();
    this->cleanup();
}

void Engine::run(const int32_t &w, const int32_t &h) {
    this->width = w;
    this->height = h;
    run();
}

void Engine::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
}

void Engine::createInstance() {
    // App Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

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
    Engine::printAvailableInstanceExtensions();
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation layers
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
    if (ENABLE_VALIDATION_LAYERS) {
        if (!checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers not available!");
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
        throw std::runtime_error("Failed to create instance!");
    }
}

void Engine::printAvailableInstanceExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available instance extensions:" << std::endl;
    for (const auto &extension: extensions) {
        std::cout << '\t' << extension.extensionName << std::endl;
    }
}

bool Engine::checkValidationLayerSupport() {
    // get available layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: VALIDATION_LAYERS) {
        bool layerFound = false;

        for (const auto &layerProperties: availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Engine::printAvailablePhysicalDevices() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

    std::cout << "Available physical devices:" << std::endl;

    for (const auto &device: devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        std::cout << "\t" << deviceProperties.deviceName << std::endl;
    }
}

void Engine::pickPhysicalDevice() {
    printAvailablePhysicalDevices();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

    for (const auto &device: devices) {
        if (isDeviceSuitable(device, true)) {
            this->physicalDevice = device;
            break;
        } else if (isDeviceSuitable(device, false)) {
            this->physicalDevice = device;
            // Keep looking for a better one
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(this->physicalDevice, &deviceProperties);
    std::cout << "Picked physical device: " << deviceProperties.deviceName << std::endl;

    if (this->physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Engine::isDeviceSuitable(VkPhysicalDevice device, bool strictMode) {
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
    QueueFamilyIndices indices = Engine::findQueueFamilies(device);
    suitable = suitable && indices.isComplete();

    // Supports required extensions
    suitable = suitable && checkDeviceExtensionSupport(device);

    // Supports required swapchain features
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    bool swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    suitable = suitable && swapchainAdequate;

    return suitable;
}

bool Engine::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end());

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    std::cout << "Available device extensions for " << deviceProperties.deviceName << ":" << std::endl;

    for (const auto &extension: availableExtensions) {
        std::cout << '\t' << extension.extensionName << std::endl;
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Engine::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);

        // Better performance if a queue supports all features together -> break if found.
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (presentSupport)) {
            indices.graphicsFamily = i;
            indices.presentFamily = i;

            break;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        if (presentSupport) {
            indices.presentFamily = i;
        }

        ++i;
    }

    return indices;
}

SwapchainSupportDetails Engine::querySwapchainSupport(VkPhysicalDevice device) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    VkSurfaceFormatKHR out = availableFormats[0];

    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            out = availableFormat;
        }
    }

    std::cout << "Picked Swapchain Surface Format: " << std::endl;
    std::cout << "\tFormat: " << out.format << std::endl;
    std::cout << "\tColor Space: " << out.colorSpace << std::endl;

    return out;
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    std::cout << "Picked Swapchain Present Mode: ";
    for (const auto &availablePresentMode: availablePresentModes) {
        // Triple Buffering: Override last frame if a new image is rendered before that one has been shown
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "Triple-Buffering" << std::endl;
            return availablePresentMode;
        }
    }

    // VSYNC
    std::cout << "V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    VkExtent2D out;
    // If the current extents are set to the maximum values,
    // the window manager is trying to tell us to set it manually.
    // Otherwise, return the current value.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        out = capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        out = {
                static_cast<uint32_t>(w),
                static_cast<uint32_t>(h)
        };

        out.width = std::clamp(out.width,
                               capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width);
        out.height = std::clamp(out.height,
                                capabilities.minImageExtent.height,
                                capabilities.maxImageExtent.height);
    }

    std::cout << "Swapchain extents set to: " << out.width << " * " << out.height << std::endl;
    return out;
}

void Engine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);
    if (indices.isUnifiedGraphicsPresentQueue()) {
        std::cout << "Found a queue that supports both graphics and presentation!" << std::endl;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // If the indices are the same, the set will merge them -> Only one single queue creation.
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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

    VkDeviceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
#pragma clang diagnostic pop

    if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Get each queue
    vkGetDeviceQueue(this->logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(this->logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void Engine::createSwapchain() {
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    // One more image than the minimum to avoid stalling if the driver is still working on the image
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }
    std::cout << "Creating the Swapchain with at least " << imageCount << " images!" << std::endl;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // Can be 2 for 3D, etc.
    // TODO switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing, instead of directly rendering to the SC
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (!indices.isUnifiedGraphicsPresentQueue()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Image is shared between queues -> no transfers!
        createInfo.queueFamilyIndexCount = 2; // Concurrent mode requires at least two indices
        createInfo.pQueueFamilyIndices = queueFamilyIndices; // Share image between these queues
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Image is owned by one queue at a time -> Perf+
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // Do not add any swapchain transforms beyond the default
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;

    // Do not blend with other windows
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;

    // Clip pixels if obscured by other window -> Perf+
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr; // Put previous swapchain here if overridden, e.g. if window size changed

    if (vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // imageCount only specified a minimum!
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, nullptr);
    this->swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, this->swapchainImages.data());
    this->swapchainImageFormat = surfaceFormat.format;
    this->swapchainExtent = extent;
}

void Engine::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
}

void Engine::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (GLFW_PRESS == glfwGetKey(this->window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(this->window, GLFW_TRUE);
        }
    }
}

void Engine::cleanup() {
    vkDestroySwapchainKHR(this->logicalDevice, this->swapchain, nullptr);

    vkDestroyDevice(this->logicalDevice, nullptr);

    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);

    vkDestroyInstance(this->instance, nullptr);

    glfwDestroyWindow(this->window);

    glfwTerminate();
}