//
// Created by Saman on 31.07.23.
//

#ifndef REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H
#define REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H

#include "preprocessor.h"
#include "util/timer.h"
#include "ecs/ecs.h"
#include "io/input_manager.h"

namespace CameraController{
    void update(const sec &delta, ECS &ecs);

    void destroy();

    static inline bool EvaluatorActiveCamera(const Components &components) {
        return components.camera != nullptr && components.transform != nullptr && components.isAlive() && components.isMainCamera;
    };
};

#endif //REALTIME_CELL_COLLAPSE_CAMERA_CONTROLLER_H
