#include "vulkan_command_buffer.h"
#include "platform/platform.h"
#include "vulkan_device.h"

void
AllocateCommandBuffers(shura_vulkan_context *Context, shura_command_buffer_allocate_info *AllocInfos,
                       u32 AllocInfoCount)
{
    ASSERT(AllocInfoCount < MAX_QUEUE_TYPE_COUNT);

    for(u32 Index = 0;
        Index < AllocInfoCount;
        ++Index)
    {
        shura_command_buffer_allocate_info *AllocateInfo = AllocInfos + Index;
        ASSERT(AllocateInfo->BufferCount < MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT);

        u32 QueueIndex = GetQueueIndexFromType((shura_queue_type)AllocateInfo->QueueType);
        shura_vulkan_device *RenderDevice = &Context->Device;

        VkCommandBufferAllocateInfo VkAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        VkAllocInfo.commandPool = RenderDevice->CommandPools[QueueIndex];
        VkAllocInfo.level = AllocateInfo->Level;
        VkAllocInfo.commandBufferCount = AllocateInfo->BufferCount;

        shura_vulkan_command_buffer *Shu_CommandBuffer = Context->CommandBuffers + QueueIndex;
        Shu_CommandBuffer->QueueType = AllocateInfo->QueueType;
        Shu_CommandBuffer->BufferCount = AllocateInfo->BufferCount;
        Shu_CommandBuffer->BufferLevel = AllocateInfo->Level;

        VkCommandBuffer Buffers[MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
        for(u32 Index = 0;
            Index < AllocateInfo->BufferCount;
            ++Index)
        {
            Buffers[Index] = Shu_CommandBuffer->BufferHandles[Index].Handle;
        }

        // NOTE: vkAllocateCommandBuffers expects an array of just VkCommandBuffer handles. But we have an array of a struct
        // which also has the recording state of this command buffer. That's why I need to have this Buffers Intermediate array
        // of just VkCommandBuffer/
        // TODO)): Check if there is a better way without using this intermediate array.
        VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &VkAllocInfo,
                                          Buffers));

        for(u32 Index = 0;
            Index < AllocateInfo->BufferCount;
            ++Index)
        {
            Shu_CommandBuffer->BufferHandles[Index].Handle = Buffers[Index];
            Shu_CommandBuffer->BufferHandles[Index].IsRecording = false;
        }

        LogOutput(LogType_Info, "%d Command Buffers created for Queue: (%s)\n", Shu_CommandBuffer->BufferCount,
                  GetQueueTypeName(AllocateInfo->QueueType));
    }
}

void
BeginCommandBuffer(shura_vulkan_command_buffer_handle *CmdBuffer, VkCommandBufferUsageFlags Usage,
                   VkCommandBufferInheritanceInfo *pInheritanace)
{
    VkCommandBufferBeginInfo BeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    BeginInfo.pNext = nullptr;
    BeginInfo.flags = Usage;
    BeginInfo.pInheritanceInfo = pInheritanace;
    VK_CHECK(vkBeginCommandBuffer(CmdBuffer->Handle, &BeginInfo));
    CmdBuffer->IsRecording = true;
}

void
BeginCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex,
                   VkCommandBufferUsageFlags Usage)
{
    ASSERT(InternalBufferIndex < Buffer->BufferCount);

    // Command Buffer should be in the reset state
    VkCommandBufferInheritanceInfo *pSecondaryBufferInfo = nullptr;
#if 0
    if(Buffer->BufferLevel == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        VkRenderPass RenderPass;
        u32 SubpassIndex = 0;
        VkFramebuffer FrameBuffer;

        SecondaryBufferInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        SecondaryBufferInfo->pNext = 0;
        SecondaryBufferInfo->renderPass = RenderPass;
        SecondaryBufferInfo->subpass = SubpassIndex;
        SecondaryBufferInfo->framebuffer = FrameBuffer;
        SecondaryBufferInfo->occlusionQueryEnable = VK_FALSE; // TODO)): Dont Know
        SecondaryBufferInfo->queryFlags = 0; // TODO)): Dont Know
        SecondaryBufferInfo->pipelineStatistics = 0; // TODO)): Dont Know
    }
#endif

    BeginCommandBuffer(&Buffer->BufferHandles[InternalBufferIndex], Usage, pSecondaryBufferInfo);
    LogOutput(LogType_Info, "Command Buffer Has Begun Recording!\n");
}

void
EndCommandBuffer(shura_vulkan_command_buffer_handle *CmdBuffer)
{
    VK_CHECK(vkEndCommandBuffer(CmdBuffer->Handle));
    CmdBuffer->IsRecording = false;
}

void
EndCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex)
{
    ASSERT(InternalBufferIndex < Buffer->BufferCount);
    EndCommandBuffer(&Buffer->BufferHandles[InternalBufferIndex]);
    LogOutput(LogType_Info, "Command Buffer Recording has ended!\n");
}
