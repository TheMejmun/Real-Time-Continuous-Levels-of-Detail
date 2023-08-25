//
// Created by Sam on 2023-03-28.
//

#include "graphics/renderer.h"
#include "graphics/uniform_buffer_object.h"
#include "graphics/vulkan/vulkan_renderpasses.h"
#include "graphics/vulkan/vulkan_swapchain.h"


void Renderer::createGraphicsPipeline() {
    // TODO pull these out of here
    auto vertShaderCode = Importinator::readFile("resources/shaders/sphere.vert.spv");
    VRB "Loaded vertex shader with byte size: " << vertShaderCode.size() ENDL;
    auto fragShaderCode = Importinator::readFile("resources/shaders/sphere.frag.spv");
    VRB "Loaded fragment shader with byte size: " << fragShaderCode.size() ENDL;

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    // Use pSpecializationInfo to specify constants

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input format info
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP for vert reuse
    inputAssembly.primitiveRestartEnable = VK_FALSE; // Break up lines and triangle in "_STRIP" with index 0xFFFF(FFFF)

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    // Optional dynamic state
    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Requires a GPU feature. Clamps to near/far plane instead of discording
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Disables output to Framebuffer
#ifdef WIREFRAME_MODE
    if (this->optionalFeatures.supportsWireframeMode) {
        rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // requires GPU feature
    } else {
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    }
#else
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
#endif
    rasterizer.lineWidth = 1.0f; // > 1.0f requires wideLines GPU feature

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Requires a GPU feature
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Depth or Stencil testing: VkPipelineDepthStencilStateCreateInfo

    // Color blend info per attached Framebuffer
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // Global Color blending
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // e.g. bitwise combination. Will disable blendEnable in attached FBs
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Define uniforms
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(VulkanDevices::logical, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) !=
        VK_SUCCESS) {
        THROW("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = this->pipelineLayout;
    pipelineInfo.renderPass = VulkanRenderPasses::renderPass; // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
    pipelineInfo.subpass = 0; // Subpass index for this pipeline
    // Index or handle of parent pipeline. -> Perf+ if available
    // Needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag in this struct
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(VulkanDevices::logical, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &this->graphicsPipeline) != VK_SUCCESS) {
        THROW("Failed to create graphics pipeline!");
    }

    // Once the pipeline is created, we don't need this anymore
    vkDestroyShaderModule(VulkanDevices::logical, fragShaderModule, nullptr);
    vkDestroyShaderModule(VulkanDevices::logical, vertShaderModule, nullptr);
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Can also be all shader stages: VK_SHADER_STAGE_ALL_GRAPHICS
    uboLayoutBinding.pImmutableSamplers = nullptr; // Relevant for image sampling

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(VulkanDevices::logical, &layoutInfo, nullptr, &this->descriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void Renderer::createDescriptorPool() {
    // Can have multiple pools, with multiple buffers each
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(VBufferManager::UBO_BUFFER_COUNT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(VBufferManager::UBO_BUFFER_COUNT);
    // poolInfo.flags = 0; // Investigate VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    // Would mean that Descriptor sets could individually be freed to their pools
    // Would allow vkFreeDescriptorSets
    // Otherwise only vkAllocateDescriptorSets and vkResetDescriptorPool

    if (vkCreateDescriptorPool(VulkanDevices::logical, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Renderer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(VBufferManager::UBO_BUFFER_COUNT, this->descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(VBufferManager::UBO_BUFFER_COUNT);
    allocInfo.pSetLayouts = layouts.data();

    this->descriptorSets.resize(VBufferManager::UBO_BUFFER_COUNT);
    if (vkAllocateDescriptorSets(VulkanDevices::logical, &allocInfo, this->descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < VBufferManager::UBO_BUFFER_COUNT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = this->bufferManager.getUniformBuffer(i);
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject); // Or VK_WHOLE_SIZE

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = this->descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0; // Descriptors can be arrays! -> index 0 here
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1; // starting at .dstArrayElement

        // Specify one of these three, depending on the type of descriptor this is
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        // Optional VkCopyDescriptorSet to copy between descriptors
        vkUpdateDescriptorSets(VulkanDevices::logical, 1, &descriptorWrite, 0, nullptr);
    }
}

void Renderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = VulkanDevices::queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(VulkanDevices::logical, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
        THROW("Failed to create command pool!");
    }

    this->bufferManager.createCommandBuffer(this->commandPool);
}

void Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start off as signaled

    if (vkCreateSemaphore(VulkanDevices::logical, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore) !=
        VK_SUCCESS ||
        vkCreateSemaphore(VulkanDevices::logical, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore) !=
        VK_SUCCESS ||
        vkCreateFence(VulkanDevices::logical, &fenceInfo, nullptr, &this->inFlightFence) != VK_SUCCESS) {
        THROW("Failed to create semaphores and/or fences!");
    }
}

void Renderer::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferUsageFlagBits.html
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        THROW("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = VulkanRenderPasses::renderPass;
    renderPassInfo.framebuffer = VulkanSwapchain::framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = VulkanSwapchain::extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

    VkBuffer vertexBuffers[] = {this->bufferManager.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    // Offset and number of bindings, buffers, and byte offsets from those buffers
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(buffer, this->bufferManager.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(VulkanSwapchain::extent.width);
    viewport.height = static_cast<float>(VulkanSwapchain::extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VulkanSwapchain::extent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    // TODO Do not directly access uniform buffer index like this
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1,
                            &this->descriptorSets[this->bufferManager.uniformBufferIndex], 0, nullptr);

    vkCmdDrawIndexed(buffer, this->bufferManager.indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        THROW("Failed to record command buffer!");
    }
}

sec Renderer::draw(const sec &delta, ECS &ecs) {
    if (VulkanSwapchain::shouldRecreateSwapchain()) {
        bool success = VulkanSwapchain::recreateSwapchain();
        if (success) {
            DBG "Created new swapchain" ENDL;
            VulkanSwapchain::needsNewSwapchain = false;
        } else {
            DBG "Failed to create new swapchain" ENDL;
            return -1;
        }
    }

    uploadRenderables(ecs);
    destroyRenderables(ecs);

    auto beforeFence = Timer::now();
    vkWaitForFences(VulkanDevices::logical, 1, &this->inFlightFence, VK_TRUE, UINT64_MAX);
    auto afterFence = Timer::now();

    uint32_t imageIndex;
    auto acquireImageResult = vkAcquireNextImageKHR(VulkanDevices::logical, VulkanSwapchain::swapchain, UINT64_MAX,
                                                    this->imageAvailableSemaphore, nullptr, &imageIndex);

    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
        DBG "Swapchain is out of date" ENDL;
        VulkanSwapchain::recreateSwapchain();
        return Timer::duration(beforeFence, afterFence); // Why not
    } else if (acquireImageResult == VK_SUBOPTIMAL_KHR) {
        DBG "Swapchain is suboptimal" ENDL;
        VulkanSwapchain::needsNewSwapchain = true;

    } else if (acquireImageResult != VK_SUCCESS) {
        THROW("Failed to acquire swapchain image!");
    }

    // Avoid deadlock if recreating -> move to after success check
    vkResetFences(VulkanDevices::logical, 1, &this->inFlightFence);

    auto commandBuffer = this->bufferManager.commandBuffer;

    updateUniformBuffer(delta, ecs);

    vkResetCommandBuffer(commandBuffer, 0); // I am not convinced this is necessary
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->imageAvailableSemaphore}; // index corresponding to wait stage
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // Wait in fragment stage
    // or VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {this->renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

//    START_TRACE
    if (vkQueueSubmit(VulkanDevices::graphicsQueue, 1, &submitInfo, this->inFlightFence) != VK_SUCCESS) {
        THROW("Failed to submit draw command buffer!");
    }
//    END_TRACE("QUEUE SUBMIT")

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {VulkanSwapchain::swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Per swapchain acquireImageResult

    vkQueuePresentKHR(VulkanDevices::presentQueue, &presentInfo);

    return Timer::duration(beforeFence, afterFence);
}