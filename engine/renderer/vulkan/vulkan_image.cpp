#include "vulkan_image.h"
#include "utils/utils.h"
#include <memory>

#if 0
#if !defined(STB_IMPORT)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMPORT
#endif

VkFormat
GetSuitableImageFormat(shoora_vulkan_device *RenderDevice, VkFormat *FormatCandidates, u32 FormatCount,
                       VkImageTiling Tiling, VkFormatFeatureFlags FormatUsage)
{
    VkFormat Result = VK_FORMAT_UNDEFINED;
    for (u32 Index = 0; Index < FormatCount; ++Index)
    {
        VkFormat Candidate = FormatCandidates[Index];

        VkFormatProperties FormatProps;
        vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Candidate, &FormatProps);

        if ((Tiling == VK_IMAGE_TILING_LINEAR &&
             ((FormatProps.linearTilingFeatures & FormatUsage) == FormatUsage)) ||
            (Tiling == VK_IMAGE_TILING_OPTIMAL &&
             ((FormatProps.optimalTilingFeatures & FormatUsage) == FormatUsage)))
        {
            Result = Candidate;
            break;
        }
    }

    return Result;
}

VkFormat
GetSuitableDepthAttachmentFormat(shoora_vulkan_device *RenderDevice)
{
    VkFormat DepthFormats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    VkFormat DepthFormat = GetSuitableImageFormat(RenderDevice, DepthFormats, ARRAY_SIZE(DepthFormats),
                                                  VK_IMAGE_TILING_OPTIMAL,
                                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    return DepthFormat;
}

void
GenerateMipMaps(const char *InputFilename, const char *OutputFilename, i32 MipLevelCount, i32 Quality,
                u64 *MipOffsets, b32 CapImageToFullHD)
{
    i32 ImageWidth, ImageHeight, BytesPerPixel;
    unsigned char *InputData = stbi_load(InputFilename, &ImageWidth, &ImageHeight, &BytesPerPixel, 0);
    if (!InputData)
    {
        fprintf(stderr, "Failed to load image: %s\n", InputFilename);
        return;
    }
    if (CapImageToFullHD && ImageWidth > 1920)
    {
        u8 *Temp = (u8 *)malloc(1920 * 1080 * BytesPerPixel);
        stbir_resize_uint8(InputData, ImageWidth, ImageHeight, 0, Temp, 1920, 1080, 0, BytesPerPixel);
        ImageWidth = 1920;
        ImageHeight = 1080;
        free(InputData);
        InputData = Temp;
    }

    i32 MipLevels = MipLevelCount;            // Example number of mip levels
    i32 CombinedWidth = (ImageWidth * 3) / 2; // Width of the combined image
    i32 CombinedHeight = ImageHeight;         // Height of the combined image
    i32 CombinedChannels = BytesPerPixel;     // Number of channels (e.g., 3 for RGB)
    i32 CombinedSize = CombinedWidth * CombinedHeight * CombinedChannels;

    u8 *CombinedData = (u8 *)malloc(CombinedSize);
    memset(CombinedData, 0, CombinedSize);

    if (!CombinedData)
    {
        fprintf(stderr, "Memory allocation failed for combined image\n");
        return;
    }

    int XOffset = 0;
    int YOffset = 0;

    for (i32 MipLevel = 0; MipLevel < MipLevels; ++MipLevel)
    {
        MipOffsets[MipLevel] = (YOffset * CombinedWidth + XOffset) * CombinedChannels;

        int MipWidth = ImageWidth >> MipLevel;
        int MipHeight = ImageHeight >> MipLevel;

        if (MipWidth < 1 || MipHeight < 1)
        {
            break;
        }

        u8 *MipmapData = (u8 *)malloc(MipWidth * MipHeight * CombinedChannels);
        if (!MipmapData)
        {
            fprintf(stderr, "Memory allocation failed for mip level %d\n", MipLevel);
            return;
        }

        stbir_resize_uint8(InputData, ImageWidth, ImageHeight, 0, MipmapData, MipWidth, MipHeight, 0,
                           CombinedChannels);

        i32 Y = 0;
        for (; Y < MipHeight; ++Y)
        {
            for (i32 X = 0; X < MipWidth; ++X)
            {
                for (i32 C = 0; C < CombinedChannels; ++C)
                {
                    CombinedData[((Y + YOffset) * CombinedWidth + XOffset + X) * CombinedChannels +
                                 C] = MipmapData[(Y * MipWidth + X) * CombinedChannels + C];
                }
            }
        }

        ASSERT((MipHeight + YOffset) <= CombinedHeight);

        if ((MipHeight + YOffset) == CombinedHeight)
        {
            XOffset += MipWidth;
            YOffset = 0;
        }
        else
        {
            YOffset += Y;
        }
        free(MipmapData);
    }

    // Save the combined image
    if (!stbi_write_jpg(OutputFilename, CombinedWidth, CombinedHeight, CombinedChannels, CombinedData, Quality))
    {
        fprintf(stderr, "Failed to write image: %s\n", OutputFilename);
    }

    // Cleanup
    stbi_image_free(InputData);
    free(CombinedData);
}

// TODO)): Refactor!
void
CreateImage(shoora_vulkan_device *RenderDevice)
{
    VkImageType ImageType = {};
    VkFormat Format;

    VkExtent3D Size;
    u32 MipMapCount = 1;
    // NOTE: If the Image is a cubemap then this layer count should be 6.
    u32 LayerCount = 1;
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;

    VkImageUsageFlags Usage;

    VkImageCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.imageType = ImageType;
    CreateInfo.format = Format;
    CreateInfo.extent = Size;
    CreateInfo.mipLevels = MipMapCount;
    CreateInfo.arrayLayers = LayerCount;
    CreateInfo.samples = Samples;
    CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = nullptr;
    CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage Image = VK_NULL_HANDLE;
    vkCreateImage(RenderDevice->LogicalDevice, &CreateInfo, 0, &Image);
    ASSERT(Image != VK_NULL_HANDLE);
}

// TODO)): Refactor!
void
BindAllocateMemoryToImage(shoora_vulkan_device *RenderDevice)
{
    VkPhysicalDevice PhysicalDevice = RenderDevice->PhysicalDevice;

    VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);

    VkDevice LogicalDevice = RenderDevice->LogicalDevice;

    // TODO)): This Image has been already created.
    VkImage Image;
    ASSERT(Image != VK_NULL_HANDLE);
    VkMemoryRequirements ImageMemoryRequirements = {};
    vkGetImageMemoryRequirements(LogicalDevice, Image, &ImageMemoryRequirements);

    VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
    // TODO)): Required additional memory properties are stored here!
    VkMemoryPropertyFlagBits MemoryProperties;

    for(u32 MemoryType = 0;
        MemoryType < PhysicalDeviceMemoryProperties.memoryTypeCount;
        ++MemoryType)
    {
        if((ImageMemoryRequirements.memoryTypeBits & (1 << MemoryType)) &&
           ((PhysicalDeviceMemoryProperties.memoryTypes[MemoryType].propertyFlags & MemoryProperties) ==
             MemoryProperties))
        {
            VkMemoryAllocateInfo AllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            AllocateInfo.pNext = nullptr;
            AllocateInfo.allocationSize = ImageMemoryRequirements.size;
            AllocateInfo.memoryTypeIndex = MemoryType;
            VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &AllocateInfo, 0, &DeviceMemory));
            break;
        }
    }

    ASSERT(DeviceMemory != VK_NULL_HANDLE);
    // Binding the memory to the image buffer
    VK_CHECK(vkBindImageMemory(RenderDevice->LogicalDevice, Image, DeviceMemory, 0));
}

// TODO)): Refactor!
void
AllocateAndBindMemoryObjectToImage(shoora_image_transition *ImageTransitionInfos, u32 ImageTransitionCount)
{
    ASSERT(ImageTransitionCount <= 16);
    VkImageMemoryBarrier ImageMemoryBarriers[16];

    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    // TODO)): ASSERT(CommandBuffer != VK_NULL_HANDLE);
    // TODO)): ASSERT(CommandBuffer is in a recording state);

    for(u32 ImageTransitionIndex = 0;
        ImageTransitionIndex < ImageTransitionCount;
        ++ImageTransitionIndex)
    {
        shoora_image_transition ImageTransitionInfo = ImageTransitionInfos[ImageTransitionIndex];

        VkImageSubresourceRange SubresourceRange = {};
        SubresourceRange.aspectMask = ImageTransitionInfo.Aspect;
        SubresourceRange.baseMipLevel = 0;
        SubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        SubresourceRange.baseArrayLayer = 0;
        SubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        VkImageMemoryBarrier *ImageMemoryBarrier = &ImageMemoryBarriers[ImageTransitionIndex];
        ImageMemoryBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        ImageMemoryBarrier->pNext = nullptr;
        ImageMemoryBarrier->srcAccessMask = ImageTransitionInfo.CurrentAccess;
        ImageMemoryBarrier->dstAccessMask = ImageTransitionInfo.NewAccess;
        ImageMemoryBarrier->oldLayout = ImageTransitionInfo.CurrentLayout;
        ImageMemoryBarrier->newLayout = ImageTransitionInfo.NewLayout;
        ImageMemoryBarrier->srcQueueFamilyIndex = ImageTransitionInfo.CurrentQueueFamily;
        ImageMemoryBarrier->dstQueueFamilyIndex = ImageTransitionInfo.NewQueueFamily;
        ImageMemoryBarrier->image = ImageTransitionInfo.Image;
        ImageMemoryBarrier->subresourceRange = SubresourceRange;
    }

    // TODO)): These two should be provided to this function by the caller.
    // Stages where the image is being referenced currently before the barrier.
    VkPipelineStageFlags GeneratingStages = {};
    // Stages where the image will be referenced after the barrier.
    VkPipelineStageFlags ConsumingStages = {};

    vkCmdPipelineBarrier(CommandBuffer, GeneratingStages, ConsumingStages, 0, 0, nullptr, 0, nullptr,
                         ImageTransitionCount, ImageMemoryBarriers);
}

// TODO)): Refactor!
void
CreateImageView(shoora_vulkan_device *RenderDevice)
{
    // TODO)): This is a created image
    VkImage Image;
    VkImageViewType ViewType;
    // NOTE: This does not necessarily be the same as the format of the image this view will be of.
    // Maybe you want to reinterpret the image data through this view.
    VkFormat ViewFormat;
    // TODO)): This is also set beforehand
    VkImageAspectFlags Aspect;

    VkImageSubresourceRange SubresourceRange = {
        .aspectMask = Aspect,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

    VkImageViewCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.image = Image;
    CreateInfo.viewType = ViewType;
    CreateInfo.format = ViewFormat;
    CreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    CreateInfo.subresourceRange = SubresourceRange;

    // TODO)): This should come from the caller.
    VkImageView ImageView = VK_NULL_HANDLE;
    VK_CHECK(vkCreateImageView(RenderDevice->LogicalDevice, &CreateInfo, 0, &ImageView));
    ASSERT(ImageView != VK_NULL_HANDLE);
}

// NOTE: Example Code for 2D Image and view
// 2D Textures with RGBA Components and 32 bits per texel
void
ExampleImage2DAndView(shoora_vulkan_device *RenderDevice)
{
    // For a normal 2D Texture
    VkImageType Type = VK_IMAGE_TYPE_2D;
    VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
    u32 MipMapCount = 1;
    u32 LayerCount = 1;
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageUsageFlagBits Usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkExtent3D Size = {1920, 1080, 0};

    // For a 2D Texture and Cupemap view
    // Image LayerCount will be 6 for the 6 faces of the cubemap
    // View Type will be VK_IMAGE_VIEW_TYPE_CUBE


    VkImage Image;

    VkImageCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.imageType = Type;
    CreateInfo.format = Format;
    CreateInfo.extent = Size;
    CreateInfo.mipLevels = MipMapCount;
    CreateInfo.arrayLayers = LayerCount;
    CreateInfo.samples = Samples;
    CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = nullptr;
    CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(RenderDevice->LogicalDevice, &CreateInfo, 0, &Image));
}

#define KILOBYTES(Num) Num*1024
#include <memory.h>
// Images, Buffers should be in the Device-Local Memory which is inside the GPU and Hence fast
// But the app running on the CPU does not have any access to it. So we get some memory in host-visible space
// and map to that GPU memory. It is also good to use this host visible memory as staging resource which helps us
// in transferring data from the CPU to GPU and back(maybe?)
void
MapUnmapHostVisibleMemory(shoora_vulkan_device *RenderDevice)
{
    VkDeviceMemory HostVisibleMemory;
    // Offset into the host visible memory
    VkDeviceSize Offset = {};
    VkDeviceSize DataSize = KILOBYTES(24);

    // TODO)): Pointer to the beginning of the data
    void *Data = 0;

    // We could keep this mapped memory pointer for the duration of the app and not unmap like we did here.
    // but before destroying the renderer and releasing resources at the end, we need to Unmap this memory.
    void *PtrToMappedMemory;

    VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, HostVisibleMemory, Offset, DataSize, 0, &PtrToMappedMemory));

    // Copy the data to GPU Memory
    memcpy(PtrToMappedMemory, Data, DataSize);

    VkMappedMemoryRange MemoryRanges[16];

    // TODO)): Do this for all mapped memory ranges
    VkMappedMemoryRange MemoryRange = MemoryRanges[0];
    MemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    MemoryRange.pNext = nullptr;
    MemoryRange.memory = HostVisibleMemory;
    MemoryRange.offset = Offset;
    MemoryRange.size = DataSize;

    // Inform the driver which parts of the memory  have changed.
    // We need to inform so taht the memory changes are instantly visible to the GPU. This act of notifying of data
    // change from the CPU is called flushing.
    VK_CHECK(vkFlushMappedMemoryRanges(RenderDevice->LogicalDevice, 1, MemoryRanges));

    // Unmap the memory
    vkUnmapMemory(RenderDevice->LogicalDevice, HostVisibleMemory);
}

// TODO)): Refactor!
// TODO)): Make sure the buffers are set for the usage intended.
// TODO)): The Destination Buffer should have usage VK_ACCESS_TRANSFER_WRITE_BIT set using a Memory Barrier or
// something!
// TODO)): After the copy operation
// NOTE: Can only be used for buffers in Host Visible Memory
void
CopyDataBetweenImageBuffers()
{
    // TODO)): Check if in recording state
    VkCommandBuffer CmdBuffer;

    VkBuffer SrcBuffer;
    VkBuffer DstBuffer;

    VkBufferCopy Regions[16];
    u32 RegionCount = 0;

    vkCmdCopyBuffer(CmdBuffer, SrcBuffer, DstBuffer, RegionCount, Regions);
}



void
CopyBufferToImage(shoora_vulkan_device *RenderDevice, VkBuffer SrcBuffer, VkImage DstImage)
{
    // TODO)): Make sure that the command buffer is in a recording state
    VkCommandBuffer CommandBuffer;
    VkImageLayout CurrentLayout;
    VkImageAspectFlags AspectMask;

    // TODO)): Buffer should have USAGE as TRANSFER_SRC_BIT
    // TODO)): Image should have USAGE TRANSFER_DST_BIT
    // TODO)):  Image should be transitioned to a layout which is good for such a transfer operation.
    //          that is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // TODO)):  We should apply a memory barrier for the image so that it changes the layout to the preferred one as mentioned above.
    //          Memory Access Type should be VK_ACCESS_TRANSFER_WRITE

    VkBufferImageCopy Regions[16];
    for(u32 Index = 0;
        Index < 16;
        ++Index)
    {
        //  The layout of the image

        // Offset from the start of the buffer
        VkDeviceSize BufferOffset;

        // Represents the data size of a row of the buffer. 0 if the data is tightly packed(acoording to the size
        // of the dest image).
        u32 BufferRowLength;

        // The Height of the imaginary image that is stored in the buffer. 0 if its tightly packed(according to the
        // size of the dest image).
        VkDeviceSize BufferImageHeight;
        // Index of the miplevel which is to be updated.
        u32 ImageMipLevel = 0;
        // 6 if the image is supposed to be for a cubemap.
        u32 ImageLayerCount = 0;
        u32 FirstArrayLayer = 0;

        VkImageSubresourceLayers SubresourceLayers;
        SubresourceLayers.aspectMask = AspectMask;
        SubresourceLayers.mipLevel = ImageMipLevel;
        SubresourceLayers.baseArrayLayer = FirstArrayLayer;
        SubresourceLayers.layerCount = ImageLayerCount;

        // Initial offset for the image sub region which is to be updated.
        VkOffset3D ImageOffsetInTexels = {};

        //  the size of the resultant image.
        VkExtent3D ImageExtent = {};

        VkBufferImageCopy Region = Regions[Index];
        Region.bufferOffset = BufferOffset;
        Region.bufferRowLength = BufferRowLength;
        Region.bufferImageHeight = BufferImageHeight;
        Region.imageSubresource = SubresourceLayers;
        Region.imageOffset = ImageOffsetInTexels;
        Region.imageExtent = ImageExtent;
    }

    vkCmdCopyBufferToImage(CommandBuffer, SrcBuffer, DstImage, CurrentLayout, 16, Regions);

    // TODO)): Transition the image layout now that the image has been created to a layout which is good for the operations
    //          it is supposed to take part in
    // TODO)): We also need to through a memory barrier, should change the AccessType for this image from TRANSFER_WRITE_BIT to
    //          the one which is a more suitable one according to the type of operations its going to take part in.
}

void
CopyImageToBuffer(shoora_vulkan_device *RenderDevice, VkImage SrcImage, VkBuffer DstBuffer)
{
    // TODO)): This has to be in recording state.
    VkCommandBuffer CmdBuffer;

    VkImageLayout CurrentImageLayout;

    VkBufferImageCopy Regions[16];
    for (u32 Index = 0; Index < 16; ++Index)
    {
        //  The layout of the image

        // Offset from the start of the buffer
        VkDeviceSize BufferOffset;

        // Represents the data size of a row of the buffer. 0 if the data is tightly packed(acoording to the size
        // of the dest image).
        u32 BufferRowLength;

        // The Height of the imaginary image that is stored in the buffer. 0 if its tightly packed(according to the
        // size of the dest image).
        VkDeviceSize BufferImageHeight;
        // Index of the miplevel which is to be updated.
        u32 ImageMipLevel = 0;
        // 6 if the image is supposed to be for a cubemap.
        u32 ImageLayerCount = 0;
        u32 FirstArrayLayer = 0;
        VkImageAspectFlags AspectMask;

        VkImageSubresourceLayers SubresourceLayers;
        SubresourceLayers.aspectMask = AspectMask;
        SubresourceLayers.mipLevel = ImageMipLevel;
        SubresourceLayers.baseArrayLayer = FirstArrayLayer;
        SubresourceLayers.layerCount = ImageLayerCount;

        // Initial offset for the image sub region which is to be updated.
        VkOffset3D ImageOffsetInTexels = {};

        //  the size of the resultant image.
        VkExtent3D ImageExtent = {};

        VkBufferImageCopy Region = Regions[Index];
        Region.bufferOffset = BufferOffset;
        Region.bufferRowLength = BufferRowLength;
        Region.bufferImageHeight = BufferImageHeight;
        Region.imageSubresource = SubresourceLayers;
        Region.imageOffset = ImageOffsetInTexels;
        Region.imageExtent = ImageExtent;
    }

    vkCmdCopyImageToBuffer(CmdBuffer, SrcImage, CurrentImageLayout, DstBuffer, 16, Regions);
    // TODO)): Change the Image and Buffer Layouts Before and After the Copy Operation using Memory Barriers.
}

// NOTE:
void
StageDataToDeviceLocalBuffer(shoora_vulkan_device *RenderDevice)
{
    VkDeviceSize DataSizeToBeUploadedInBytes = {};
    void *DataSourcePtr;

    // NOTE: We have to use a staging buffer because GPU(Device-Local Memory) is not accessible to the CPU. So we need to have
    // an intermediate buffer to transfer the data
    VkBuffer StagingBuffer;

    // TODO)): Usage should be set to VK)BUFFER_USAGE_TRANSFER_SRC_BIT
    // TODO)): CreateBuffer();
    VkDeviceMemory MemoryObjectForStagingBuffer;
    // TODO)): AllocateHostVisibleMemoryForStagingBuffer()
    // TODO)): BindAboveMemoryToStagingBuffer()

    // TODO)): MapStagingBufferMemoryWhichReturnsAPointer();
    // TODO)): CopyDataToStagingBufferPointerUsingMemCpy()
    // TODO)): UnmapMemory()

    VkBuffer DestBufferInDeviceLocalMemory;
    // TODO)): MemoryBarrierForThisBufferToSupportTransferDestination();
    // TODO)): CopyDataFromStagingBufferToDeviceLocalBuffer()

    // TODO)): MemoryBarrier for the DeviceLocalBuffer to transition from TRANSFER_WRITE to The ActualUsage which
    //          it was meant to do!
    //
    // TODO)): End Recording of the Command Buffer
    // TODO)): Take a list of semaphores and fence and submit the Command Buffer to the transfer queue if possible.

    // TODO)): After the Fence is Signaled, meaning the command was processed fully on the queue, Destroy the
    //          StagingBuffer and Free its memeory.
}

b32
DestroyImageView(shoora_vulkan_device *RenderDevice, VkImageView ImageView)
{
    b32 Result = false;
    if(ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(RenderDevice->LogicalDevice, ImageView, 0);
        ImageView = VK_NULL_HANDLE;
        Result = true;
    }

    return Result;
}

b32
DestroyImage(shoora_vulkan_device *RenderDevice, VkImage Image)
{
    b32 Result = false;
    if(Image != VK_NULL_HANDLE)
    {
        vkDestroyImage(RenderDevice->LogicalDevice, Image, 0);
        Image = VK_NULL_HANDLE;
        Result = true;
    }

    return Result;
}

b32 DestroyBufferView(shoora_vulkan_device *RenderDevice, VkBufferView View)
{
    b32 Result = false;
    if(View != VK_NULL_HANDLE)
    {
        vkDestroyBufferView(RenderDevice->LogicalDevice, View, 0);
        View = VK_NULL_HANDLE;
        Result = true;
    }

    return Result;
}

b32 DestroyBuffer(shoora_vulkan_device *RenderDevice, VkBuffer Buffer)
{
    b32 Result = false;
    if(Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(RenderDevice->LogicalDevice, Buffer, 0);
        Buffer = VK_NULL_HANDLE;
        Result = true;
    }

    return Result;
}

b32
FreeMemory(shoora_vulkan_device *RenderDevice, VkDeviceMemory Memory)
{
    b32 Result = false;
    if (Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(RenderDevice->LogicalDevice, Memory, 0);
        Memory = VK_NULL_HANDLE;
        Result = true;
    }

    return Result;
}
#endif
