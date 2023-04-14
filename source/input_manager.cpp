//
// Created by Sam on 2023-04-08.
//

#include <GLFW/glfw3.h>
#include <iostream>
#include "input_manager.h"
#include "printer.h"

static InputManager *instance;

void InputManager::create(GLFWwindow *w) {
    INF "Creating InputManager" ENDL;

    this->window = w;
    instance = this;

    glfwSetKeyCallback(window, _callback);
}

void InputManager::_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    instance->processInput(window, key, scancode, action, mods);
}

void InputManager::processInput(GLFWwindow *w, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) InputManager::handleKey(&this->closeWindow, action);
    if (key == GLFW_KEY_M) InputManager::handleKey(&this->toggleFullscreen, action);
}

void InputManager::handleKey(uint8_t *key, int actionCode) {
    switch (actionCode) {
        case GLFW_PRESS:
        case GLFW_REPEAT:
            switch (*key) {
                case IM_DOWN_EVENT: // Down -> Held
                    *key = IM_HELD;
                    break;
                case IM_HELD:
                    break;
                case IM_UP_EVENT: // Up -> Down
                case IM_RELEASED: // Released -> Down
                    *key = IM_DOWN_EVENT;
                    break;
            }
            break;
        case GLFW_RELEASE:
            switch (*key) {
                case IM_DOWN_EVENT: // Down -> Up
                case IM_HELD: // Held -> Up
                    *key = IM_UP_EVENT;
                    break;
                case IM_UP_EVENT: // Up -> Released
                    *key = IM_RELEASED;
                    break;
                case IM_RELEASED:
                    break;
            }
            break;
        default:
            // When consuming up and down events
            switch (*key) {
                case IM_DOWN_EVENT:
                case IM_HELD:
                    *key = IM_HELD;
                    break;
                case IM_UP_EVENT:
                case IM_RELEASED:
                    *key = IM_RELEASED;
                    break;
            }
            break;
    }
}

void InputManager::poll() {
    glfwPollEvents();
}

uint8_t InputManager::getKeyState(uint8_t key) {
    uint8_t out;
    switch (key) {
        case IM_CLOSE_WINDOW:
            out = this->closeWindow;
            break;
        case IM_FULLSCREEN:
            out = this->toggleFullscreen;
            break;
        default:
            out = IM_RELEASED;
            break;
    }
    return out;
}

uint8_t InputManager::consumeKeyState(uint8_t key) {
    uint8_t out;
    switch (key) {
        case IM_CLOSE_WINDOW:
            out = this->closeWindow;
            InputManager::handleKey(&this->closeWindow, -1);
            break;
        case IM_FULLSCREEN:
            out = this->toggleFullscreen;
            InputManager::handleKey(&this->toggleFullscreen, -1);
            break;
        default:
            out = IM_RELEASED;
            break;
    }
    return out;
}