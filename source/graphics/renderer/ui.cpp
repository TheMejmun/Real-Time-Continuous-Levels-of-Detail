//
// Created by Saman on 26.08.23.
//


#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_imgui.h"

UiState *Renderer::getUiState() {
    return &this->state.uiState;
}

void Renderer::resetMesh() {
    this->meshBufferToUse = 0;
}

void Renderer::drawUi(){
    this->state.uiState.currentMeshVertices = VulkanBuffers::vertexCount[this->meshBufferToUse];
    this->state.uiState.currentMeshTriangles = VulkanBuffers::indexCount[this->meshBufferToUse] / 3;
    VulkanImgui::draw(this->state);
}