#if !defined(RENDERER_FRONTEND_H)

#include "defines.h"
#if defined SHU_RENDERER_BACKEND_VULKAN
#include "renderer/vulkan/_vulkan_renderer.h"
#endif

struct renderer_context
{
#if defined SHU_RENDERER_BACKEND_VULKAN
    vulkan_context VulkanContext;
#endif
};

void InitializeRenderer(renderer_context *RendererContext, const char *AppName);
void DestroyRenderer(renderer_context *RendererContext);

#define RENDERER_FRONTEND_H
#endif // RENDERER_H