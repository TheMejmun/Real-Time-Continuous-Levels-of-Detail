//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_TRIANGLE_H
#define REALTIME_CELL_COLLAPSE_TRIANGLE_H

#include "renderable.h"

class Triangle {
public:
    Renderable renderable{{
                                  Vertex{{0.0f, -0.5f, 0.0f},
                                         {1.0f, 0.0f,  0.0f}},
                                  Vertex{{0.5f, 0.5f, 0.0f},
                                         {0.0f, 1.0f, 0.0f}},
                                  Vertex{{-0.5f, 0.5f, 0.0f},
                                         {0.0f,  0.0f, 1.0f}}
                          }};
};

#endif //REALTIME_CELL_COLLAPSE_TRIANGLE_H
