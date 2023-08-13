#include "vulkan_buffer.h"

// TODO)): Refactor
void
CreateBuffer(shoora_vulkan_device *RenderDevice)
{
    VkDeviceSize SizeInBytes;
    VkBufferUsageFlags Usage;
    VkBufferCreateFlags CreateFlags;

    // NOTE: EXLUSIVE_SHARING_MODE involves an ownership transfer where we tell the driver that this buffer should be
    // allowed to be used in queues from a different queue family.
    VkSharingMode SharingMode;

    VkBufferCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = CreateFlags;
    CreateInfo.size = SizeInBytes;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = SharingMode;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = nullptr;

    VkBuffer Buffer = VK_NULL_HANDLE;
    VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &CreateInfo, 0, &Buffer));
    ASSERT(Buffer != VK_NULL_HANDLE);
}

// TODO)): Refactor!
void
AllocateBufferMemory(shoora_vulkan_device *RenderDevice)
{
    VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(RenderDevice->PhysicalDevice, &PhysicalDeviceMemoryProperties);

    VkBuffer Buffer;
    VkMemoryRequirements MemoryRequirements = {};
    vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, Buffer, &MemoryRequirements);
    
    VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
    VkMemoryPropertyFlagBits MemoryProperties = {};

    // Iterate over Physical Device Memory Properties to see if the memory requirements for our buffer are
    // satisfied.
    for(u32 MemoryType = 0;
        MemoryType < PhysicalDeviceMemoryProperties.memoryTypeCount;
        ++MemoryType)
    {
        if ((MemoryRequirements.memoryTypeBits & (1 << MemoryType)) &&
            ((PhysicalDeviceMemoryProperties.memoryTypes[MemoryType].propertyFlags & MemoryProperties) ==
             MemoryProperties))
        {
            VkMemoryAllocateInfo BufferMemoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            BufferMemoryAllocateInfo.pNext = nullptr;
            BufferMemoryAllocateInfo.allocationSize = MemoryRequirements.size;
            BufferMemoryAllocateInfo.memoryTypeIndex = MemoryType;

            VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &BufferMemoryAllocateInfo, 0, &DeviceMemory));

            break;
        }
        else
        {
            continue;
        }
    }

    ASSERT(DeviceMemory != VK_NULL_HANDLE);

    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, Buffer, DeviceMemory, 0));
}


// TODO)): Refactor!
// NOTE: We must specify to the driver the usage pattern of the buffer during its creation and also when its about
// to be used when we change the usage of the buffer, we have to set the "BufferMemoryBarrier"
void
SetBufferMemoryBarrier(shoora_buffer_transition_info *BufferTransitionInfos, u32 BufferTransitionCount)
{
    // TODO)): If QueueFamily is getting changed for the buffer. Assert that the buffer's sharing mode was set to
    // exclusive mode
    // TODO)): ASSERT that the current Access is not the same as the new access for all transitions sent here.

    VkBufferMemoryBarrier MemoryBarriers[16];
    VkBuffer Buffer = VK_NULL_HANDLE;

    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    // TODO)): ASSERT that the Command Buffer here is in the recording state.

    for(u32 Index = 0;
        Index < BufferTransitionCount;
        ++Index)
    {
        shoora_buffer_transition_info TransitionInfo = BufferTransitionInfos[Index];

        VkBufferMemoryBarrier MemoryBarrier = MemoryBarriers[Index];
        MemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        MemoryBarrier.pNext = nullptr;
        MemoryBarrier.srcAccessMask = TransitionInfo.CurrentAccess;
        MemoryBarrier.dstAccessMask = TransitionInfo.NewAccess;
        MemoryBarrier.srcQueueFamilyIndex = TransitionInfo.CurrentQueueFamily;
        MemoryBarrier.dstQueueFamilyIndex = TransitionInfo.NewQueueFamily;
        MemoryBarrier.buffer = Buffer;
        MemoryBarrier.offset = 0;
        MemoryBarrier.size = VK_WHOLE_SIZE;
    }

    // NOTE: Stages which were using the buffer so far
    VkPipelineStageFlags GeneratingStages = {};
    // NOTE: Stages where the buffer will be used AFTER the barrier has been created/set.
    VkPipelineStageFlags ConsumingStages = {};

    vkCmdPipelineBarrier(CommandBuffer, GeneratingStages, ConsumingStages, 0, 0, nullptr, BufferTransitionCount,
                         MemoryBarriers, 0, nullptr);
}

// TODO)): Refactor
void
CreateBufferView(shoora_vulkan_device *RenderDevice)
{
    // TODO)): An already created buffer passed into this function
    // TODO)): ASSERT that the buffer is not a VK_NULL_HANDLE
    VkBuffer Buffer;

    // TODO)): How the buffer's contents should be interpreted.
    VkFormat Format;

    // TODO)): Select part of the buffer's memory which we want to create a view of.
    VkDeviceSize Offset = 0;
    // TODO)): Size of the buffer
    VkDeviceSize Range = 0;

    VkBufferViewCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.buffer = Buffer;
    CreateInfo.format = Format;
    CreateInfo.offset = Offset;
    CreateInfo.range = Range;

    VkBufferView View = VK_NULL_HANDLE;
    VK_CHECK(vkCreateBufferView(RenderDevice->LogicalDevice, &CreateInfo, 0, &View));
    ASSERT(View != VK_NULL_HANDLE);
}

void
CopyDataBetweenBuffers()
{
    // TODO)): Make sure the command buffer is in a recording state
    VkCommandBuffer CommandBuffer;

    VkBuffer SourceBuffer = {};
    ASSERT(SourceBuffer != VK_NULL_HANDLE);

    VkBuffer DestinationBuffer = {};
    ASSERT(DestinationBuffer != VK_NULL_HANDLE);

    VkBufferCopy Region;
    Region.srcOffset = 0;
    Region.dstOffset = 0;
    Region.size = VkDeviceSize{1920};

    vkCmdCopyBuffer(CommandBuffer, SourceBuffer, DestinationBuffer, 1, &Region);


}