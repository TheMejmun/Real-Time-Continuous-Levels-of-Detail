//
// Created by Saman on 24.08.23.
//

#include "graphics/vulkan/vulkan_validation.h"

#include <vulkan/vulkan.h>
#include <cstring>

// Constant
#ifdef NDEBUG
extern const bool VulkanValidation::ENABLE_VALIDATION_LAYERS = false;
#else
extern const bool VulkanValidation::ENABLE_VALIDATION_LAYERS = true;
#endif
extern const std::vector<const char *> VulkanValidation::VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

bool VulkanValidation::checkValidationLayerSupport() {
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