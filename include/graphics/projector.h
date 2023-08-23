//
// Created by Sam on 2023-04-15.
//

#ifndef REALTIME_CELL_COLLAPSE_PROJECTOR_H
#define REALTIME_CELL_COLLAPSE_PROJECTOR_H

#include <glm/glm.hpp>
#include "physics/transformer.h"

struct Projector {
    float fovYRadians = glm::radians(45.0f);
    float zNear = 0.01f;
    float zFar = 100.0f;
    glm::vec4 worldUp = {0, -1, 0, 0};
    glm::vec4 cameraFront = {0, 0, 1, 1};

    [[nodiscard]] glm::mat4 getProjection(float aspectRatio) const;

    [[nodiscard]] glm::mat4 getView(const Transformer4 &eye) const;
};

#endif //REALTIME_CELL_COLLAPSE_PROJECTOR_H
