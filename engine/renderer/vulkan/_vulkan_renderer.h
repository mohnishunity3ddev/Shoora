#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "vulkan_defines.h"

#include "volk/volk.h"

struct vulkan_context
{
    VkInstance Instance;
};

void InitializeVulkanRenderer(vulkan_context *VulkanContext, const char *AppName);
void DestroyVulkanRenderer(vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H