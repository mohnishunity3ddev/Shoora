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
void CreateSwapchain(shoora_vulkan_context *Context, u32 WindowWidth, u32 WindowHeight, shoora_vulkan_swapchain_create_info *ShuraSwapchainInfo = nullptr);
void CreateSwapchainFramebuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                                 VkRenderPass RenderPass);

void AcquireNextSwapchainImage(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                               shoora_vulkan_semaphore_handle *SignalSemaphore);

void DestroyPresentationSurface(shoora_vulkan_context *Context);
void DestroySwapchain(shoora_vulkan_context *Context);

#define VULKAN_SWAPCHAIN_H
#endif // VULKAN_SWAPCHAIN_H