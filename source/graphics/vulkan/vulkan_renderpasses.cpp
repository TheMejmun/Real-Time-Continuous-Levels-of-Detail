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

    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VulkanSwapchain::imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Before rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // After rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Before rendering
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // After rendering

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // -> layout(location = 0) out vec4 outColor
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Color buffer

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanSwapchain::findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Single subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Is graphics subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Output attachment
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // To avoid layout transitions before the image has been acquired
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before/after the current render pass
    dependency.dstSubpass = 0; // dstSubpass > srcSubpass !!! (unless VK_SUBPASS_EXTERNAL)
    // dependency.srcStageMask // Wait for swapchain
    dependency.srcAccessMask = 0;
    // dependency.dstStageMask // The stage to wait in
    // dependency.dstAccessMask // The operation that should wait
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Add depth attachment
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (vkCreateRenderPass(VulkanDevices::logical, &renderPassInfo, nullptr, &VulkanRenderPasses::renderPass) !=
        VK_SUCCESS) {
        THROW("Failed to create render pass!");
    }
}

void VulkanRenderPasses::destroy() {
    INF "Destroying VulkanRenderPasses" ENDL;

    vkDestroyRenderPass(VulkanDevices::logical, VulkanRenderPasses::renderPass, nullptr);
}