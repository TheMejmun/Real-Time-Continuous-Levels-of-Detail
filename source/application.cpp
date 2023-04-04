//
// Created by Sam on 2023-03-28.
//

#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include "application.h"


void Application::run() {
    this->initWindow();
    this->initVulkan();
    this->mainLoop();
    this->cleanup();
}

void Application::run(const int32_t &w, const int32_t &h) {
    this->width = w;
    this->height = h;
    run();
}

void Application::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
}

void Application::createInstance() {
    // App Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

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
    Application::printAvailableVkExtensions();
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

void Application::printAvailableVkExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto &extension: extensions) {
        std::cout << '\t' << extension.extensionName << std::endl;
    }
}

bool Application::checkValidationLayerSupport() {
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

void Application::printAvailablePhysicalDevices() {
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

void Application::pickPhysicalDevice() {
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

bool Application::isDeviceSuitable(VkPhysicalDevice device, bool strictMode) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool suitable = true;
    if (strictMode) {
        suitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

//    suitable = suitable && deviceFeatures. = VK_KHR_swapchain

    QueueFamilyIndices indices = Application::findQueueFamilies(device);
    suitable = suitable && indices.isComplete();

    return suitable;
}

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value();
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        ++i;
    }

    return indices;
}

void Application::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Define the features we will use as queried in isDeviceSuitable
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
#pragma clang diagnostic pop

    if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(this->device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

void Application::initVulkan() {
    createInstance();
    pickPhysicalDevice();
    createLogicalDevice();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (GLFW_PRESS == glfwGetKey(this->window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(this->window, GLFW_TRUE);
        }
    }
}

void Application::cleanup() {
    vkDestroyDevice(device, nullptr);

    vkDestroyInstance(this->instance, nullptr);

    glfwDestroyWindow(this->window);

    glfwTerminate();
}