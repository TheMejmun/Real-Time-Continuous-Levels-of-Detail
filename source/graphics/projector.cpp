//
// Created by Sam on 2023-04-15.
//
#include <glm/ext/matrix_clip_space.hpp>
#include "graphics/projector.h"

glm::mat4 Projector::getProjection(float aspectRatio) const {
   return glm::perspective(
            this->fovYRadians,
            aspectRatio,
            this->zNear,
            this->zFar);
}