#include "vulkan_input_info.h"

struct shoora_wait_semaphore_info
{
    // Wait for this semaphore to be signaled
    VkSemaphore Semaphore;
    // The pipeline stage where the wait should occur IF the wait occurs.
    VkPipelineStageFlags WaitingStage;
};

struct shoora_work_submit_info
{
    shoora_queue_type QueueTypeToSubmitTo;
    shoora_wait_semaphore_info *WaitSemaphoreInfos; u32 WaitSemaphoreInfoCount;
    shoora_vulkan_command_buffer_handle *CmdBufferHandles; u32 CmdBufferCount;
    VkSemaphore *SignalSemaphores; u32 SignalSemaphoreCount;
    VkFence Fence;
};

void
SubmitWork(shoora_vulkan_context *Context, shoora_work_submit_info WorkInfo)
{
    VkQueue QueueHandle = GetQueueHandle(&Context->Device, WorkInfo.QueueTypeToSubmitTo);

    // Command Buffers should be in a recording state.
    for (u32 Index = 0; Index < WorkInfo.CmdBufferCount; ++Index)
    {
        shoora_vulkan_command_buffer_handle *CmdBuffer = WorkInfo.CmdBufferHandles + Index;
        ASSERT(CmdBuffer->IsRecording);
    }

    VkSemaphore WaitSemaphores[16] = {};
    VkPipelineStageFlags WaitingStages[16] = {};
    for (u32 Index = 0; Index < WorkInfo.WaitSemaphoreInfoCount; ++Index)
    {
        WaitSemaphores[Index] = WorkInfo.WaitSemaphoreInfos[Index].Semaphore;
        WaitingStages[Index] = WorkInfo.WaitSemaphoreInfos[Index].WaitingStage;
    }

    VkCommandBuffer CommandBuffers[16] = {};
    for (u32 Index = 0; Index < WorkInfo.WaitSemaphoreInfoCount; ++Index)
    {
        CommandBuffers[Index] = WorkInfo.CmdBufferHandles[Index].Handle;
    }


    VkSubmitInfo SubmitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    SubmitInfo.pNext = nullptr;
    SubmitInfo.waitSemaphoreCount = WorkInfo.WaitSemaphoreInfoCount;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitingStages;
    SubmitInfo.commandBufferCount = WorkInfo.CmdBufferCount;
    SubmitInfo.pCommandBuffers = CommandBuffers;
    SubmitInfo.signalSemaphoreCount = WorkInfo.SignalSemaphoreCount;
    SubmitInfo.pSignalSemaphores = WorkInfo.SignalSemaphores;

    VK_CHECK(vkQueueSubmit(QueueHandle, 1, &SubmitInfo, WorkInfo.Fence));
}

void
InitializeVulkanRenderer(shoora_vulkan_context *Context, shoora_app_info *AppInfo)
{
    VK_CHECK(volkInitialize());

    ShuraInstanceCreateInfo.AppName = AppInfo->AppName;
    CreateVulkanInstance(Context, &ShuraInstanceCreateInfo);
    volkLoadInstance(Context->Instance);

#ifdef _DEBUG
    SetupDebugCallbacks(Context, DebugCreateInfo);
#endif

    CreatePresentationSurface(Context, &Context->Swapchain.Surface);
    CreateDeviceNQueuesNCommandPools(Context, &DeviceCreateInfo);
    volkLoadDevice(Context->Device.LogicalDevice);

    CreateSwapchain(Context, &SwapchainInfo);

    AllocateCommandBuffers(Context, Shu_BufferAllocInfos, ARRAY_SIZE(Shu_BufferAllocInfos));

    shoora_vulkan_command_buffer *CmdBufferGroup = GetCommandBufferGroupForQueue(Context, QueueType_Graphics);
    u32 CmdBufferInternalIndex = CmdBufferGroup->BufferCount - 1;

    ResetAllCommandPools(&Context->Device, true);

    BeginCommandBuffer(CmdBufferGroup, CmdBufferInternalIndex, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    EndCommandBuffer(CmdBufferGroup, CmdBufferInternalIndex);

    CreateAllSemaphores(Context);
    CreateAllFences(Context);
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    DestroyAllSemaphores(Context);
    DestroyAllFences(Context);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(&Context->Device);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}
