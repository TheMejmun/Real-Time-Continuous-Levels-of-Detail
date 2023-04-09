//
// Created by Sam on 2023-04-09.
//

#include "ecs.h"

void ECS::create(uint32_t size) {
    this->max_entities = size;
    this->renderables.resize(size);
}