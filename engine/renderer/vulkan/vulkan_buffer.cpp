#include "vulkan_buffer.h"
#include <memory>

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

shoora_buffer_info
CreateBuffer(shoora_vulkan_device *RenderDevice, VkBufferUsageFlags Usage, VkSharingMode SharingMode,
             VkMemoryPropertyFlags DesiredMemoryType, u8 *pData, size_t DataSize)
{
    shoora_buffer_info Result;

    VkBuffer Buffer;
    VkDeviceMemory Memory;
    VkBufferCreateInfo BufferCreateInfo;
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.flags = 0;
    BufferCreateInfo.size = DataSize;
    BufferCreateInfo.usage = Usage;
    BufferCreateInfo.sharingMode = SharingMode;
    // This buffer is not shared across queues.
    BufferCreateInfo.queueFamilyIndexCount = 0;
    BufferCreateInfo.pQueueFamilyIndices = nullptr;

    VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &BufferCreateInfo, nullptr, &Buffer));

    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, Buffer, &MemRequirements);

    VkMemoryAllocateInfo MemoryAllocateInfo;
    MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemoryAllocateInfo.pNext = nullptr;
    MemoryAllocateInfo.allocationSize = MemRequirements.size;
    MemoryAllocateInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, MemRequirements.memoryTypeBits,
                                                             DesiredMemoryType);

    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &MemoryAllocateInfo, nullptr, &Memory));

    if(pData != nullptr)
    {
        ASSERT(DesiredMemoryType != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        u8 *pBufferData;
        VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, Memory, 0, MemRequirements.size, 0,
                            (void **)&pBufferData));
        memcpy(pBufferData, pData, DataSize);
        vkUnmapMemory(RenderDevice->LogicalDevice, Memory);
        pBufferData = nullptr;
    }

    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, Buffer, Memory, 0));

    Result.Buffer = Buffer;
    Result.Memory = Memory;

    return Result;
}

void
CopyBuffers(shoora_vulkan_device *RenderDevice, VkBuffer *SrcBuffers, VkBuffer *DstBuffers, size_t *CopySizes,
            u32 BufferCount)
{
    VkCommandBuffer CopyBuffer;

    VkCommandBufferAllocateInfo AllocInfo;
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.pNext = nullptr;
    AllocInfo.commandPool = RenderDevice->TransferCommandPoolTransient;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &AllocInfo,
                                      &CopyBuffer));

    VkCommandBufferBeginInfo BeginInfo;
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.pNext = nullptr;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // This is not a secondary command buffer
    BeginInfo.pInheritanceInfo = nullptr;

    VK_CHECK(vkBeginCommandBuffer(CopyBuffer, &BeginInfo));

        VkBufferCopy CopyRegion;
        CopyRegion.srcOffset = 0;
        CopyRegion.dstOffset = 0;

        for(u32 Index = 0;
            Index < BufferCount;
            ++Index)
        {
            CopyRegion.size = CopySizes[Index];
            vkCmdCopyBuffer(CopyBuffer, SrcBuffers[Index], DstBuffers[Index], 1, &CopyRegion);
        }

    VK_CHECK(vkEndCommandBuffer(CopyBuffer));

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CopyBuffer;

    VkFence Fence;
    VkFenceCreateInfo FenceCreateInfo = {};
    FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(RenderDevice->LogicalDevice, &FenceCreateInfo, nullptr, &Fence));

    VK_CHECK(vkQueueSubmit(GetQueueHandle(RenderDevice, QueueType_Transfer), 1, &SubmitInfo, Fence));
    VK_CHECK(vkWaitForFences(RenderDevice->LogicalDevice, 1, &Fence, VK_TRUE, SHU_DEFAULT_FENCE_TIMEOUT));

    vkDestroyFence(RenderDevice->LogicalDevice, Fence, nullptr);
    vkFreeCommandBuffers(RenderDevice->LogicalDevice, RenderDevice->TransferCommandPoolTransient, 1, &CopyBuffer);
}

void
CreateVertexBuffer(shoora_vulkan_device *RenderDevice, shoora_vertex_info *Vertices, u32 VertexCount, u32 *Indices,
                   u32 IndexCount, shoora_vulkan_buffer *outVertexBuffer, shoora_vulkan_buffer *outIndexBuffer)
{
    size_t RequiredVertexBufferSize = VertexCount*sizeof(shoora_vertex_info);
    size_t RequiredIndexBufferSize = IndexCount*sizeof(u32);

    shoora_buffer_info StagingVertexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_SHARING_MODE_EXCLUSIVE,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                          (u8 *)Vertices, RequiredVertexBufferSize);
    shoora_buffer_info VertexBuffer = CreateBuffer(RenderDevice,
                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                   nullptr, RequiredVertexBufferSize);

    shoora_buffer_info StagingIndexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_SHARING_MODE_EXCLUSIVE,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                          (u8 *)Indices, RequiredIndexBufferSize);
    shoora_buffer_info IndexBuffer = CreateBuffer(RenderDevice,
                                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                   nullptr, RequiredIndexBufferSize);

    VkBuffer SourceBuffers[] = {StagingVertexBuffer.Buffer, StagingIndexBuffer.Buffer};
    VkBuffer DestinationBuffers[] = {VertexBuffer.Buffer, IndexBuffer.Buffer};
    size_t CopySizes[] = {RequiredVertexBufferSize, RequiredIndexBufferSize};
    CopyBuffers(RenderDevice, SourceBuffers, DestinationBuffers, CopySizes, ARRAY_SIZE(SourceBuffers));

    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingVertexBuffer.Buffer, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, StagingVertexBuffer.Memory, nullptr);
    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingIndexBuffer.Buffer, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, StagingIndexBuffer.Memory, nullptr);

    outVertexBuffer->Handle = VertexBuffer.Buffer;
    outVertexBuffer->Memory = VertexBuffer.Memory;

    outIndexBuffer->Handle = IndexBuffer.Buffer;
    outIndexBuffer->Memory = IndexBuffer.Memory;

    LogOutput(LogType_Info, "Created Vertex buffer and Index buffers\n");
}

// TODO)): Instead of creating 4 separate uniform buffers, create a single one with each having an offset.
void
CreateUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffers,
                     u32 UniformBufferCount, size_t Size)
{
    VkMemoryRequirements MemReqs;

    VkBufferCreateInfo BufferInfo = {};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkMemoryAllocateInfo MemAllocInfo = {};
    MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    for(u32 Index = 0;
        Index < UniformBufferCount;
        ++Index)
    {
        shoora_vulkan_buffer *pUniformBuffer = pUniformBuffers + Index;
        VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &BufferInfo, nullptr, &pUniformBuffer->Handle));

        vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, pUniformBuffer->Handle, &MemReqs);

        MemAllocInfo.allocationSize = MemReqs.size;
        MemAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, MemReqs.memoryTypeBits,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        pUniformBuffer->MemSize = Size;

        VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &MemAllocInfo, nullptr,
                                  &pUniformBuffer->Memory));
        VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, pUniformBuffer->Handle, pUniformBuffer->Memory, 0));
        VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, pUniformBuffer->Memory, 0, MemAllocInfo.allocationSize,
                             0, (void **)&pUniformBuffer->pMapped));
    }

    LogOutput(LogType_Info, "Created Uniform Buffers\n");
}

void
DestroyUniformBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffer)
{
    vkUnmapMemory(RenderDevice->LogicalDevice, pUniformBuffer->Memory);
    pUniformBuffer->pMapped = nullptr;

    vkFreeMemory(RenderDevice->LogicalDevice, pUniformBuffer->Memory, nullptr);
    pUniformBuffer->Memory = VK_NULL_HANDLE;

    vkDestroyBuffer(RenderDevice->LogicalDevice, pUniformBuffer->Handle, nullptr);
    pUniformBuffer->Handle = VK_NULL_HANDLE;
}

void
DestroyUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffers,
                      u32 UniformBufferCount)
{
    for(u32 Index = 0;
        Index < UniformBufferCount;
        ++Index)
    {
        shoora_vulkan_buffer *pUniformBuffer = pUniformBuffers + Index;
        DestroyUniformBuffer(RenderDevice, pUniformBuffer);
    }

    LogOutput(LogType_Warn, "Destroyed Uniform Buffers\n");
}

void
DestroyVertexBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pVertexBuffer,
                    shoora_vulkan_buffer *pIndexBuffer)
{
    vkDestroyBuffer(RenderDevice->LogicalDevice, pVertexBuffer->Handle, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, pVertexBuffer->Memory, nullptr);

    vkDestroyBuffer(RenderDevice->LogicalDevice, pIndexBuffer->Handle, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, pIndexBuffer->Memory, nullptr);

    LogOutput(LogType_Warn, "Destroyed Vertex and index buffers\n");
}