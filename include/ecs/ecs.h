//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_ECS_H
#define REALTIME_CELL_COLLAPSE_ECS_H

#include <vector>
#include <optional>
#include <tuple>
#include <memory>
#include <functional>
#include "graphics/render_mesh.h"
#include "graphics/projector.h"

struct Components {
    bool is_destroyed = false;
    bool will_destroy = false;
    uint32_t index = 0;
    RenderMesh *render_mesh = nullptr;
    Transformer4 *transform = nullptr;
    bool is_main_camera = false;
    Projector *camera = nullptr;
    bool is_rotating_sphere = false;

    /**
     * Destroys all contained components.
     *
     * Warning: Do not call this manually. ECS calls this automatically when required.
     */
    void destroy() {
        delete this->render_mesh;
        this->render_mesh = nullptr;
        delete this->transform;
        this->transform = nullptr;
        delete this->camera;
        this->camera = nullptr;

        this->is_destroyed = true;
        this->will_destroy = false;
    }

    /**
     * Convenience function
     * @return True, if these components are not and will not be destroyed this frame.
     */
    [[nodiscard]] bool isAlive() const {
        return !is_destroyed && !will_destroy;
    }
};

class ECS {
public:
    void create();

    void destroy();

    uint32_t insert(Components &entityComponents);

    // In every frame, always do inserts first, and deletions after. So that the renderer has time to handle allocation
    void remove(const uint32_t &index);

    std::vector<Components *>
    requestComponents(const std::function<bool(const Components &)> &evaluator);

private:
    void destroyReferences(const uint32_t &index);

    std::vector<Components> components{};
};

#endif //REALTIME_CELL_COLLAPSE_ECS_H
