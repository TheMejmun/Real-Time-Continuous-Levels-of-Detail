//
// Created by Saman on 27.08.23.
//

#include "graphics/vulkan/vulkan_swapchain.h"
#include "ecs/systems/mesh_simplifier_controller.h"
#include "ecs/systems/camera_controller.h"
#include "ecs/entities/camera.h"
#include "io/printer.h"
#include "util/timer.h"
#include "util/performance_logging.h"

#include <thread>
#include <limits>
#include <unordered_set>
#include <set>
#include <algorithm>

//#define OUTPUT_MAPPINGS
#define CACHE_LOCALITY_MODE

const uint32_t MAX_PIXELS_PER_VERTEX = 1;
const uint32_t MAX_INDEX = std::numeric_limits<uint32_t>::max();

std::thread thread;
uint32_t simplifiedMeshCalculationThreadFrameCounter = 0;
chrono_sec_point simplifiedMeshCalculationThreadStartedTime{};
bool meshCalculationDone = false;

struct SVO { // Simplification Vertex Object
    bool set = false;
    uint32_t index = 0;
    glm::vec3 worldPos{};
    float depth = 0.0f;
};

inline float distance2(const glm::vec3 &a, const glm::vec3 &b) {
    return (a.x - b.x) * (a.x - b.x) +
           (a.y - b.y) * (a.y - b.y) +
           (a.z - b.z) * (a.z - b.z);
}

struct Triangle {
    uint32_t id1;
    uint32_t id2;
    uint32_t id3;

    // This function is used by unordered_set to compare
    bool operator==(const Triangle &other) const {
        return (this->id1 == other.id1 &&
                this->id2 == other.id2 &&
                this->id3 == other.id3);
    }

    // This function is used by unordered_set to hash
    // If the IDs get too high, we may get collisions
    size_t operator()(const Triangle &triangle) const noexcept {
        uint32_t maxBytes = sizeof(size_t);
        uint32_t allowedBytesPerId = maxBytes / 3;
        uint32_t allowedBitsPerId = CHAR_BIT * allowedBytesPerId;

        size_t hash = (((triangle.id1
                << allowedBitsPerId) + triangle.id2)
                << allowedBitsPerId) + triangle.id3;
        return hash;
    };

    // compare for order.
    bool operator<(const Triangle &rhs) const {
        uint32_t maxBytes = sizeof(size_t);
        uint32_t allowedBytesPerId = maxBytes / 3;
        uint32_t allowedBitsPerId = CHAR_BIT * allowedBytesPerId;

        size_t lhsHash = (((this->id1
                << allowedBitsPerId) + this->id2)
                << allowedBitsPerId) + this->id3;
        size_t rhsHash = (((rhs.id1
                << allowedBitsPerId) + rhs.id2)
                << allowedBitsPerId) + rhs.id3;

        return lhsHash < rhsHash;
    }

};

// Change the triangle to start with the lowest index, but retain the face direction
Triangle orientTriangle(const Triangle &triangle) {
    if (triangle.id1 < triangle.id2 && triangle.id1 < triangle.id3) {
        // id1 is smallest
        return triangle;
    } else if (triangle.id2 < triangle.id1 && triangle.id2 < triangle.id1) {
        // id2 is smallest
        return {triangle.id2, triangle.id3, triangle.id1};
    } else {
        // id3 is smallest
        return {triangle.id3, triangle.id1, triangle.id2};
    }
}

class IndexLut {
public:
    uint32_t getMapping(uint32_t forIndex) {
        uint32_t found = forIndex;
        while (found != MAX_INDEX && this->indexMappings[found] != 0) {
            // because the stored mappings are +1
            found = this->indexMappings[found] == MAX_INDEX ? MAX_INDEX : this->indexMappings[found] - 1;
        }
        return found;
    }

    void insertMapping(uint32_t from, uint32_t to) {
        this->indexMappings[from] = to == MAX_INDEX ? MAX_INDEX : to + 1;
    }

    bool isSet(uint32_t index) {
        return this->indexMappings[index] != 0;
    }

    void resize(size_t size) {
        this->indexMappings.resize(size);
    }

private:
    // Treat 0 as not set -> all ids += 1
    std::vector<uint32_t> indexMappings{};
};

void simplify(const Components *camera, const Components *components) {
    // Init
    const auto model = components->transform->forward;
    const auto normalModel = glm::transpose(components->transform->inverse);
    const auto cameraPos = camera->transform->getPosition();
    const auto view = camera->camera->getView(*camera->transform);
    const auto proj = camera->camera->getProjection(VulkanSwapchain::aspectRatio);

    auto &to = *components->renderMeshSimplifiable;
    auto &from = *components->renderMesh;
    to.vertices.clear();
    to.indices.clear();

    const uint32_t rasterWidth = VulkanSwapchain::framebufferWidth / MAX_PIXELS_PER_VERTEX;
    const uint32_t rasterHeight = VulkanSwapchain::framebufferHeight / MAX_PIXELS_PER_VERTEX;
    std::vector<SVO> indicesRaster{};
    indicesRaster.resize(rasterWidth * rasterHeight);
    DBG "Using raster " << rasterWidth << " * " << rasterHeight << " for mesh simplification" ENDL;

    IndexLut lut{};
    lut.resize(from.vertices.size());
    uint32_t newVertexCount = 0;

    // Calculate raster positions

    for (uint32_t i = 0; i < from.vertices.size(); ++i) {
        const glm::vec4 worldPos = model * glm::vec4(from.vertices[i].pos, 1.0f);

        // Is facing away from camera
        if (glm::dot(glm::vec4(cameraPos, 1.0f) - worldPos,
                     normalModel * glm::vec4(from.vertices[i].normal, 1.0f)) < 0) {
            lut.insertMapping(i, MAX_INDEX);
            continue;
        }

        const glm::vec4 projectedPos = proj * view * worldPos;
        const long x = lroundf((projectedPos.x * 0.5f / projectedPos.w + 0.5f) * static_cast<float>(rasterWidth));
        const long y = lroundf((projectedPos.y * 0.5f / projectedPos.w + 0.5f) * static_cast<float>(rasterHeight));

        if (x < 0 || x >= rasterWidth || y < 0 || y >= rasterHeight) {
            lut.insertMapping(i, MAX_INDEX);
            continue;
        }

        const float depth = projectedPos.z / projectedPos.w;
        const uint32_t rasterIndex = y * rasterWidth + x;

        if (!indicesRaster[rasterIndex].set) {
            ++newVertexCount;
            indicesRaster[rasterIndex] = {
                    .set = true,
                    .index = i,
                    .worldPos = worldPos,
                    .depth = depth
            };
        } else if (indicesRaster[rasterIndex].depth > depth) {
            // stored vertex is farther away
            lut.insertMapping(indicesRaster[rasterIndex].index, i); // Map previously stored to the current vertex
            indicesRaster[rasterIndex] = {
                    .set = true,
                    .index = i,
                    .worldPos = worldPos,
                    .depth = depth
            };
        } else {
            // stored vertex is closer to the camera than the current
            lut.insertMapping(i, indicesRaster[rasterIndex].index); // Map previously stored to the current vertex
        }
    }

    // Map the used vertices' indices to skip unused ones
    std::vector<bool> isVertexUsed{};
    isVertexUsed.resize(from.vertices.size());

    // Filter triangles
    std::unordered_set<Triangle, Triangle> triangles{}; // Ordered set
    for (uint32_t i = 0; i < from.indices.size(); i += 3) {
        const uint32_t id1 = lut.getMapping(from.indices[i]);
        const uint32_t id2 = lut.getMapping(from.indices[i + 1]);
        const uint32_t id3 = lut.getMapping(from.indices[i + 2]);

        if (id1 == MAX_INDEX || id2 == MAX_INDEX || id3 == MAX_INDEX ||
            id1 == id2 || id1 == id3 || id2 == id3)
            continue;

        isVertexUsed[id1] = true;
        isVertexUsed[id2] = true;
        isVertexUsed[id3] = true;

        const Triangle triangle = orientTriangle({id1, id2, id3});
        triangles.insert(triangle);
    }

    // Push
#ifdef CACHE_LOCALITY_MODE
    to.indices.reserve(triangles.size() * 3);
    to.vertices.reserve(to.indices.size()); // Duplicate vertices for good cache locality

    for (const auto &[id1, id2, id3]: triangles) {
        to.indices.push_back(to.vertices.size());
        to.vertices.push_back(from.vertices[id1]);
        to.indices.push_back(to.vertices.size());
        to.vertices.push_back(from.vertices[id2]);
        to.indices.push_back(to.vertices.size());
        to.vertices.push_back(from.vertices[id3]);
    }

#else
    std::vector<uint32_t> usedVertexIndexMappings;
    usedVertexIndexMappings.resize(from.vertices.size());

    to.indices.reserve(triangles.size() * 3);
    to.vertices.reserve(newVertexCount);

    // Initialise the first value to enable the usedVertexIndexMappings[i] == 0 condition
    const uint32_t firstIndex = triangles.begin()->id1;
    usedVertexIndexMappings.push_back(firstIndex);
    to.vertices.emplace_back(from.vertices[firstIndex]);

    for (const auto &[id1, id2, id3]: triangles) {
        if (usedVertexIndexMappings[id1] == 0) {
            usedVertexIndexMappings[id1] = to.vertices.size();
            to.indices.push_back(to.vertices.size());
            to.vertices.push_back(from.vertices[id1]);
        } else {
            to.indices.push_back(usedVertexIndexMappings[id1]);
        }

        if (usedVertexIndexMappings[id2] == 0) {
            usedVertexIndexMappings[id2] = to.vertices.size();
            to.indices.push_back(to.vertices.size());
            to.vertices.push_back(from.vertices[id2]);
        } else {
            to.indices.push_back(usedVertexIndexMappings[id2]);
        }

        if (usedVertexIndexMappings[id3] == 0) {
            usedVertexIndexMappings[id3] = to.vertices.size();
            to.indices.push_back(to.vertices.size());
            to.vertices.push_back(from.vertices[id3]);
        } else {
            to.indices.push_back(usedVertexIndexMappings[id3]);
        }
    }
#endif
}

void MeshSimplifierController::update(ECS &ecs, sec *timeTaken, uint32_t *framesTaken) {
    if (thread.joinable()) {
        simplifiedMeshCalculationThreadFrameCounter++;
        if (meshCalculationDone) {
            DBG "Mesh calculation thread took " << simplifiedMeshCalculationThreadFrameCounter << " frames" ENDL;
            thread.join();
            *timeTaken = Timer::duration(simplifiedMeshCalculationThreadStartedTime, Timer::now());
            *framesTaken = simplifiedMeshCalculationThreadFrameCounter;
        }
    } else {
        auto entities = ecs.requestEntities(MeshSimplifierController::EvaluatorToSimplify);
        auto camera = ecs.requestEntities(CameraController::EvaluatorActiveCamera)[0];

        if (!entities.empty()) {
            meshCalculationDone = false;
            simplifiedMeshCalculationThreadFrameCounter = 0;
            simplifiedMeshCalculationThreadStartedTime = Timer::now();

            auto function = [=](bool &done) {
                for (auto components: entities) {
                    if (components->renderMeshSimplifiable->simplifiedMeshMutex.try_lock()) {
                        PerformanceLogging::meshCalculationStarted();
                        simplify(camera, components);
                        components->renderMeshSimplifiable->updateSimplifiedMesh = true;
                        PerformanceLogging::meshCalculationFinished();
                        components->renderMeshSimplifiable->simplifiedMeshMutex.unlock();
                    }
                }
                done = true;
            };

            thread = std::thread(function, std::ref(meshCalculationDone));
        }
    }
}

void MeshSimplifierController::destroy() {
    if (thread.joinable())
        thread.join();
}