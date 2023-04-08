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
    std::cout<<"Creating Application"<<std::endl;

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

        renderer->draw();

        if (this->inputManager->closeWindow) {
            this->windowManager->close();
        }

        // Bench
        chrono_sec_point time = Timer::now();
        this->currentFPS = Timer::FPS(this->lastTimestamp, time);
        this->lastTimestamp = time;
    }
}

void Application::destroy() {
    std::cout<<"Destroying Application"<<std::endl;

    this->renderer->destroy();
    this->windowManager->destroy();
}
