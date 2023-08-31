//
// Created by Saman on 01.08.23.
//

#include "ecs/systems/sphere_controller.h"
#include "io/printer.h"


bool doSphereRotation = false;

void SphereController::update(const sec &delta, ECS &ecs) {
    auto &inputState = *ecs.requestEntities(InputManagerController::EvaluatorInputManagerEntity)[0]->inputState;

    if (inputState.toggleRotation == IM_DOWN_EVENT) {
        doSphereRotation = !doSphereRotation;
    }

    if (doSphereRotation) {
        auto spheres = ecs.requestEntities(SphereController::EvaluatorRotatingSphere);

        for (auto sphere: spheres) {
            sphere->transform->rotate(
                    glm::radians(15.0f * static_cast<float >(delta)),
                    glm::vec3(0, 1, 0));
        }
    }
}

void SphereController::destroy() {

}