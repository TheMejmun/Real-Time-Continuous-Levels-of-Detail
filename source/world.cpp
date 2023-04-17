//
// Created by Saman on 17.04.23.
//

#include "world.h"
#include "importer.h"
#include "colors.h"

void World::create() {
    auto mesh = Importinator::importMesh("resources/models/earth.glb");
    this->renderable.indices = std::move(mesh.indices);
    this->renderable.vertices = std::move(mesh.vertices);
    for (auto &v: this->renderable.vertices) {
        v.color = Color::random().getLAB();
    }
    this->renderable.model.scale(0.002f);
}

void World::destroy() {

}