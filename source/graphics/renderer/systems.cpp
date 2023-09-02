//
// Created by Saman on 18.04.23.
//

#include "graphics/renderer.h"
#include "graphics/uniform_buffer_object.h"
#include "graphics/vulkan/vulkan_swapchain.h"
#include "util/performance_logging.h"

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
    if (VulkanBuffers::waitingForFence) {
        DBG "Waiting for fence" ENDL;
        if (VulkanBuffers::isTransferQueueReady()) {
            DBG "Ready" ENDL;
            VulkanBuffers::finishTransfer();
        } else {
            DBG "Not ready" ENDL;
            return;
        }
    }
    const auto startTime = Timer::now();
    auto entities = ecs.requestEntities(Renderer::EvaluatorToAllocateSimplifiedMesh);

    uint32_t bufferToUse = 1;
    if (VulkanBuffers::meshBufferToUse == 1) bufferToUse = 2;

    bool uploadedAny = false;

    for (auto components: entities) {
        if (components->renderMeshSimplifiable->simplifiedMeshMutex.try_lock()) {
            PerformanceLogging::meshUploadStarted();
            auto &mesh = *components->renderMeshSimplifiable;
            // TODO this upload produced a bad access error
            VulkanBuffers::uploadMesh(mesh.vertices, mesh.indices, true, bufferToUse);
            mesh.isAllocated = true;
            mesh.bufferIndex = bufferToUse;
            mesh.updateSimplifiedMesh = false;
            PerformanceLogging::meshUploadFinished();
            mesh.simplifiedMeshMutex.unlock();

            uploadedAny = true;
        }
    }

    if (uploadedAny) {
        // Treat this like a return
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