#if !defined(VULKAN_RENDER_PASS_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

void CreateRenderPass(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                      VkRenderPass *RenderPass);

void DestroyRenderPass(shoora_vulkan_device *RenderDevice, VkRenderPass RenderPass);

#define VULKAN_RENDER_PASS_H
#endif // VULKAN_RENDER_PASS_H