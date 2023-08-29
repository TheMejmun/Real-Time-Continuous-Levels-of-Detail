//
// Created by Saman on 27.08.23.
//

#include "graphics/vulkan/vulkan_swapchain.h"
#include "ecs/systems/mesh_simplifier_controller.h"
#include "ecs/systems/camera_controller.h"
#include "ecs/entities/camera.h"
#include "io/printer.h"

#include <thread>
#include <limits>
#include <unordered_set>
#include <set>
#include <algorithm>

//#define OUTPUT_MAPPINGS

std::thread thread;
bool isRunning = false;
const uint32_t MAX_PIXELS_PER_VERTEX = 1;
const uint32_t MAX_INDEX = std::numeric_limits<uint32_t>::max();
uint32_t simplifiedMeshCalculationThreadFrameCounter = 0;


struct SVO { // Simplification Vertex Object
    bool set = false;
    uint32_t index = 0;
    glm::vec3 worldPos{};
    uint32_t screenX = 0;
    uint32_t screenY = 0;
    float depth = 0.0f;
};

inline float distance2(const glm::vec3 &a, const glm::vec3 &b) {
    return (a.x - b.x) * (a.x - b.x) +
           (a.y - b.y) * (a.y - b.y) +
           (a.z - b.z) * (a.z - b.z);
}

float minDepth(const float depth1, const float depth2, const float depth3) {
    return (depth1 < depth2 && depth1 < depth3) ?
           depth1 : (
                   (depth2 < depth1 && depth2 < depth3) ?
                   depth2 : depth3
           );
}

struct Triangle {
    uint32_t id1;
    uint32_t id2;
    uint32_t id3;
    float minDepth = 0.0f;

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
        return {triangle.id2, triangle.id3, triangle.id1, triangle.minDepth};
    } else {
        // id3 is smallest
        return {triangle.id3, triangle.id1, triangle.id2, triangle.minDepth};
    }
}

class IndexLut {
public:
    uint32_t getMapping(uint32_t forIndex) {
        uint32_t found = forIndex;
        while (found != MAX_INDEX && this->indexMappings[found] != 0) {
            found = this->indexMappings[found] == MAX_INDEX ? MAX_INDEX : this->indexMappings[found] - 1; // because the stored mappings are +1
        }
        return found;
    }

    void insertMapping(uint32_t from, uint32_t to) {
        this->indexMappings[from] = to == MAX_INDEX ? MAX_INDEX : to + 1;
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

    auto &to = *components->renderMeshSimplified;
    auto &from = *components->renderMesh;
    to.vertices = from.vertices; // TODO
    to.indices.clear();

    const uint32_t rasterWidth = VulkanSwapchain::framebufferWidth / MAX_PIXELS_PER_VERTEX;
    const uint32_t rasterHeight = VulkanSwapchain::framebufferHeight / MAX_PIXELS_PER_VERTEX;
    std::vector<SVO> indicesRaster{};
    indicesRaster.resize(rasterWidth * rasterHeight);
    DBG "Using raster " << rasterWidth << " * " << rasterHeight << " for mesh simplification" ENDL;

    IndexLut lut{};
    lut.resize(from.vertices.size());

    // Calculate raster positions
    for (uint32_t i = 0; i < from.vertices.size(); ++i) {
        const glm::vec4 worldPos = model * glm::vec4(from.vertices[i].pos, 1.0f);

        // Is facing away from camera
        if (glm::dot(glm::vec4(cameraPos, 1.0f) - worldPos, normalModel * glm::vec4(from.vertices[i].normal, 1.0f)) < 0) {
            lut.insertMapping(i, MAX_INDEX);
            continue;
        }

        const glm::vec4 projectedPos = proj * view * worldPos;
        const float depth = projectedPos.z / projectedPos.w;
        const long x = lroundf((projectedPos.x * 0.5f / projectedPos.w + 0.5f) * static_cast<float>(rasterWidth));
        const long y = lroundf((projectedPos.y * 0.5f / projectedPos.w + 0.5f) * static_cast<float>(rasterHeight));
        const uint32_t rasterIndex = y * rasterWidth + x;

        if (x < 0 || x >= rasterWidth || y < 0 || y >= rasterHeight) {
            continue;
        }

        if (!indicesRaster[rasterIndex].set) {
            indicesRaster[rasterIndex] = {
                    .set = true,
                    .index = i,
                    .worldPos = worldPos,
                    .screenX = static_cast<uint32_t>(x),
                    .screenY = static_cast<uint32_t>(y),
                    .depth = depth
            };
        } else if (indicesRaster[rasterIndex].depth > depth) {
            // stored vertex is farther away
            lut.insertMapping(indicesRaster[rasterIndex].index, i); // Map previously stored to the current vertex
            indicesRaster[rasterIndex] = {
                    .set = true,
                    .index = i,
                    .worldPos = worldPos,
                    .screenX = static_cast<uint32_t>(x),
                    .screenY = static_cast<uint32_t>(y),
                    .depth = depth
            };
        } else {
            // stored vertex is closer to the camera than the current
            lut.insertMapping(i, indicesRaster[rasterIndex].index); // Map previously stored to the current vertex
        }

#ifdef OUTPUT_MAPPINGS
        auto mappedId = lut.getMapping(i);
        auto result = std::find_if(indicesRaster.begin(), indicesRaster.end(), [&mappedId](const SVO &obj) { return obj.index == mappedId; });
        uint32_t index;
        if (result != indicesRaster.end())
            index = std::distance(indicesRaster.begin(), result);
        SVO mappedTo = indicesRaster[index];

        if (mappedTo.index != i) {
            printf("\tid: %d, screen: x: %d, y: %d, z: %1.3f,\tworld: x: %1.3f, y: %1.3f, z: %1.3f\n",
                   i, x, y, depth, worldPos.x, worldPos.y, worldPos.z);
            printf("->\tid: %d, screen: x: %d, y: %d, z: %1.3f,\tworld: x: %1.3f, y: %1.3f, z: %1.3f\n",
                   mappedId, mappedTo.screenX, mappedTo.screenY, mappedTo.depth, mappedTo.worldPos.x, mappedTo.worldPos.y, mappedTo.worldPos.z);
        }
#endif
    }
    DBG "Calculated raster positions" ENDL;

    // Count vertices & store depth
    uint32_t newVertexCount = 0;
    std::vector<float> mappedDepth{};
    mappedDepth.resize(from.vertices.size());
    for (auto &svo: indicesRaster) {
        if (svo.set) {
            newVertexCount++;
            mappedDepth[svo.index] = svo.depth;
        }
    }
    DBG "Using " << newVertexCount << " vertices, instead of " << from.vertices.size() << " before." ENDL;

//    // Map original vertices to reduced ones
//    std::vector<uint32_t> indexMappings{};
//    indexMappings.resize(from.vertices.size());
//    std::vector<float> mappedDepth{};
//    mappedDepth.resize(from.vertices.size());
//
//    for (uint32_t i = 0; i < from.vertices.size(); ++i) {
//        // For each original vertex, find the closest new one
//        float closestDistance = std::numeric_limits<float>::max();
//        SVO *closest = nullptr;
//
//        for (auto &svo: reducedSVOs) {
//            // Check distance and pick index of closest -> put into mappings
//            const float distance = distance2(svo.worldPos, from.vertices[i].pos);
//            if (distance < closestDistance) {
//                closest = &svo;
//                closestDistance = distance;
//            }
//        }
//
//        indexMappings[i] = closest->index;
//        mappedDepth[i] = closest->depth;
//    }
//    DBG "Mapped indices" ENDL;

    // Filter triangles
    std::unordered_set<Triangle, Triangle> triangles{}; // Ordered set
    for (uint32_t i = 0; i < from.indices.size(); i += 3) {
        const uint32_t id1 = lut.getMapping(from.indices[i]);
        const uint32_t id2 = lut.getMapping(from.indices[i + 1]);
        const uint32_t id3 = lut.getMapping(from.indices[i + 2]);

        if (id1 == MAX_INDEX || id2 == MAX_INDEX || id3 == MAX_INDEX ||
            id1 == id2 || id1 == id3 || id2 == id3)
            continue;

        const Triangle triangle = orientTriangle({id1, id2, id3,
                                                  minDepth(mappedDepth[id1],
                                                           mappedDepth[id2],
                                                           mappedDepth[id3])
                                                 });
        triangles.insert(triangle);
    }
    DBG "Filtered triangles" ENDL;

    // Sort
    std::vector<Triangle> trianglesSorted{};
    trianglesSorted.reserve(triangles.size());
    for (auto it = triangles.begin(); it != triangles.end();) {
        trianglesSorted.push_back(triangles.extract(it++).value());
    }
    auto sorter = [&](const Triangle &lhs, const Triangle &rhs) -> bool {
        return lhs.minDepth > rhs.minDepth;
    };
    std::sort(trianglesSorted.begin(), trianglesSorted.end(), sorter);
    DBG "Sorted by depth" ENDL;

    // Push
    to.indices.reserve(trianglesSorted.size());
    for (const Triangle &triangle: trianglesSorted) {
        to.indices.push_back(triangle.id1);
        to.indices.push_back(triangle.id2);
        to.indices.push_back(triangle.id3);
    }

    DBG "Using " << to.indices.size() / 3 << " triangles, instead of " << from.indices.size() / 3 << " before." ENDL;
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

        if (!entities.empty()) {
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
    if (isRunning)
        thread.join();
}