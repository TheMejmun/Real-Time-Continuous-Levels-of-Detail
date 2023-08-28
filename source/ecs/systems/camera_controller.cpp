//
// Created by Saman on 31.07.23.
//

#include "ecs/systems/camera_controller.h"
#include "io/printer.h"

void CameraController::update(const sec &delta, ECS &ecs, InputManager &inputManager) {
    auto &camera = *ecs.requestEntities(CameraController::EvaluatorActiveCamera)[0];

    int move = 0;
    if (inputManager.getKeyState(IM_MOVE_FORWARD) == IM_DOWN_EVENT ||
        inputManager.getKeyState(IM_MOVE_FORWARD) == IM_HELD) {
        move += 1;
    }
    if (inputManager.getKeyState(IM_MOVE_BACKWARD) == IM_DOWN_EVENT ||
        inputManager.getKeyState(IM_MOVE_BACKWARD) == IM_HELD) {
        move -= 1;
    }
    camera.transform->translate(glm::vec3(0, 0, delta * move));
}

void CameraController::destroy() {

}