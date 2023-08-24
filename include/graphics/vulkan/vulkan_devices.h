//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_DEVICES_H
#define REALTIME_CELL_COLLAPSE_VULKAN_DEVICES_H

#include <vulkan/vulkan.h>

namespace VulkanDevices {
    struct OptionalFeatures {
        bool supportsWireframeMode = false;
        bool physicalDeviceFeatures2 = false;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        [[nodiscard]] bool isComplete() const;

        [[nodiscard]] bool isUnifiedGraphicsPresentQueue() const;

        [[nodiscard]] bool hasUniqueTransferQueue() const;

        void print();
    };

    extern const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS;
    extern const std::string PORTABILITY_EXTENSION;
    extern VkPhysicalDevice physicalDevice;
    extern VkDevice logicalDevice;
    extern VkQueue graphicsQueue;
    extern VkQueue presentQueue;
    extern QueueFamilyIndices queueFamilyIndices;
    extern OptionalFeatures optionalFeatures;

    void pickPhysicalDevice();

    void printAvailablePhysicalDevices();

    bool isDeviceSuitable(VkPhysicalDevice device, bool strictMode);

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    static bool checkDevicePortabilityMode(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_DEVICES_H
