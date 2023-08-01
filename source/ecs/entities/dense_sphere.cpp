//
// Created by Saman on 18.04.23.
//

#include "ecs/entities/dense_sphere.h"
#include "util/importer.h"
#include "graphics/colors.h"

DenseSphere::DenseSphere() {
    auto mesh = Importinator::importMesh("resources/models/dense_sphere.glb");
    this->components.render_mesh = new RenderMesh();
    this->components.render_mesh->indices = std::move(mesh.indices);
    // this->components.renderable->indices = { 0, 500, 1000 };
    this->components.render_mesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.render_mesh->vertices) {
        v.color = Color::random().getLAB();
    }

    this->components.transform = new Transformer4();
    this->components.transform->scale(2.0f);

    this->components.is_rotating_sphere = true;
}