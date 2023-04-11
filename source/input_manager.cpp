//
// Created by Sam on 2023-04-08.
//

#include <GLFW/glfw3.h>
#include <iostream>
#include "input_manager.h"
#include "printer.h"

void InputManager::create(GLFWwindow *w) {
    INF "Creating InputManager" ENDL;

    this->window = w;
}

void InputManager::poll() {
    glfwPollEvents();

    this->closeWindow = GLFW_PRESS == glfwGetKey(this->window, GLFW_KEY_ESCAPE);
}