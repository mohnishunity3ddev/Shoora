#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "vulkan_defines.h"

#include "volk/volk.h"

struct shura_vulkan_context
{
    VkInstance Instance;
    VkDevice LogicalDevice;
};

void InitializeVulkanRenderer(shura_vulkan_context *VulkanContext, const char *AppName);
void DestroyVulkanRenderer(shura_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H