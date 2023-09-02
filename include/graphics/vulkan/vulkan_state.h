//
// Created by Saman on 26.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_STATE_H
#define REALTIME_CELL_COLLAPSE_VULKAN_STATE_H

#include "preprocessor.h"

#include <vulkan/vulkan.h>

struct VulkanState {
    VkCommandPool commandPool = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_VULKAN_STATE_H
