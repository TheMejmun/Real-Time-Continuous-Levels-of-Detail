//
// Created by Saman on 17.04.23.
//

#ifndef REALTIME_CELL_COLLAPSE_WORLD_H
#define REALTIME_CELL_COLLAPSE_WORLD_H

#include "renderable.h"
#include "ecs.h"

class World {
public:
    World();

    void upload(ECS &ecs);

    Components components{};
};

#endif //REALTIME_CELL_COLLAPSE_WORLD_H
