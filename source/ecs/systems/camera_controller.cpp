//
// Created by Saman on 31.07.23.
//

#include "ecs/systems/camera_controller.h"
#include "io/printer.h"

void CameraController::create() {
    INF "Creating CameraController" ENDL;
}

void CameraController::update(const sec &delta, ECS &ecs, InputManager &input_manager) {
    auto &camera = *ecs.requestComponents(CameraController::EvaluatorActiveCamera)[0];

    int move = 0;
    if (input_manager.getKeyState(IM_MOVE_FORWARD) == IM_DOWN_EVENT ||
        input_manager.getKeyState(IM_MOVE_FORWARD) == IM_HELD) {
        move += 1;
    }
    if (input_manager.getKeyState(IM_MOVE_BACKWARD) == IM_DOWN_EVENT ||
        input_manager.getKeyState(IM_MOVE_BACKWARD) == IM_HELD) {
        move -= 1;
    }
    camera.transform->translate(glm::vec3(0, 0, delta * move));
}

void CameraController::destroy() {
    INF "Destroying CameraController" ENDL;
}