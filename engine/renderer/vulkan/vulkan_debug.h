#if !defined(VULKAN_DEBUG_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shoora_vulkan_debug_create_info
{
    VkDebugUtilsMessageSeverityFlagsEXT SeverityFlags;
    VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags;
    VkDebugReportFlagsEXT ReportFlags;
};

b32 SetVkObjectName(shoora_vulkan_context *Context, void *Object, VkObjectType ObjectType, const char *Name);
b32 SetupDebugCallbacks(shoora_vulkan_context *Context, shoora_vulkan_debug_create_info DebugCreateInfo);
void DestroyDebugUtilHandles(shoora_vulkan_context *Context);
void SetObjectName(const shoora_vulkan_device *RenderDevice, u64 Handle, VkObjectType HandleType,
                   const char *HandleName);

#define VULKAN_DEBUG_H
#endif // VULKAN_DEBUG_H