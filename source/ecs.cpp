//
// Created by Sam on 2023-04-09.
//

#include "ecs.h"
#include "printer.h"
#include <stdexcept>

void ECS::create() {
}

uint32_t ECS::insert(const Components& entityComponents) {
	for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
		if (!this->isOccupied[i]) {
			destroyReferences(i); // Clean up old references before overriding pointers
			this->renderables[i] = std::move(entityComponents.renderable); // Move reference here
			this->isOccupied[i] = true;
			return i;
		}
	}

	this->isOccupied.push_back(true);
	this->renderables.push_back(entityComponents.renderable);
	return isOccupied.size() - 1;
}

void ECS::destroy() {
	for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
		destroyReferences(i);
	}
}

void ECS::remove(const uint32_t& index) {
	if (this->isOccupied[index]) {
		this->isOccupied[index] = false;
	}
	else {
		THROW("Attempted to remove non-existent entity from ECS.");
	}
}

void ECS::destroyReferences(const uint32_t& index) {
	if (this->renderables[index] != nullptr) delete this->renderables[index];
	this->renderables[index] = nullptr;
}

template<typename Evaluator>
std::vector<Renderable*> ECS::requestRenderables(const Evaluator& evaluator) {
	std::vector<Renderable*> out{};

	for (uint32_t i = 0; i < this->isOccupied.size(); ++i) {
		if (evaluator(this->renderables[i])) {
			out.push_back(this->renderables[i]);
		}
	}

	return out;
}