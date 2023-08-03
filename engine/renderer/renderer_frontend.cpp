#include "renderer/renderer_frontend.h"
#if defined(SHU_RENDERER_BACKEND_VULKAN)
#include "renderer/vulkan/_vulkan_renderer.h"
#endif


void
InitializeRenderer(renderer_context *RendererContext, const char *AppName)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    InitializeVulkanRenderer(&RendererContext->VulkanContext, AppName);
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
