//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_TRIANGLE_H
#define REALTIME_CELL_COLLAPSE_TRIANGLE_H

#include "renderable.h"
#include "colors.h"

class Triangle {
public:
    Renderable renderable{
            .vertices{
                    {{0.0f,  0.0f,  0.0f},
                            Color::fromRGB({1.0f, 1.0f, 1.0f}).setLumaLab(0.9).getLAB()},
                    {{0.5f,  -0.5f, 0.0f},
                            Color::fromRGB({0.0f, 0.0f, 1.0f}).setLumaLab(0.4).getLAB()},
                    {{0.5f,  0.5f,  0.0f},
                            Color::fromRGB({1.0f, 0.0f, 0.0f}).setLumaLab(0.4).getLAB()},
                    {{-0.5f, -0.5f, 0.0f},
                            Color::fromRGB({1.0f, 0.0f, 0.0f}).setLumaLab(0.4).getLAB()},
                    {{-0.5f, 0.5f,  0.0f},
                            Color::fromRGB({0.0f, 0.0f, 1.0f}).setLumaLab(0.4).getLAB()}
            },
            .indices{
                    0, 3, 1,
                    0, 1, 2,
                    0, 2, 4,
                    0, 4, 3,

                    // Backwards facing:
                    0, 1, 3,
                    0, 2, 1,
                    0, 4, 2,
                    0, 3, 4,
            }};
};

#endif //REALTIME_CELL_COLLAPSE_TRIANGLE_H
