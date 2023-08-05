#if !defined(VULKAN_DEBUG_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shura_vulkan_debug_create_info
{
    VkDebugUtilsMessageSeverityFlagsEXT SeverityFlags;
    VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags;
    VkDebugReportFlagsEXT ReportFlags;
};

b32 SetVkObjectName(shura_vulkan_context *Context, void *Object, VkObjectType ObjectType, const char *Name);
b32 SetupDebugCallbacks(shura_vulkan_context *Context, shura_vulkan_debug_create_info DebugCreateInfo);
void DestroyDebugUtilHandles(shura_vulkan_context *Context);

#define VULKAN_DEBUG_H
#endif // VULKAN_DEBUG_H