//
// Created by Sam on 2023-04-08.
//

#include <iostream>
#include "io/input_manager.h"
#include "io/printer.h"

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
    if (key == GLFW_KEY_W) InputManager::handleKey(&this->moveForward, action);
    if (key == GLFW_KEY_S) InputManager::handleKey(&this->moveBackward, action);
    if (key == GLFW_KEY_SPACE) InputManager::handleKey(&this->toggleRotation, action);
}

KeyState *InputManager::keySwitch(const KeyCode &key) {
    KeyState *out;
    switch (key) {
        case IM_CLOSE_WINDOW:
            out = &this->closeWindow;
            break;
        case IM_FULLSCREEN:
            out = &this->toggleFullscreen;
            break;
        case IM_MOVE_FORWARD:
            out = &this->moveForward;
            break;
        case IM_MOVE_BACKWARD:
            out = &this->moveBackward;
            break;
        case IM_TOGGLE_ROTATION:
            out = &this->toggleRotation;
            break;
        default:
            return nullptr;
    }
    return out;
}

void InputManager::handleKey(KeyState *key, const int &actionCode) {
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

void InputManager::poll() {
    glfwPollEvents();
}

#pragma clang diagnostic pop

KeyState InputManager::getKeyState(const KeyCode &key) {
    return *keySwitch(key);;
}

KeyState InputManager::consumeKeyState(const KeyCode &key) {
    auto keyRef = keySwitch(key);
    auto out = *keyRef;
    InputManager::handleKey(keyRef, -1);
    return out;
}