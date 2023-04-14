//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
#define REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H

#include <memory>

#define IM_DOWN_EVENT 0x1
#define IM_HELD 0x2
#define IM_UP_EVENT 0x3
#define IM_RELEASED 0x4

#define IM_CLOSE_WINDOW 0x0
#define IM_FULLSCREEN 0x1

class InputManager {
public:
    void create(GLFWwindow *window);

    void poll();

    uint8_t getKeyState(uint8_t key);

    uint8_t consumeKeyState(uint8_t key);

private:
    static void _callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void processInput(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void handleKey(uint8_t *key, int actionCode);

    GLFWwindow *window = nullptr;

    uint8_t closeWindow = IM_RELEASED;
    uint8_t toggleFullscreen = IM_RELEASED;
};

#endif //REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
