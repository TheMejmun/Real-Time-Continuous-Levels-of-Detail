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

void Renderer::uploadSimplifiedMeshesThreadHelper(ECS &ecs) {
    if (this->simplifiedMeshAllocationThreadRunning) {
        this->simplifiedMeshAllocationThreadFrameCounter++;
        if (this->simplifiedMeshAllocationThread.joinable()) {
            this->simplifiedMeshAllocationThread.join();
            DBG "Upload thread took " << this->simplifiedMeshAllocationThreadFrameCounter << " frames" ENDL;
            this->simplifiedMeshAllocationThreadRunning = false;
        }
    } else {
        this->simplifiedMeshAllocationThreadRunning = true;
        this->simplifiedMeshAllocationThreadFrameCounter = 0;
        this->simplifiedMeshAllocationThread = std::thread([&] {
            uploadSimplifiedMeshes(ecs);
        });
    }
}

void Renderer::uploadSimplifiedMeshes(ECS &ecs) {
    auto entities = ecs.requestEntities(Renderer::EvaluatorToAllocateSimplifiedMesh);

    int bufferToUse = 1 - VulkanBuffers::simplifiedMeshBuffersIndex;
    if (bufferToUse < 0) bufferToUse = 0;

    for (auto components: entities) {
        if (components->simplifiedMeshMutex == nullptr) {
            components->simplifiedMeshMutex = new std::mutex{};
        }
        if (components->simplifiedMeshMutex->try_lock()) {
            auto &mesh = *components->renderMeshSimplified;
            VulkanBuffers::uploadVertices(mesh.vertices, bufferToUse);
            VulkanBuffers::uploadIndices(mesh.indices, bufferToUse);
            mesh.isAllocated = true;
            mesh.bufferIndex = bufferToUse;
            components->updateSimplifiedMesh = false;
            components->simplifiedMeshMutex->unlock();
        }
    }

    // TODO test if this actually modifies the renderer by reference
    this->meshBufferToUse = bufferToUse;
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