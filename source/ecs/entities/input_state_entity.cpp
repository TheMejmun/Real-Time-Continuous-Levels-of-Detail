//
// Created by Saman on 31.08.23.
//

#include "ecs/entities/input_state_entity.h"

InputStateEntity::InputStateEntity() {
    this->components.inputState = std::make_unique<InputState>();
}