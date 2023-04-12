//
// Created by Sam on 2023-04-11.
//

#include "renderer.h"

bool QueueFamilyIndices::isComplete() const {
    return this->graphicsFamily.has_value() &&
           this->presentFamily.has_value() &&
           this->transferFamily.has_value();
}

bool QueueFamilyIndices::isUnifiedGraphicsPresentQueue() const {
    if (!this->graphicsFamily.has_value() ||
        !this->presentFamily.has_value())
        return false;
    return this->graphicsFamily.value() == this->presentFamily.value();
}

void QueueFamilyIndices::print() {
    DBG
            "QueueFamilyIndices:"
            << " Graphics: " << this->graphicsFamily.value()
            << " Present: " << this->presentFamily.value()
            << " Transfer: " << this->transferFamily.value()
            ENDL;
}