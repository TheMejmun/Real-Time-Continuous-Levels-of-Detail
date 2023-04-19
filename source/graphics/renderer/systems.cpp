//
// Created by Saman on 18.04.23.
//

#include "graphics/renderer.h"
#include "graphics/uniform_buffer_object.h"

// SYSTEMS THAT PLUG INTO THE ECS

void Renderer::uploadRenderables(ECS &ecs) {
    auto renderables = ecs.requestRenderables(Renderer::EvaluatorToAllocate);
    for (auto &renderable: renderables) {
        if (!renderable->isAllocated) {
            this->bufferManager.uploadVertices(renderable->vertices);
            this->bufferManager.uploadIndices(renderable->indices);
            renderable->isAllocated = true;
        }
    }
}

void Renderer::destroyRenderables(ECS &ecs) {
    auto renderables = ecs.requestRenderables(Renderer::EvaluatorToDeallocate);
    for (auto &renderable: renderables) {
        THROW("TODO");
    }
}

void Renderer::updateUniformBuffer(const sec &delta, const Camera &camera, ECS&ecs) {
    auto renderables = ecs.requestRenderables(Renderer::EvaluatorToDraw);
    // TODO not just for one object
    UniformBufferObject ubo{};
    renderables[0]->model.rotate(
            glm::radians(15.0f * static_cast<float >(delta)),
            glm::vec3(0, 1, 0));
    ubo.model = renderables[0]->model.forward;

    ubo.view = camera.view.forward; // Identity

    ubo.proj = camera.getProjection(
            static_cast<float >(this->swapchainExtent.width) / static_cast<float >(this->swapchainExtent.height));

    // TODO replace with push constants for small objects:
    // https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html

    this->bufferManager.nextUniformBuffer();
    memcpy(this->bufferManager.getCurrentUniformBufferMapping(), &ubo, sizeof(ubo));
}