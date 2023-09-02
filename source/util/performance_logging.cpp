//
// Created by Saman on 02.09.23.
//

#include "util/performance_logging.h"
#include "util/timer.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sstream>

extern const sec PerformanceLogging::LOG_DURATION = 10.0;

bool active = false;

uint32_t frameCount = 0;
std::optional<chrono_sec_point> lastCalculationStarted{};
std::optional<chrono_sec_point> lastUploadStarted{};
std::vector<sec> calculationDurations{};
std::vector<sec> uploadDurations{};

void PerformanceLogging::newFrame() {
    ++frameCount;
}

void PerformanceLogging::meshCalculationStarted() {
    lastCalculationStarted = Timer::now();
}

void PerformanceLogging::meshCalculatiodFinished() {
    if (lastCalculationStarted.has_value())
        calculationDurations.push_back(Timer::duration(lastCalculationStarted.value(), Timer::now()));
}

void PerformanceLogging::meshUploadStarted() {
    lastUploadStarted = Timer::now();
}

void PerformanceLogging::meshUploadFinished() {
    if (lastUploadStarted.has_value())
        uploadDurations.push_back(Timer::duration(lastUploadStarted.value(), Timer::now()));
}

void PerformanceLogging::update(UiState &uiState) {
    if (uiState.loggingStarted) {
        if (!active) {
            // Reset and start
            frameCount = 0;
            lastCalculationStarted.reset();
            lastUploadStarted.reset();
            calculationDurations.clear();
            uploadDurations.clear();
            active = true;
        }

        if (Timer::duration(uiState.loggingStartTime, Timer::now()) >= PerformanceLogging::LOG_DURATION) {
            // Done

            // mkdir varies between OSes
#ifdef _WIN32
            mkdir("output");
#else
            mkdir("output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

            std::stringstream nameBuilder{};
            nameBuilder << "output/performance_log";
            if (uiState.isMonkeyMesh)
                nameBuilder << "_monkey";
            else
                nameBuilder << "_sphere";
            if (uiState.runMeshSimplifier)
                nameBuilder << "_with_cell_collapse";
            nameBuilder << ".txt";

            std::ofstream file;
            file.open(nameBuilder.str());

            file << "Average FPS: "
                 << (frameCount / PerformanceLogging::LOG_DURATION)
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
            for (auto x: calculationDurations) totalUploadDuration += x;
            file << "Average mesh upload duration: "
                 << (totalUploadDuration / static_cast<double>(uploadDurations.size()))
                 << "\n";

            file.close();

            active = false;
            uiState.loggingStarted = false;
        }
    }
}