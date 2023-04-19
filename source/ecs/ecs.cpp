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
    for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
        if (!this->isOccupied[i]) {
            destroyReferences(i); // Clean up old references before overriding pointers
            this->renderables[i] = entityComponents.renderable; // Move reference here -> param cant be const
            this->isOccupied[i] = true;
            entityComponents.renderable->componentIndex = i;
            return i;
        }
    }

    this->isOccupied.push_back(true);
    this->renderables.push_back(entityComponents.renderable);
    entityComponents.renderable->componentIndex = isOccupied.size() - 1;
    return isOccupied.size() - 1;
}

void ECS::destroy() {
    INF "Destroying ECS" ENDL;

    for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
        destroyReferences(i);
    }
}

void ECS::remove(const uint32_t &index) {
    if (this->isOccupied[index]) {
        this->isOccupied[index] = false;
    } else {
        THROW("Attempted to remove non-existent entity from ECS.");
    }
}

void ECS::destroyReferences(const uint32_t &index) {
    if (this->renderables[index] != nullptr) delete this->renderables[index];
    this->renderables[index] = nullptr;
}

std::vector<Renderable *> ECS::requestRenderables(const std::function<bool(const bool &, const Renderable *)> &evaluator) {
    std::vector<Renderable *> out{};

    for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
        if (evaluator(this->isOccupied[i], this->renderables[i])) {
            out.push_back(this->renderables[i]);
        }
    }

    return out;
}