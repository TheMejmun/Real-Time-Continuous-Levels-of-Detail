//
// Created by Saman on 17.04.23.
//

#include "ecs/world.h"
#include "util/importer.h"
#include "graphics/colors.h"

World::World() {
    auto mesh = Importinator::importMesh("resources/models/earth.glb");
    this->components.renderable = new Renderable();
    this->components.renderable->indices = std::move(mesh.indices);
    this->components.renderable->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderable->vertices) {
        v.color = Color::random().getLAB();
    }
    this->components.renderable->model.scale(0.002f);
}