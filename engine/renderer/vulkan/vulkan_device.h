#if !defined(VULKAN_DEVICE_H)

#include "defines.h"

enum shura_queue_type
{
    QueueType_None,
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
    shura_queue_type Type = QueueType_None;
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

 void CreateDeviceNQueuesNCommandPool(struct shura_vulkan_context *VulkanContext,
                                      shura_device_create_info *ShuraDeviceCreateInfo);
 void DestroyLogicalDevice(struct shura_vulkan_device *RenderDevice);

#define VULKAN_DEVICE_H
#endif // VULKAN_DEVICE_H