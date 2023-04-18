//
// Created by Saman on 18.04.23.
//

#ifndef REALTIME_CELL_COLLAPSE_ENTITY_H
#define REALTIME_CELL_COLLAPSE_ENTITY_H

#include "ecs.h"

class Entity{
public:
    void upload(ECS &ecs);

    Components components{};
};

#endif //REALTIME_CELL_COLLAPSE_ENTITY_H
