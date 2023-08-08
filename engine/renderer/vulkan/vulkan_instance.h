#if !defined(VULKAN_INSTANCE_H)

#include "defines.h"
#include <volk/volk.h>

struct shoora_instance_create_info
{
    const char *AppName;
    const char **ppRequiredInstanceExtensions;
    u32 RequiredInstanceExtensionCount;
    const char **ppRequiredInstanceLayers;
    u32 RequiredInstanceLayerCount;
};

struct shoora_vulkan_context;

b32 CreateVulkanInstance(shoora_vulkan_context *VulkanContext, shoora_instance_create_info *ShuraInstanceCreateInfo);
void DestroyVulkanInstance(shoora_vulkan_context *VulkanContext);

#define VULKAN_INSTANCE_H
#endif // VULKAN_INSTANCE_H