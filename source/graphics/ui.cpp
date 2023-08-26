//
// Created by Saman on 26.08.23.
//

#include <imgui.h>
#include "graphics/ui.h"

void UI::update(RenderState &state) {
    ImGui::Begin("Performance");
    sec last_frametime = state.uiState.fps.frametimesLastSecond.back();
    ImGui::Text("Total frame time:\t%1.4f", last_frametime);
    ImGui::Text("Frames per second:\t%d", state.uiState.fps.currentFPS());
    ImGui::End();
}