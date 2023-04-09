//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_ECS_H
#define REALTIME_CELL_COLLAPSE_ECS_H

#include <vector>
#include "renderable.h"

class ECS{
public:
    void create(uint32_t size);
private:
    uint32_t max_entities;
    std::vector<Renderable> renderables;
};

#endif //REALTIME_CELL_COLLAPSE_ECS_H
