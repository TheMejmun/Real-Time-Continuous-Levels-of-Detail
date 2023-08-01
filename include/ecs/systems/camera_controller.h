//
// Created by Saman on 31.07.23.
//

#ifndef REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H
#define REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H

#include "util/timer.h"
#include "ecs/ecs.h"
#include "io/input_manager.h"

class CameraController{
public:
    void create();

    void destroy();

    void update(const sec &delta, ECS &ecs, InputManager &inputManager);

private:
    static inline bool EvaluatorActiveCamera(const Components &components) {
        return components.camera != nullptr && components.transform != nullptr && components.isAlive() && components.isMainCamera;
    };
};

#endif //REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H
