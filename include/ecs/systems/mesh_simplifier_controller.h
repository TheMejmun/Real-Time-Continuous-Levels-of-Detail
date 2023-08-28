//
// Created by Saman on 27.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H
#define REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H

#include "util/timer.h"
#include "ecs/ecs.h"

namespace MeshSimplifierController {
    void update(ECS &ecs);

    void destroy();

    static inline bool EvaluatorToSimplify(const Components &components) {
        return components.renderMesh != nullptr && components.transform != nullptr && components.isAlive() &&
               components.simplifyMesh && !components.updateSimplifiedMesh;
    };
};
#endif //REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H
