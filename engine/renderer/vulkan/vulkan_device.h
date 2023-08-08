#if !defined(VULKAN_DEVICE_H)

#include "defines.h"
struct shoora_vulkan_device;

enum shoora_queue_type
{
    QueueType_Graphics,
    QueueType_Compute,
    QueueType_Transfer,

    // TODO)): Read more on Sparse and Protected Queues
    QueueType_Sparse,
    QueueType_Protected,

    QueueType_Count
};

struct shoora_queue_info
{
    shoora_queue_type Type = QueueType_Count;
    u32 QueueCount;
    // TODO)): Make this dynamic.
    f32 Priorities[2] = {1.0f, 1.0f};
    u32 FamilyIndex = -1;
};

struct shoora_command_pool_create_info
{
    VkCommandPoolCreateFlags CreateFlags;
    shoora_queue_type QueueType;
};

struct shoora_device_create_info
{
    const char **ppRequiredExtensions;
    const u32 RequiredExtensionCount;
    const VkPhysicalDeviceFeatures *DesiredFeatures;
    shoora_queue_info *pQueueCreateInfos;
    const u32 QueueCreateInfoCount;

    shoora_command_pool_create_info *pCommandPoolCreateInfos;
    u32 CommandPoolCount;
};

 void CreateDeviceNQueuesNCommandPools(struct shoora_vulkan_context *VulkanContext,
                                      shoora_device_create_info *ShuraDeviceCreateInfo);
 u32 GetQueueIndexFromType(shoora_queue_type Type);
 const char *GetQueueTypeName(shoora_queue_type Type);

 void ResetCommandPool(shoora_vulkan_device *RenderDevice, u32 InternalIndex, b32 ReleaseResources);
 void ResetAllCommandPools(shoora_vulkan_device *RenderDevice, b32 ReleaseResources);

 VkQueue GetQueueHandle(shoora_vulkan_device *RenderDevice, shoora_queue_type QueueType);
 void DestroyLogicalDevice(shoora_vulkan_device *RenderDevice);
#define VULKAN_DEVICE_H
#endif // VULKAN_DEVICE_H