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
        VulkanBuffers::uploadVertices(mesh.vertices);
        VulkanBuffers::uploadIndices(mesh.indices);
        mesh.isAllocated = true;
    }
}

void Renderer::uploadSimplifiedMeshes(ECS &ecs) {
    const auto startTime = Timer::now();
    auto entities = ecs.requestEntities(Renderer::EvaluatorToAllocateSimplifiedMesh);

    int simplifiedMeshBuffer = 1 - VulkanBuffers::simplifiedMeshBuffersIndex;
    if (simplifiedMeshBuffer < 0 || simplifiedMeshBuffer > 1) simplifiedMeshBuffer = 0;

    bool uploadedAny = false;

    for (auto components: entities) {
        if (components->renderMeshSimplifiable->simplifiedMeshMutex.try_lock()) {
            auto &mesh = *components->renderMeshSimplifiable;
            // TODO this upload produced a bad access error
            VulkanBuffers::uploadVertices(mesh.vertices, simplifiedMeshBuffer);
            VulkanBuffers::uploadIndices(mesh.indices, simplifiedMeshBuffer);
            mesh.isAllocated = true;
            mesh.bufferIndex = simplifiedMeshBuffer;
            mesh.updateSimplifiedMesh = false;
            mesh.simplifiedMeshMutex.unlock();

            uploadedAny = true;
        }
    }

    if (uploadedAny) {
        // Treat this like a return
        this->meshBufferToUse = simplifiedMeshBuffer + 1;
        this->state.uiState.meshUploadTimeTaken = Timer::duration(startTime, Timer::now());
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

    ubo.proj = camera.camera->getProjection(VulkanSwapchain::aspectRatio);

    // TODO replace with push constants for small objects:
    // https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html

    VulkanBuffers::nextUniformBuffer();
    memcpy(VulkanBuffers::getCurrentUniformBufferMapping(), &ubo, sizeof(ubo));
}