//
// Created by Saman on 18.04.23.
//

#include "graphics/renderer.h"
#include "graphics/uniform_buffer_object.h"

// SYSTEMS THAT PLUG INTO THE ECS

void Renderer::uploadRenderables(ECS &ecs) {
    auto components_array = ecs.requestComponents(Renderer::EvaluatorToAllocate);
    for (auto components: components_array) {
        auto &mesh = *components->render_mesh;
        this->bufferManager.uploadVertices(mesh.vertices);
        this->bufferManager.uploadIndices(mesh.indices);
        mesh.is_allocated = true;
    }
}

void Renderer::destroyRenderables(ECS &ecs) {
    auto components_array = ecs.requestComponents(Renderer::EvaluatorToDeallocate);
    for (auto components: components_array) {
        auto &mesh = *components->render_mesh;
        THROW("TODO");
    }
}

void Renderer::updateUniformBuffer(const sec &delta, ECS &ecs) {
    auto components_array = ecs.requestComponents(Renderer::EvaluatorToDraw);

    auto &sphere = *components_array[0];
    // TODO not just for one object
    UniformBufferObject ubo{};
    ubo.model = sphere.transform->forward;

    auto &camera = *ecs.requestComponents(Renderer::EvaluatorActiveCamera)[0];

    ubo.view = camera.transform->forward; // Identity

    ubo.proj = camera.camera->getProjection(
            static_cast<float >(this->swapchainExtent.width) / static_cast<float >(this->swapchainExtent.height));

    // TODO replace with push constants for small objects:
    // https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html

    this->bufferManager.nextUniformBuffer();
    memcpy(this->bufferManager.getCurrentUniformBufferMapping(), &ubo, sizeof(ubo));
}