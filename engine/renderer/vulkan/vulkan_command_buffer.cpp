#include "vulkan_command_buffer.h"
#include "platform/platform.h"
#include "vulkan_device.h"

VkCommandBuffer
CreateTransientCommandBuffer(shoora_vulkan_device *RenderDevice, VkCommandPool CommandPool,
                             VkCommandBufferLevel Level, b32 Begin)
{
    VkCommandBufferAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.pNext = nullptr;
    AllocInfo.commandPool = CommandPool;
    AllocInfo.level = Level;
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer CmdBuffer;
    VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &AllocInfo, &CmdBuffer));

    if(Begin)
    {
        VkCommandBufferBeginInfo BeginInfo = {};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        BeginInfo.pNext = nullptr;
        BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(CmdBuffer, &BeginInfo));
    }

    return CmdBuffer;
}

void
FlushTransientCommandBuffer(shoora_vulkan_device *RenderDevice, VkCommandBuffer CmdBuffer, VkQueue Queue,
                            VkCommandPool Pool, b32 Free)
{
    if(CmdBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    VK_CHECK(vkEndCommandBuffer(CmdBuffer));

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CmdBuffer;

    VkFenceCreateInfo FenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence Fence;
    VK_CHECK(vkCreateFence(RenderDevice->LogicalDevice, &FenceInfo, nullptr, &Fence));

    VK_CHECK(vkQueueSubmit(Queue, 1, &SubmitInfo, Fence));
    VK_CHECK(vkWaitForFences(RenderDevice->LogicalDevice, 1, &Fence, VK_TRUE, SHU_DEFAULT_FENCE_TIMEOUT));
    
    vkDestroyFence(RenderDevice->LogicalDevice, Fence, nullptr);

    if(Free)
    {
        vkFreeCommandBuffers(RenderDevice->LogicalDevice, Pool, 1, &CmdBuffer);
    }
}

#if 0
shoora_vulkan_command_buffer_handle *
GetCommandBufferGroupForQueue(shoora_vulkan_context *Context, shoora_queue_type Type)
{
    // ASSERT(Type < SHU_VK_MAX_QUEUE_FAMILY_COUNT);
    // NOTE: This will fire if during the vulkan device setup, this queue was not asked!
    // ASSERT(Type < Context->Device.QueueFamilyCount);

    // shoora_vulkan_command_buffer_handle *pCmdBufferGroup = &Context->CommandBuffers[Type];
    // return pCmdBufferGroup;
    return nullptr;
}

void
AllocateCommandBuffers(shoora_vulkan_context *Context, shoora_command_buffer_allocate_info *AllocInfos,
                       u32 AllocInfoCount)
{
    // ASSERT(AllocInfoCount < SHU_VK_MAX_QUEUE_FAMILY_COUNT);

    // for(u32 Index = 0;
    //     Index < AllocInfoCount;
    //     ++Index)
    // {
    //     shoora_command_buffer_allocate_info *AllocateInfo = AllocInfos + Index;
    //     ASSERT(AllocateInfo->BufferCount < SHU_VK_MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT);

    //     u32 QueueIndex = GetQueueIndexFromType((shoora_queue_type)AllocateInfo->QueueType);
    //     shoora_vulkan_device *RenderDevice = &Context->Device;

    //     VkCommandBufferAllocateInfo VkAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    //     VkAllocInfo.commandPool = RenderDevice->CommandPools[QueueIndex];
    //     VkAllocInfo.level = AllocateInfo->Level;
    //     VkAllocInfo.commandBufferCount = AllocateInfo->BufferCount;

    //     shoora_vulkan_command_buffer_handle *Shu_CommandBuffer = Context->CommandBuffers + QueueIndex;
    //     Shu_CommandBuffer->QueueType = AllocateInfo->QueueType;
    //     Shu_CommandBuffer->BufferCount = AllocateInfo->BufferCount;
    //     Shu_CommandBuffer->BufferLevel = AllocateInfo->Level;

    //     VkCommandBuffer Buffers[SHU_VK_MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    //     for(u32 Index = 0;
    //         Index < AllocateInfo->BufferCount;
    //         ++Index)
    //     {
    //         Buffers[Index] = Shu_CommandBuffer->BufferHandles[Index].Handle;
    //     }

    //     // NOTE: vkAllocateCommandBuffers expects an array of just VkCommandBuffer handles. But we have an array of a struct
    //     // which also has the recording state of this command buffer. That's why I need to have this Buffers Intermediate array
    //     // of just VkCommandBuffer/
    //     // TODO)): Check if there is a better way without using this intermediate array.
    //     VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &VkAllocInfo,
    //                                       Buffers));

    //     for(u32 Index = 0;
    //         Index < AllocateInfo->BufferCount;
    //         ++Index)
    //     {

    //         shoora_vulkan_command_buffer_handle_handle *BufferHandle = &Shu_CommandBuffer->BufferHandles[Index];
    //         BufferHandle->Handle = Buffers[Index];
    //         BufferHandle->IsRecording = false;
    //         BufferHandle->CommandPool = &RenderDevice->CommandPools[QueueIndex];
    //     }

    //     LogOutput(LogType_Info, "%d Command Buffers created for Queue: (%s)\n", Shu_CommandBuffer->BufferCount,
    //               GetQueueTypeName(AllocateInfo->QueueType));
    // }
}

void
BeginCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBuffer, VkCommandBufferUsageFlags Usage,
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
BeginCommandBuffer(shoora_vulkan_command_buffer *BufferGroup, u32 InternalBufferIndex,
                   VkCommandBufferUsageFlags Usage)
{
    ASSERT(InternalBufferIndex < BufferGroup->BufferCount);

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

    BeginCommandBuffer(&BufferGroup->BufferHandles[InternalBufferIndex], Usage, pSecondaryBufferInfo);
    LogOutput(LogType_Info, "Command Buffer associated with Queue(%s) Has Begun Recording!\n",
              GetQueueTypeName(BufferGroup->QueueType));
}

void
EndCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBuffer)
{
    VK_CHECK(vkEndCommandBuffer(CmdBuffer->Handle));
    CmdBuffer->IsRecording = false;
}

void
EndCommandBuffer(shoora_vulkan_command_buffer *BufferGroup, u32 InternalBufferIndex)
{
    ASSERT(InternalBufferIndex < BufferGroup->BufferCount);
    EndCommandBuffer(&BufferGroup->BufferHandles[InternalBufferIndex]);
    LogOutput(LogType_Info, "Command Buffer associated with Queue(%s) Recording has ended!\n",
              GetQueueTypeName(BufferGroup->QueueType));
}

void
ResetCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBufferHandle, b32 ReleaseResources)
{
    VkCommandBufferResetFlags Flags = ReleaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0;
    VK_CHECK(vkResetCommandBuffer(CmdBufferHandle->Handle, Flags));
}

void
ResetCommandBuffer(shoora_vulkan_command_buffer *BufferGroup, u32 InternalBufferIndex, b32 ReleaseResources)
{
    ASSERT(InternalBufferIndex < BufferGroup->BufferCount);
    ResetCommandBuffer(&BufferGroup->BufferHandles[InternalBufferIndex], ReleaseResources);
}

void
FreeCommandBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_command_buffer_handle *CmdBuffer)
{
    vkFreeCommandBuffers(RenderDevice->LogicalDevice, *CmdBuffer->CommandPool, 1, &CmdBuffer->Handle);
}

void
FreeAllCommandBuffers(shoora_vulkan_context *Context)
{
    // shoora_vulkan_device *RenderDevice = &Context->Device;
    // for(u32 QueueTypeIndex = 0;
    //     QueueTypeIndex < RenderDevice->QueueFamilyCount;
    //     ++QueueTypeIndex)
    // {
    //     shoora_vulkan_command_buffer *CmdBuffer = Context->CommandBuffers + QueueTypeIndex;

    //     ASSERT(CmdBuffer->BufferCount > 0);
    //     ASSERT(CmdBuffer->QueueType == (shoora_queue_type)QueueTypeIndex);

    //     VkCommandBuffer IntermediateCommandBuffers[SHU_VK_MAX_COMMAND_BUFFERS_PER_QUEUE_COUNT];
    //     for(u32 Index2 = 0;
    //         Index2 < CmdBuffer->BufferCount;
    //         ++Index2)
    //     {

    //         // NOTE: A Command Buffer which is still recording commands and has not been ended should not be freed.
    //         if(CmdBuffer->BufferHandles[Index2].IsRecording)
    //         {
    //             LogOutput(LogType_Warn,
    //                       "Command Buffer for Queue(%s) Index(%d) has not been ended yet. Should "
    //                       "not be allowed to be freed. Still doing it anyway. Check your code brother!\n",
    //                       GetQueueTypeName(CmdBuffer->QueueType), Index2);
    //         }

    //         IntermediateCommandBuffers[Index2] = CmdBuffer->BufferHandles[Index2].Handle;
    //     }

    //     vkFreeCommandBuffers(RenderDevice->LogicalDevice, RenderDevice->CommandPools[QueueTypeIndex],
    //                          CmdBuffer->BufferCount, IntermediateCommandBuffers);
    // }
}
#endif