//
// Created by Sam on 2023-04-15.
//

#ifndef REALTIME_CELL_COLLAPSE_TRANSFORMER_H
#define REALTIME_CELL_COLLAPSE_TRANSFORMER_H

#include <glm/glm.hpp>

class Transformer4 {
public:
    void translate(glm::vec3 translation);

    void scale(float scale);

    void scale(glm::vec3 scale);

    void rotate(float radians, glm::vec3 axis);

    glm::mat4 forward{1.0f}, inverse{1.0f};

    glm::vec3 getPosition() const;
};

#endif //REALTIME_CELL_COLLAPSE_TRANSFORMER_H
