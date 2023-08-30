//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_MEMORY_H
#define REALTIME_CELL_COLLAPSE_VULKAN_MEMORY_H

#include "preprocessor.h"

#include <cstdint>
#include <vulkan/vulkan.h>

namespace VulkanMemory {
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
}
#endif //REALTIME_CELL_COLLAPSE_VULKAN_MEMORY_H
