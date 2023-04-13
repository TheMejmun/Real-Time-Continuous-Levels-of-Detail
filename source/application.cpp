//
// Created by Sam on 2023-04-08.
//

#include <iomanip>
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

        this->lastTimestamp = Timer::now();

        // Render
        auto gpuWaitTime = this->renderer.draw();

        // Benchmark
        auto time = Timer::now();
        this->deltaTime = Timer::duration(this->lastTimestamp, time);
        this->currentGPUWaitTime = gpuWaitTime;
        this->currentFPS = (uint32_t) Timer::fps(this->deltaTime);

        this->lastTimestamp = time;

        FPS std::fixed << std::setprecision(7) <<
                       "FPS: " <<
                       this->currentFPS <<
                       " Frame time: " <<
                       this->deltaTime <<
                       " GPU wait time: " <<
                       this->currentGPUWaitTime ENDL;
    }
}

void Application::destroy() {
    INF "Destroying Application" ENDL;

    this->renderer.destroy();
    this->windowManager.destroy();
}
