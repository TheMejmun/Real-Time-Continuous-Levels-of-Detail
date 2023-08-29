//
// Created by Saman on 26.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDER_STATE_H
#define REALTIME_CELL_COLLAPSE_RENDER_STATE_H

#include "graphics/ui_state.h"
#include "graphics/vulkan/vulkan_state.h"
#include "util/timer.h"
#include "util/glfw_include.h"

#include <string>

struct RenderState {
    std::string title{};

    GLFWwindow* window = nullptr;

    UiState uiState{};

    VulkanState vulkanState{};
};

#endif //REALTIME_CELL_COLLAPSE_RENDER_STATE_H
