//
// Created by Saman on 18.04.23.
//

#include "ecs/entity.h"

void Entity::upload(ECS &ecs) {
    ecs.insert(this->components);
}