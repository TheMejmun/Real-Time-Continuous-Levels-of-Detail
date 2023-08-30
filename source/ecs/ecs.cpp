//
// Created by Sam on 2023-04-09.
//

#include "ecs/ecs.h"
#include "io/printer.h"
#include "ecs/entities/dense_sphere.h"

#include <stdexcept>

void ECS::create() {
    INF "Creating ECS" ENDL;
}

uint32_t ECS::insert(Components &entityComponents) {
    for (uint32_t i = 0; i < this->entities.size(); ++i) {
        if (this->entities[i].willDestroy) {
            destroyReferences(i); // Clean up old references before overriding pointers
        }
        if (this->entities[i].isDestroyed) {
            this->entities[i] = entityComponents; // Move reference here -> param cant be const
            entityComponents.index = i;
            return i;
        }
    }

    this->entities.push_back(entityComponents);
    entityComponents.index = this->entities.size() - 1;
    return entityComponents.index;
}

void ECS::destroy() {
    INF "Destroying ECS" ENDL;

    for (uint32_t i = 0; i < this->entities.size(); ++i) {
        destroyReferences(i);
    }
}

void ECS::remove(const uint32_t &index) {
    this->entities[index].willDestroy = true;
}

void ECS::destroyReferences(const uint32_t &index) {
    this->entities[index].destroy();
}

std::vector<Components *>
ECS::requestEntities(const std::function<bool(const Components &)> &evaluator) {
    std::vector<Components *> out{};

    for (auto &component: this->entities) {
        if (evaluator(component)) {
            out.push_back(&component);
        }
    }

    return out;
}