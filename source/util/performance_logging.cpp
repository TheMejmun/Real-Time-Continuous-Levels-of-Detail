//
// Created by Saman on 02.09.23.
//

#include "util/performance_logging.h"
#include "util/timer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <direct.h>

extern const sec PerformanceLogging::LOG_DURATION = 60.0;
const uint32_t EXPECTED_MAX_FRAME_COUNT =
        static_cast<uint32_t>(PerformanceLogging::LOG_DURATION) * 2000; // seconds * frames

bool active = false;

std::vector<FrameTimes> frames{};

std::optional<chrono_sec_point> lastCalculationStarted{};
std::optional<chrono_sec_point> lastUploadStarted{};
std::vector<sec> calculationDurations{};
std::vector<sec> uploadDurations{};

std::vector<MeshStatistics> meshStatistics{};

void PerformanceLogging::newFrame(const FrameTimes &frameTimes) {
    if (active)
        frames.push_back(frameTimes);
}

void PerformanceLogging::meshCalculationStarted() {
    if (active)
        lastCalculationStarted = Timer::now();
}

void PerformanceLogging::meshCalculationFinished() {
    if (lastCalculationStarted.has_value())
        calculationDurations.push_back(Timer::duration(lastCalculationStarted.value(), Timer::now()));
}

void PerformanceLogging::meshUploadStarted() {
    if (active)
        lastUploadStarted = Timer::now();
}

void PerformanceLogging::meshUploadFinished(const MeshStatistics &stats) {
    if (lastUploadStarted.has_value()) {
        uploadDurations.push_back(Timer::duration(lastUploadStarted.value(), Timer::now()));
        meshStatistics.push_back(stats);
    }
}

void PerformanceLogging::update(UiState &uiState) {
    if (uiState.loggingStarted) {
        if (!active) {
            // Reset and start
            frames.clear();
            frames.reserve(EXPECTED_MAX_FRAME_COUNT);
            lastCalculationStarted.reset();
            lastUploadStarted.reset();
            calculationDurations.clear();
            uploadDurations.clear();
            meshStatistics.clear();
            active = true;
        }

        if (Timer::duration(uiState.loggingStartTime, Timer::now()) >= PerformanceLogging::LOG_DURATION) {
            // Done

            // mkdir varies between OSes
#ifdef _WIN32
            _mkdir("output");
#else
            mkdir("output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

            std::stringstream nameBuilder{};
            nameBuilder << std::setprecision(2) << std::fixed;
            nameBuilder << "output/performance_log";
            nameBuilder << "_z" << uiState.cameraZ;
            if (uiState.isMonkeyMesh)
                nameBuilder << "_monkey";
            else
                nameBuilder << "_sphere";
            if (uiState.runMeshSimplifier)
                nameBuilder << "_with_cell_collapse";
            nameBuilder << ".txt";

            std::ofstream file;
            file << std::setprecision(4) << std::fixed;
            file.open(nameBuilder.str());

            file << "Average FPS: "
                 << (static_cast<double>(frames.size()) / PerformanceLogging::LOG_DURATION)
                 << "\n";

            double totalCpuWaitTime = 0.0;
            for (auto x: frames) totalCpuWaitTime += x.cpuWaitTime;
            file << "Average cpu wait time: "
                 << (totalCpuWaitTime / static_cast<double>(frames.size()))
                 << "\n";

            double totalTotalFrameTime = 0.0;
            for (auto x: frames) totalTotalFrameTime += x.totalFrameTime;
            file << "Average total frame time: "
                 << (totalTotalFrameTime / static_cast<double>(frames.size()))
                 << "\n";

            file << "Average mesh calculation count per second: "
                 << (static_cast<double>(calculationDurations.size()) / PerformanceLogging::LOG_DURATION)
                 << "\n";

            double totalCalculationDuration = 0.0;
            for (auto x: calculationDurations) totalCalculationDuration += x;
            file << "Average mesh calculation duration: "
                 << (totalCalculationDuration / static_cast<double>(calculationDurations.size()))
                 << "\n";

            file << "Average mesh upload count per second: "
                 << (static_cast<double>(uploadDurations.size()) / PerformanceLogging::LOG_DURATION)
                 << "\n";

            double totalUploadDuration = 0.0;
            for (auto x: uploadDurations) totalUploadDuration += x;
            file << "Average mesh upload duration: "
                 << (totalUploadDuration / static_cast<double>(uploadDurations.size()))
                 << "\n";

            double averageMeshVertices = 0.0;
            for (auto x: meshStatistics)
                averageMeshVertices +=
                        static_cast<double>(x.vertexCount) / static_cast<double>(meshStatistics.size());
            file << "Average mesh vertex count: "
                 << averageMeshVertices
                 << "\n";

            double averageMeshTriangles = 0.0;
            for (auto x: meshStatistics)
                averageMeshTriangles +=
                        static_cast<double>(x.triangleCount) / static_cast<double>(meshStatistics.size());
            file << "Average mesh triangle count: "
                 << averageMeshTriangles
                 << "\n";

            file.close();

            active = false;
            uiState.loggingStarted = false;
        }
    }
}