#if !defined(RENDERER_FRONTEND_H)

#include "defines.h"
#include "platform/platform.h"
#if defined SHU_RENDERER_BACKEND_VULKAN
#include "renderer/vulkan/vulkan_renderer.h"
#endif

struct renderer_context
{
#if defined SHU_RENDERER_BACKEND_VULKAN
    shoora_vulkan_context VulkanContext;
#endif
};

void InitializeRenderer(renderer_context *RendererContext, shoora_app_info *AppInfo);
void DrawFrame();
void DestroyRenderer(renderer_context *RendererContext);

#define RENDERER_FRONTEND_H
#endif // RENDERER_H