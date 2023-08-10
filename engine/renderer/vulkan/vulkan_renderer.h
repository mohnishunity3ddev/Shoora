#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"
#include "vulkan_device.h"

struct shoora_vulkan_command_buffer_handle
{
    VkCommandBuffer Handle;
    b32 IsRecording;
    VkCommandPool *CommandPool;
};

struct shoora_vulkan_debug
{
    VkDebugUtilsMessengerEXT Messenger;
    VkDebugReportCallbackEXT ReportCallback;
};

struct shoora_vulkan_swapchain
{
    VkSurfaceKHR Surface;
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;

    VkSurfaceFormatKHR ImageFormat;
    VkExtent2D ImageSize;
    VkImageUsageFlags ImageUsageFlags;
    VkPresentModeKHR PresentMode;
    VkSurfaceTransformFlagBitsKHR TransformFlagBits;

    VkSwapchainKHR SwapchainHandle;

    // TODO)): Make this Dynamic!
    VkImage SwapchainImages[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    u32 SwapchainImageCount;
};

struct shoora_vulkan_queue
{
    // TODO: Shouldn't this be an array!
    VkQueue Handle;
    u32 Count;
    u32 FamilyIndex;
    // TODO)): Make this Dynamic
    f32 Priorities[8] =
    {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
};

struct shoora_vulkan_device
{
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties DeviceProperties;
    VkPhysicalDeviceFeatures DeviceFeatures;

    VkDevice LogicalDevice;

    // TODO)): Make these Dynamic. Use Pointers instead.
    shoora_vulkan_queue Queues[SHU_VK_MAX_QUEUE_TYPE_COUNT];
    VkCommandPool CommandPools[SHU_VK_MAX_QUEUE_TYPE_COUNT];
    u32 QueueTypeCount;
};

struct shoora_vulkan_command_buffer
{
    shoora_queue_type QueueType;
    VkCommandBufferLevel BufferLevel;
    // VkCommandBuffer BufferHandles[MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    // TODO)): Make these Dynamic!
    shoora_vulkan_command_buffer_handle BufferHandles[SHU_VK_MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    u32 BufferCount;
};

struct shoora_vulkan_semaphore_handle
{
    VkSemaphore Handle;
    b32 IsSignaled;
};

struct shoora_vulkan_fence_handle
{
    VkFence Handle;
    b32 IsSignaled;
};

struct shoora_vulkan_synchronization
{
    shoora_vulkan_semaphore_handle Semaphores[SHU_VK_MAX_SEMAPHORE_COUNT];
    u32 SemaphoreCount;
    shoora_vulkan_fence_handle Fences[SHU_VK_MAX_FENCE_COUNT];
    u32 FenceCount;
};

struct shoora_vulkan_context
{
    VkInstance Instance;
    shoora_vulkan_debug Debug;

    shoora_vulkan_device Device;
    shoora_vulkan_swapchain Swapchain;

    // TODO)): Maybe we should move this information into the vulkan_device struct?
    // since command buffers are associated with queues which are there in the device struct only.
    shoora_vulkan_command_buffer CommandBuffers[SHU_VK_MAX_QUEUE_TYPE_COUNT];

    shoora_vulkan_synchronization SyncHandles;
};

void InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo);
void DestroyVulkanRenderer(shoora_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H