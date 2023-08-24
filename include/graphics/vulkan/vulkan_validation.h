//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_VALIDATION_H
#define REALTIME_CELL_COLLAPSE_VULKAN_VALIDATION_H

#include <vector>

namespace VulkanValidation {
    extern const bool ENABLE_VALIDATION_LAYERS;
    extern const std::vector<const char *> VALIDATION_LAYERS;

    bool checkValidationLayerSupport();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_VALIDATION_H
