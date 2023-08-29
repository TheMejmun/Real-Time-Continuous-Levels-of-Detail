//
// Created by Saman on 26.08.23.
//

#include "graphics/ui.h"

#include <imgui.h>

void UI::update(UiState &state) {
    if (state.returnToOriginalMeshBuffer) {
        state.returnToOriginalMeshBuffer = false;
    }

    ImGui::Begin("Realtime Cell Collapse");

    ImGui::SeparatorText("Performance");

    if(!state.fps.frametimesLastSecond.empty()) {
        sec lastFrametime = state.fps.frametimesLastSecond.back();
        ImGui::Text("Total frame time: %1.4f seconds", lastFrametime);
    }else{
        ImGui::Text("Total frame time: >1 second");
    }
    ImGui::Text("Frames per second: %d", state.fps.currentFPS());

    ImGui::SeparatorText("Mesh Info");

    ImGui::Text("Current vertex count: %d", state.currentMeshVertices);
    ImGui::Text("Current triangle count: %d", state.currentMeshTriangles);
    // ImGui::Text("Current projected mesh size: %3.1f px^2", state.currentMeshSizeSq); TODO
    if (ImGui::Button("Use original"))
        state.returnToOriginalMeshBuffer = true;

    ImGui::SeparatorText("Mesh Optimizer");

    ImGui::Text("Took: %3.4f seconds", state.meshSimplifierTimeTaken);
    ImGui::Text("Took: %d frames", state.meshSimplifierFramesTaken);
    if (state.runMeshSimplifier) {
        if (ImGui::Button("Stop"))
            state.runMeshSimplifier = false;
    } else {
        if (ImGui::Button("Start"))
            state.runMeshSimplifier = true;
    }

    ImGui::SeparatorText("Mesh Upload");

    ImGui::Text("Took: %3.4f seconds", state.meshUploadTimeTaken);
    ImGui::Text("Took: %d frames", state.meshUploadFramesTaken);

    ImGui::SeparatorText("Controls");

    ImGui::Text("W: Move camera forwards");
    ImGui::Text("S: Move camera backwards");
    ImGui::Text("Space: Start/Stop rotation");
    ImGui::Text("M: Maximize/Minimize window");
    ImGui::Text("Esc: Exit");


    ImGui::End();
}