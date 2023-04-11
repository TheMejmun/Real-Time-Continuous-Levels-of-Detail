//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_APPLICATION_H
#define REALTIME_CELL_COLLAPSE_APPLICATION_H

#include <memory>
#include <string>
#include "renderer.h"
#include "timer.h"
#include "window_manager.h"
#include "input_manager.h"
#include "ecs.h"
#include "printer.h"

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
    sec currentFrameTime;
    sec currentGPUWaitTime;
    uint32_t currentFPS = 0;
};

#endif //REALTIME_CELL_COLLAPSE_APPLICATION_H
