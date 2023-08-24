//
// Created by Saman on 18.04.23.
//

#include "graphics/renderer.h"
#include "graphics/uniform_buffer_object.h"
#include "graphics/vulkan/vulkan_swapchain.h"

// SYSTEMS THAT PLUG INTO THE ECS

void Renderer::uploadRenderables(ECS &ecs) {
    auto entities = ecs.requestEntities(Renderer::EvaluatorToAllocate);
    for (auto components: entities) {
        auto &mesh = *components->renderMesh;
        this->bufferManager.uploadVertices(mesh.vertices);
        this->bufferManager.uploadIndices(mesh.indices);
        mesh.is_allocated = true;
    }
}

void Renderer::destroyRenderables(ECS &ecs) {
    auto entities = ecs.requestEntities(Renderer::EvaluatorToDeallocate);
    for (auto components: entities) {
        auto &mesh = *components->renderMesh;
        THROW("TODO");
    }
}

void Renderer::updateUniformBuffer(const sec &delta, ECS &ecs) {
    auto entities = ecs.requestEntities(Renderer::EvaluatorToDraw);

    auto &sphere = *entities[0];
    // TODO not just for one object
    UniformBufferObject ubo{};
    ubo.model = sphere.transform->forward;

    auto &camera = *ecs.requestEntities(Renderer::EvaluatorActiveCamera)[0];

    ubo.view = camera.camera->getView(*camera.transform);

    ubo.proj = camera.camera->getProjection(
            static_cast<float >(VulkanSwapchain::extent.width) / static_cast<float >(VulkanSwapchain::extent.height));

    // TODO replace with push constants for small objects:
    // https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html

    this->bufferManager.nextUniformBuffer();
    memcpy(this->bufferManager.getCurrentUniformBufferMapping(), &ubo, sizeof(ubo));
}