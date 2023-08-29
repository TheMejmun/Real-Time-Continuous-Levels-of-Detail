//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDER_MESH_H
#define REALTIME_CELL_COLLAPSE_RENDER_MESH_H

#include "graphics/vertex.h"
#include "physics/transformer.h"

#include <vector>
#include <vulkan/vulkan.h>

struct RenderMesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    bool isAllocated = false;
    int bufferIndex = 0;
};

#endif //REALTIME_CELL_COLLAPSE_RENDER_MESH_H
