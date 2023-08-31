//
// Created by Saman on 31.07.23.
//

#include "ecs/systems/camera_controller.h"
#include "io/printer.h"

void CameraController::update(const sec &delta, ECS &ecs) {
    auto &camera = *ecs.requestEntities(CameraController::EvaluatorActiveCamera)[0];
    auto &inputState = *ecs.requestEntities(InputController::EvaluatorInputManagerEntity)[0]->inputState;

    int move = 0;
    if (inputState.moveForward == IM_DOWN_EVENT ||
        inputState.moveForward == IM_HELD) {
        move += 1;
    }
    if (inputState.moveBackward == IM_DOWN_EVENT ||
        inputState.moveBackward == IM_HELD) {
        move -= 1;
    }
    camera.transform->translate(glm::vec3(0, 0, delta * move));
}

void CameraController::destroy() {

}