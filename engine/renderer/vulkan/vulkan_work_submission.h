#if !defined(VULKAN_WORK_SUBMISSION_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"
#include "vulkan_synchronization.h"

#if 0
struct shoora_wait_semaphore_info
{
    // Wait for this semaphore to be signaled
    VkSemaphore Semaphore;
    // The pipeline stage where the wait should occur IF the wait occurs.
    VkPipelineStageFlags WaitingStage;
};

struct shoora_work_submit_info
{
    shoora_wait_semaphore_info *WaitSemaphoreInfos;
    u32 WaitSemaphoreInfoCount;
    shoora_vulkan_command_buffer_handle *CmdBufferHandles;
    u32 CmdBufferCount;
    VkSemaphore *SignalSemaphores;
    u32 SignalSemaphoreCount;
    VkFence Fence;
};

void SubmitCommandBufferToQueue(shoora_vulkan_context *Context, VkQueue SubmissionQueue,
                                shoora_work_submit_info WorkInfo);
void SubmitCommandBuffersToQueue(shoora_vulkan_context *Context, VkQueue FirstQueueToSubmitTo,
                                 shoora_work_submit_info FirstWorkInfo, VkQueue SecondQueueToSubmitTo,
                                 shoora_work_submit_info SecondWorkInfo, shoora_wait_semaphore_info *SyncInfos,
                                 u32 SyncInfoCount);
b32 IsCommandBufferFinishedProcessing(shoora_vulkan_context *Context, VkQueue SubmissionQueue,
                                      shoora_work_submit_info WorkInfo);

// Hand the application until all command buffers have been processed in the given Queue.
// Performance Implications!
void QueueWaitIdle(VkQueue QueueHandle);

// Hand the application until all command buffers have been processed in all the queues in the given GPU device.
// Performance Implications!
// NOTE: Usually called before exiting the application!
void DeviceWaitIdle(VkDevice Device);
#endif

#define VULKAN_WORK_SUBMISSION_H
#endif // VULKAN_WORK_SUBMISSION_H