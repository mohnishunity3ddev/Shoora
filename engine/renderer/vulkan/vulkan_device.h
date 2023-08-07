#if !defined(VULKAN_DEVICE_H)

#include "defines.h"
struct shura_vulkan_device;

enum shura_queue_type
{
    QueueType_Graphics,
    QueueType_Compute,
    QueueType_Transfer,

    // TODO)): Read more on Sparse and Protected Queues
    QueueType_Sparse,
    QueueType_Protected,

    QueueType_Count
};

struct shura_queue_info
{
    shura_queue_type Type = QueueType_Count;
    u32 QueueCount;
    // TODO)): Make this dynamic.
    f32 Priorities[2] = {1.0f, 1.0f};
    u32 FamilyIndex = -1;
};

struct shura_command_pool_create_info
{
    VkCommandPoolCreateFlags CreateFlags;
    shura_queue_type QueueType;
};

struct shura_device_create_info
{
    const char **ppRequiredExtensions;
    const u32 RequiredExtensionCount;
    const VkPhysicalDeviceFeatures *DesiredFeatures;
    shura_queue_info *pQueueCreateInfos;
    const u32 QueueCreateInfoCount;

    shura_command_pool_create_info *pCommandPoolCreateInfos;
    u32 CommandPoolCount;
};

 void CreateDeviceNQueuesNCommandPools(struct shura_vulkan_context *VulkanContext,
                                      shura_device_create_info *ShuraDeviceCreateInfo);
 u32 GetQueueIndexFromType(shura_queue_type Type);
 const char *GetQueueTypeName(shura_queue_type Type);

 void ResetCommandPool(shura_vulkan_device *RenderDevice, u32 InternalIndex, b32 ReleaseResources);
 void ResetAllCommandPools(shura_vulkan_device *RenderDevice, b32 ReleaseResources);

void DestroyLogicalDevice(shura_vulkan_device *RenderDevice);
#define VULKAN_DEVICE_H
#endif // VULKAN_DEVICE_H