#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"

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
    VkImage SwapchainImages[8];
    u32 SwapchainImageCount;
};

struct shura_vulkan_device
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

    // TODO)): The Queues are actually arrays since there can be multiple queues from the same family which do a specific same task
    VkQueue GraphicsQueue;
    u32 GraphicsQueueCount;
    u32 GraphicsQueueFamilyIndex;
    VkQueue ComputeQueue;
    u32 ComputeQueueCount;
    u32 ComputeQueueFamilyIndex;
    VkQueue TransferQueue;
    u32 TransferQueueCount;
    u32 TransferQueueFamilyIndex;
};

struct shura_vulkan_context
{
    VkInstance Instance;
    shura_vulkan_device Device;

    shura_vulkan_debug Debug;
    shura_vulkan_swapchain Swapchain;

    VkSemaphore Semaphore;
    VkFence Fence;
};

void InitializeVulkanRenderer(shura_vulkan_context *VulkanContext, shura_app_info *AppInfo);
void DestroyVulkanRenderer(shura_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H