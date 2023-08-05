#if !defined(VULKAN_SYNCHRONIZATION_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shura_vulkan_semaphore_create_info
{
};

struct shura_vulkan_fence_create_info
{
};

void CreateSemaphore(shura_vulkan_context *Context, shura_vulkan_semaphore_create_info SemaphoreInfo);
void CreateFence(shura_vulkan_context *Context, shura_vulkan_fence_create_info FenceInfo);

#define VULKAN_SYNCHRONIZATION_H
#endif // VULKAN_SYNCHRONIZATION_H