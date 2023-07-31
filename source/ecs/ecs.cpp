//
// Created by Sam on 2023-04-09.
//

#include "ecs/ecs.h"
#include "io/printer.h"
#include <stdexcept>
#include "ecs/world.h"
#include "ecs/dense_sphere.h"

void ECS::create() {
    INF "Creating ECS" ENDL;
}

uint32_t ECS::insert(Components &entityComponents) {
    for (uint32_t i = 0; i < this->components.size(); ++i) {
        if (this->components[i].will_destroy) {
            destroyReferences(i); // Clean up old references before overriding pointers
        }
        if (this->components[i].is_destroyed) {
            this->components[i] = entityComponents; // Move reference here -> param cant be const
            entityComponents.index = i;
            return i;
        }
    }

    this->components.push_back(entityComponents);
    entityComponents.index = this->components.size() - 1;
    return entityComponents.index;
}

void ECS::destroy() {
    INF "Destroying ECS" ENDL;

    for (uint32_t i = 0; i < this->components.size(); ++i) {
        destroyReferences(i);
    }
}

void ECS::remove(const uint32_t &index) {
    this->components[index].will_destroy = true;
}

void ECS::destroyReferences(const uint32_t &index) {
    this->components[index].destroy();
}

std::vector<Components *>
ECS::requestComponents(const std::function<bool(const Components &)> &evaluator) {
    std::vector<Components *> out{};

    for (auto &component: this->components) {
        if (evaluator(component)) {
            out.push_back(&component);
        }
    }

    return out;
}