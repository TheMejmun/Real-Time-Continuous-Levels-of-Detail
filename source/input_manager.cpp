//
// Created by Sam on 2023-04-08.
//

#include <GLFW/glfw3.h>
#include <iostream>
#include "input_manager.h"

void InputManager::create(GLFWwindow* window) {
    std::cout<<"Creating InputManager"<<std::endl;

    this->window = window;
}

void InputManager::poll() {
    glfwPollEvents();

    this->closeWindow = GLFW_PRESS == glfwGetKey(this->window, GLFW_KEY_ESCAPE);
}