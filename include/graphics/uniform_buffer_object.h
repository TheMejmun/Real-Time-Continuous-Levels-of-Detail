//
// Created by Sam on 2023-04-13.
//

#ifndef REALTIME_CELL_COLLAPSE_UNIFORM_BUFFER_OBJECT_H
#define REALTIME_CELL_COLLAPSE_UNIFORM_BUFFER_OBJECT_H

#include "preprocessor.h"

#include <glm/glm.hpp>

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

#endif //REALTIME_CELL_COLLAPSE_UNIFORM_BUFFER_OBJECT_H
