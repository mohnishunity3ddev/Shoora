#include "vulkan_command_buffer.h"
#include "platform/platform.h"
#include "vulkan_device.h"

void
AllocateCommandBuffers(shura_vulkan_context *Context, shura_command_buffer_allocate_info *AllocInfos,
                       u32 AllocInfoCount)
{
    for(u32 Index = 0;
        Index < AllocInfoCount;
        ++Index)
    {
        shura_command_buffer_allocate_info *AllocateInfo = AllocInfos + Index;

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

        VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &VkAllocInfo,
                                          Shu_CommandBuffer->BufferHandles));
        
        LogOutput(LogType_Info, "%d Command Buffers created for Queue: (%s)\n", Shu_CommandBuffer->BufferCount,
                  GetQueueTypeName(AllocateInfo->QueueType));
    }
}

void
BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags Usage,
                   VkCommandBufferInheritanceInfo *pInheritanace, b32 *pIsRecording)
{
    VkCommandBufferBeginInfo BeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    BeginInfo.pNext = nullptr;
    BeginInfo.flags = Usage;
    BeginInfo.pInheritanceInfo = pInheritanace;
    VK_CHECK(vkBeginCommandBuffer(CommandBuffer, &BeginInfo));
    *pIsRecording = true;
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

    BeginCommandBuffer(Buffer->BufferHandles[InternalBufferIndex], Usage, pSecondaryBufferInfo,
                       &Buffer->RecordingBuffers[InternalBufferIndex]);
    LogOutput(LogType_Info, "Command Buffer Has Begun Recording!\n");
}

void
EndCommandBuffer(VkCommandBuffer Buffer, b32 *pIsRecording)
{
    VK_CHECK(vkEndCommandBuffer(Buffer));
    *pIsRecording = false;
}

void
EndCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex)
{
    ASSERT(InternalBufferIndex < Buffer->BufferCount);
    EndCommandBuffer(Buffer->BufferHandles[InternalBufferIndex],
                     &Buffer->RecordingBuffers[InternalBufferIndex]);
    LogOutput(LogType_Info, "Command Buffer Recording has ended!\n");
}
