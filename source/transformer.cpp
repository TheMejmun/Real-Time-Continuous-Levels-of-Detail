//
// Created by Sam on 2023-04-15.
//

#include <glm/ext/matrix_transform.hpp>
#include "transformer.h"

void Transformer4::rotate(float radians, glm::vec3 axis) {
    this->forward = glm::rotate(this->forward, radians, axis);
    this->inverse = glm::rotate(this->inverse, -radians, axis);
}

void Transformer4::translate(glm::vec3 translation) {
    this->forward = glm::translate(this->forward, translation);
    this->inverse = glm::translate(this->inverse, -translation);
}