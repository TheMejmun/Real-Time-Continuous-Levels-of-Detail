//
// Created by Saman on 02.09.23.
//

#ifndef REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H
#define REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H

#include "util/timer.h"
#include "graphics/ui_state.h"

namespace PerformanceLogging {
    extern const sec LOG_DURATION;

    void newFrame();

    void meshCalculationStarted();

    void meshCalculatiodFinished();

    void meshUploadStarted();

    void meshUploadFinished();

    void update(UiState& uiState);
}

#endif //REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H
