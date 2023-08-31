//
// Created by Saman on 31.08.23.
//

#include "ecs/entities/monkey.h"
#include "util/importer.h"
#include "graphics/colors.h"

Monkey::Monkey() {
    auto mesh = Importinator::importMesh("resources/models/monkey.glb");

    this->components.renderMesh = std::make_unique<RenderMesh>();
    this->components.renderMesh->indices = std::move(mesh.indices);
    this->components.renderMesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderMesh->vertices) {
        v.color = Color::random().getLAB();
    }
    this->components.renderMeshSimplifiable = std::make_unique<RenderMeshSimplifiable>();

    this->components.transform = std::make_unique<Transformer4>();
    this->components.transform->scale(1.0f);
    this->components.transform->rotate(glm::radians(180.0), glm::vec3(0, 1, 0));

    this->components.isRotatingSphere = true;
}