#include "renderer/renderer_frontend.h"
#if defined(SHU_RENDERER_BACKEND_VULKAN)
#include "renderer/vulkan/vulkan_renderer.h"
#endif


void
InitializeRenderer(renderer_context *RendererContext, shoora_app_info *AppInfo)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    InitializeVulkanRenderer(&RendererContext->VulkanContext, AppInfo);
#else
#error non-vulkan renderers are not supported at the moment!
#endif
}

void
DestroyRenderer(renderer_context *RendererContext)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    DestroyVulkanRenderer(&RendererContext->VulkanContext);
#else
#error non-vulkan renderers are not supported at the moment!
#endif
}
