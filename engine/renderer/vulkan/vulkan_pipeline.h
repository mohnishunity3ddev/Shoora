#if !defined(VULKAN_PIPELINE_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

void CreateGraphicsPipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                            const char *FragmentShaderFile, shoora_vulkan_pipeline *pPipeline);

void CreateWireframePipeline(shoora_vulkan_context *Context, const char *VertexShaderFile,
                             const char *FragmentShaderFile);

void DestroyPipelines(shoora_vulkan_device *RenderDevice, shoora_vulkan_pipeline *Pipeline);

#define VULKAN_PIPELINE_H
#endif // VULKAN_PIPELINE_H