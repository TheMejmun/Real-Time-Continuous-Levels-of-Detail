//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDERABLE_H
#define REALTIME_CELL_COLLAPSE_RENDERABLE_H

#include <vector>
#include "vertex.h"
#include "physics/transformer.h"
#include <vulkan/vulkan.h>

struct RenderMesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    bool is_allocated = false;
};

#endif //REALTIME_CELL_COLLAPSE_RENDERABLE_H
