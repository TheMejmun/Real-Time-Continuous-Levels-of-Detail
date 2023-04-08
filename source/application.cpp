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

    this->windowManager = std::make_unique<WindowManager>();
    this->windowManager->create(this->title);

    this->inputManager = std::make_unique<InputManager>();
    this->inputManager->create(windowManager->window);

    this->renderer = std::make_unique<Renderer>();
    this->renderer->create(this->title, windowManager->window);
}

void Application::mainLoop() {
    while (!this->windowManager->shouldClose()) {
        this->inputManager->poll();

        this->currentGPUWaitTime = renderer->draw();

        if (this->inputManager->closeWindow) {
            this->windowManager->close();
        }

        // Bench
        chrono_sec_point time = Timer::now();
        this->currentFrameTime = Timer::duration(this->lastTimestamp, time);
        auto fps = (double) Timer::FPS( this->currentFrameTime);
        auto fps_old = (double) this->currentFPS;
        this->currentFPS = (uint32_t) ((fps_old * SMOOTH_FPS_DISPLAY_BIAS + fps) / (SMOOTH_FPS_DISPLAY_BIAS + 1.0));
        this->lastTimestamp = time;

        windowManager->updateTitle(std::string("FPS: ") +
                                   std::to_string(this->currentFPS) +
                                   std::string(" Frame time: ") +
                                   std::to_string(this->currentFrameTime) +
                                   std::string(" GPU wait time: ") +
                                   std::to_string(this->currentGPUWaitTime));
    }
}

void Application::destroy() {
    std::cout << "Destroying Application" << std::endl;

    this->renderer->destroy();
    this->windowManager->destroy();
}
