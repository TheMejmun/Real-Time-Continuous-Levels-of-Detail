//
// Created by Saman on 18.04.23.
//

#include "ecs/entities/dense_sphere.h"
#include "util/importer.h"
#include "graphics/colors.h"

DenseSphere::DenseSphere() {
//    auto mesh = Importinator::importMesh("resources/models/dense_sphere.glb");
    auto mesh = Importinator::importMesh("resources/models/monkey.glb");
    this->components.renderMesh = new RenderMesh();
    this->components.renderMesh->indices = std::move(mesh.indices);
    // this->entities.renderable->indices = { 0, 500, 1000 };
    this->components.renderMesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderMesh->vertices) {
        v.color = Color::random().getLAB();
    }

    this->components.transform = new Transformer4();
    this->components.transform->scale(2.0f);

    this->components.isRotatingSphere = true;
}