//
// Created by Saman on 18.04.23.
//

#include "ecs/dense_sphere.h"
#include "util/importer.h"
#include "graphics/colors.h"

DenseSphere::DenseSphere() {
    auto mesh = Importinator::importMesh("resources/models/dense_sphere.glb");
    this->components.renderable = new Renderable();
    this->components.renderable->indices = std::move(mesh.indices);
    this->components.renderable->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderable->vertices) {
        v.color = Color::random().getLAB();
    }
//    this->components.renderable->model.scale(0.002f);
}