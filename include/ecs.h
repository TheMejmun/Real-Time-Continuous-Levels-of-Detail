//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_ECS_H
#define REALTIME_CELL_COLLAPSE_ECS_H

#include <vector>
#include <optional>
#include <tuple>
#include "renderable.h"

struct Components {
    Renderable *renderable;
};

class ECS {
public:
    void create(uint32_t size);

    uint32_t insert(const Components &entityComponents);

    void remove(uint32_t index);

    std::tuple<std::vector<bool>, std::vector<Renderable *>> requestRenderables(uint16_t flags);

private:
    uint32_t max_entities;
    std::vector<bool> isOccupied{};
    std::vector<Renderable *> renderables{};
};

#endif //REALTIME_CELL_COLLAPSE_ECS_H