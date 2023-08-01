//
// Created by Saman on 17.04.23.
//

#include "ecs/entities/world.h"
#include "util/importer.h"
#include "graphics/colors.h"

World::World() {
    auto mesh = Importinator::importMesh("resources/models/earth.glb");
    this->components.render_mesh = new RenderMesh();
    this->components.render_mesh->indices = std::move(mesh.indices);
    this->components.render_mesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.render_mesh->vertices) {
        v.color = Color::random().getLAB();
    }

    this->components.transform = new Transformer4();
    this->components.transform->scale(0.002f);

    this->components.is_rotating_sphere = true;
}