//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_ECS_H
#define REALTIME_CELL_COLLAPSE_ECS_H

#include "preprocessor.h"
#include "ecs/components.h"

#include <mutex>
#include <vector>
#include <optional>
#include <tuple>
#include <memory>
#include <functional>

class ECS {
public:
    void create();

    void destroy();

    uint32_t insert(Components &entityComponents);

    // In every frame, always do inserts first, and deletions after. So that the renderer has time to handle allocation
    void remove(const uint32_t &index);

    std::vector<Components *>
    requestEntities(const std::function<bool(const Components &)> &evaluator);

private:
    void destroyReferences(const uint32_t &index);

    std::vector<Components> entities{};
};

#endif //REALTIME_CELL_COLLAPSE_ECS_H
