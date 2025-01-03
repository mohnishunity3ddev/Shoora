#include "renderer/renderer_frontend.h"

#if defined(SHU_RENDERER_BACKEND_VULKAN)
#include "renderer/vulkan/vulkan_renderer.h"
static shoora_vulkan_context vkContext;
#endif

#if SHU_VULKAN_EXAMPLE
#include "vulkan/__example.h"
#endif

void
InitializeRenderer(renderer_context *RendererContext, shoora_app_info *AppInfo)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    RendererContext->Context = &vkContext;
    InitializeVulkanRenderer(RendererContext->Context, AppInfo);
#elif SHU_VULKAN_EXAMPLE
    ExampleMain();
#else
#error non-vulkan renderers are not supported at the moment!
#endif
}

void
DrawFrame(shoora_platform_frame_packet *FramePacket)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    DrawFrameInVulkan(FramePacket);
#else
#endif
}

void
DestroyRenderer(renderer_context *RendererContext)
{
#if defined(SHU_RENDERER_BACKEND_VULKAN)
    DestroyVulkanRenderer(RendererContext->Context);
    RendererContext->Context = nullptr;
#elif SHU_VULKAN_EXAMPLE
#else
#error non-vulkan renderers are not supported at the moment!
#endif
}
