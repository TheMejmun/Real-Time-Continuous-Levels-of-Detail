//
// Created by Sam on 2023-04-11.
//

#include "renderer.h"

bool QueueFamilyIndices::isComplete() const {
    return this->graphicsFamily.has_value() &&
           this->presentFamily.has_value();
}

bool QueueFamilyIndices::isUnifiedGraphicsPresentQueue() const {
    if (!this->isComplete()) return false;
    return this->graphicsFamily.value() == this->presentFamily.value();
}