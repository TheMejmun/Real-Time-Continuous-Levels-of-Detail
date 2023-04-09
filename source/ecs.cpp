//
// Created by Sam on 2023-04-09.
//

#include "ecs.h"
#include <stdexcept>

void ECS::create(uint32_t size) {
    this->max_entities = size;
    this->isOccupied.resize(size);
    this->renderables.resize(size);
}

uint32_t ECS::insert(const Components &entityComponents) {
    for (uint32_t i = 0; i < this->max_entities; ++i) {
        if (!this->isOccupied[i]) {
            this->renderables[i] = entityComponents.renderable;
            this->isOccupied[i] = true;
            return i;
        }
    }
    throw std::runtime_error("ECS is full. Can not add more entities.");
}

void ECS::remove(uint32_t index) {
    if (this->isOccupied[index]) {
        this->isOccupied[index] = false;
    } else {
        throw std::runtime_error("Attempted to remove non-existent entity from ECS.");
    }
}

std::tuple<std::vector<bool>, std::vector<Renderable *>> ECS::requestRenderables(uint16_t flags) {
    std::vector<bool> out{};
    out.resize(this->max_entities);

    for (uint32_t i = 0; i < this->max_entities; ++i) {
        if ((flags & FLAG_TO_ALLOCATE) > 0) {
            out[i] = out[i] || (this->isOccupied[i] && (this->renderables[i]->allocatedBuffer == nullptr));
        }

        if ((flags & FLAG_TO_DEALLOCATE) > 0) {
            out[i] = out[i] || (!this->isOccupied[i] && (this->renderables[i]->allocatedBuffer != nullptr));
        }

        if ((flags & FLAG_TO_RENDER) > 0) {
            out[i] = out[i] || (this->isOccupied[i] && (this->renderables[i]->allocatedBuffer != nullptr));
        }
    }
}