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
#include "renderer.h"
#include "importer.h"

bool QueueFamilyIndices::isComplete() const {
    return this->graphicsFamily.has_value() &&
           this->presentFamily.has_value();
}

bool QueueFamilyIndices::isUnifiedGraphicsPresentQueue() const {
    if (!this->isComplete()) return false;
    return this->graphicsFamily.value() == this->presentFamily.value();
}

void Renderer::create(const std::string &t, GLFWwindow *w) {
    std::cout << "Creating Renderer" << std::endl;

    this->window = w;
    this->title = t;
    this->initVulkan();
}

void Renderer::createInstance() {
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
    Renderer::printAvailableInstanceExtensions();
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

void Renderer::printAvailableInstanceExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available instance extensions:" << std::endl;
    for (const auto &extension: extensions) {
        std::cout << '\t' << extension.extensionName << std::endl;
    }
}

bool Renderer::checkValidationLayerSupport() {
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

void Renderer::createSurface() {
    if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Renderer::printAvailablePhysicalDevices() {
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

void Renderer::pickPhysicalDevice() {
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

    std::cout << "Available device extensions for " << deviceProperties.deviceName << ":" << std::endl;

    for (const auto &extension: availableExtensions) {
        std::cout << '\t' << extension.extensionName << std::endl;
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
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

SwapchainSupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device) {
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

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
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

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
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

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    VkExtent2D out;
    // If the current extents are set to the maximum values,
    // the window manager is trying to tell us to set it manually.
    // Otherwise, return the current value.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        out = capabilities.currentExtent;
    } else {
        int w,h;
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

    this->framebufferWidth=out.width;
    this->framebufferHeight=out.height;

    std::cout << "Swapchain extents set to: " << out.width << " * " << out.height << std::endl;
    return out;
}

void Renderer::createLogicalDevice() {
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

bool Renderer::recreateSwapchain() {
    std::cout << "Recreate Swapchain" << std::endl;

    // May need to recreate render pass here if e.g. window moves to HDR monitor

    vkDeviceWaitIdle(this->logicalDevice);

    destroySwapchain();

    return createSwapchain();
}

bool Renderer::createSwapchain() {
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    if (extent.width < 1 || extent.height < 1) {
        std::cout << "Invalid swapchain extents. Retry later!" << std::endl;
        this->needsNewSwapchain = true;
        return false;
    }

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
        throw std::runtime_error("Failed to create swap chain!");
    }

    // imageCount only specified a minimum!
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, nullptr);
    this->swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, this->swapchainImages.data());
    this->swapchainImageFormat = surfaceFormat.format;
    this->swapchainExtent = extent;

    createImageViews();
    createRenderPass();
    createFramebuffers();

    return true;
}

void Renderer::destroySwapchain() {
    for (auto &swapchainFramebuffer: this->swapchainFramebuffers) {
        vkDestroyFramebuffer(this->logicalDevice, swapchainFramebuffer, nullptr);
    }

    vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);

    for (auto &swapchainImageView: this->swapchainImageViews) {
        vkDestroyImageView(this->logicalDevice, swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(this->logicalDevice, this->swapchain, nullptr);
}

void Renderer::createImageViews() {
    this->swapchainImageViews.resize(this->swapchainImages.size());

    for (size_t i = 0; i < this->swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 1D - 3D or Cube maps
        createInfo.format = this->swapchainImageFormat;

        // Can swizzle all components to be mapped to a single channel, or map to constants, etc.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Color, no mipmapping, single layer
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1; // No 3D

        if (vkCreateImageView(this->logicalDevice, &createInfo, nullptr, &this->swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void Renderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = this->swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Before rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // After rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Before rendering
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // After rendering

    // Only a single attachment for now
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // -> layout(location = 0) out vec4 outColor
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Color buffer

    // Single subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Is graphics subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Output attachment

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // To avoid layout transitions before the image has been acquired
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before/after the current render pass
    dependency.dstSubpass = 0; // dstSubpass > srcSubpass !!! (unless VK_SUBPASS_EXTERNAL)
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait for swapchain
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // The stage to wait in
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // The operation that should wait
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void Renderer::createGraphicsPipeline() {
    // TODO pull these out of here
    auto vertShaderCode = Importer::readFile("resources/shaders/triangle.vert.spv");
    std::cout << "Loaded vertex shader with byte size: " << vertShaderCode.size() << std::endl;
    auto fragShaderCode = Importer::readFile("resources/shaders/triangle.frag.spv");
    std::cout << "Loaded fragment shader with byte size: " << fragShaderCode.size() << std::endl;

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    // Use pSpecializationInfo to specify constants

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input format info
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional array of structs
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional array of structs

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP for vert reuse
    inputAssembly.primitiveRestartEnable = VK_FALSE; // Break up lines and triangle in "_STRIP" with index 0xFFFF(FFFF)

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    // Optional dynamic state
    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Requires a GPU feature. Clamps to near/far plane instead of discording
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Disables output to Framebuffer
#ifdef WIREFRAME_MODE
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // requires GPU feature
#else
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
#endif
    rasterizer.lineWidth = 1.0f; // > 1.0f requires wideLines GPU feature

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Requires a GPU feature
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Depth or Stencil testing: VkPipelineDepthStencilStateCreateInfo

    // Color blend info per attached Framebuffer
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // Global color blending
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // e.g. bitwise combination. Will disable blendEnable in attached FBs
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Define uniforms
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = this->pipelineLayout;
    pipelineInfo.renderPass = this->renderPass; // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
    pipelineInfo.subpass = 0; // Subpass index for this pipeline
    // Index or handle of parent pipeline. -> Perf+ if available
    // Needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag in this struct
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(this->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &this->graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    // Once the pipeline is created, we don't need this anymore
    vkDestroyShaderModule(this->logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(this->logicalDevice, vertShaderModule, nullptr);
}

void Renderer::createFramebuffers() {
    this->swapchainFramebuffers.resize(this->swapchainImageViews.size());

    for (size_t i = 0; i < this->swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
                this->swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = this->swapchainExtent.width;
        framebufferInfo.height = this->swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(this->logicalDevice, &framebufferInfo, nullptr, &this->swapchainFramebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void Renderer::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createGraphicsPipeline();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

void Renderer::destroy() {
    std::cout << "Destroying Renderer" << std::endl;

    // Wait until resources are not actively being used anymore
    vkDeviceWaitIdle(this->logicalDevice);

    vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphore, nullptr);
    vkDestroyFence(this->logicalDevice, this->inFlightFence, nullptr);

    vkDestroyCommandPool(this->logicalDevice, this->commandPool, nullptr);
    vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);
    destroySwapchain();
    vkDestroyDevice(this->logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
}

VkShaderModule Renderer::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // Cast the pointer. Vectors already handle proper memory alignment.
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void Renderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(this->physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void Renderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->commandPool;
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandPoolCreateFlagBits.html
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted for execution, but not called from other command buffers
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY can not be submitted, but called from other command buffers
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &this->commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start off as signaled

    if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphores and/or fences!");
    }
}

void Renderer::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferUsageFlagBits.html
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->renderPass;
    renderPassInfo.framebuffer = this->swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->swapchainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->swapchainExtent.width);
    viewport.height = static_cast<float>(this->swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->swapchainExtent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdDraw(buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

bool Renderer::shouldRecreateSwapchain() {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    bool framebufferChanged = w != this->framebufferWidth || h != this->framebufferHeight;

    return this->needsNewSwapchain || framebufferChanged;
}

sec Renderer::draw() {
    if (shouldRecreateSwapchain()) {
        bool success = recreateSwapchain();
        if (success) {
            std::cout << "Created new swapchain" << std::endl;
            this->needsNewSwapchain = false;
        } else {
            std::cout << "Failed to create new swapchain" << std::endl;
            return 0;
        }
    }

    auto beforeFence = Timer::now();
    vkWaitForFences(this->logicalDevice, 1, &this->inFlightFence, VK_TRUE, UINT64_MAX);
    auto afterFence = Timer::now();

    uint32_t imageIndex;
    auto acquireImageResult = vkAcquireNextImageKHR(this->logicalDevice, this->swapchain, UINT64_MAX,
                                                    this->imageAvailableSemaphore, nullptr, &imageIndex);

    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
        std::cout << "Swapchain is out of date" << std::endl;
        recreateSwapchain();
        return Timer::duration(beforeFence, afterFence); // Why not

    } else if (acquireImageResult == VK_SUBOPTIMAL_KHR) {
        std::cout << "Swapchain is suboptimal" << std::endl;
        this->needsNewSwapchain = true;

    } else if (acquireImageResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }


    // Avoid deadlock if recreating -> move to after success check
    vkResetFences(this->logicalDevice, 1, &this->inFlightFence);

    vkResetCommandBuffer(this->commandBuffer, 0); // I am not convinced this is necessary
    recordCommandBuffer(this->commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->imageAvailableSemaphore}; // index corresponding to wait stage
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // Wait in fragment stage
    // or VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->commandBuffer;

    VkSemaphore signalSemaphores[] = {this->renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {this->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Per swapchain acquireImageResult

    vkQueuePresentKHR(presentQueue, &presentInfo);

    return Timer::duration(beforeFence, afterFence);
}