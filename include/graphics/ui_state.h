//
// Created by Saman on 26.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_UI_STATE_H
#define REALTIME_CELL_COLLAPSE_UI_STATE_H

#include "util/timer.h"

struct UiState {
    FPSCounter fps = {};

    uint32_t currentMeshVertices = 0;
    uint32_t currentMeshTriangles = 0;
    float currentMeshSizeSq = 0.0f;

    sec meshSimplifierTimeTaken = 0.0f;
    uint32_t meshSimplifierFramesTaken = 0;
    bool runMeshSimplifier = false;
    bool returnToOriginalMeshBuffer = false;

    sec meshUploadTimeTaken = 0.0f;
    uint32_t meshUploadFramesTaken = 0;
};

#endif //REALTIME_CELL_COLLAPSE_UI_STATE_H
