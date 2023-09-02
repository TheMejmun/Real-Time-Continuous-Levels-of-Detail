//
// Created by Saman on 27.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H
#define REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H

#include "preprocessor.h"
#include "util/timer.h"
#include "ecs/ecs.h"

namespace MeshSimplifierController {
    void update(ECS &ecs, sec *timeTaken, uint32_t *framesTaken);

    void destroy();

    static inline bool EvaluatorToSimplify(const Components &components) {
        return components.renderMesh != nullptr && components.transform != nullptr && components.isAlive() &&
               components.renderMeshSimplifiable != nullptr && !components.renderMeshSimplifiable->updateSimplifiedMesh;
    };
};
#endif //REALTIME_CELL_COLLAPSE_MESH_SIMPLIFIER_CONTROLLER_H
