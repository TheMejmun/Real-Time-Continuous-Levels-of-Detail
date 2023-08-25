//
// Created by Sam on 2023-03-28.
//

#ifndef REALTIME_CELL_COLLAPSE_RENDERER_H
#define REALTIME_CELL_COLLAPSE_RENDERER_H

#include "util/glfw_include.h"
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <set>
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include "renderer.h"
#include "util/importer.h"
#include "vertex.h"
#include "triangle.h"
#include "util/timer.h"
#include "io/printer.h"
#include "graphics/vulkan/vulkan_buffers.h"
#include "projector.h"
#include "ecs/ecs.h"
#include "util/glm_include.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "graphics/vulkan/vulkan_instance.h"

//#define WIREFRAME_MODE

//const int MAX_FRAMES_IN_FLIGHT = 2;
// TODO https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight

class Renderer {
public:
    void create(const std::string &title, GLFWwindow *window);

    sec draw(const sec &delta, ECS &ecs);

    void destroy();

private:

    void createGraphicsPipeline();

    void createDescriptorSetLayout();

    // TODO Take out delta time
    void updateUniformBuffer(const sec &delta, ECS &ecs);

    void createDescriptorPool();

    void createDescriptorSets();

    void initVulkan();

    VkShaderModule createShaderModule(const std::vector<char> &code);

    void createCommandPool();

    // TODO void createTextureImage();

    void createSyncObjects();

    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);

    static inline bool EvaluatorActiveCamera(const Components &components) {
        return components.camera != nullptr && components.transform != nullptr && components.isAlive() &&
               components.isMainCamera;
    };

    static inline bool EvaluatorToAllocate(const Components &components) {
        return components.renderMesh != nullptr && components.isAlive() && !components.renderMesh->is_allocated;
    };

    static inline bool EvaluatorToDeallocate(const Components &components) {
        return components.renderMesh != nullptr && components.willDestroy && components.renderMesh->is_allocated;
    };

    static inline bool EvaluatorToDraw(const Components &components) {
        return components.renderMesh != nullptr && components.transform != nullptr && components.isAlive() &&
               components.renderMesh->is_allocated;
    };

    void uploadRenderables(ECS &ecs);

    void destroyRenderables(ECS &ecs);

    chrono_sec_point lastTimestamp = Timer::now();

    std::string title;
    GLFWwindow *window = nullptr;

    VBufferManager bufferManager{};

    // Depth testing
    // TODO destroy:
//    VkImage depthImage;
//    VkDeviceMemory depthImageMemory;
//    VkImageView depthImageView;

    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkDescriptorPool descriptorPool = nullptr;
    std::vector<VkDescriptorSet> descriptorSets{}; // Will be cleaned up with pool
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline graphicsPipeline = nullptr;
    VkCommandPool commandPool = nullptr;

    VkSemaphore imageAvailableSemaphore = nullptr;
    VkSemaphore renderFinishedSemaphore = nullptr;
    VkFence inFlightFence = nullptr;
};

#endif //REALTIME_CELL_COLLAPSE_RENDERER_H
