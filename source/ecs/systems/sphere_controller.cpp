//
// Created by Saman on 01.08.23.
//

#include "ecs/systems/sphere_controller.h"
#include "io/printer.h"

void SphereController::create() {
    INF "Creating SphereController" ENDL;
}

void SphereController::update(const sec &delta, ECS &ecs, InputManager &inputManager) {
    if (inputManager.consumeKeyState(IM_TOGGLE_ROTATION) == IM_DOWN_EVENT) {
        this->rotate = !this->rotate;
    }

    if (this->rotate) {
        auto spheres = ecs.requestComponents(SphereController::EvaluatorRotatingSphere);

        for (auto sphere: spheres) {
            sphere->transform->rotate(
                    glm::radians(15.0f * static_cast<float >(delta)),
                    glm::vec3(0, 1, 0));
        }
    }
}

void SphereController::destroy() {
    INF "Destroying SphereController" ENDL;
}