//
// Created by Sam on 2023-04-15.
//

#ifndef REALTIME_CELL_COLLAPSE_TRANSFORMER_H
#define REALTIME_CELL_COLLAPSE_TRANSFORMER_H

#include <glm/glm.hpp>

class Transformer4{
public:
    void rotate(float radians, glm::vec3 axis);

    void translate(glm::vec3 translation);

    glm::mat4 forward{1.0f}, inverse{1.0f};
};

#endif //REALTIME_CELL_COLLAPSE_TRANSFORMER_H
