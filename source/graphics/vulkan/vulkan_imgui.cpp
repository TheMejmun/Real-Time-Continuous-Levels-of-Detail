//
// Created by Saman on 26.08.23.
//


#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include "graphics/vulkan/vulkan_imgui.h"
#include "io/printer.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "graphics/vulkan/vulkan_swapchain.h"
#include "graphics/vulkan/vulkan_renderpasses.h"
#include "graphics/vulkan/vulkan_buffers.h"
#include <sstream>

VkDescriptorPool uiDescriptorPool;
float scale = 1.0f;
ImVec2 scaleVec2 = {scale, scale};

static void checkVkResult(VkResult err) {
    if (err == 0) return;

    if (err < 0) {
        std::stringstream sstream;
        sstream << "Failed to create Vulkan GUI. VkResult: " << err;
        THROW(sstream.str());
    }
}

void VulkanImgui::create(RenderState &state) {
    INF "Creating VulkanImgui" ENDL;

    // https://github.com/ocornut/imgui/blob/master/examples/example_glfw_vulkan/main.cpp

    // Descriptor pool:
    VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;
    checkVkResult(
            vkCreateDescriptorPool(VulkanDevices::logical, &poolInfo, nullptr, &uiDescriptorPool)
    );

    // Context:
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.WantCaptureMouse = true;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Style:
    ImGui::StyleColorsDark();

    // Renderer:
    ImGui_ImplGlfw_InitForVulkan(state.window, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = VulkanInstance::instance;
    initInfo.PhysicalDevice = VulkanDevices::physical;
    initInfo.Device = VulkanDevices::logical;
    initInfo.QueueFamily = VulkanDevices::queueFamilyIndices.graphicsFamily.value();
    initInfo.Queue = VulkanDevices::graphicsQueue;
    initInfo.DescriptorPool = uiDescriptorPool; // TODO
    initInfo.Subpass = 0;
    initInfo.MinImageCount = VulkanSwapchain::minImageCount;
    initInfo.ImageCount = VulkanSwapchain::imageCount;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&initInfo, VulkanRenderPasses::renderPass);

    // Fonts:
    VkCommandPool command_pool = state.vulkanState.commandPool;
    VkCommandBuffer command_buffer = VulkanBuffers::commandBuffer;

    checkVkResult(vkResetCommandPool(VulkanDevices::logical, command_pool, 0));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    checkVkResult(vkBeginCommandBuffer(command_buffer, &beginInfo));

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo endInfo = {};
    endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    endInfo.commandBufferCount = 1;
    endInfo.pCommandBuffers = &command_buffer;
    checkVkResult(vkEndCommandBuffer(command_buffer));
    // TODO arbitrarily picked this queue because it was global
    checkVkResult(vkQueueSubmit(VulkanBuffers::transferQueue, 1, &endInfo, VK_NULL_HANDLE));

    checkVkResult(vkDeviceWaitIdle(VulkanDevices::logical));
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    VulkanImgui::recalculateScale(state);
}

void VulkanImgui::recalculateScale(RenderState &state) {
    int fbWidth, fbHeight, wWidth, wHeight;
    glfwGetFramebufferSize(state.window, &fbWidth, &fbHeight);
    glfwGetWindowSize(state.window, &wWidth, &wHeight);
    printf("Window width: %d,\tframebuffer width: %d\n", wWidth, fbWidth);
    printf("Window height: %d,\tframebuffer height: %d\n", wHeight, fbHeight);

    scale = (static_cast<float>(fbWidth) / static_cast<float>(wWidth));
    scaleVec2 = {scale, scale};
}

void VulkanImgui::draw(RenderState &state) {
    int width, height;
    glfwGetFramebufferSize(state.window, &width, &height);
    ImGui::GetIO().DisplaySize = {static_cast<float>(VulkanSwapchain::framebufferWidth),
                                  static_cast<float>(VulkanSwapchain::framebufferHeight)};
    ImGui::GetIO().DisplaySize = {static_cast<float>(width),
                                  static_cast<float>(height)};
    ImGui::GetIO().DisplayFramebufferScale = scaleVec2;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Performance");
    sec last_frametime = state.uiState.fps.frametimesLastSecond.back();
    ImGui::Text("Total frame time:\t%1.4f", last_frametime);
    ImGui::Text("Frames per second:\t%d", state.uiState.fps.currentFPS());
    ImGui::End();

    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();
    draw_data->FramebufferScale = scaleVec2;
    ImGui_ImplVulkan_RenderDrawData(draw_data, VulkanBuffers::commandBuffer);
}

void VulkanImgui::destroy() {
    INF "Destroying VulkanImgui" ENDL;

    vkDeviceWaitIdle(VulkanDevices::logical);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(VulkanDevices::logical, uiDescriptorPool, nullptr);
}