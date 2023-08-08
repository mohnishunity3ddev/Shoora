#include "vulkan_synchronization.h"

// -------------------------------------------------------------------------------------------------------
// SEMAPHORES --------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
void
CreateSemaphore(shoora_vulkan_device *RenderDevice, VkSemaphore *pSemaphore)
{
    VkSemaphoreCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    VK_CHECK(vkCreateSemaphore(RenderDevice->LogicalDevice, &CreateInfo, 0, pSemaphore));
}
void
DestroySemaphore(shoora_vulkan_device *RenderDevice, VkSemaphore *pSemaphore)
{
    vkDestroySemaphore(RenderDevice->LogicalDevice, *pSemaphore, 0);
}
void
CreateSemaphore(shoora_vulkan_context *Context, u32 InternalIndex)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Semaphores));
    shoora_vulkan_semaphore_handle *Semaphore = &Context->SyncHandles.Semaphores[InternalIndex];
    Semaphore->IsSignaled = false;
    CreateSemaphore(&Context->Device, &Semaphore->Handle);
    LogOutput(LogType_Info, "Semaphore[%d] Created!\n", InternalIndex);
}
void
DestroySemaphore(shoora_vulkan_context *Context, u32 InternalIndex)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Semaphores));
    shoora_vulkan_semaphore_handle *Semaphore = &Context->SyncHandles.Semaphores[InternalIndex];
    DestroySemaphore(&Context->Device, &Semaphore->Handle);
    LogOutput(LogType_Info, "Semaphore[%d] Destroyed!\n", InternalIndex);
}
void
CreateAllSemaphores(shoora_vulkan_context *Context)
{
    u32 SemaphoreCount = ARRAY_SIZE(Context->SyncHandles.Semaphores);
    ASSERT(SemaphoreCount <= SHU_VK_MAX_SEMAPHORE_COUNT);
    for(u32 Index = 0;
        Index < SemaphoreCount;
        ++Index)
    {
        CreateSemaphore(Context, Index);
    }
    LogOutput(LogType_Info, "Created %d Semaphores!\n", SemaphoreCount);
}
void
DestroyAllSemaphores(shoora_vulkan_context *Context)
{
    u32 SemaphoreCount = ARRAY_SIZE(Context->SyncHandles.Semaphores);
    ASSERT(SemaphoreCount <= SHU_VK_MAX_SEMAPHORE_COUNT);
    for(u32 Index = 0;
        Index < SemaphoreCount;
        ++Index)
    {
        DestroySemaphore(Context, Index);
    }
    LogOutput(LogType_Info, "Destroyed %d Semaphores!\n", SemaphoreCount);
}

// -------------------------------------------------------------------------------------------------------
// FENCES ------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
void
CreateFence(shoora_vulkan_device *RenderDevice, VkFence *pFence, b32 IsSignaled)
{
    VkFenceCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = IsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VK_CHECK(vkCreateFence(RenderDevice->LogicalDevice, &CreateInfo, 0, pFence));
}
void
DestroyFence(shoora_vulkan_device *RenderDevice, VkFence *pFence)
{
    vkDestroyFence(RenderDevice->LogicalDevice, *pFence, 0);
}
void
CreateFence(shoora_vulkan_context *Context, u32 InternalIndex, b32 IsSignaled)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Fences));
    shoora_vulkan_fence_handle *Fence = &Context->SyncHandles.Fences[InternalIndex];
    Fence->IsSignaled = IsSignaled;
    CreateFence(&Context->Device, &Fence->Handle, IsSignaled);
    LogOutput(LogType_Info, "Fence[%d] Created!\n", InternalIndex);
}
void
DestroyFence(shoora_vulkan_context *Context, u32 InternalIndex)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Fences));
    shoora_vulkan_fence_handle *Fence = &Context->SyncHandles.Fences[InternalIndex];
    DestroyFence(&Context->Device, &Fence->Handle);
    LogOutput(LogType_Info, "Fence[%d] Destroyed!\n", InternalIndex);
}
void
CreateAllFences(shoora_vulkan_context *Context, b32 IsSignaled)
{
    u32 FenceCount = ARRAY_SIZE(Context->SyncHandles.Fences);
    ASSERT(FenceCount <= SHU_VK_MAX_FENCE_COUNT);
    for(u32 Index = 0;
        Index < FenceCount;
        ++Index)
    {
        CreateFence(Context, Index, IsSignaled);
    }
    LogOutput(LogType_Info, "Created %d Fences!\n", FenceCount);
}
void
DestroyAllFences(shoora_vulkan_context *Context)
{
    u32 FenceCount = ARRAY_SIZE(Context->SyncHandles.Fences);
    ASSERT(FenceCount <= SHU_VK_MAX_FENCE_COUNT);
    for(u32 Index = 0;
        Index < FenceCount;
        ++Index)
    {
        DestroyFence(Context, Index);
    }
    LogOutput(LogType_Info, "Destroyed %d Fences!\n", FenceCount);
}

b32
WaitForFence(shoora_vulkan_device *RenderDevice, VkFence *Fence, u64 TimeoutInSeconds)
{
    b32 Result = false;
    VkResult WaitResult = vkWaitForFences(RenderDevice->LogicalDevice, 1, Fence, VK_TRUE, NANOSECONDS(TimeoutInSeconds));

    if(WaitResult == VK_SUCCESS)
    {
        LogOutput(LogType_Warn, "Wait for Fence successful!\n");
        Result = true;
    }
    else if(WaitResult == VK_TIMEOUT)
    {
        LogOutput(LogType_Warn, "There was a timeout in Waiting for fence!");
    }
    else
    {
        LogOutput(LogType_Error, "There was a problem in Waiting for fence!");
    }
    return Result;
}
b32
WaitForAllFences(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount, VkBool32 WaitForAll,
                 u64 TimeoutInSeconds)
{
    b32 Result = false;

    VkResult WaitResult = vkWaitForFences(RenderDevice->LogicalDevice, FenceCount, Fences, WaitForAll,
                                          NANOSECONDS(TimeoutInSeconds));

    if(WaitResult == VK_SUCCESS)
    {
        LogOutput(LogType_Warn, "Wait for Fence(s) successful!\n");
        Result = true;
    }
    else if(WaitResult == VK_TIMEOUT)
    {
        LogOutput(LogType_Warn, "There was a timeout in Waiting for fence!");
    }
    else
    {
        LogOutput(LogType_Error, "There was a problem in Waiting for fence!");
    }
    return Result;
}
b32
AreFencesSafeToUse(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount)
{
    b32 Result = false;
    VkResult WaitResult = vkWaitForFences(RenderDevice->LogicalDevice, FenceCount, Fences, VK_TRUE, 1);

    // NOTE: If Fences are safe to use if they are UNSignaled. They become Signaled when some operations are
    // completed on the GPU It is our responsibility to reset them in order to use them again.
    if(WaitResult != VK_SUCCESS)
    {
        Result = true;
    }
    return Result;
}
void
ResetFences(shoora_vulkan_device *RenderDevice, VkFence *Fences, u32 FenceCount)
{
    VK_CHECK(vkResetFences(RenderDevice->LogicalDevice, FenceCount, Fences));
}
