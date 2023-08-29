//
// Created by Sam on 2023-04-08.
//

#include "io/window_manager.h"
#include "io/printer.h"

#include <iostream>

void WindowManager::create(const std::string &t) {
    INF "Creating WindowManager" ENDL;

    this->title = t;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
    this->monitor = glfwGetPrimaryMonitor();
    pollMonitorResolution();
}

void WindowManager::updateTitle(const std::string &t) {
    this->title = t;
    if (this->window != nullptr) {
        glfwSetWindowTitle(this->window, this->title.c_str());
    }
}

bool WindowManager::shouldClose() const {
    return glfwWindowShouldClose(this->window);
}

void WindowManager::close() const {
    glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

void WindowManager::toggleFullscreen() {
    this->isMaximized = !this->isMaximized;
    if (this->isMaximized) {
        pollWindowPosition(); // For restoring later
        glfwSetWindowMonitor(this->window, glfwGetPrimaryMonitor(),
                             0, 0,
                             this->monitorParams->width, this->monitorParams->height,
                             this->monitorParams->refreshRate);
    } else {
        glfwSetWindowMonitor(this->window, nullptr,
                             this->windowPosX, this->windowPosY,
                             this->width, this->height, GLFW_DONT_CARE);
    }
}


void WindowManager::pollMonitorResolution() {
    // Supposed to be valid until monitor disconnected, but seems to change between fullscreen and windowed
    this->monitorParams = glfwGetVideoMode(this->monitor);
    int count;
    auto params = glfwGetVideoModes(this->monitor, &count);

    for (int i = 0; i < count; ++i) {
        DBG "Got monitor info:" <<
                                "\n\tWidth:\t" << params[i].width <<
                                "\n\tHeight:\t" << params[i].height <<
                                "\n\tRed Bits:\t" << params[i].redBits <<
                                "\n\tGreen Bits:\t" << params[i].greenBits <<
                                "\n\tBlue Bits:\t" << params[i].blueBits <<
                                "\n\tHz:\t" << params[i].refreshRate ENDL;
        this->monitorParams = &params[i];
    }
}

void WindowManager::pollWindowPosition() {
    glfwGetWindowPos(this->window, &this->windowPosX, &this->windowPosY);
}

void WindowManager::destroy() const {
    INF "Destroying WindowManager" ENDL;

    glfwDestroyWindow(this->window);

    glfwTerminate();
}