//
// Created by Saman on 26.08.23.
//

#include "util/timer.h"

void FPSCounter::update(sec lastFrametime) {
    this->frametimesLastSecond.push_back(lastFrametime);

    while (totalTime() > 1) {
        this->frametimesLastSecond.pop_front();
    }
}

sec FPSCounter::totalTime() {
    sec total = 0.0;
    for (const auto &frametime: this->frametimesLastSecond) {
        total += frametime;
    }

    return total;
}