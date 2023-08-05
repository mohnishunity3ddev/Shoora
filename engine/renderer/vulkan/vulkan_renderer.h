#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"

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

struct shura_vulkan_context
{
    VkInstance Instance;

    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

    VkQueue GraphicsQueue;
    VkQueue ComputeQueue;
    VkQueue TransferQueue;

    shura_vulkan_swapchain Swapchain;
};

void InitializeVulkanRenderer(shura_vulkan_context *VulkanContext, shura_app_info *AppInfo);
void DestroyVulkanRenderer(shura_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H