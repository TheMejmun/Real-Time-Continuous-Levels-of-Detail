//
// Created by Saman on 27.08.23.
//

#include "ecs/systems/mesh_simplifier_controller.h"
#include <thread>
#include "graphics/vulkan/vulkan_swapchain.h"
#include "io/printer.h"
#include "ecs/systems/camera_controller.h"
#include "ecs/entities/camera.h"
#include <limits>

std::thread thread;
bool isRunning = false;
const uint32_t MAX_PIXELS_PER_VERTEX = 2;
//const uint32_t MAX_INDEX = std::numeric_limits<uint32_t>::max();
uint32_t simplifiedMeshCalculationThreadFrameCounter = 0;

struct SVO { // Simplification Vertex Object
    bool set = false;
    uint32_t index = 0;
    glm::vec3 worldPos{};
    glm::vec3 projectedPos{};
    float depth = 0.0f;
};

void simplify(const Components *camera, const Components *components) {
    // Init
    auto model = components->transform->forward;
    auto view = camera->camera->getView(*camera->transform);
    auto proj = camera->camera->getProjection(VulkanSwapchain::aspectRatio);

    auto &to = *components->renderMeshSimplified;
    auto &from = *components->renderMesh;

    to.vertices = from.vertices; // TODO
    to.indices.clear();

    uint32_t rasterWidth = VulkanSwapchain::framebufferWidth / MAX_PIXELS_PER_VERTEX;
    uint32_t rasterHeight = VulkanSwapchain::framebufferHeight / MAX_PIXELS_PER_VERTEX;
    DBG "Using raster " << rasterWidth << " * " << rasterHeight << " for meshSimplification" ENDL;

    std::vector<SVO> indicesRaster{};
    indicesRaster.resize(rasterWidth * rasterHeight);

    // Calculate raster positions
    for (uint32_t i = 0; i < from.vertices.size(); ++i) {
        auto projectedPos = proj * view * model * glm::vec4(from.vertices[i].pos, 1.0f);
        long x = static_cast<long>(projectedPos.x) / MAX_PIXELS_PER_VERTEX + rasterWidth / 2;
        long y = static_cast<long>(projectedPos.y) / MAX_PIXELS_PER_VERTEX + rasterHeight / 2;
        float depth = projectedPos.z;
        uint32_t rasterIndex = y * rasterWidth + x;

        // DBG x << " " << y ENDL;

        if (x < 0 || x > rasterWidth || y < 0 || y > rasterHeight) {
            continue;
        }

        if (!indicesRaster[rasterIndex].set || indicesRaster[rasterIndex].depth > depth) {
            indicesRaster[rasterIndex] = {
                    .set = true,
                    .index = i,
                    .worldPos = from.vertices[i].pos,
                    .projectedPos = projectedPos,
                    .depth = depth
            };
        }
    }

    // Count
    uint32_t newVertexCount = 0;
    for (auto &x: indicesRaster) {
        if (x.set) ++newVertexCount;
    }
    DBG "Using " << newVertexCount << " vertices, instead of " << from.vertices.size() << " vertices before." ENDL;

    to.indices = from.indices; // TODO
}

bool meshCalculationDone = false;

void MeshSimplifierController::update(ECS &ecs) {
    if (isRunning) {
        simplifiedMeshCalculationThreadFrameCounter++;
        if (meshCalculationDone && thread.joinable()) {
            DBG "Mesh calculation thread took " << simplifiedMeshCalculationThreadFrameCounter << " frames" ENDL;
            thread.join();
            isRunning = false;
        }
    } else {
        auto entities = ecs.requestEntities(MeshSimplifierController::EvaluatorToSimplify);
        auto camera = ecs.requestEntities(CameraController::EvaluatorActiveCamera)[0];

        if (entities.size() > 0) {
            isRunning = true;
            meshCalculationDone = false;
            simplifiedMeshCalculationThreadFrameCounter = 0;

            auto function = [=](bool &done) {
                for (auto components: entities) {
                    if (components->simplifiedMeshMutex == nullptr) {
                        components->simplifiedMeshMutex = new std::mutex{};
                    }
                    if (components->simplifiedMeshMutex->try_lock()) {
                        if (components->renderMeshSimplified == nullptr) {
                            components->renderMeshSimplified = new RenderMesh();
                        }

                        simplify(camera, components);
                        components->updateSimplifiedMesh = true;
                        components->simplifiedMeshMutex->unlock();
                    }
                }
                done = true;
            };

            thread = std::thread(function, std::ref(meshCalculationDone));
        }
    }
}

void MeshSimplifierController::destroy() {
    thread.join();
}