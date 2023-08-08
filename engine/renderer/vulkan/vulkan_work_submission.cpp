#include "vulkan_work_submission.h"

// content of vulkan_work_submission.cpp goes here
// It is the caller's responsibility to set the Fence's IsRecoding state internally.
void
SubmitCommandBufferToQueue(shoora_vulkan_context *Context, VkQueue SubmissionQueue,
                           shoora_work_submit_info WorkInfo)
{
    VkQueue QueueHandle = SubmissionQueue;

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

    // Here, we submit one batch of command buffers(1 Submit Info) but we can submit multiple to increase
    // performance!
    VK_CHECK(vkQueueSubmit(QueueHandle, 1, &SubmitInfo, WorkInfo.Fence));
}

// We submit two batches of command buffers here. And also synchronize their submission to the queue
void
SubmitCommandBuffersToQueue(shoora_vulkan_context *Context, VkQueue FirstQueueToSubmitTo,
                            shoora_work_submit_info FirstWorkInfo, VkQueue SecondQueueToSubmitTo,
                            shoora_work_submit_info SecondWorkInfo, shoora_wait_semaphore_info *SyncInfos,
                            u32 SyncInfoCount)
{
    VkSemaphore SignalSemaphoresForFirstBatch[16];
    for (u32 Index = 0; Index < SyncInfoCount; ++Index)
    {
        SignalSemaphoresForFirstBatch[Index] = SyncInfos[Index].Semaphore;
    }

    // Signal these semaphores when the first batch is finished.
    FirstWorkInfo.SignalSemaphores = SignalSemaphoresForFirstBatch;
    FirstWorkInfo.SignalSemaphoreCount = SyncInfoCount;
    SubmitCommandBufferToQueue(Context, FirstQueueToSubmitTo, FirstWorkInfo);

    // Wait on these semaphores at the specified pipeline stages before starting this batch
    SecondWorkInfo.WaitSemaphoreInfos = SyncInfos;
    SecondWorkInfo.WaitSemaphoreInfoCount = SyncInfoCount;
    SubmitCommandBufferToQueue(Context, SecondQueueToSubmitTo, SecondWorkInfo);
}

// Command Buffers are processed and finish on the GPU behind the scene, the way the application knows about is
// through fences
b32
IsCommandBufferFinishedProcessing(shoora_vulkan_context *Context, VkQueue SubmissionQueue,
                                  shoora_work_submit_info WorkInfo)
{
    b32 Result;
    VkFence Fence = GetFirstUnsignaledFence(&Context->SyncHandles);
    ASSERT(Fence != VK_NULL_HANDLE);

    WorkInfo.Fence = Fence;
    SubmitCommandBufferToQueue(Context, SubmissionQueue, WorkInfo);

    b32 FenceWasSignaled = WaitForFence(&Context->Device, &WorkInfo.Fence, NANOSECONDS(1));
    if (FenceWasSignaled)
    {
        LogOutput(LogType_Info, "Submission to queue done!\n");
        Result = true;
    }

    return Result;
}

// Hand the application until all command buffers have been processed in the given Queue.
// Performance Implications!
void
QueueWaitIdle(VkQueue QueueHandle)
{
    VK_CHECK(vkQueueWaitIdle(QueueHandle));
}

// Hand the application until all command buffers have been processed in all the queues in the given GPU device.
// Performance Implications!
void
DeviceWaitIdle(VkDevice Device)
{
    VK_CHECK(vkDeviceWaitIdle(Device));
}
