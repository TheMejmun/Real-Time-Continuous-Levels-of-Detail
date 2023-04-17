//
// Created by Saman on 17.04.23.
//

#ifndef REALTIME_CELL_COLLAPSE_WORLD_H
#define REALTIME_CELL_COLLAPSE_WORLD_H

#include "renderable.h"

class World{
public:
    void create();

    void destroy();

    Renderable renderable{};
};

#endif //REALTIME_CELL_COLLAPSE_WORLD_H
