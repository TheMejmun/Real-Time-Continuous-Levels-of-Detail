//
// Created by Saman on 02.09.23.
//

#ifndef REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H
#define REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H

#include "util/timer.h"
#include "graphics/ui_state.h"

struct FrameTimes {
    sec cpuWaitTime;
    sec totalFrameTime;
};

struct MeshStatistics {
    size_t vertexCount;
    size_t triangleCount;
};

namespace PerformanceLogging {
    extern const sec LOG_DURATION;

    void newFrame(const FrameTimes &frameTimes);

    void meshCalculationStarted();

    void meshCalculationFinished();

    void meshUploadStarted();

    void meshUploadFinished(const MeshStatistics &meshStatistics);

    void update(UiState &uiState);
}

#endif //REALTIME_CELL_COLLAPSE_PERFORMANCE_LOGGING_H
