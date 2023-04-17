//
// Created by Sam on 2023-03-28.
//

#include "renderer.h"
#include "uniform_buffer_object.h"

void Renderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = this->swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Before rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // After rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Before rendering
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

    if (vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS) {
        THROW("Failed to create render pass!");
    }
}

void Renderer::createGraphicsPipeline() {
    // TODO pull these out of here
    auto vertShaderCode = Importinator::readFile("resources/shaders/triangle.vert.spv");
    DBG "Loaded vertex shader with byte size: " << vertShaderCode.size() ENDL;
    auto fragShaderCode = Importinator::readFile("resources/shaders/triangle.frag.spv");
    DBG "Loaded fragment shader with byte size: " << fragShaderCode.size() ENDL;

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
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // requires GPU feature
#else
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
#endif
    rasterizer.lineWidth = 1.0f; // > 1.0f requires wideLines GPU feature

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

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

    if (vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) !=
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
    pipelineInfo.renderPass = this->renderPass; // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
    pipelineInfo.subpass = 0; // Subpass index for this pipeline
    // Index or handle of parent pipeline. -> Perf+ if available
    // Needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag in this struct
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(this->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &this->graphicsPipeline) != VK_SUCCESS) {
        THROW("Failed to create graphics pipeline!");
    }

    // Once the pipeline is created, we don't need this anymore
    vkDestroyShaderModule(this->logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(this->logicalDevice, vertShaderModule, nullptr);
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

    if (vkCreateDescriptorSetLayout(this->logicalDevice, &layoutInfo, nullptr, &this->descriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void Renderer::updateUniformBuffer(const sec &delta, const Camera &camera, const ECS &ecs) {
    // TODO
    UniformBufferObject ubo{};
    this->world.renderable.model.rotate(
            glm::radians(30.0f * static_cast<float >(delta)),
            glm::vec3(0, 1, 0));
    ubo.model = this->world.renderable.model.forward;

    ubo.view = camera.view.forward; // Identity

    ubo.proj = camera.getProjection(
            static_cast<float >(this->swapchainExtent.width) / static_cast<float >(this->swapchainExtent.height));

    // TODO replace with push constants for small objects:
    // https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html

    this->bufferManager.nextUniformBuffer();
    memcpy(this->bufferManager.getCurrentUniformBufferMapping(), &ubo, sizeof(ubo));
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

    if (vkCreateDescriptorPool(this->logicalDevice, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS) {
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
    if (vkAllocateDescriptorSets(this->logicalDevice, &allocInfo, this->descriptorSets.data()) != VK_SUCCESS) {
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
        vkUpdateDescriptorSets(this->logicalDevice, 1, &descriptorWrite, 0, nullptr);
    }
}

void Renderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // for vkResetCommandBuffer
    // Use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT if buffer is very short-lived
    poolInfo.queueFamilyIndex = this->queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
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

    if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFence) != VK_SUCCESS) {
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
    renderPassInfo.renderPass = this->renderPass;
    renderPassInfo.framebuffer = this->swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->swapchainExtent;

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
    viewport.width = static_cast<float>(this->swapchainExtent.width);
    viewport.height = static_cast<float>(this->swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->swapchainExtent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    // TODO Do not directly access uniform buffer index like this
//    DBG this->bufferManager.uniformBufferIndex ENDL;
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1,
                            &this->descriptorSets[this->bufferManager.uniformBufferIndex], 0, nullptr);

    vkCmdDrawIndexed(buffer, static_cast<uint32_t>(this->world.renderable.indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        THROW("Failed to record command buffer!");
    }
}

sec Renderer::draw(const sec &delta, const Camera &camera, const ECS &ecs) {
    if (shouldRecreateSwapchain()) {
        bool success = recreateSwapchain();
        if (success) {
            DBG "Created new swapchain" ENDL;
            this->needsNewSwapchain = false;
        } else {
            DBG "Failed to create new swapchain" ENDL;
            return -1;
        }
    }

    auto beforeFence = Timer::now();
    vkWaitForFences(this->logicalDevice, 1, &this->inFlightFence, VK_TRUE, UINT64_MAX);
    auto afterFence = Timer::now();

    uint32_t imageIndex;
    auto acquireImageResult = vkAcquireNextImageKHR(this->logicalDevice, this->swapchain, UINT64_MAX,
                                                    this->imageAvailableSemaphore, nullptr, &imageIndex);

    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
        DBG "Swapchain is out of date" ENDL;
        recreateSwapchain();
        return Timer::duration(beforeFence, afterFence); // Why not
    } else if (acquireImageResult == VK_SUBOPTIMAL_KHR) {
        DBG "Swapchain is suboptimal" ENDL;
        this->needsNewSwapchain = true;

    } else if (acquireImageResult != VK_SUCCESS) {
        THROW("Failed to acquire swapchain image!");
    }

    // Avoid deadlock if recreating -> move to after success check
    vkResetFences(this->logicalDevice, 1, &this->inFlightFence);

    auto commandBuffer = this->bufferManager.commandBuffer;

    updateUniformBuffer(delta, camera, ecs);

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
    if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFence) != VK_SUCCESS) {
        THROW("Failed to submit draw command buffer!");
    }
//    END_TRACE("QUEUE SUBMIT")

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {this->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Per swapchain acquireImageResult

    vkQueuePresentKHR(presentQueue, &presentInfo);

    return Timer::duration(beforeFence, afterFence);
}