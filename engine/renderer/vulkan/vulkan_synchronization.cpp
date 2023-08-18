#include "vulkan_synchronization.h"

#if 0
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
    ++Context->SyncHandles.SemaphoreCount;
    LogOutput(LogType_Info, "Semaphore[%d] Created!\n", InternalIndex);
}
void
DestroySemaphore(shoora_vulkan_context *Context, u32 InternalIndex)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Semaphores));
    shoora_vulkan_semaphore_handle *Semaphore = &Context->SyncHandles.Semaphores[InternalIndex];
    DestroySemaphore(&Context->Device, &Semaphore->Handle);
    --Context->SyncHandles.SemaphoreCount;
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
    ++Context->SyncHandles.FenceCount;
    LogOutput(LogType_Info, "Fence[%d] Created!\n", InternalIndex);
}
void
DestroyFence(shoora_vulkan_context *Context, u32 InternalIndex)
{
    ASSERT(InternalIndex < ARRAY_SIZE(Context->SyncHandles.Fences));
    shoora_vulkan_fence_handle *Fence = &Context->SyncHandles.Fences[InternalIndex];
    DestroyFence(&Context->Device, &Fence->Handle);
    --Context->SyncHandles.FenceCount;
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
IsFenceSafeToUse(shoora_vulkan_device *RenderDevice, shoora_vulkan_fence_handle *FenceHandle)
{
    b32 Result = false;
    VkResult WaitResult = vkWaitForFences(RenderDevice->LogicalDevice, 1, &FenceHandle->Handle, VK_TRUE, 1);

    // NOTE: If Fences are safe to use if they are UNSignaled. They become Signaled when some operations are
    // completed on the GPU It is our responsibility to reset them in order to use them again.
    if (WaitResult != VK_SUCCESS)
    {
        Result = true;
        ASSERT(FenceHandle->IsSignaled == false);
    }
    else
    {
        ASSERT(FenceHandle->IsSignaled);
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
VkFence
GetFirstUnsignaledFence(shoora_vulkan_synchronization *SyncHandles)
{
    ASSERT(SyncHandles->FenceCount <= SHU_VK_MAX_FENCE_COUNT);
    VkFence Result = VK_NULL_HANDLE;
    for(u32 Index = 0;
        Index < SyncHandles->FenceCount;
        ++Index)
    {
        shoora_vulkan_fence_handle FenceHandle = SyncHandles->Fences[Index];
        if(!FenceHandle.IsSignaled)
        {
            Result = FenceHandle.Handle;
            break;
        }
    }
    // TODO)): Right now, we make sure we have enough fences. Later on, if this fails, we should create a new fence
    // and store it internally.
    ASSERT(Result != VK_NULL_HANDLE);

    return Result;
}
#endif

void
CreateSynchronizationPrimitives(shoora_vulkan_device *RenderDevice, shoora_vulkan_synchronization *SyncObjects)
{
    VkSemaphoreCreateInfo SemaphoreCreateInfo;
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SemaphoreCreateInfo.pNext = nullptr;
    SemaphoreCreateInfo.flags = 0;

    VkFenceCreateInfo FenceCreateInfo;
    FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceCreateInfo.pNext = nullptr;
    FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        VK_CHECK(vkCreateSemaphore(RenderDevice->LogicalDevice, &SemaphoreCreateInfo, nullptr,
                                   &SyncObjects->ImageAvailableSemaphores[Index].Handle));

        VK_CHECK(vkCreateSemaphore(RenderDevice->LogicalDevice, &SemaphoreCreateInfo, nullptr,
                                   &SyncObjects->RenderFinishedSemaphores[Index].Handle));

        VK_CHECK(vkCreateFence(RenderDevice->LogicalDevice, &FenceCreateInfo, nullptr,
                               &SyncObjects->Fences[Index].Handle));
        SyncObjects->Fences[Index].IsSignaled = true;
    }

    LogOutput(LogType_Info, "Created all Synchronization Primitives!\n");
}

void
DestroyAllSynchronizationPrimitives(shoora_vulkan_device *RenderDevice, shoora_vulkan_synchronization *SyncObjects)
{
    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        shoora_vulkan_semaphore_handle *Semaphore = nullptr;

        Semaphore = SyncObjects->ImageAvailableSemaphores + Index;
        vkDestroySemaphore(RenderDevice->LogicalDevice, Semaphore->Handle, nullptr);
        Semaphore->IsSignaled = false;
        Semaphore->Handle = VK_NULL_HANDLE;

        Semaphore = SyncObjects->RenderFinishedSemaphores + Index;
        vkDestroySemaphore(RenderDevice->LogicalDevice, Semaphore->Handle, nullptr);
        Semaphore->IsSignaled = false;
        Semaphore->Handle = VK_NULL_HANDLE;
        Semaphore = nullptr;

        shoora_vulkan_fence_handle *Fence = &SyncObjects->Fences[Index];
        vkDestroyFence(RenderDevice->LogicalDevice, Fence->Handle, nullptr);
        Fence->Handle = VK_NULL_HANDLE;
        Fence->IsSignaled = false;
    }
}

shoora_vulkan_fence_handle *
GetCurrentFrameFencePtr(shoora_vulkan_synchronization *SyncHandles, u32 FrameIndex)
{
    ASSERT(FrameIndex >= 0 && FrameIndex < SHU_MAX_FRAMES_IN_FLIGHT);
    shoora_vulkan_fence_handle *pFenceHandle = (shoora_vulkan_fence_handle *)&SyncHandles->Fences[FrameIndex];
    ASSERT(pFenceHandle != nullptr && pFenceHandle->Handle != VK_NULL_HANDLE);
    return pFenceHandle;
}

shoora_vulkan_semaphore_handle *
GetImageAvailableSemaphorePtr(shoora_vulkan_synchronization *SyncHandles, u32 FrameIndex)
{
    ASSERT(FrameIndex >= 0 && FrameIndex < SHU_MAX_FRAMES_IN_FLIGHT);
    shoora_vulkan_semaphore_handle *pImageAvailableSemaphore = &SyncHandles->ImageAvailableSemaphores[FrameIndex];
    ASSERT(pImageAvailableSemaphore && pImageAvailableSemaphore->Handle != VK_NULL_HANDLE);

    return pImageAvailableSemaphore;
}

shoora_vulkan_semaphore_handle *
GetRenderFinishedSemaphorePtr(shoora_vulkan_synchronization *SyncHandles, u32 FrameIndex)
{
    ASSERT(FrameIndex >= 0 && FrameIndex < SHU_MAX_FRAMES_IN_FLIGHT);
    shoora_vulkan_semaphore_handle *pRenderFinishedSemaphore = &SyncHandles->RenderFinishedSemaphores[FrameIndex];

    ASSERT(pRenderFinishedSemaphore && pRenderFinishedSemaphore->Handle != VK_NULL_HANDLE);
    return pRenderFinishedSemaphore;
}
