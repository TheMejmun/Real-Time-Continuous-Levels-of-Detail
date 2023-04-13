//
// Created by Sam on 2023-04-11.
//

#include "queue_family_indices.h"
#include "printer.h"

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

bool QueueFamilyIndices::hasUniqueTransferQueue() const {
    if (!this->graphicsFamily.has_value() ||
        !this->presentFamily.has_value())
        return false;
    return !(this->graphicsFamily.value() == this->transferFamily.value());
}

void QueueFamilyIndices::print() {
    DBG
            "QueueFamilyIndices:"
            << " Graphics: " << this->graphicsFamily.value()
            << " Present: " << this->presentFamily.value()
            << " Transfer: " << this->transferFamily.value()
            ENDL;
}