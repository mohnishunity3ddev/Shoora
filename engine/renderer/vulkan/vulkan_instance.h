#if !defined(VULKAN_INSTANCE_H)

#include "defines.h"
#include <volk/volk.h>

struct shura_instance_create_info
{
    const char *AppName;
    const char **ppRequiredInstanceExtensions;
    u32 RequiredInstanceExtensionCount;
    const char **ppRequiredInstanceLayers;
    u32 RequiredInstanceLayerCount;
};

struct shura_vulkan_context;

b32 CreateVulkanInstance(shura_vulkan_context *VulkanContext, shura_instance_create_info *ShuraInstanceCreateInfo);
void DestroyVulkanInstance(shura_vulkan_context *VulkanContext);

#define VULKAN_INSTANCE_H
#endif // VULKAN_INSTANCE_H