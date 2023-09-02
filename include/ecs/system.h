//
// Created by Saman on 30.08.23.
//

#ifndef REALTIME_CELL_COLLAPSE_SYSTEM_H
#define REALTIME_CELL_COLLAPSE_SYSTEM_H

#include "util/timer.h"
#include "ecs/ecs.h"

class System {
public:
    virtual void destroy() = 0;

    virtual void update(sec delta, ECS &ecs) = 0;
};

#endif //REALTIME_CELL_COLLAPSE_SYSTEM_H
