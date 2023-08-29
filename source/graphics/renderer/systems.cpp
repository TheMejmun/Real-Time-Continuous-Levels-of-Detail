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

uint32_t simplifiedMeshUploadThreadFrameCounter = 0;
chrono_sec_point simplifiedMeshUploadThreadStartedTime{};
bool simplifiedMeshUploadDone = false;
bool uploadedAnySimplifiedMeshes = false;

void Renderer::uploadSimplifiedMeshesThreadHelper(ECS &ecs) {
    if (this->simplifiedMeshAllocationThreadRunning) {
        simplifiedMeshUploadThreadFrameCounter++;
        if (simplifiedMeshUploadDone && this->simplifiedMeshAllocationThread.joinable()) {
            this->simplifiedMeshAllocationThread.join();
            if (uploadedAnySimplifiedMeshes) {
                this->state.uiState.meshUploadTimeTaken = Timer::duration(simplifiedMeshUploadThreadStartedTime, Timer::now());
                this->state.uiState.meshUploadFramesTaken = simplifiedMeshUploadThreadFrameCounter;
            }
            this->simplifiedMeshAllocationThreadRunning = false;
        }
    } else {
        this->simplifiedMeshAllocationThreadRunning = true;
        simplifiedMeshUploadThreadFrameCounter = 0;
        simplifiedMeshUploadDone = false;
        simplifiedMeshUploadThreadStartedTime = Timer::now();
        uploadedAnySimplifiedMeshes = false;
        auto function = [this](ECS &ecs, uint32_t &bufferToUseAfter, bool &done, bool &uploadedAnySimplifiedMeshes) {
            Renderer::uploadSimplifiedMeshes(ecs, bufferToUseAfter, uploadedAnySimplifiedMeshes);
            done = true;
        };

        this->simplifiedMeshAllocationThread = std::thread(function, std::ref(ecs), std::ref(this->meshBufferToUse),
                                                           std::ref(simplifiedMeshUploadDone), std::ref(uploadedAnySimplifiedMeshes));
    }
}

void Renderer::uploadSimplifiedMeshes(ECS &ecs, uint32_t &bufferToUseAfter, bool &uploadedAny) {
    auto entities = ecs.requestEntities(Renderer::EvaluatorToAllocateSimplifiedMesh);

    int simplifiedMeshBuffer = 1 - VulkanBuffers::simplifiedMeshBuffersIndex;
    if (simplifiedMeshBuffer < 0 || simplifiedMeshBuffer > 1) simplifiedMeshBuffer = 0;


    for (auto components: entities) {
        if (components->simplifiedMeshMutex == nullptr) {
            components->simplifiedMeshMutex = new std::mutex{};
        }
        if (components->simplifiedMeshMutex->try_lock()) {
            auto &mesh = *components->renderMeshSimplified;
            // TODO this upload produced a bad access error
            VulkanBuffers::uploadVertices(mesh.vertices, simplifiedMeshBuffer);
            VulkanBuffers::uploadIndices(mesh.indices, simplifiedMeshBuffer);
            mesh.isAllocated = true;
            mesh.bufferIndex = simplifiedMeshBuffer;
            components->updateSimplifiedMesh = false;
            components->simplifiedMeshMutex->unlock();

            uploadedAny = true;
        }
    }

    if (uploadedAny) {
        // Treat this like a return
        bufferToUseAfter = simplifiedMeshBuffer + 1;
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