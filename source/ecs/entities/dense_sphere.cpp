//
// Created by Saman on 18.04.23.
//

#include "ecs/entities/dense_sphere.h"
#include "util/importer.h"
#include "graphics/colors.h"

//#define MONKEY
//#define SMALL_SPHERE

DenseSphere::DenseSphere() {

#ifdef MONKEY
    auto mesh = Importinator::importMesh("resources/models/monkey.glb");
#elif defined SMALL_SPHERE
    auto mesh = Importinator::importMesh("resources/models/earth.glb");
#else
    auto mesh = Importinator::importMesh("resources/models/dense_sphere.glb");
#endif

    this->components.renderMesh = new RenderMesh();
    this->components.renderMesh->indices = std::move(mesh.indices);
    // this->entities.renderable->indices = { 0, 500, 1000 };
    this->components.renderMesh->vertices = std::move(mesh.vertices);
    for (auto &v: this->components.renderMesh->vertices) {
        v.color = Color::random().getLAB();
    }

    this->components.transform = new Transformer4();

#ifdef MONKEY
    this->components.transform->scale(1.0f);
#elif defined SMALL_SPHERE
    this->components.transform->scale(0.001f);
#else
    this->components.transform->scale(2.0f);
#endif

    this->components.isRotatingSphere = true;

    this->components.simplifyMesh = true;
}