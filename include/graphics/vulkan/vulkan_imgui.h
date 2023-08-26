//
// Created by Saman on 26.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_VULKAN_IMGUI_H
#define REALTIME_CELL_COLLAPSE_VULKAN_IMGUI_H

#include "graphics/ui_state.h"
#include "graphics/render_state.h"

namespace VulkanImgui {
    void create(RenderState &state);

    void draw(RenderState &state);

    void recalculateScale(RenderState &state);

    void destroy();
}

#endif //REALTIME_CELL_COLLAPSE_VULKAN_IMGUI_H
