#if !defined(VULKAN_PIPELINE_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

// Helpers
VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo(VkDescriptorSetLayout *pSetLayouts, u32 SetLayoutCount = 1);
VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyInfo(VkPrimitiveTopology Topology,
                                                            VkBool32 PrimitiveRestartEnable);

VkPipelineRasterizationStateCreateInfo GetRasterizationInfo(VkPolygonMode PolygonMode, VkCullModeFlags CullMode,
                                                            VkFrontFace FrontFace,
                                                            VkPipelineRasterizationStateCreateFlags Flags = 0);
VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendInfo(u32 AttachmentCount,
                                                              const VkPipelineColorBlendAttachmentState *pAttachments);
VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilInfo(VkBool32 DepthTestEnable,
                                                                  VkBool32 DepthWriteEnable,
                                                                  VkCompareOp DepthCompareOp);
VkViewport GetViewport(f32 Width, f32 Height, f32 MinDepth, f32 MaxDepth);
VkPipelineViewportStateCreateInfo GetPipelineViewportInfo(uint32_t ViewportCount, uint32_t ScissorCount,
                                                          VkPipelineViewportStateCreateFlags Flags = 0);
VkPipelineMultisampleStateCreateInfo GetPipelineMultiSampleInfo(VkSampleCountFlagBits Samples,
                                                                VkPipelineMultisampleStateCreateFlags Flags = 0);
VkPipelineDynamicStateCreateInfo GetPipelineDynamicStateInfo(u32 DynamicStateCount, VkDynamicState *pDynamicStates,
                                                        VkPipelineDynamicStateCreateFlags Flags = 0);
VkVertexInputBindingDescription GetPipelineVertexInputBinding(u32 Binding, u32 Stride,
                                                              VkVertexInputRate InputRate);
VkVertexInputAttributeDescription GetPipelineVertexAttribInfo(u32 Binding, u32 Location, VkFormat Format,
                                                              u32 Offset);
VkPipelineShaderStageCreateInfo GetShaderStageInfo(VkShaderModule Shader, VkShaderStageFlagBits StageFlags,
                                                   const char *EntryPointName);

VkPipelineVertexInputStateCreateInfo PipelineVertexInputInfo();

VkGraphicsPipelineCreateInfo GetGraphicsPipelineInfo(VkPipelineLayout Layout, VkRenderPass RenderPass,
                                                     VkPipelineCreateFlags Flags = 0);

void CreateGraphicsPipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                            const char *FragmentShaderFile, shoora_vulkan_pipeline *pPipeline);

void CreateWireframePipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                             const char *FragmentShaderFile);

void DestroyPipelines(shoora_vulkan_device *RenderDevice, shoora_vulkan_pipeline *Pipeline);

#define VULKAN_PIPELINE_H
#endif // VULKAN_PIPELINE_H