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
VkPipelineShaderStageCreateInfo GetShaderStageInfo(shoora_vulkan_device *RenderDevice, const char *ShaderFile,
                                                   VkShaderStageFlagBits StageFlags, const char *EntryPointName);

VkPipelineVertexInputStateCreateInfo PipelineVertexInputInfo();
VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputInfo(u32 VertexBindingCount,
                                                                VkVertexInputBindingDescription *VertexBindingDesc,
                                                                u32 AttribCount,
                                                                VkVertexInputAttributeDescription *Attribs);

VkGraphicsPipelineCreateInfo GetPipelineCreateInfo(VkPipelineLayout Layout, VkRenderPass RenderPass,
                                                     VkPipelineCreateFlags Flags = 0);

VkSpecializationMapEntry GetSpecializationMapEntry(u32 ConstantID, u32 Offset, size_t Size);
VkSpecializationInfo GetSpecializationInfo(u32 MapEntryCount, VkSpecializationMapEntry *MapEntries,
                                           size_t DataSize, const void *Data);

void CreateGraphicsPipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                            const char *FragmentShaderFile, shoora_vulkan_graphics_pipeline *pPipeline,
                            VkRenderPass RenderPass = VK_NULL_HANDLE, b32 EnableMultisampling = false);

void CreateWireframePipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                             const char *FragmentShaderFile);

void DestroyPipeline(shoora_vulkan_device *RenderDevice, shoora_vulkan_graphics_pipeline *Pipeline);

#define VULKAN_PIPELINE_H
#endif // VULKAN_PIPELINE_H