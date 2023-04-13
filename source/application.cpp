//
// Created by Sam on 2023-04-08.
//

#include "application.h"
#include "printer.h"

void Application::run() {
    init();
    mainLoop();
    destroy();
}

void Application::init() {
    INF "Creating Application" ENDL;

    this->ecs.create(256);

    this->windowManager.create(this->title);

    this->inputManager.create(this->windowManager.window);

    this->renderer.create(this->title, this->windowManager.window);
}

void Application::mainLoop() {
    while (!this->windowManager.shouldClose()) {
        // Input
        this->inputManager.poll();
        if (this->inputManager.closeWindow) {
            this->windowManager.close();
        }

        // Render
        auto gpuWaitTime = this->renderer.draw();

        // Benchmark
        auto time = Timer::now();
        auto frameTime = Timer::duration(this->lastTimestamp, time);
        this->currentFrameTime = frameTime;
        this->currentGPUWaitTime = gpuWaitTime;
        this->currentFPS = (uint32_t) Timer::fps(this->currentFrameTime);

        this->lastTimestamp = time;

        FPS std::string("FPS: ") +
            std::to_string(this->currentFPS) +
            std::string(" Frame time: ") +
            std::to_string(this->currentFrameTime) +
            std::string(" GPU wait time: ") +
            std::to_string(this->currentGPUWaitTime) ENDL;
    }
}

void Application::destroy() {
    INF "Destroying Application" ENDL;

    this->renderer.destroy();
    this->windowManager.destroy();
}
