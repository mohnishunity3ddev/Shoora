#if !defined(VULKAN_SYNCHRONIZATION_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

// -------------------------------------------------------------------------------------------------------
// SEMAPHORES --------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
void CreateSemaphore(shoora_vulkan_device *RenderDevice, VkSemaphore *pSemaphore);
void CreateSemaphore(shoora_vulkan_context *Context, u32 InternalIndex);
void CreateAllSemaphores(shoora_vulkan_context *Context);
void DestroySemaphore(shoora_vulkan_device *RenderDevice, VkSemaphore *pSemaphore);
void DestroySemaphore(shoora_vulkan_context *Context, u32 InternalIndex);
void DestroyAllSemaphores(shoora_vulkan_context *Context);

// -------------------------------------------------------------------------------------------------------
// FENCES ------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
void CreateFence(shoora_vulkan_device *RenderDevice, VkFence *pFence, b32 IsSignaled);
void CreateFence(shoora_vulkan_context *Context, u32 InternalIndex, b32 IsSignaled);
void CreateAllFences(shoora_vulkan_context *Context, b32 IsSignaled = false);
void DestroyFence(shoora_vulkan_device *RenderDevice, VkFence *pFence);
void DestroyFence(shoora_vulkan_context *Context, u32 InternalIndex);
void DestroyAllFences(shoora_vulkan_context *Context);

b32 WaitForFence(shoora_vulkan_device *RenderDevice, VkFence *Fence, u64 TimeoutInSeconds = 0);
b32 WaitForAllFences(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount,
                     VkBool32 WaitForAll = VK_TRUE, u64 TimeoutInSeconds = 0);
b32 AreFencesSafeToUse(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount);
void ResetFences(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount);
VkFence GetFirstUnsignaledFence(shoora_vulkan_synchronization *SyncHandles);

#define VULKAN_SYNCHRONIZATION_H
#endif // VULKAN_SYNCHRONIZATION_H