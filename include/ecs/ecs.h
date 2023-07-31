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
#include "graphics/renderable.h"

struct Components {
    bool is_destroyed = false;
    bool will_destroy = false;
    uint32_t index = 0;
    RenderMesh *render_mesh = nullptr;
    Transformer4 *transform = nullptr;

    /**
     * Destroys all contained components.
     *
     * Warning: Do not call this manually. ECS calls this automatically when required.
     */
    void destroy() {
        delete render_mesh;
        render_mesh = nullptr;
        delete transform;
        transform = nullptr;

        is_destroyed = true;
        will_destroy = false;
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
