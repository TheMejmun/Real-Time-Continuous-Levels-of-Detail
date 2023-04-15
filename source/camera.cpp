//
// Created by Sam on 2023-04-15.
//
#include <glm/ext/matrix_clip_space.hpp>
#include "camera.h"

glm::mat4 Camera::getProjection(float aspectRatio) {
   return glm::perspective(
            this->fovYRadians,
            aspectRatio,
            this->zNear,
            this->zFar);
}