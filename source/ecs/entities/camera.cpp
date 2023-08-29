//
// Created by Saman on 31.07.23.
//

#include "ecs/entities/camera.h"
#include "util/importer.h"
#include "graphics/colors.h"

Camera::Camera() {
    this->components.transform = new Transformer4();
    this->components.transform->translate(glm::vec3(0, 0, -5));
    this->components.camera = new Projector();
}