#include "vulkan_imgui.h"
#include "platform/windows/win_platform.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_image.h"
#include "vulkan_pipeline.h"
#include "vulkan_shaders.h"

void
SetStyle(ImGuiStyle AppStyle, u32 index)
{
    switch (index)
    {
        case 0:
        {
            ImGuiStyle &Style = ImGui::GetStyle();
            Style = AppStyle;
            break;
        }
        case 1:
            ImGui::StyleColorsClassic();
            break;
        case 2:
            ImGui::StyleColorsDark();
            break;
        case 3:
            ImGui::StyleColorsLight();
            break;
    }
}

void
InitializeImGui(shoora_vulkan_imgui *ImGuiContext, vec2 ScreenDim)
{
    ImGuiStyle *Style = &ImGuiContext->UIStyle;

    *Style = ImGui::GetStyle();
    Style->Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    Style->Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    Style->Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    Style->Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    Style->Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    SetStyle(ImGuiContext->UIStyle, 2);

    ImGuiIO &IO = ImGui::GetIO();
    IO.DisplaySize = ImVec2(ScreenDim.x, ScreenDim.y);
    IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

#if defined(_WIN32)
    IO.KeyMap[ImGuiKey_Tab] = SU_TAB;
    IO.KeyMap[ImGuiKey_LeftArrow] = SU_LEFTARROW;
    IO.KeyMap[ImGuiKey_RightArrow] = SU_RIGHTARROW;
    IO.KeyMap[ImGuiKey_UpArrow] = SU_UPARROW;
    IO.KeyMap[ImGuiKey_DownArrow] = SU_DOWNARROW;
    IO.KeyMap[ImGuiKey_Backspace] = SU_BACKSPACE;
    IO.KeyMap[ImGuiKey_Enter] = SU_RETURN;
    IO.KeyMap[ImGuiKey_Space] = SU_SPACE;
    IO.KeyMap[ImGuiKey_Delete] = SU_DELETE;
#endif
}

void
InitializeResources(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImGUIContext, VkRenderPass RenderPass)
{
    ImGuiIO &IO = ImGui::GetIO();

    u8 *FontData = nullptr;
    i32 TexWidth, TexHeight;

    IO.Fonts->GetTexDataAsRGBA32(&FontData, &TexWidth, &TexHeight);
    VkDeviceSize UploadSize = TexWidth*TexHeight*4*sizeof(char);

    // Create Image resources to hold font data which can be used in shaders in the future.
    CreateSimpleImage2D(RenderDevice, Vec2(TexWidth, TexHeight), VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT, &ImGUIContext->FontImage,
                        &ImGUIContext->FontMemory, &ImGUIContext->FontImageView);

    // Create buffer to store store font data.
    shoora_vulkan_buffer StagingBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                      VK_SHARING_MODE_EXCLUSIVE,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      FontData, UploadSize);

    // Get command buffer to store transfer data from buffer to image
    VkCommandBuffer CopyCmdBuffer = CreateTransientCommandBuffer(RenderDevice, RenderDevice->GraphicsCommandPool,
                                                                 VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Pipeline Barrier to tell the API about the different transfer operations happening here.
    // Basically set the appropriate image layout for the transfer op. Set the image for optimal
    // transfer operation.
    SetImageLayout(CopyCmdBuffer, ImGUIContext->FontImage, VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Copy
    VkBufferImageCopy BufferCopy = {};
    BufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BufferCopy.imageSubresource.layerCount = 1;
    BufferCopy.imageExtent.width = TexWidth;
    BufferCopy.imageExtent.height = TexHeight;
    BufferCopy.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(CopyCmdBuffer, StagingBuffer.Handle, ImGUIContext->FontImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferCopy);

    // Prepare for shader read
    SetImageLayout(CopyCmdBuffer, ImGUIContext->FontImage, VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    FlushTransientCommandBuffer(RenderDevice, CopyCmdBuffer, RenderDevice->GraphicsQueue,
                                RenderDevice->GraphicsCommandPool, true);
    DestroyBuffer(RenderDevice, &StagingBuffer);


    // Descriptor Set Stuff
    CreateSampler2D(RenderDevice, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                    &ImGUIContext->FontSampler);
    VkDescriptorPoolSize PoolSize = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
    CreateDescriptorPool(RenderDevice, 1, &PoolSize, 2, &ImGUIContext->DescriptorPool);
    auto SetLayoutBinding = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                          VK_SHADER_STAGE_FRAGMENT_BIT);
    CreateDescriptorSetLayout(RenderDevice, &SetLayoutBinding, 1, &ImGUIContext->DescriptorSetLayout);
    AllocateDescriptorSets(RenderDevice, ImGUIContext->DescriptorPool, 1, &ImGUIContext->DescriptorSetLayout,
                           &ImGUIContext->DescriptorSet);
    auto FontDescriptor = GetImageDescriptorInfo(ImGUIContext->FontSampler, ImGUIContext->FontImageView,
                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    UpdateImageDescriptorSet(RenderDevice, ImGUIContext->DescriptorSet, 0,
                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &FontDescriptor);


    //? Pipeline Stuff
    // TODO)): Pipeline Cache stuff

    VkPushConstantRange PushConstantRange = {};
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(shoora_imgui_push_constant_block);

    auto PipelineLayoutInfo = GetPipelineLayoutCreateInfo(&ImGUIContext->DescriptorSetLayout, 1);
    PipelineLayoutInfo.pushConstantRangeCount = 1;
    PipelineLayoutInfo.pPushConstantRanges = &PushConstantRange;
    VK_CHECK(vkCreatePipelineLayout(RenderDevice->LogicalDevice, &PipelineLayoutInfo, nullptr,
                                    &ImGUIContext->PipelineLayout));

    auto InputAssemblyInfo = GetInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

    auto RasterizationState = GetRasterizationInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
                                                   VK_FRONT_FACE_CLOCKWISE);

    VkPipelineColorBlendAttachmentState BlendAttachment{};
    BlendAttachment.blendEnable = VK_TRUE;
    BlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    BlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    BlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    BlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    BlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    auto ColorBlendInfo = GetPipelineColorBlendInfo(1, &BlendAttachment);

    auto ViewPortInfo = GetPipelineViewportInfo(1, 1);

    auto MultiSampleInfo = GetPipelineMultiSampleInfo(VK_SAMPLE_COUNT_1_BIT);

    auto DepthStencilInfo = GetPipelineDepthStencilInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    auto DynamicStateInfo = GetPipelineDynamicStateInfo(ARRAY_SIZE(DynamicStates), DynamicStates);

    auto VertexInputBinding = GetPipelineVertexInputBinding(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
    VkVertexInputAttributeDescription VertexAttributes[] =
    {
        GetPipelineVertexAttribInfo(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
        GetPipelineVertexAttribInfo(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
        GetPipelineVertexAttribInfo(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
    };
    auto VertexInputState = PipelineVertexInputInfo();
    VertexInputState.vertexBindingDescriptionCount = 1;
    VertexInputState.pVertexBindingDescriptions = &VertexInputBinding;
    VertexInputState.vertexAttributeDescriptionCount = ARRAY_SIZE(VertexAttributes);
    VertexInputState.pVertexAttributeDescriptions = VertexAttributes;

    VkShaderModule VertexShader = CreateShaderModule(RenderDevice, "shaders/spirv/imgui.ui.vert.spv");
    VkShaderModule FragmentShader = CreateShaderModule(RenderDevice, "shaders/spirv/imgui.ui.frag.spv");
    VkPipelineShaderStageCreateInfo ShaderStages[] =
    {
        GetShaderStageInfo(VertexShader, VK_SHADER_STAGE_VERTEX_BIT, "main"),
        GetShaderStageInfo(FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT, "main")
    };

    auto GraphicsPipelineCreateInfo = GetGraphicsPipelineInfo(ImGUIContext->PipelineLayout,
                                                              RenderPass);
    GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyInfo;
    GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationState;
    GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendInfo;
    GraphicsPipelineCreateInfo.pMultisampleState = &MultiSampleInfo;
    GraphicsPipelineCreateInfo.pViewportState = &ViewPortInfo;
    GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilInfo;
    GraphicsPipelineCreateInfo.pDynamicState = &DynamicStateInfo;
    GraphicsPipelineCreateInfo.stageCount = ARRAY_SIZE(ShaderStages);
    GraphicsPipelineCreateInfo.pStages = ShaderStages;
    GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputState;

    VK_CHECK(vkCreateGraphicsPipelines(RenderDevice->LogicalDevice, 0, 1, &GraphicsPipelineCreateInfo, nullptr,
                                       &ImGUIContext->Pipeline));

    vkDestroyShaderModule(RenderDevice->LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(RenderDevice->LogicalDevice, FragmentShader, nullptr);
}

void
PrepareImGui(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImGuiContext, vec2 ScreenDim,
             VkRenderPass RenderPass)
{
    ImGuiContext->RenderDevice = RenderDevice;

    ImGui::CreateContext();

    ImGuiIO& IO = ImGui::GetIO();
    // TODO)): Set a UIOverlay in the device to decide this scale. This likely changes font size.
    IO.FontGlobalScale = 1.0f;

    ImGuiStyle &Style = ImGui::GetStyle();
    // TODO)): Set a UIOverlay in the device to decide this scale. This likely changes font size.
    Style.ScaleAllSizes(1.0f);

    InitializeImGui(ImGuiContext, ScreenDim);
    InitializeResources(RenderDevice, ImGuiContext, RenderPass);
}

void
ImGuiNewFrame()
{
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();
}

void
ImGuiUpdateBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext)
{
    ImDrawData *ImDrawData = ImGui::GetDrawData();

    VkDeviceSize VertexBufferSize = ImDrawData->TotalVtxCount*sizeof(ImDrawVert);
    VkDeviceSize IndexBufferSize = ImDrawData->TotalIdxCount*sizeof(ImDrawIdx);

    if((VertexBufferSize == 0) || (IndexBufferSize == 0))
    {
        return;
    }

    if((ImContext->VertexBuffer.Handle == VK_NULL_HANDLE) ||
       (ImContext->VertexCount != ImDrawData->TotalVtxCount))
    {
        DestroyBuffer(RenderDevice, &ImContext->VertexBuffer);
        ImContext->VertexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                               VK_SHARING_MODE_EXCLUSIVE,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               nullptr, VertexBufferSize);
        ImContext->VertexCount = ImDrawData->TotalVtxCount;
    }

    if((ImContext->IndexBuffer.Handle == VK_NULL_HANDLE) ||
       (ImContext->IndexCount != ImDrawData->TotalIdxCount))
    {
        DestroyBuffer(RenderDevice, &ImContext->IndexBuffer);
        ImContext->IndexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                              VK_SHARING_MODE_EXCLUSIVE,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                              nullptr, IndexBufferSize);
        ImContext->IndexCount = ImDrawData->TotalIdxCount;
    }

    // Upload data
    ImDrawVert *VtxDst = (ImDrawVert *)ImContext->VertexBuffer.pMapped;
    ImDrawIdx *IdxDst = (ImDrawIdx *)ImContext->IndexBuffer.pMapped;

    for(int Index = 0; Index < ImDrawData->CmdListsCount; Index++)
    {
        const ImDrawList *CmdList = ImDrawData->CmdLists[Index];
        memcpy(VtxDst, CmdList->VtxBuffer.Data, CmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(IdxDst, CmdList->IdxBuffer.Data, CmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        VtxDst += CmdList->VtxBuffer.Size;
        IdxDst += CmdList->IdxBuffer.Size;
    }
}

void
ImGuiDrawFrame(VkCommandBuffer CmdBuffer, shoora_vulkan_imgui *ImContext)
{
    ImGuiIO &IO = ImGui::GetIO();

    vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ImContext->PipelineLayout, 0, 1,
                            &ImContext->DescriptorSet, 0, nullptr);
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ImContext->Pipeline);

    VkViewport Viewport = GetViewport(IO.DisplaySize.x, IO.DisplaySize.y, 0.0f, 1.0f);
    vkCmdSetViewport(CmdBuffer, 0, 1, &Viewport);

    // UI scale and translate via push constants
    ImContext->PushConstantBlock.Scale = Vec2(2.0f/IO.DisplaySize.x, 2.0f/IO.DisplaySize.y);
    ImContext->PushConstantBlock.Translate = Vec2(-1.0f, -1.0f);
    vkCmdPushConstants(CmdBuffer, ImContext->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(shoora_imgui_push_constant_block), &ImContext->PushConstantBlock);

    // Render commands
    ImDrawData *ImDrawData = ImGui::GetDrawData();
    int32_t VertexOffset = 0;
    int32_t IndexOffset = 0;

    if (ImDrawData->CmdListsCount > 0)
    {
        VkDeviceSize Offsets[1] = {0};
        vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &ImContext->VertexBuffer.Handle, Offsets);
        vkCmdBindIndexBuffer(CmdBuffer, ImContext->IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT16);

        for(i32 OuterIndex = 0;
            OuterIndex < ImDrawData->CmdListsCount;
            OuterIndex++)
        {
            ImDrawList *pCmdList = ImDrawData->CmdLists[OuterIndex];

            for(i32 InnerIndex = 0;
                InnerIndex < pCmdList->CmdBuffer.Size;
                InnerIndex++)
            {
                ImDrawCmd *pCmd = &pCmdList->CmdBuffer[InnerIndex];

                VkRect2D ScissorRect;
                ScissorRect.offset.x = MAX((int32_t)(pCmd->ClipRect.x), 0);
                ScissorRect.offset.y = MAX((int32_t)(pCmd->ClipRect.y), 0);
                ScissorRect.extent.width = (u32)(pCmd->ClipRect.z - pCmd->ClipRect.x);
                ScissorRect.extent.height = (u32)(pCmd->ClipRect.w - pCmd->ClipRect.y);
                vkCmdSetScissor(CmdBuffer, 0, 1, &ScissorRect);

                vkCmdDrawIndexed(CmdBuffer, pCmd->ElemCount, 1, IndexOffset, VertexOffset, 0);

                IndexOffset += pCmd->ElemCount;
            }

            VertexOffset += pCmdList->VtxBuffer.Size;
        }
    }
}

void
ImGuiUpdateInput(b32 LMouseClicked, vec2 MousePos)
{
    ImGuiIO &IO = ImGui::GetIO();

    IO.MousePos = ImVec2(MousePos.x, MousePos.y);
    IO.MouseDown[0] = LMouseClicked;
}

void
ImGuiCleanup(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext)
{
    DestroyBuffer(RenderDevice, &ImContext->VertexBuffer);
    DestroyBuffer(RenderDevice, &ImContext->IndexBuffer);
    vkDestroySampler(RenderDevice->LogicalDevice, ImContext->FontSampler, nullptr);
    vkDestroyImageView(RenderDevice->LogicalDevice, ImContext->FontImageView, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, ImContext->FontMemory, nullptr);
    vkDestroyImage(RenderDevice->LogicalDevice, ImContext->FontImage, nullptr);

    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, ImContext->DescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(RenderDevice->LogicalDevice, ImContext->DescriptorPool, nullptr);

    vkDestroyPipelineLayout(RenderDevice->LogicalDevice, ImContext->PipelineLayout, nullptr);
    vkDestroyPipeline(RenderDevice->LogicalDevice, ImContext->Pipeline, nullptr);
}
