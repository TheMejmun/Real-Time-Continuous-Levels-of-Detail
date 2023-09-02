//
// Created by Saman on 30.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_COMPONENTS_H
#define REALTIME_CELL_COLLAPSE_COMPONENTS_H

#include "preprocessor.h"
#include "graphics/render_mesh.h"
#include "graphics/render_mesh_simplifiable.h"
#include "graphics/projector.h"
#include "io/input_state.h"

#include <mutex>
#include <memory>

struct Components {
    bool isDestroyed = false;
    bool willDestroy = false;
    uint32_t index = 0;

    std::unique_ptr<InputState> inputState{nullptr};

    std::unique_ptr<RenderMesh> renderMesh{nullptr};
    std::unique_ptr<RenderMeshSimplifiable> renderMeshSimplifiable{nullptr};

    std::unique_ptr<Transformer4> transform{nullptr};

    std::unique_ptr<Projector> camera{nullptr};
    bool isMainCamera = false;

    bool isRotatingSphere = false;

    /**
     * Destroys all contained entities.
     *
     * Warning: Do not call this manually. ECS calls this automatically when required.
     */
    void destroy() {
        this->inputState.reset(nullptr);
        this->renderMesh.reset(nullptr);
        this->renderMeshSimplifiable.reset(nullptr);
        this->transform.reset(nullptr);
        this->camera.reset(nullptr);

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
