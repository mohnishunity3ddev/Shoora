#include "vulkan_buffer.h"
#include <memory>

shoora_vulkan_buffer
CreateBuffer(shoora_vulkan_device *RenderDevice, VkBufferUsageFlags Usage, VkSharingMode SharingMode,
                           VkMemoryPropertyFlags DesiredMemoryType, u8 *pData, size_t DataSize)
{
    shoora_vulkan_buffer Result;

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

    u8 *pBufferData;
    VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, Memory, 0, MemRequirements.size, 0, (void **)&pBufferData));

    if (pData != nullptr)
    {
        ASSERT(DesiredMemoryType != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
               "You cannot expect to provide data for the buffer and have a GPU Local Memory since you can't "
               "directly store data from the CPU to the GPU. You have to use a staging buffer to upload the data "
               "to GPU Local Memory.");
        memcpy(pBufferData, pData, DataSize);
        vkUnmapMemory(RenderDevice->LogicalDevice, Memory);
        pBufferData = nullptr;
    }

    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, Buffer, Memory, 0));

    Result =
    {
        .Handle = Buffer, .Memory = Memory,
        .MemSize = DataSize, .pMapped = pBufferData
    };

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

    auto StagingVertexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                            VK_SHARING_MODE_EXCLUSIVE,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                            (u8 *)Vertices, RequiredVertexBufferSize);
    auto VertexBuffer = CreateBuffer(RenderDevice,
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                     VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr,
                                     RequiredVertexBufferSize);

    auto StagingIndexBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_SHARING_MODE_EXCLUSIVE,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           (u8 *)Indices, RequiredIndexBufferSize);
    auto IndexBuffer = CreateBuffer(RenderDevice,
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr,
                                    RequiredIndexBufferSize);

    VkBuffer SourceBuffers[] = {StagingVertexBuffer.Handle, StagingIndexBuffer.Handle};
    VkBuffer DestinationBuffers[] = {VertexBuffer.Handle, IndexBuffer.Handle};
    size_t CopySizes[] = {RequiredVertexBufferSize, RequiredIndexBufferSize};
    CopyBuffers(RenderDevice, SourceBuffers, DestinationBuffers, CopySizes, ARRAY_SIZE(CopySizes));

    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingVertexBuffer.Handle, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, StagingVertexBuffer.Memory, nullptr);
    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingIndexBuffer.Handle, nullptr);
    vkFreeMemory(RenderDevice->LogicalDevice, StagingIndexBuffer.Memory, nullptr);

    outVertexBuffer->Handle = VertexBuffer.Handle;
    outVertexBuffer->Memory = VertexBuffer.Memory;

    outIndexBuffer->Handle = IndexBuffer.Handle;
    outIndexBuffer->Memory = IndexBuffer.Memory;

    LogOutput(LogType_Info, "Created Vertex buffer and Index buffers\n");
}

// TODO)): Instead of creating 4 separate uniform buffers, create a single one with each having an offset.
void
CreateUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffers,
                     u32 UniformBufferCount, size_t Size)
{
    LogInfoUnformatted("Inside Create Uniform Buffers!\n");
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
        LogInfo("Creating Uniform Buffers[%d]!\n", Index);

        vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, pUniformBuffer->Handle, &MemReqs);

        MemAllocInfo.allocationSize = MemReqs.size;

        MemAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, MemReqs.memoryTypeBits,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        pUniformBuffer->MemSize = Size;

        VK_CHECK_RESULT(vkAllocateMemory(RenderDevice->LogicalDevice, &MemAllocInfo, nullptr,
                                         &pUniformBuffer->Memory));
        LogInfoUnformatted("Allocation Complete!\n");

        VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, pUniformBuffer->Handle, pUniformBuffer->Memory, 0));
        VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, pUniformBuffer->Memory, 0, MemAllocInfo.allocationSize,
                             0, (void **)&pUniformBuffer->pMapped));
    }

    LogOutput(LogType_Info, "Created Uniform Buffers\n");
}

void
DestroyBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pBuffer)
{
    if(pBuffer->pMapped)
    {
        vkUnmapMemory(RenderDevice->LogicalDevice, pBuffer->Memory);
        pBuffer->pMapped = nullptr;
    }

    if(pBuffer->Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(RenderDevice->LogicalDevice, pBuffer->Memory, nullptr);
        pBuffer->Memory = VK_NULL_HANDLE;
    }

    if(pBuffer->Handle != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(RenderDevice->LogicalDevice, pBuffer->Handle, nullptr);
        pBuffer->Handle = VK_NULL_HANDLE;
    }

    pBuffer->MemSize = 0;
}

void
DestroyUniformBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffer)
{
    DestroyBuffer(RenderDevice, pUniformBuffer);
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
    DestroyBuffer(RenderDevice, pVertexBuffer);
    DestroyBuffer(RenderDevice, pIndexBuffer);

    LogOutput(LogType_Warn, "Destroyed Vertex and index buffers\n");
}