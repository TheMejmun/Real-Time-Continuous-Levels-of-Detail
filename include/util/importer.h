//
// Created by Sam on 2023-04-07.
//

#ifndef REALTIME_CELL_COLLAPSE_IMPORTER_H
#define REALTIME_CELL_COLLAPSE_IMPORTER_H

#include "graphics/vertex.h"

#include <vector>
#include <string>

namespace Importinator {
    struct Mesh{
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    std::vector<char> readFile(const std::string &filename);

    Mesh importMesh(const std::string &filename);
}

#endif //REALTIME_CELL_COLLAPSE_IMPORTER_H
