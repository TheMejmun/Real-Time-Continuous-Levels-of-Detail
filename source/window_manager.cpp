//
// Created by Sam on 2023-04-08.
//

#include <iostream>
#include "window_manager.h"

void WindowManager::create(const std::string &t) {
    std::cout << "Creating WindowManager" << std::endl;

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

void WindowManager::destroy() const {
    std::cout << "Destroying WindowManager" << std::endl;

    glfwDestroyWindow(this->window);

    glfwTerminate();
}