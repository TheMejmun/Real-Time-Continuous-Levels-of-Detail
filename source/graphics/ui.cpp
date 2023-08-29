//
// Created by Saman on 26.08.23.
//

#include "graphics/ui.h"

#include <imgui.h>

void UI::update(RenderState &state) {
    if (state.uiState.returnToOriginalMeshBuffer) {
        state.uiState.returnToOriginalMeshBuffer = false;
    }

    ImGui::Begin("Realtime Cell Collapse");

    if(!state.uiState.fps.frametimesLastSecond.empty()) {
        sec lastFrametime = state.uiState.fps.frametimesLastSecond.back();
        ImGui::Text("Total frame time: %1.4f seconds", lastFrametime);
    }else{
        ImGui::Text("Total frame time: >1 second");
    }
    ImGui::Text("Frames per second: %d", state.uiState.fps.currentFPS());

    ImGui::SeparatorText("Mesh Info");

    ImGui::Text("Current vertex count: %d", state.uiState.currentMeshVertices);
    ImGui::Text("Current triangle count: %d", state.uiState.currentMeshTriangles);
    ImGui::Text("Current projected mesh size: %3.1f px^2", state.uiState.currentMeshSizeSq);
    if (ImGui::Button("Use original"))
        state.uiState.returnToOriginalMeshBuffer = true;

    ImGui::SeparatorText("Mesh Optimizer");

    ImGui::Text("Took: %3.2f seconds", state.uiState.meshSimplifierTimeTaken);
    ImGui::Text("Took: %d frames", state.uiState.meshSimplifierFramesTaken);
    if (state.uiState.runMeshSimplifier) {
        if (ImGui::Button("Stop"))
            state.uiState.runMeshSimplifier = false;
    } else {
        if (ImGui::Button("Start"))
            state.uiState.runMeshSimplifier = true;
    }

    ImGui::SeparatorText("Mesh Upload");

    ImGui::Text("Took: %3.2f seconds", state.uiState.meshUploadTimeTaken);
    ImGui::Text("Took: %d frames", state.uiState.meshUploadFramesTaken);


    ImGui::End();
}