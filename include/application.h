//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_APPLICATION_H
#define REALTIME_CELL_COLLAPSE_APPLICATION_H

#include "preprocessor.h"
#include "graphics/renderer.h"
#include "util/timer.h"
#include "io/window_manager.h"
#include "io/input_manager.h"
#include "ecs/ecs.h"
#include "io/printer.h"

#include <memory>
#include <string>

class Application {
public:
    void run();

    std::string title;
private:
    void init();

    void mainLoop();

    void destroy();

    ECS ecs{};
    Renderer renderer{};
    WindowManager windowManager{};
    InputManager inputManager{};

    chrono_sec_point lastTimestamp = Timer::now();
    sec currentCpuWaitTime;
    uint32_t currentFPS = 0;
    sec deltaTime = 0;
};

#endif //REALTIME_CELL_COLLAPSE_APPLICATION_H
