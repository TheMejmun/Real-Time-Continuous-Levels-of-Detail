//
// Created by Sam on 2023-04-09.
//

#include "ecs.h"
#include "printer.h"
#include <stdexcept>

void ECS::create() {
}

uint32_t ECS::insert(const Components &entityComponents) {
    for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
        if (!this->isOccupied[i]) {
            this->renderables[i] = entityComponents.renderable;
            this->isOccupied[i] = true;
            return i;
        }
    }

    this->isOccupied.push_back(true);
    this->renderables.push_back(entityComponents.renderable);
    return isOccupied.size() - 1;
}

void ECS::remove(uint32_t index) {
    if (this->isOccupied[index]) {
        this->isOccupied[index] = false;
    } else {
        THROW("Attempted to remove non-existent entity from ECS.");
    }
}

std::vector<Renderable *> ECS::requestRenderables(uint16_t flags) {
    std::vector<Renderable *> out{};

    for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
        bool use = false;

        if ((flags & FLAG_RENDERABLE_TO_ALLOCATE) > 0) {
            // use |= (this->isOccupied[i] && (this->renderables[i]->allocatedBuffer == nullptr));
            THROW("TODO");
        }

        if ((flags & FLAG_RENDERABLE_TO_DEALLOCATE) > 0) {
            // use |=  (!this->isOccupied[i] && (this->renderables[i]->allocatedBuffer != nullptr));
            THROW("TODO");
        }

        if ((flags & FLAG_RENDERABLE_TO_RENDER) > 0) {
            // use |= (this->isOccupied[i] && (this->renderables[i]->allocatedBuffer != nullptr));
            THROW("TODO");
        }

        if (use) {
            out.push_back(this->renderables[i]);
        }
    }

    return out;
}