//
// Created by Sam on 2023-04-15.
//

#ifndef REALTIME_CELL_COLLAPSE_CAMERA_H
#define REALTIME_CELL_COLLAPSE_CAMERA_H

#include <glm/glm.hpp>
#include "transformer.h"

struct Camera{
    Transformer4 view;
    float fovYRadians = glm::radians(45.0f);
    float zNear = 0.01f;
    float zFar = 100.0f;

    [[nodiscard]] glm::mat4 getProjection(float aspectRatio) const;
};

#endif //REALTIME_CELL_COLLAPSE_CAMERA_H
