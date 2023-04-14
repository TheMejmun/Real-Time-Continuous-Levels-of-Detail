//
// Created by Sam on 2023-04-08.
//

#include <iostream>
#include "window_manager.h"
#include "printer.h"

void WindowManager::create(const std::string &t) {
    INF "Creating WindowManager" ENDL;

    this->title = t;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
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
        DBG "Max" ENDL;
        glfwGetWindowPos(this->window, &this->posX, &this->posY);
        glfwSetWindowMonitor(this->window, glfwGetPrimaryMonitor(), 0, 0, this->width, this->height, GLFW_DONT_CARE);
    } else {
        DBG "Min" ENDL;
        glfwSetWindowMonitor(this->window, nullptr,
                             this->posX, this->posY,
                             this->width, this->height, GLFW_DONT_CARE);
    }
}

void WindowManager::destroy() const {
    INF "Destroying WindowManager" ENDL;

    glfwDestroyWindow(this->window);

    glfwTerminate();
}