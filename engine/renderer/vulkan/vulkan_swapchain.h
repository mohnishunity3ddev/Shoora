#if !defined(VULKAN_SWAPCHAIN_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shoora_vulkan_swapchain_create_info
{
    VkPresentModeKHR DesiredPresentMode;
    VkImageUsageFlags DesiredImageUsages;
    VkSurfaceTransformFlagBitsKHR DesiredTransformFlagBits;

    VkFormat DesiredImageFormat;
    VkColorSpaceKHR DesiredImageColorSpace;
};

struct shoora_vulkan_image_present_info
{
    VkSwapchainKHR Swapchain;
    u32 ImageIndex;
};

void CreatePresentationSurface(shoora_vulkan_context *Context, VkSurfaceKHR *Surface);
void CreateSwapchain(shoora_vulkan_context *Context, shoora_vulkan_swapchain_create_info *ShuraSwapchainInfo);

u32 AcquireNextSwapchainImage(shoora_vulkan_context *Context, u32 SemaphoreInternalIndex, u32 FenceInternalIndex);

void DestroyPresentationSurface(shoora_vulkan_context *Context);
void DestroySwapchain(shoora_vulkan_context *Context);

#define VULKAN_SWAPCHAIN_H
#endif // VULKAN_SWAPCHAIN_H