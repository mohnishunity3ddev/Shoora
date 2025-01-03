#if !defined(RENDERER_FRONTEND_H)

#include "defines.h"
#include "platform/platform.h"

struct renderer_context
{
#if defined SHU_RENDERER_BACKEND_VULKAN
    struct shoora_vulkan_context *Context;
#endif
};

void InitializeRenderer(renderer_context *RendererContext, shoora_app_info *AppInfo);
void DrawFrame(shoora_platform_frame_packet *FramePacket);
void DestroyRenderer(renderer_context *RendererContext);

#define RENDERER_FRONTEND_H
#endif // RENDERER_H