//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_RENDERPASSES_H
#define REALTIME_CELL_COLLAPSE_VULKAN_RENDERPASSES_H

#include "preprocessor.h"

#include <vulkan/vulkan.h>

namespace VulkanRenderPasses{
    extern VkRenderPass renderPass;

    void create();
    void destroy();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_RENDERPASSES_H
