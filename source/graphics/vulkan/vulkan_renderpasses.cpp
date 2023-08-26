//
// Created by Saman on 24.08.23.
//

#include "graphics/vulkan/vulkan_renderpasses.h"
#include "graphics/vulkan/vulkan_devices.h"
#include "io/printer.h"
#include "graphics/vulkan/vulkan_swapchain.h"

VkRenderPass VulkanRenderPasses::renderPass = nullptr;

void VulkanRenderPasses::create() {
    INF "Creating VulkanRenderPasses" ENDL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VulkanSwapchain::imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Before rendering
//    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Imgui
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // After rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Before rendering
//    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Imgui
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // After rendering

    // Only a single attachment for now
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // -> layout(location = 0) out vec4 outColor
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Color buffer

    // Single subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Is graphics subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Output attachment

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // To avoid layout transitions before the image has been acquired
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before/after the current render pass
    dependency.dstSubpass = 0; // dstSubpass > srcSubpass !!! (unless VK_SUBPASS_EXTERNAL)
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait for swapchain
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // The stage to wait in
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // The operation that should wait
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(VulkanDevices::logical, &renderPassInfo, nullptr, &VulkanRenderPasses::renderPass) !=
        VK_SUCCESS) {
        THROW("Failed to create render pass!");
    }
}

void VulkanRenderPasses::destroy() {
    INF "Destroying VulkanRenderPasses" ENDL;

    vkDestroyRenderPass(VulkanDevices::logical, VulkanRenderPasses::renderPass, nullptr);
}