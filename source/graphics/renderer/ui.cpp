//
// Created by Saman on 26.08.23.
//


#include "graphics/renderer.h"

void Renderer::updateLastFrametime(sec delta) {
    this->state.uiState.last_frametime = delta;
}

void Renderer::drawUi() {

}