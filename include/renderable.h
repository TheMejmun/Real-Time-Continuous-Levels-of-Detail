//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDERABLE_H
#define REALTIME_CELL_COLLAPSE_RENDERABLE_H

#include <vector>
#include "vertex.h"
#include <vulkan/vulkan.h>

const uint16_t FLAG_RENDERABLE_TO_ALLOCATE = 0b1;
const uint16_t FLAG_RENDERABLE_TO_DEALLOCATE = 0b10;
const uint16_t FLAG_RENDERABLE_TO_RENDER = 0b100;

struct Renderable {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

#endif //REALTIME_CELL_COLLAPSE_RENDERABLE_H
