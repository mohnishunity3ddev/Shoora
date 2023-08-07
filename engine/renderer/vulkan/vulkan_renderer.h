#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"
#include "vulkan_device.h"

#define MAX_QUEUE_TYPE_COUNT 4
#define MAX_SWAPCHAIN_IMAGE_COUNT 8
#define MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT 8

struct shura_vulkan_debug
{
    VkDebugUtilsMessengerEXT Messenger;
    VkDebugReportCallbackEXT ReportCallback;
};

struct shura_vulkan_swapchain
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
    VkImage SwapchainImages[MAX_SWAPCHAIN_IMAGE_COUNT];
    u32 SwapchainImageCount;
};

struct shura_vulkan_queue
{
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

struct shura_vulkan_device
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

    // TODO)): Make these Dynamic. Use Pointers instead.
    shura_vulkan_queue Queues[MAX_QUEUE_TYPE_COUNT];
    VkCommandPool CommandPools[MAX_QUEUE_TYPE_COUNT];
    u32 QueueTypeCount;
};

struct shura_vulkan_command_buffer_handle
{
    VkCommandBuffer Handle;
    b32 IsRecording;
};

struct shura_vulkan_command_buffer
{
    shura_queue_type QueueType;
    VkCommandBufferLevel BufferLevel;
    // VkCommandBuffer BufferHandles[MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    shura_vulkan_command_buffer_handle BufferHandles[MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    b32 RecordingBuffers[MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    u32 BufferCount;
};

struct shura_vulkan_context
{
    VkInstance Instance;
    shura_vulkan_device Device;

    shura_vulkan_debug Debug;
    shura_vulkan_swapchain Swapchain;
    shura_vulkan_command_buffer CommandBuffers[MAX_QUEUE_TYPE_COUNT];

    VkSemaphore Semaphore;
    VkFence Fence;
};

void InitializeVulkanRenderer(shura_vulkan_context *VulkanContext, shura_app_info *AppInfo);
void DestroyVulkanRenderer(shura_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H