//
// Created by Saman on 26.08.23.
//


#include "graphics/renderer.h"

void Renderer::updateLastFrametime(sec delta) {
    this->state.uiState.fps.update(delta);
}

void Renderer::drawUi() {

}