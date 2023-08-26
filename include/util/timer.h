//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_TIMER_H
#define REALTIME_CELL_COLLAPSE_TIMER_H

#include <iostream>
#include <chrono>
#include <cmath>
#include <deque>

using chrono_clock = std::chrono::steady_clock;
using chrono_sec = std::chrono::duration<double>;
using chrono_sec_point = std::chrono::time_point<chrono_clock, chrono_sec>;
using sec = double;

namespace Timer {
    inline chrono_sec_point now() {
        return chrono_clock::now();
    }

    inline sec duration(const chrono_sec_point &time1, const chrono_sec_point &time2) {
        return std::abs((sec) (time1 - time2).count());
    }

    inline uint32_t fps(sec frameTimeSec) {
        return (int) (1.0 / frameTimeSec);
    }

    inline uint32_t fps(const chrono_sec_point &time1, const chrono_sec_point &time2) {
        return fps(duration(time1, time2));
    }
};

class FPSCounter {
public:
    void update(sec lastFrametime);

    inline uint32_t currentFPS() {
        return this->frametimesLastSecond.size();
    }

    std::deque<sec> frametimesLastSecond{};
private:
    sec totalTime();
};

#endif //REALTIME_CELL_COLLAPSE_TIMER_H
