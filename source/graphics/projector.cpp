//
// Created by Sam on 2023-04-15.
//
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <tic.h>
#include "graphics/projector.h"
#include "io/printer.h"

glm::mat4 Projector::getProjection(float aspectRatio) const {
    return glm::perspective(
            this->fovYRadians,
            aspectRatio,
            this->zNear,
            this->zFar);
}

// TODO static?
glm::mat4 Projector::getView(const Transformer4 &eye) const {
    auto position = eye.getPosition();

    auto centerVec4 = eye.forward * this->cameraFront;
    glm::vec3 center = {centerVec4.x, centerVec4.y, centerVec4.z};

    auto upVec4 = eye.forward * this->worldUp;
    glm::vec3 up = {upVec4.x, upVec4.y, upVec4.z};

    // DBG "eye:\tpos:\t" << position.x << ", " << position.y << ", " << position.z ENDL;
    // DBG "eye:\tcenter:\t" << center.x << ", " << center.y << ", " << center.z ENDL;
    // DBG "eye:\tup:\t" << up.x << ", " << up.y << ", " << up.z ENDL;

    return glm::lookAt(position, center, up);
}