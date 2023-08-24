//
// Created by Saman on 24.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_INSTANCE_H
#define REALTIME_CELL_COLLAPSE_VULKAN_INSTANCE_H

#include <string>
#include <vulkan/vulkan.h>

namespace VulkanInstance {
    extern VkInstance instance;

    void create(const std::string &title);

    void destroy();

    void printAvailableExtensions();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_INSTANCE_H
