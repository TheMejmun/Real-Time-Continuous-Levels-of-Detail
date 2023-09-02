//
// Created by Saman on 31.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDER_MESH_SIMPLIFIABLE_H
#define REALTIME_CELL_COLLAPSE_RENDER_MESH_SIMPLIFIABLE_H

#include "preprocessor.h"
#include "graphics/vertex.h"

#include <vector>
#include <mutex>

struct RenderMeshSimplifiable {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    bool isAllocated = false;
    uint32_t bufferIndex = 0;
    bool updateSimplifiedMesh = false;
    std::mutex simplifiedMeshMutex = std::mutex{};
};

#endif //REALTIME_CELL_COLLAPSE_RENDER_MESH_SIMPLIFIABLE_H
