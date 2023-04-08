//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_WINDOW_MANAGER_H
#define REALTIME_CELL_COLLAPSE_WINDOW_MANAGER_H

#include <memory>
#include <string>
#include <GLFW/glfw3.h>

const int32_t DEFAULT_WIDTH = 1280;
const int32_t DEFAULT_HEIGHT = 720;

class WindowManager {
public:
    void create(const std::string &title);

    void destroy();

     bool shouldClose();

     void close();

    GLFWwindow *window = nullptr;
private:
    int32_t width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;
    std::string title;
};

#endif //REALTIME_CELL_COLLAPSE_WINDOW_MANAGER_H
