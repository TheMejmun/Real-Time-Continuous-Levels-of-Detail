//
// Created by Sam on 2023-04-08.
//

#ifndef REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H
#define REALTIME_CELL_COLLAPSE_INPUT_MANAGER_H

#include "preprocessor.h"
#include "ecs/entity.h"
#include "ecs/system.h"
#include "ecs/entities/input_state_entity.h"

#include <GLFW/glfw3.h>
#include <memory>

class InputController : public System {
public:
    void create(GLFWwindow *window, ECS &ecs);

    virtual void destroy() override;

    virtual void update(sec delta, ECS &ecs) override;

    static inline bool EvaluatorInputManagerEntity(const Components &components) {
        return components.isAlive() && components.inputState != nullptr;
    };

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
