//
// Created by Sam on 2023-04-12.
//

#ifndef REALTIME_CELL_COLLAPSE_QUEUE_FAMILY_INDICES_H
#define REALTIME_CELL_COLLAPSE_QUEUE_FAMILY_INDICES_H

#include <optional>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    [[nodiscard]] bool isComplete() const;

    [[nodiscard]] bool isUnifiedGraphicsPresentQueue() const;

    [[nodiscard]] bool hasUniqueTransferQueue() const;

    void print();
};

#endif //REALTIME_CELL_COLLAPSE_QUEUE_FAMILY_INDICES_H
