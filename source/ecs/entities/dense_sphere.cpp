//
// Created by Saman on 18.04.23.
//

#include "ecs/entities/dense_sphere.h"
#include "util/importer.h"
#include "graphics/colors.h"

DenseSphere::DenseSphere() {
    auto mesh = Importinator::importMesh("resources/models/dense_sphere.glb");

    this->components.renderMesh = std::make_unique<RenderMesh>();
    this->components.renderMesh->indices = std::move(mesh.indices);
    this->components.renderMesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderMesh->vertices) {
        v.color = Color::random().getLAB();
    }
    this->components.renderMeshSimplifiable = std::make_unique<RenderMeshSimplifiable>();

    this->components.transform = std::make_unique<Transformer4>();
    this->components.transform->scale(2.0f);

    this->components.isRotatingSphere = true;
}