#include "vulkan_pipeline.h"
#include "vulkan_shaders.h"
#include "vulkan_vertex_definitions.h"

VkPipelineLayoutCreateInfo
GetPipelineLayoutCreateInfo(VkDescriptorSetLayout *pSetLayouts, u32 SetLayoutCount)
{
    VkPipelineLayoutCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    CreateInfo.setLayoutCount = SetLayoutCount;
    CreateInfo.pSetLayouts = pSetLayouts;

    return CreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo
GetInputAssemblyInfo(VkPrimitiveTopology Topology, VkBool32 PrimitiveRestartEnable)
{
    VkPipelineInputAssemblyStateCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    CreateInfo.flags = 0;
    CreateInfo.topology = Topology;
    CreateInfo.primitiveRestartEnable = PrimitiveRestartEnable;

    return CreateInfo;
}

VkPipelineRasterizationStateCreateInfo
GetRasterizationInfo(VkPolygonMode PolygonMode, VkCullModeFlags CullMode, VkFrontFace FrontFace,
                     VkPipelineRasterizationStateCreateFlags Flags)
{
    VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
    RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationInfo.polygonMode = PolygonMode;
    RasterizationInfo.cullMode = CullMode;
    RasterizationInfo.frontFace = FrontFace;
    RasterizationInfo.flags = Flags;
    RasterizationInfo.depthClampEnable = VK_FALSE;
    RasterizationInfo.lineWidth = 1.0f;
    return RasterizationInfo;
}

VkPipelineColorBlendStateCreateInfo
GetPipelineColorBlendInfo(u32 AttachmentCount, const VkPipelineColorBlendAttachmentState *pAttachments)
{
    VkPipelineColorBlendStateCreateInfo ColorBlendStateInfo{};
    ColorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendStateInfo.attachmentCount = AttachmentCount;
    ColorBlendStateInfo.pAttachments = pAttachments;
    return ColorBlendStateInfo;
}

VkPipelineDepthStencilStateCreateInfo
GetPipelineDepthStencilInfo(VkBool32 DepthTestEnable, VkBool32 DepthWriteEnable, VkCompareOp DepthCompareOp)
{
    VkPipelineDepthStencilStateCreateInfo DepthStencilStateInfo = {};
    DepthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilStateInfo.depthTestEnable = DepthTestEnable;
    DepthStencilStateInfo.depthWriteEnable = DepthWriteEnable;
    DepthStencilStateInfo.depthCompareOp = DepthCompareOp;
    DepthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    return DepthStencilStateInfo;
}

VkPipelineViewportStateCreateInfo
GetPipelineViewportInfo(uint32_t ViewportCount, uint32_t ScissorCount,
                        VkPipelineViewportStateCreateFlags Flags)
{
    VkPipelineViewportStateCreateInfo PipelineViewportCreateInfo = {};
    PipelineViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    PipelineViewportCreateInfo.viewportCount = ViewportCount;
    PipelineViewportCreateInfo.scissorCount = ScissorCount;
    PipelineViewportCreateInfo.flags = Flags;
    return PipelineViewportCreateInfo;
}

VkViewport
GetViewport(f32 Width, f32 Height, f32 MinDepth, f32 MaxDepth)
{
    VkViewport Viewport = {};
    Viewport.width = Width;
    Viewport.height = Height;
    Viewport.minDepth = MinDepth;
    Viewport.maxDepth = MaxDepth;

    return Viewport;
}

VkPipelineMultisampleStateCreateInfo
GetPipelineMultiSampleInfo(VkSampleCountFlagBits Samples, VkPipelineMultisampleStateCreateFlags Flags)
{
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleInfo = {};
    PipelineMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    PipelineMultisampleInfo.rasterizationSamples = Samples;
    PipelineMultisampleInfo.flags = Flags;
    return PipelineMultisampleInfo;
}

VkPipelineDynamicStateCreateInfo
GetPipelineDynamicStateInfo(u32 DynamicStateCount, VkDynamicState *pDynamicStates,
                       VkPipelineDynamicStateCreateFlags Flags)
{
    VkPipelineDynamicStateCreateInfo PipelineDynamicStateInfo = {};
    PipelineDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    PipelineDynamicStateInfo.pDynamicStates = pDynamicStates;
    PipelineDynamicStateInfo.dynamicStateCount = DynamicStateCount;
    PipelineDynamicStateInfo.flags = Flags;
    return PipelineDynamicStateInfo;
}

VkVertexInputBindingDescription
GetPipelineVertexInputBinding(u32 Binding, u32 Stride, VkVertexInputRate InputRate)
{
    VkVertexInputBindingDescription VertexInputBindDescription = {};
    VertexInputBindDescription.binding = Binding;
    VertexInputBindDescription.stride = Stride;
    VertexInputBindDescription.inputRate = InputRate;

    return VertexInputBindDescription;
}

VkVertexInputAttributeDescription
GetPipelineVertexAttribInfo(u32 Binding, u32 Location, VkFormat Format, u32 Offset)
{
    VkVertexInputAttributeDescription VertexInputAttribDescription = {};
    VertexInputAttribDescription.location = Location;
    VertexInputAttribDescription.binding = Binding;
    VertexInputAttribDescription.format = Format;
    VertexInputAttribDescription.offset = Offset;
    return VertexInputAttribDescription;
}

VkPipelineVertexInputStateCreateInfo
PipelineVertexInputInfo()
{
    VkPipelineVertexInputStateCreateInfo VertexInputStateInfo{};
    VertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    return VertexInputStateInfo;
}

VkGraphicsPipelineCreateInfo
GetGraphicsPipelineInfo(VkPipelineLayout Layout, VkRenderPass RenderPass, VkPipelineCreateFlags Flags)
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = Layout;
    pipelineCreateInfo.renderPass = RenderPass;
    pipelineCreateInfo.flags = Flags;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    return pipelineCreateInfo;
}





void
DestroyPipelineLayout(shoora_vulkan_device *RenderDevice, VkPipelineLayout PipelineLayout)
{
    vkDestroyPipelineLayout(RenderDevice->LogicalDevice, PipelineLayout, nullptr);
    LogOutput(LogType_Warn, "Pipeline Layout Destroyed!\n");
}

void
DestroyPipelines(shoora_vulkan_device *RenderDevice, shoora_vulkan_pipeline *Pipeline)
{
    DestroyPipelineLayout(RenderDevice, Pipeline->GraphicsPipelineLayout);
    vkDestroyPipeline(RenderDevice->LogicalDevice, Pipeline->GraphicsPipeline, nullptr);
    DestroyPipelineLayout(RenderDevice, Pipeline->WireframePipelineLayout);
    vkDestroyPipeline(RenderDevice->LogicalDevice, Pipeline->WireframeGraphicsPipeline, nullptr);
    LogOutput(LogType_Warn, "Graphics Pipeline Destroyed!\n");
}

void
DestroyShaderModule(shoora_vulkan_device *RenderDevice, VkShaderModule ShaderModule)
{
    vkDestroyShaderModule(RenderDevice->LogicalDevice, ShaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo
GetShaderStageInfo(VkShaderModule Shader, VkShaderStageFlagBits StageFlags, const char *EntryPointName)
{
    VkPipelineShaderStageCreateInfo StageInfo;
    StageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    StageInfo.pNext = nullptr;
    StageInfo.flags = 0;
    StageInfo.stage = StageFlags;
    StageInfo.module = Shader;
    StageInfo.pName = EntryPointName;
    StageInfo.pSpecializationInfo = nullptr;

    return StageInfo;
}

void
CreateGraphicsPipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                       const char *FragmentShaderFile, shoora_vulkan_pipeline *pPipeline)
{
    ASSERT(pPipeline != nullptr);

    shoora_vulkan_device *RenderDevice = &Context->Device;

    //? Shader Stages
    auto VertexShader = CreateShaderModule(RenderDevice, VertexShaderFile);
    auto FragmentShader = CreateShaderModule(RenderDevice, FragmentShaderFile);
    const char *ShaderCodeEntryPoint = "main";
    auto VertexShaderStageInfo = GetShaderStageInfo(VertexShader, VK_SHADER_STAGE_VERTEX_BIT,
                                                    ShaderCodeEntryPoint);
    auto FragmentShaderStageInfo = GetShaderStageInfo(FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      ShaderCodeEntryPoint);
    VkPipelineShaderStageCreateInfo ShaderStageInfos[] = {VertexShaderStageInfo, FragmentShaderStageInfo};

    //? Vertex Input
    auto VertexBindingDescription = GetVertexBindingDescription();
    VkVertexInputAttributeDescription AttributeDescriptions[16];
    u32 AttributeCount;
    GetVertexAttributeDescriptions(AttributeDescriptions, &AttributeCount);
    VkPipelineVertexInputStateCreateInfo VertexInputInfo;
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInfo.pNext = nullptr;
    VertexInputInfo.flags = 0;
    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.pVertexBindingDescriptions = &VertexBindingDescription;
    VertexInputInfo.vertexAttributeDescriptionCount = AttributeCount;
    VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions;

    //? Input Assembly
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
    InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyInfo.pNext = nullptr;
    InputAssemblyInfo.flags = 0;
    InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    //? Viewport Info
    u32 Width = Context->Swapchain.ImageDimensions.width;
    u32 Height = Context->Swapchain.ImageDimensions.height;
    VkViewport Viewport = {};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = (f32)Width;
    Viewport.height = (f32)Height;
    VkRect2D Scissor = {.offset = {0, 0}, .extent = {.width = Width, .height = Height}};

    VkPipelineViewportStateCreateInfo ViewportInfo;
    ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportInfo.pNext = nullptr;
    ViewportInfo.flags = 0;
    ViewportInfo.viewportCount = 1;
    ViewportInfo.pViewports = &Viewport;
    ViewportInfo.scissorCount = 1;
    ViewportInfo.pScissors = &Scissor;

    //? Rasterizer Info
    VkPipelineRasterizationStateCreateInfo RasterizerInfo;
    RasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerInfo.pNext = nullptr;
    RasterizerInfo.flags = 0;
    RasterizerInfo.depthClampEnable = VK_FALSE;
    RasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterizerInfo.depthBiasEnable = VK_FALSE;
    RasterizerInfo.depthBiasConstantFactor = 0.0f;
    RasterizerInfo.depthBiasClamp = 0.0f;
    RasterizerInfo.depthBiasSlopeFactor = 0.0f;
    RasterizerInfo.lineWidth = 1.0f;

    //? Multisample Info
    VkPipelineMultisampleStateCreateInfo MultisampleInfo;
    MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleInfo.pNext = nullptr;
    MultisampleInfo.flags = 0;
    MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleInfo.sampleShadingEnable = VK_FALSE;
    MultisampleInfo.minSampleShading = 1.0f;
    MultisampleInfo.pSampleMask = nullptr;
    MultisampleInfo.alphaToCoverageEnable = VK_FALSE;
    MultisampleInfo.alphaToOneEnable = VK_FALSE;

    //? Depth Stencil Info
    VkPipelineDepthStencilStateCreateInfo DepthStencilInfo = {};
    DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilInfo.pNext = nullptr;
    DepthStencilInfo.flags = 0;
    DepthStencilInfo.depthTestEnable = VK_TRUE;
    DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    DepthStencilInfo.depthWriteEnable = VK_TRUE;

    // DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    // DepthStencilInfo.stencilTestEnable = VK_FALSE;
    // DepthStencilInfo.front = 0;
    // DepthStencilInfo.back = 0;
    DepthStencilInfo.minDepthBounds = 0.0f;
    DepthStencilInfo.maxDepthBounds = 1.0f;

    //? Color Blend Info
    VkPipelineColorBlendAttachmentState BlendAttachment;
    BlendAttachment.blendEnable = VK_TRUE;
    // NOTE: Src is the color calculated by the fragment shader. Dst is the color already there in the framebuffer
    // attachment.
    BlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    BlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    BlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    BlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    BlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    BlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
    ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendInfo.pNext = nullptr;
    ColorBlendInfo.flags = 0;
    ColorBlendInfo.logicOpEnable = VK_FALSE;
    ColorBlendInfo.logicOp = VK_LOGIC_OP_CLEAR;
    ColorBlendInfo.attachmentCount = 1;
    ColorBlendInfo.pAttachments = &BlendAttachment;
    ColorBlendInfo.blendConstants[0] = 0.0f;
    ColorBlendInfo.blendConstants[1] = 0.0f;
    ColorBlendInfo.blendConstants[2] = 0.0f;
    ColorBlendInfo.blendConstants[3] = 0.0f;

    //? Pipeline Layout
    ASSERT((pPipeline->GraphicsPipelineLayout != VK_NULL_HANDLE));

    //? Dynamic State
    VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};
    VkPipelineDynamicStateCreateInfo DynamicsInfo = {};
    DynamicsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicsInfo.dynamicStateCount = ARRAY_SIZE(DynamicStates);
    DynamicsInfo.pDynamicStates = DynamicStates;

    VkGraphicsPipelineCreateInfo PipelineCreateInfo = {};
    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.stageCount = ARRAY_SIZE(ShaderStageInfos);
    PipelineCreateInfo.pStages = ShaderStageInfos;
    PipelineCreateInfo.pVertexInputState = &VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyInfo;
    PipelineCreateInfo.pViewportState = &ViewportInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizerInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendInfo;
    PipelineCreateInfo.pDynamicState = &DynamicsInfo;
    PipelineCreateInfo.layout = pPipeline->GraphicsPipelineLayout;
    PipelineCreateInfo.renderPass = Context->GraphicsRenderPass;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineCreateInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(RenderDevice->LogicalDevice, VK_NULL_HANDLE, 1, &PipelineCreateInfo,
                                       nullptr, &pPipeline->GraphicsPipeline));

    LogOutput(LogType_Debug, "Created Graphics Pipeline!\n");

    vkDestroyShaderModule(RenderDevice->LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(RenderDevice->LogicalDevice, FragmentShader, nullptr);
}

#if 1
void
CreateWireframePipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                        const char *FragmentShaderFile)
{
    shoora_vulkan_device *RenderDevice = &Context->Device;

    //? Shader Stages
    auto VertexShader = CreateShaderModule(RenderDevice, VertexShaderFile);
    auto FragmentShader = CreateShaderModule(RenderDevice, FragmentShaderFile);
    const char *ShaderCodeEntryPoint = "main";
    auto VertexShaderStageInfo = GetShaderStageInfo(VertexShader, VK_SHADER_STAGE_VERTEX_BIT,
                                                    ShaderCodeEntryPoint);
    auto FragmentShaderStageInfo = GetShaderStageInfo(FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      ShaderCodeEntryPoint);
    VkPipelineShaderStageCreateInfo ShaderStageInfos[] = {VertexShaderStageInfo, FragmentShaderStageInfo};

    //? Vertex Input
    auto VertexBindingDescription = GetVertexBindingDescription();
    VkVertexInputAttributeDescription AttributeDescriptions[16];
    u32 AttributeCount;
    GetVertexAttributeDescriptions(AttributeDescriptions, &AttributeCount);
    VkPipelineVertexInputStateCreateInfo VertexInputInfo;
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInfo.pNext = nullptr;
    VertexInputInfo.flags = 0;
    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.pVertexBindingDescriptions = &VertexBindingDescription;
    VertexInputInfo.vertexAttributeDescriptionCount = AttributeCount;
    VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions;

    //? Input Assembly
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
    InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyInfo.pNext = nullptr;
    InputAssemblyInfo.flags = 0;
    InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    //? Viewport Info
    u32 Width = Context->Swapchain.ImageDimensions.width;
    u32 Height = Context->Swapchain.ImageDimensions.height;
    VkViewport Viewport = {};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = (f32)Width;
    Viewport.height = (f32)Height;
    VkRect2D Scissor = {.offset = {0, 0}, .extent = {.width = Width, .height = Height}};

    VkPipelineViewportStateCreateInfo ViewportInfo;
    ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportInfo.pNext = nullptr;
    ViewportInfo.flags = 0;
    ViewportInfo.viewportCount = 1;
    ViewportInfo.pViewports = &Viewport;
    ViewportInfo.scissorCount = 1;
    ViewportInfo.pScissors = &Scissor;

    //? Rasterizer Info
    VkPipelineRasterizationStateCreateInfo RasterizerInfo;
    RasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerInfo.pNext = nullptr;
    RasterizerInfo.flags = 0;
    RasterizerInfo.depthClampEnable = VK_FALSE;
    RasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizerInfo.polygonMode = VK_POLYGON_MODE_LINE;
    RasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterizerInfo.depthBiasEnable = VK_FALSE;
    RasterizerInfo.depthBiasConstantFactor = 0.0f;
    RasterizerInfo.depthBiasClamp = 0.0f;
    RasterizerInfo.depthBiasSlopeFactor = 0.0f;
    RasterizerInfo.lineWidth = 5.0f;

    //? Multisample Info
    VkPipelineMultisampleStateCreateInfo MultisampleInfo;
    MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleInfo.pNext = nullptr;
    MultisampleInfo.flags = 0;
    MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleInfo.sampleShadingEnable = VK_FALSE;
    MultisampleInfo.minSampleShading = 1.0f;
    MultisampleInfo.pSampleMask = nullptr;
    MultisampleInfo.alphaToCoverageEnable = VK_FALSE;
    MultisampleInfo.alphaToOneEnable = VK_FALSE;

    //? Depth Stencil Info
    VkPipelineDepthStencilStateCreateInfo DepthStencilInfo = {};
    DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilInfo.pNext = nullptr;
    DepthStencilInfo.flags = 0;
    DepthStencilInfo.depthTestEnable = VK_TRUE;
    DepthStencilInfo.depthWriteEnable = VK_TRUE;
    DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    DepthStencilInfo.stencilTestEnable = VK_FALSE;
    DepthStencilInfo.minDepthBounds = 0.0f;
    DepthStencilInfo.maxDepthBounds = 1.0f;

    //? Color Blend Info
    VkPipelineColorBlendAttachmentState BlendAttachment;
    BlendAttachment.blendEnable = VK_FALSE;
    // NOTE: Src is the color calculated by the fragment shader. Dst is the color already there in the framebuffer
    // attachment.
    BlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    BlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    BlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    BlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    BlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    BlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    BlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
    ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendInfo.pNext = nullptr;
    ColorBlendInfo.flags = 0;
    ColorBlendInfo.logicOpEnable = VK_FALSE;
    ColorBlendInfo.logicOp = VK_LOGIC_OP_CLEAR;
    ColorBlendInfo.attachmentCount = 1;
    ColorBlendInfo.pAttachments = &BlendAttachment;
    ColorBlendInfo.blendConstants[0] = 0.0f;
    ColorBlendInfo.blendConstants[1] = 0.0f;
    ColorBlendInfo.blendConstants[2] = 0.0f;
    ColorBlendInfo.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VK_CHECK(vkCreatePipelineLayout(RenderDevice->LogicalDevice, &PipelineLayoutInfo, nullptr,
                                    &Context->Pipeline.WireframePipelineLayout));

    //? Dynamic State
    VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo DynamicsInfo = {};
    DynamicsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicsInfo.dynamicStateCount = ARRAY_SIZE(DynamicStates);
    DynamicsInfo.pDynamicStates = DynamicStates;

    VkGraphicsPipelineCreateInfo WireframePipelineCreateInfos[1] = {};
    WireframePipelineCreateInfos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    WireframePipelineCreateInfos[0].stageCount = ARRAY_SIZE(ShaderStageInfos);
    WireframePipelineCreateInfos[0].pStages = ShaderStageInfos;
    WireframePipelineCreateInfos[0].pVertexInputState = &VertexInputInfo;
    WireframePipelineCreateInfos[0].pInputAssemblyState = &InputAssemblyInfo;
    WireframePipelineCreateInfos[0].pViewportState = &ViewportInfo;
    WireframePipelineCreateInfos[0].pRasterizationState = &RasterizerInfo;
    WireframePipelineCreateInfos[0].pMultisampleState = &MultisampleInfo;
    WireframePipelineCreateInfos[0].pDepthStencilState = &DepthStencilInfo;
    WireframePipelineCreateInfos[0].pColorBlendState = &ColorBlendInfo;
    WireframePipelineCreateInfos[0].pDynamicState = &DynamicsInfo;
    WireframePipelineCreateInfos[0].layout = Context->Pipeline.WireframePipelineLayout;
    WireframePipelineCreateInfos[0].renderPass = Context->GraphicsRenderPass;
    WireframePipelineCreateInfos[0].subpass = 0;
    WireframePipelineCreateInfos[0].basePipelineHandle = VK_NULL_HANDLE;
    WireframePipelineCreateInfos[0].basePipelineIndex = -1;

    VkPipeline Pipelines[1];

    VK_CHECK(vkCreateGraphicsPipelines(RenderDevice->LogicalDevice, VK_NULL_HANDLE,
                                       ARRAY_SIZE(WireframePipelineCreateInfos), WireframePipelineCreateInfos,
                                       nullptr, Pipelines));

    Context->Pipeline.WireframeGraphicsPipeline = Pipelines[0];

    LogOutput(LogType_Debug, "Created Wireframe Pipeline!\n");

    vkDestroyShaderModule(RenderDevice->LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(RenderDevice->LogicalDevice, FragmentShader, nullptr);
}
#endif

#if 0
void
CreateShaderModule(shoora_vulkan_device *RenderDevice, const char *ShaderFile, VkShaderModule *Module)
{
    // Load the Spirv shader.
    // TODO)): Load the file, get its contents and size using ShaderFile.
    u8 SrcCode[2048];
    u64 SrcSize = 512;

    VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
    ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ShaderModuleCreateInfo.pNext = nullptr;
    ShaderModuleCreateInfo.flags = 0;
    ShaderModuleCreateInfo.codeSize = SrcSize;
    ShaderModuleCreateInfo.pCode = (u32 *)SrcCode;

    VK_CHECK(vkCreateShaderModule(RenderDevice->LogicalDevice, &ShaderModuleCreateInfo, nullptr, Module));
    ASSERT(*Module != VK_NULL_HANDLE);
}

void
SpecifyShaderStages(shoora_vulkan_device *RenderDevice)
{
    VkPipelineShaderStageCreateInfo StageCreateInfos[32];

    VkShaderModule VertexShader;
    CreateShaderModule(RenderDevice, "vertex_shader.vert", &VertexShader);
    VkPipelineShaderStageCreateInfo *VertexShaderCreateInfo = &StageCreateInfos[0];
    VertexShaderCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexShaderCreateInfo->pNext = nullptr;
    VertexShaderCreateInfo->flags = 0;
    VertexShaderCreateInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexShaderCreateInfo->module = VertexShader;
    VertexShaderCreateInfo->pName = "main";
    VertexShaderCreateInfo->pSpecializationInfo = nullptr;

    VkShaderModule FragmentShader;
    CreateShaderModule(RenderDevice, "fragment_shader.frag", &FragmentShader);
    VkPipelineShaderStageCreateInfo *FragmentShaderCreateInfo = &StageCreateInfos[1];
    FragmentShaderCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentShaderCreateInfo->pNext = nullptr;
    FragmentShaderCreateInfo->flags = 0;
    FragmentShaderCreateInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentShaderCreateInfo->module = FragmentShader;
    FragmentShaderCreateInfo->pName = "main";
    FragmentShaderCreateInfo->pSpecializationInfo = nullptr;
}

struct Vertex
{
    f32 x, y, z;
    f32 nx, ny, nz;
    f32 tx, ty;
};

void
SpecifyVertexAttributes(shoora_vulkan_device *RenderDevice)
{
    VkVertexInputBindingDescription BindingDescriptions[32];
    BindingDescriptions[0].binding = 0;
    BindingDescriptions[0].stride = 8*sizeof(f32);
    BindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription Attributes[32];
    VkVertexInputAttributeDescription *PositionAttribute = &Attributes[0];
    PositionAttribute->binding = 0;
    PositionAttribute->location = 0;
    PositionAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    PositionAttribute->offset = 0;
    VkVertexInputAttributeDescription *NormalAttribute = &Attributes[1];
    NormalAttribute->binding = 0;
    NormalAttribute->location = 1;
    NormalAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    NormalAttribute->offset = 3*sizeof(f32);
    VkVertexInputAttributeDescription *TexCoordAttribute = &Attributes[2];
    NormalAttribute->binding = 0;
    NormalAttribute->location = 2;
    NormalAttribute->format = VK_FORMAT_R32G32_SFLOAT;
    NormalAttribute->offset = 6*sizeof(f32);

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo;
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputStateCreateInfo.pNext = nullptr;
    VertexInputStateCreateInfo.flags = 0;
    VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    VertexInputStateCreateInfo.pVertexBindingDescriptions = BindingDescriptions;
    VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 3;
    VertexInputStateCreateInfo.pVertexAttributeDescriptions = Attributes;
}

// what kind of polygons are drawn from the supplied vertices.
// primitives wth adjacency require geomeetry shader feature
void
SpecifyPipelineInputAssemblyState()
{
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo;
    InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyCreateInfo.pNext = nullptr;
    InputAssemblyCreateInfo.flags = 0;
    InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
}

// In case we are using tessellation
// Also needs tessellationShader feature enabled in the physical device.
void
SpecifyPipelineTessellationState()
{
    VkPipelineTessellationStateCreateInfo TessCreateInfo;
    TessCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    TessCreateInfo.pNext = nullptr;
    TessCreateInfo.flags = 0;
    // Number of vertices that form a patch
    TessCreateInfo.patchControlPoints = 3;
}

// If the rendering is to be performed in more than one viewport, we need a new feature called multiViewport.
void
SpecifyPipelineViewportAndScissorTestState()
{
    u32 Width = 1920;
    u32 Height = 1080;

    VkViewport Viewport;
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = Width;
    Viewport.height = Height;
    Viewport.minDepth = 0;
    Viewport.maxDepth = 1;

    VkRect2D Scissor;
    Scissor.extent = {Width, Height};
    Scissor.offset = {0, 0};

    VkPipelineViewportStateCreateInfo ViewportCreateInfo;
    ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportCreateInfo.pNext = nullptr;
    ViewportCreateInfo.flags = 0;
    ViewportCreateInfo.viewportCount = 1;
    ViewportCreateInfo.pViewports = &Viewport;
    ViewportCreateInfo.scissorCount = 1;
    ViewportCreateInfo.pScissors = &Scissor;
}

void
SpecifyPipelineRasterizationState()
{
    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo;
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.pNext = nullptr;
    RasterizerCreateInfo.flags = 0;
    // if true, requires a depthclampenable feature in the device.
    RasterizerCreateInfo.depthClampEnable = VK_TRUE;
    // true if we want to disable rasterization
    RasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    // MODE_LINE and _POINTS requires a fillModeNonSolid Feature
    RasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    // Back face culling
    RasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE;
    // vertices specified in clockwise fashion are the font faces.
    RasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // if depth values need to be offset by a value
    RasterizerCreateInfo.depthBiasEnable = VK_TRUE;
    // what values that is which depth values are offsetted by.
    RasterizerCreateInfo.depthBiasConstantFactor = 0.35f;
    // The max or min value of the bias which will offset the depth value
    RasterizerCreateInfo.depthBiasClamp = 0.05f;
    RasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;
    // Width of the lines rendered. If the width > 1.0 is specified, wideLines feature is required.
    RasterizerCreateInfo.lineWidth = 1.0f;
}

void
SpecifyPipelineMultisampleRate()
{
    VkPipelineMultisampleStateCreateInfo MultiSampleCreateInfo;
    MultiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultiSampleCreateInfo.pNext = nullptr;
    MultiSampleCreateInfo.flags = 0;
    MultiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    // Requires a sampleSahding feature.
    MultiSampleCreateInfo.sampleShadingEnable = VK_FALSE;
    MultiSampleCreateInfo.minSampleShading = 1.0f;
    MultiSampleCreateInfo.pSampleMask = nullptr;
    MultiSampleCreateInfo.alphaToCoverageEnable = VK_TRUE;
    // Turns the alpha to 1.0f. Requires feature
    MultiSampleCreateInfo.alphaToOneEnable = VK_FALSE;
}

void
SpecifyPipelineDepthAndStencilState()
{
    VkStencilOpState StencilState = {};

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo;
    DepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilCreateInfo.pNext = nullptr;
    DepthStencilCreateInfo.flags = 0;
    DepthStencilCreateInfo.depthTestEnable = VK_TRUE;
    DepthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    DepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    // requires depthBounds feature
    DepthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    DepthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    DepthStencilCreateInfo.front = StencilState;
    DepthStencilCreateInfo.back = StencilState;
    DepthStencilCreateInfo.minDepthBounds = 0.0f;
    DepthStencilCreateInfo.maxDepthBounds = 1.0f;
}

void
SpecifyPipelineBlendState()
{
    VkPipelineColorBlendAttachmentState BlendState;
    // Feature independentBlend for having differnet values here.
    BlendState.blendEnable = VK_TRUE;
    BlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    BlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendState.colorBlendOp = VK_BLEND_OP_ADD;
    BlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    BlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendState.alphaBlendOp = VK_BLEND_OP_ADD;
    BlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo BlendStateCreateInfo;
    BlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    BlendStateCreateInfo.pNext = nullptr;
    BlendStateCreateInfo.flags = 0;
    BlendStateCreateInfo.logicOpEnable = VK_FALSE;
    BlendStateCreateInfo.logicOp = VK_LOGIC_OP_AND;
    BlendStateCreateInfo.attachmentCount = 1;
    BlendStateCreateInfo.pAttachments = &BlendState;
    BlendStateCreateInfo.blendConstants[0] = 1.0f;
    BlendStateCreateInfo.blendConstants[1] = 1.0f;
    BlendStateCreateInfo.blendConstants[2] = 1.0f;
    BlendStateCreateInfo.blendConstants[3] = 1.0f;
}


void
SpecifyDynamicStates()
{
    VkDynamicState DynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
        VK_DYNAMIC_STATE_DEPTH_BIAS,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo;
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.pNext = nullptr;
    DynamicStateCreateInfo.flags = 0;
    DynamicStateCreateInfo.dynamicStateCount = ARRAY_SIZE(DynamicStates);
    DynamicStateCreateInfo.pDynamicStates = DynamicStates;
}

// what kind of resources can be accessed by the pipeline.
void
CreatePipelineLayout(shoora_vulkan_device *RenderDevice)
{
    VkDescriptorSetLayout DescriptorSetLayouts[4];

    VkPushConstantRange PushConstantRanges[4];
    PushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    PushConstantRanges[0].offset = 0;
    PushConstantRanges[0].size = 8*sizeof(f32);

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo;
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.pNext = nullptr;
    PipelineLayoutCreateInfo.flags = 0;
    PipelineLayoutCreateInfo.setLayoutCount = 4;
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayouts;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges = PushConstantRanges;

    VkPipelineLayout PipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(RenderDevice->LogicalDevice, &PipelineLayoutCreateInfo, nullptr,
                                    &PipelineLayout));
}

void
SpecifyGraphicsPipelineCreationParameters()
{
    // Optimized, AllowDerivatives(Other pipeline can derive from this one), Derivative(This pipeline derives from
    // another one) etc.
    VkPipelineCreateFlags PipelineCreateFlags;

    // TODO)): Create Shader Stages. Vertex shader/fragment shader.
    VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[4];

    // TODO)): Vertex Attributes, Bindings etc.
    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfos;

    // TODO)): How Vertices are assembled into polygons.
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo;

    // TODO)): If Tessellation is enabled.
    VkPipelineTessellationStateCreateInfo TessCreateInfo;

    // TODO)): Viewport and Scissor tests
    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo;

    // TODO)): Properties of Rasterization
    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo;

    // TODO)): If Multisampling is enabled.
    VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo;

    // TODO)): Depth Stencil tests if enabled.
    VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo;

    // TODO)): Color Blending
    VkPipelineColorBlendStateCreateInfo BlendCreateInfo;

    // TODO)): Dynamic States
    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo;

    VkRenderPass RenderPass;
    u32 Subpass;
    VkPipelineLayout PipelineLayout;

    VkPipeline GraphicsPipeline;

    VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo;
    GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    GraphicsPipelineCreateInfo.pNext = nullptr;
    GraphicsPipelineCreateInfo.flags = PipelineCreateFlags;
    GraphicsPipelineCreateInfo.stageCount = ARRAY_SIZE(ShaderStageCreateInfos);
    GraphicsPipelineCreateInfo.pStages = ShaderStageCreateInfos;
    GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfos;
    GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
    GraphicsPipelineCreateInfo.pTessellationState = &TessCreateInfo;
    GraphicsPipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
    GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
    GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
    GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
    GraphicsPipelineCreateInfo.pColorBlendState = &BlendCreateInfo;
    GraphicsPipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    GraphicsPipelineCreateInfo.layout = PipelineLayout;
    GraphicsPipelineCreateInfo.renderPass = RenderPass;
    GraphicsPipelineCreateInfo.subpass = Subpass;
    GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    GraphicsPipelineCreateInfo.basePipelineIndex = -1;
}

void
CreatePipelineCacheObject(shoora_vulkan_device *RenderDevice)
{
    u8 CacheData[32];

    VkPipelineCacheCreateInfo CacheCreateInfo;
    CacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    CacheCreateInfo.pNext = nullptr;
    CacheCreateInfo.flags = 0;
    CacheCreateInfo.initialDataSize = ARRAY_SIZE(CacheData);
    CacheCreateInfo.pInitialData = CacheData;

    VkPipelineCache Cache;
    vkCreatePipelineCache(RenderDevice->LogicalDevice, &CacheCreateInfo, nullptr, &Cache);
}


#include <memory>

void
RetrieveDataFromPipelineCache(shoora_vulkan_device *RenderDevice, VkPipelineCache PipelineCache, u8 *CacheData)
{
    u64 DataSize;
    vkGetPipelineCacheData(RenderDevice->LogicalDevice, PipelineCache, &DataSize, nullptr);
    CacheData = (u8 *)malloc(DataSize);
    vkGetPipelineCacheData(RenderDevice->LogicalDevice, PipelineCache, &DataSize, (void *)PipelineCache);
}

void
MergeMultiplePipelineCacheObjects(shoora_vulkan_device *RenderDevice, VkPipelineCache *TargetPipelineCache)
{
    VkPipelineCache CachesToBeMerged[32];

    vkMergePipelineCaches(RenderDevice->LogicalDevice, *TargetPipelineCache, ARRAY_SIZE(CachesToBeMerged),
                          CachesToBeMerged);
}

void
CreateGraphicsPipelineExample(shoora_vulkan_device *RenderDevice)
{
    VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo;
    VkPipelineCache Cache;

    VkPipeline Pipeline;
    vkCreateGraphicsPipelines(RenderDevice->LogicalDevice, Cache, 1, &GraphicsPipelineCreateInfo, nullptr,
                              &Pipeline);

}


// COmpute Pipelines cannot be used inside a render pass.
void
CreateComputePipeline(shoora_vulkan_device *RenderDevice)
{
    VkPipelineShaderStageCreateInfo ComputeShaderStageCreateInfo;
    VkPipelineLayout ComputePipelineLayout;

    VkComputePipelineCreateInfo ComputeCreateInfo;
    ComputeCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    ComputeCreateInfo.pNext = nullptr;
    ComputeCreateInfo.flags = 0;
    ComputeCreateInfo.stage = ComputeShaderStageCreateInfo;
    ComputeCreateInfo.layout = ComputePipelineLayout;
    ComputeCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    ComputeCreateInfo.basePipelineIndex = -1;

    VkPipeline ComputePipeline;
    vkCreateComputePipelines(RenderDevice->LogicalDevice, VK_NULL_HANDLE, 1, &ComputeCreateInfo, nullptr,
                             &ComputePipeline);
}

void
BindPipelineObject(VkCommandBuffer CmdBuffer, VkPipeline Pipeline)
{
    // TODO)): If this is a Graphics Pipeline, make sure a renderpass was started.
    // TODO)): If this is a Compute Pipeline, make sure renderpass was NOT started.
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
}

void
DestroyPipelineCache(shoora_vulkan_device *RenderDevice, VkPipelineCache PipelineCache)
{
    vkDestroyPipelineCache(RenderDevice->LogicalDevice, PipelineCache, nullptr);
}
#endif
