//
// Created by Saman on 30.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_COMPONENTS_H
#define REALTIME_CELL_COLLAPSE_COMPONENTS_H

#include "preprocessor.h"
#include "graphics/render_mesh.h"
#include "graphics/projector.h"
#include "io/input_state.h"

#include <mutex>

struct Components {
    bool isDestroyed = false;
    bool willDestroy = false;
    uint32_t index = 0;

    InputState *inputState = nullptr;

    RenderMesh *renderMesh = nullptr;
    RenderMesh *renderMeshSimplified = nullptr;
    // TODO move these somewhere else
    bool updateSimplifiedMesh = false;
    std::mutex *simplifiedMeshMutex = new std::mutex{};
    bool simplifyMesh = false;

    Transformer4 *transform = nullptr;

    bool isMainCamera = false;
    Projector *camera = nullptr;

    bool isRotatingSphere = false;

    /**
     * Destroys all contained entities.
     *
     * Warning: Do not call this manually. ECS calls this automatically when required.
     */
    void destroy() {
        delete this->inputState;
        this->inputState = nullptr;
        delete this->renderMesh;
        this->renderMesh = nullptr;
        delete this->transform;
        this->transform = nullptr;
        delete this->camera;
        this->camera = nullptr;

        delete this->simplifiedMeshMutex;
        this->simplifiedMeshMutex = nullptr;

        this->isDestroyed = true;
        this->willDestroy = false;
    }

    /**
     * Convenience function
     * @return True, if these entities are not and will not be destroyed this frame.
     */
    [[nodiscard]] bool isAlive() const {
        return !isDestroyed && !willDestroy;
    }
};

#endif //REALTIME_CELL_COLLAPSE_COMPONENTS_H
