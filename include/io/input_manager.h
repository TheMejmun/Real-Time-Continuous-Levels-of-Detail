//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
#define REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H

#include <memory>
#include "util/glfw_include.h"

#define IM_DOWN_EVENT 0x1
#define IM_HELD 0x2
#define IM_UP_EVENT 0x3
#define IM_RELEASED 0x4

#define IM_CLOSE_WINDOW 0x0
#define IM_FULLSCREEN 0x1
#define IM_MOVE_FORWARD 0x2
#define IM_MOVE_BACKWARD 0x3

#define IM_TOGGLE_ROTATION 0x4

using KeyState = uint8_t;
using KeyCode = uint16_t;

class InputManager {
public:
    void create(GLFWwindow *window);

    void poll();

    KeyState getKeyState(const KeyCode &key);

    KeyState consumeKeyState(const KeyCode &key);

private:
    static void _callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void processInput(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void handleKey(KeyState *key, const int &actionCode);

    KeyState *keySwitch(const KeyCode &key);

    GLFWwindow *window = nullptr;

    KeyState closeWindow = IM_RELEASED;
    KeyState toggleFullscreen = IM_RELEASED;
    KeyState moveForward = IM_RELEASED;
    KeyState moveBackward = IM_RELEASED;
    KeyState toggleRotation = IM_RELEASED;
};

#endif //REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
