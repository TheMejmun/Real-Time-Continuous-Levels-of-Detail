//
// Created by Sam on 2023-04-08.
//

#include "io/input_manager.h"
#include "io/printer.h"

#include <iostream>

static InputManagerController *instance;

void InputManagerController::create(GLFWwindow *w) {
    INF "Creating InputManager" ENDL;

    this->window = w;
    instance = this;

    glfwSetKeyCallback(window, _callback);
}

void InputManagerController::update(sec delta, ECS &ecs) {
    InputManagerController::handleKey(keySwitch(IM_CLOSE_WINDOW), -1);
    InputManagerController::handleKey(keySwitch(IM_FULLSCREEN), -1);
    InputManagerController::handleKey(keySwitch(IM_MOVE_FORWARD), -1);
    InputManagerController::handleKey(keySwitch(IM_MOVE_BACKWARD), -1);
    InputManagerController::handleKey(keySwitch(IM_TOGGLE_ROTATION), -1);

    glfwPollEvents();

    auto entities = ecs.requestEntities(InputManagerController::EvaluatorInputManagerEntity);

    for (auto e: entities) {
        auto state = e->inputState;
        state->closeWindow = this->closeWindow;
        state->toggleFullscreen = this->toggleFullscreen;
        state->moveForward = this->moveForward;
        state->moveBackward = this->moveBackward;
        state->toggleRotation = this->toggleRotation;
    }
}

void InputManagerController::_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    instance->processInput(window, key, scancode, action, mods);
}

void InputManagerController::processInput(GLFWwindow *w, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) InputManagerController::handleKey(&this->closeWindow, action);
    if (key == GLFW_KEY_M) InputManagerController::handleKey(&this->toggleFullscreen, action);
    if (key == GLFW_KEY_W) InputManagerController::handleKey(&this->moveForward, action);
    if (key == GLFW_KEY_S) InputManagerController::handleKey(&this->moveBackward, action);
    if (key == GLFW_KEY_SPACE) InputManagerController::handleKey(&this->toggleRotation, action);
}

KeyState *InputManagerController::keySwitch(const KeyCode &key) {
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

void InputManagerController::handleKey(KeyState *key, const int &actionCode) {
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

void InputManagerController::destroy() {}
