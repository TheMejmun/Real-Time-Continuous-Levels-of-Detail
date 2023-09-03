//
// Created by Saman on 31.07.23.
//

#include "ecs/entities/camera.h"
#include "util/importer.h"
#include "graphics/colors.h"

#define DISTANCE_WITH_100_PERCENT_SCREEN_HEIGHT_SPHERE (-2.65)

Camera::Camera() {
    this->components.transform = std::make_unique<Transformer4>();
//    this->components.transform->translate(glm::vec3(0, 0, DISTANCE_WITH_100_PERCENT_SCREEN_HEIGHT_SPHERE / 2));
    this->components.transform->translate(glm::vec3(0, 0, DISTANCE_WITH_100_PERCENT_SCREEN_HEIGHT_SPHERE));
//    this->components.transform->translate(glm::vec3(0, 0, DISTANCE_WITH_100_PERCENT_SCREEN_HEIGHT_SPHERE * 10));
    this->components.camera = std::make_unique<Projector>();
}