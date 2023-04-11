//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_TRIANGLE_H
#define REALTIME_CELL_COLLAPSE_TRIANGLE_H

#include "renderable.h"

class Triangle {
public:
    Renderable renderable{
            .vertices{
                    {{0.5f,  -0.5f, 0.0f},
                            {1.0f, 1.0f, 1.0f}},
                    {{0.5f,  0.5f,  0.0f},
                            {1.0f, 0.0f, 0.0f}},
                    {{-0.5f, -0.5f, 0.0f},
                            {0.0f, 0.0f, 1.0f}},
                    {{-0.5f, 0.5f,  0.0f},
                            {1.0f, 1.0f, 1.0f}}
            },
            .indices{
                    0, 1, 2, 2, 3, 0
            }};
};

#endif //REALTIME_CELL_COLLAPSE_TRIANGLE_H
