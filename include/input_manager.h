//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
#define REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H

#include <memory>

class InputManager {
public:
    void create(GLFWwindow*  window);

    void poll();

    bool closeWindow = false;

private:
    GLFWwindow* window = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
