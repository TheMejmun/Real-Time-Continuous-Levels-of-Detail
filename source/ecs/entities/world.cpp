//
// Created by Saman on 17.04.23.
//

#include "ecs/entities/world.h"
#include "util/importer.h"
#include "graphics/colors.h"

World::World() {
    auto mesh = Importinator::importMesh("resources/models/earth.glb");
    this->components.renderMesh = new RenderMesh();
    this->components.renderMesh->indices = std::move(mesh.indices);
    this->components.renderMesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderMesh->vertices) {
        v.color = Color::random().getLAB();
    }

    this->components.transform = new Transformer4();
    this->components.transform->scale(0.002f);

    this->components.isRotatingSphere = true;
}