//
// Created by Sam on 2023-04-08.
//

#include "application.h"

void Application::run() {
    init();
    mainLoop();
    destroy();
}

void Application::init() {
    std::cout << "Creating Application" << std::endl;

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
        this->currentFrameTime =
                ((this->currentFrameTime * SMOOTH_FPS_DISPLAY_BIAS + frameTime) / (SMOOTH_FPS_DISPLAY_BIAS + 1.0));
        this->currentGPUWaitTime =
                ((this->currentGPUWaitTime * SMOOTH_FPS_DISPLAY_BIAS + gpuWaitTime) / (SMOOTH_FPS_DISPLAY_BIAS + 1.0));
        this->currentFPS = (uint32_t) Timer::FPS(this->currentFrameTime);

        this->lastTimestamp = time;

        auto perfText = std::string("FPS: ") +
                        std::to_string(this->currentFPS) +
                        std::string(" Frame time: ") +
                        std::to_string(this->currentFrameTime) +
                        std::string(" GPU wait time: ") +
                        std::to_string(this->currentGPUWaitTime);
#ifdef PRINT_PERFORMANCE_METRICS
        std::cout << perfText << std::endl;
#endif
    }
}

void Application::destroy() {
    std::cout << "Destroying Application" << std::endl;

    this->renderer.destroy();
    this->windowManager.destroy();
}
