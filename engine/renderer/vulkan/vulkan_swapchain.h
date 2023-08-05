#if !defined(VULKAN_SWAPCHAIN_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shura_vulkan_swapchain_create_info
{
    VkPresentModeKHR DesiredPresentMode;
    VkImageUsageFlags DesiredImageUsages;
    VkSurfaceTransformFlagBitsKHR DesiredTransformFlagBits;

    VkFormat DesiredImageFormat;
    VkColorSpaceKHR DesiredImageColorSpace;
};

void CreatePresentationSurface(shura_vulkan_context *Context, VkSurfaceKHR *Surface);
void CreateSwapchain(shura_vulkan_context *Context, shura_vulkan_swapchain_create_info *ShuraSwapchainInfo);

void DestroyPresentationSurface(shura_vulkan_context *Context);
void DestroySwapchain(shura_vulkan_context *Context);

#define VULKAN_SWAPCHAIN_H
#endif // VULKAN_SWAPCHAIN_H