//
// Created by Sam on 2023-04-15.
//

#include "util/glm_include.h"
#include "physics/transformer.h"

#include <glm/ext/matrix_transform.hpp>

void Transformer4::translate(glm::vec3 translation) {
    this->forward = glm::translate(this->forward, translation);
    this->inverse = glm::translate(this->inverse, -translation);
}

void Transformer4::scale(float scale) {
    this->forward = glm::scale(this->forward, glm::vec3(scale));
    this->inverse = glm::scale(this->inverse, glm::vec3(1.0f / scale));
}

void Transformer4::scale(glm::vec3 scale) {
    this->forward = glm::scale(this->forward, scale);
    this->inverse = glm::scale(this->inverse, 1.0f / scale);
}

void Transformer4::rotate(float radians, glm::vec3 axis) {
    this->forward = glm::rotate(this->forward, radians, axis);
    this->inverse = glm::rotate(this->inverse, -radians, axis);
}

glm::vec3 Transformer4::getPosition() const {
    auto vec4 = this->forward * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return {vec4.x, vec4.y, vec4.z};
}