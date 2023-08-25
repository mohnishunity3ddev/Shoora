#include "vulkan_image.h"
#include "utils/utils.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "loaders/image/image_loader.h"
#include <memory>

static VkFormat ThreeChannelColorCandidates[] = {VK_FORMAT_R8G8B8_UNORM,   VK_FORMAT_R8G8B8_SNORM,
                                                 VK_FORMAT_R8G8B8_USCALED, VK_FORMAT_R8G8B8_SSCALED,
                                                 VK_FORMAT_R8G8B8_UINT,    VK_FORMAT_R8G8B8_SINT,
                                                 VK_FORMAT_R8G8B8_SRGB,    VK_FORMAT_B8G8R8_UNORM,
                                                 VK_FORMAT_B8G8R8_SNORM};

static VkFormat FourChannelColorCandidates[] = {VK_FORMAT_R8G8B8A8_UNORM,               VK_FORMAT_R8G8B8A8_SNORM,
                                                VK_FORMAT_R8G8B8A8_USCALED,             VK_FORMAT_R8G8B8A8_SSCALED,
                                                VK_FORMAT_R8G8B8A8_UINT,                VK_FORMAT_R8G8B8A8_SINT,
                                                VK_FORMAT_R8G8B8A8_SRGB,                VK_FORMAT_B8G8R8A8_UNORM,
                                                VK_FORMAT_B8G8R8A8_SNORM,               VK_FORMAT_B8G8R8A8_USCALED,
                                                VK_FORMAT_B8G8R8A8_SSCALED,             VK_FORMAT_B8G8R8A8_UINT,
                                                VK_FORMAT_B8G8R8A8_SINT,                VK_FORMAT_B8G8R8A8_SRGB,
                                                VK_FORMAT_A8B8G8R8_UNORM_PACK32,        VK_FORMAT_A8B8G8R8_SNORM_PACK32,
                                                VK_FORMAT_A8B8G8R8_USCALED_PACK32,      VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
                                                VK_FORMAT_A8B8G8R8_UINT_PACK32,         VK_FORMAT_A8B8G8R8_SINT_PACK32,
                                                VK_FORMAT_A8B8G8R8_SRGB_PACK32,         VK_FORMAT_A2R10G10B10_UNORM_PACK32,
                                                VK_FORMAT_A2R10G10B10_SNORM_PACK32,     VK_FORMAT_A2R10G10B10_USCALED_PACK32,
                                                VK_FORMAT_A2R10G10B10_SSCALED_PACK32,   VK_FORMAT_A2R10G10B10_UINT_PACK32,
                                                VK_FORMAT_A2R10G10B10_SINT_PACK32,      VK_FORMAT_A2B10G10R10_UNORM_PACK32,
                                                VK_FORMAT_A2B10G10R10_SNORM_PACK32,     VK_FORMAT_A2B10G10R10_USCALED_PACK32,
                                                VK_FORMAT_A2B10G10R10_SSCALED_PACK32,   VK_FORMAT_A2B10G10R10_UINT_PACK32,
                                                VK_FORMAT_A2B10G10R10_SINT_PACK32};

void
CreateSampler2D(shoora_vulkan_device *RenderDevice, VkFilter Filter, VkSamplerMipmapMode MipmapMode,
                VkSamplerAddressMode AddressMode, VkBorderColor BorderColor, VkSampler *pSampler)
{
    VkSamplerCreateInfo SamplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    SamplerInfo.maxAnisotropy = 1.0f;
    SamplerInfo.magFilter = Filter;
    SamplerInfo.minFilter = Filter;
    SamplerInfo.mipmapMode = MipmapMode;
    SamplerInfo.addressModeU = AddressMode;
    SamplerInfo.addressModeV = AddressMode;
    SamplerInfo.addressModeW = AddressMode;
    SamplerInfo.borderColor = BorderColor;

    VK_CHECK(vkCreateSampler(RenderDevice->LogicalDevice, &SamplerInfo, nullptr, pSampler));
}

void
CreateImageView2D(shoora_vulkan_device *RenderDevice, VkImage Image, VkFormat Format, VkImageAspectFlags Aspect,
                  VkImageView *pImageView)
{
    VkImageViewCreateInfo ViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    ViewInfo.image = Image;
    ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ViewInfo.format = Format;
    ViewInfo.subresourceRange.levelCount = 1;
    ViewInfo.subresourceRange.layerCount = 1;
    ViewInfo.subresourceRange.aspectMask = Aspect;

    VK_CHECK(vkCreateImageView(RenderDevice->LogicalDevice, &ViewInfo, nullptr, pImageView));
}

void
CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, vec2u Dim, VkFormat Format, VkImageUsageFlags Usage,
                    VkImageAspectFlags Aspect, VkImage *pImage, VkDeviceMemory *pMemory, VkImageView *pView)
{
    VkImageCreateInfo ImageInfo = {};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.mipLevels = 1;
    ImageInfo.arrayLayers = 1;
    ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    ImageInfo.format = Format;
    ImageInfo.usage = Usage;
    ImageInfo.extent.width = Dim.x;
    ImageInfo.extent.height = Dim.y;
    ImageInfo.extent.depth = 1;

    VK_CHECK(vkCreateImage(RenderDevice->LogicalDevice, &ImageInfo, nullptr, pImage));

    VkMemoryRequirements MemReqs;
    vkGetImageMemoryRequirements(RenderDevice->LogicalDevice, *pImage, &MemReqs);
    VkMemoryAllocateInfo MemAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    MemAllocInfo.allocationSize = MemReqs.size;
    MemAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, MemReqs.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &MemAllocInfo, nullptr, pMemory));
    VK_CHECK(vkBindImageMemory(RenderDevice->LogicalDevice, *pImage, *pMemory, 0));

    CreateImageView2D(RenderDevice, *pImage, Format, Aspect, pView);
}

void
CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, vec2u Dim, VkFormat Format, VkImageUsageFlags Usage,
                    VkImageAspectFlags Aspect, shoora_vulkan_image *pImage)
{
    CreateSimpleImage2D(RenderDevice, Dim, Format, Usage, Aspect, &pImage->Handle, &pImage->ImageMemory,
                        &pImage->ImageView);
}



void
SetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout OldImageLayout,
               VkImageLayout NewImageLayout, VkImageSubresourceRange SubresourceRange,
               VkPipelineStageFlags SrcStage, VkPipelineStageFlags DstStage)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.oldLayout = OldImageLayout;
    ImageMemoryBarrier.newLayout = NewImageLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = Image;
    ImageMemoryBarrier.subresourceRange = SubresourceRange;

    // SrcAccessMask are operations that should be complete on the old layout before transitioning to the new
    // layout.
    switch(OldImageLayout)
    {
        // Image layout does not matter.
        case VK_IMAGE_LAYOUT_UNDEFINED:
        {
            ImageMemoryBarrier.srcAccessMask = 0;
        } break;

        // Only for images laid out in memorg in linear fashion which can be used by CPUs since linear images
        // are good for reading/writing by CPUs.
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        } break;

        // Image is a color attachment.
        // Make sure any writes to the color buffer have been finished.
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        } break;

        // Image is a depth attachment.
        // Make sure any writes to the depth buffer have been finished.
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        } break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        {
            // Make sure any reads from the image have been finished
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        } break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        {
            // Make sure any writes to  the image have been finished
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        } break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        {
            // The image is meant to be read from in a shader.
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        } break;

        default:
        {
            ASSERT(!"This Layout is not handled yet!");
        }
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch(NewImageLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        {
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        } break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        {
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        } break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        {
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        } break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        {
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            ImageMemoryBarrier.dstAccessMask = ImageMemoryBarrier.dstAccessMask |
                                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        } break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        {
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if(ImageMemoryBarrier.srcAccessMask == 0)
            {
                ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }

            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        } break;

        default:
        {
            ASSERT(!"This Layout is not handled yet!");
        }
    }

    vkCmdPipelineBarrier(CmdBuffer, SrcStage, DstStage, 0,
                         0, nullptr, 0, nullptr,
                         1, &ImageMemoryBarrier);
}

void
SetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageAspectFlags Aspect, VkImageLayout OldImageLayout,
               VkImageLayout NewImageLayout, VkPipelineStageFlags SrcStage, VkPipelineStageFlags DstStage)
{
    VkImageSubresourceRange SubresourceRange = {};
    SubresourceRange.aspectMask = Aspect;
    SubresourceRange.baseArrayLayer = 0;
    SubresourceRange.baseMipLevel = 0;
    SubresourceRange.layerCount = 1;
    SubresourceRange.levelCount = 1;

    SetImageLayout(CmdBuffer, Image, OldImageLayout, NewImageLayout, SubresourceRange, SrcStage, DstStage);
}

VkFormat
GetSuitableImageFormat(shoora_vulkan_device *RenderDevice, VkFormat *FormatCandidates, u32 FormatCount,
                       VkImageTiling Tiling, VkFormatFeatureFlags FormatUsage, VkFormat Desired = VK_FORMAT_UNDEFINED)
{
    VkFormat Result = VK_FORMAT_UNDEFINED;

    if(Desired != VK_FORMAT_UNDEFINED)
    {
        VkFormatProperties FormatProps;
        vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Desired, &FormatProps);

        if ((Tiling == VK_IMAGE_TILING_LINEAR &&
                ((FormatProps.linearTilingFeatures & FormatUsage) == FormatUsage)) ||
            (Tiling == VK_IMAGE_TILING_OPTIMAL &&
                ((FormatProps.optimalTilingFeatures & FormatUsage) == FormatUsage)))
        {
            Result = Desired;
        }
    }

    if(Result == VK_FORMAT_UNDEFINED)
    {
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
    }

    ASSERT(Result != VK_FORMAT_UNDEFINED);
    return Result;
}

void
CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, const char *ImageFilename,
                           shoora_vulkan_image_sampler *pImageSampler)
{
    LogInfoUnformatted("Creating Image Sampler!\n");
    shoora_image_data ImageData = LoadImageFile(ImageFilename);

    VkFormat DesiredImageFormat = VK_FORMAT_UNDEFINED;
    VkFormat *Candidates = nullptr;
    u32 CandidateCount = 0;

#if 0
    if(ImageData.NumChannels == 3)
    {
        DesiredImageFormat = VK_FORMAT_R8G8B8_SRGB;
        Candidates = ThreeChannelColorCandidates;
        CandidateCount = ARRAY_SIZE(ThreeChannelColorCandidates);
    }
    else
#endif
    {
        DesiredImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
        Candidates = FourChannelColorCandidates;
        CandidateCount = ARRAY_SIZE(FourChannelColorCandidates);
    }

    VkFormat ImageFormat = GetSuitableImageFormat(RenderDevice, Candidates, CandidateCount,
                                                  VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
                                                  DesiredImageFormat);

    CreateSimpleImage2D(RenderDevice, Vec2<u32>(ImageData.Dim.w, ImageData.Dim.h), ImageFormat,
                        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
                        &pImageSampler->Image);

    // Create buffer to store store font data.
    shoora_vulkan_buffer StagingBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                      VK_SHARING_MODE_EXCLUSIVE,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      ImageData.Data, ImageData.TotalSize);
    LogInfoUnformatted("Created Staging Buffer!\n");

    // Get command buffer to store transfer data from buffer to image
    VkCommandBuffer CopyCmdBuffer = CreateTransientCommandBuffer(RenderDevice, RenderDevice->GraphicsCommandPool,
                                                                 VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    SetImageLayout(CopyCmdBuffer, pImageSampler->Image.Handle, VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_HOST_BIT,
                   VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Copy
    VkBufferImageCopy BufferCopy = {};
    BufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BufferCopy.imageSubresource.layerCount = 1;
    BufferCopy.imageExtent.width = ImageData.Dim.w;
    BufferCopy.imageExtent.height = ImageData.Dim.h;
    BufferCopy.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(CopyCmdBuffer, StagingBuffer.Handle, pImageSampler->Image.Handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferCopy);
    FreeImageData(&ImageData);

    // Prepare for shader read
    SetImageLayout(CopyCmdBuffer, pImageSampler->Image.Handle, VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    FlushTransientCommandBuffer(RenderDevice, CopyCmdBuffer, RenderDevice->GraphicsQueue,
                                RenderDevice->GraphicsCommandPool, true);
    DestroyBuffer(RenderDevice, &StagingBuffer);

    // Descriptor Set Stuff
    CreateSampler2D(RenderDevice, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                    &pImageSampler->Sampler);
    LogInfoUnformatted("Created Image Sampler!\n");
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
DestroyImage2D(shoora_vulkan_device *RenderDevice, shoora_vulkan_image *pImage)
{
    if(pImage->ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(RenderDevice->LogicalDevice, pImage->ImageView, nullptr);
        pImage->ImageView = VK_NULL_HANDLE;
    }

    if(pImage->ImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(RenderDevice->LogicalDevice, pImage->ImageMemory, nullptr);
        pImage->ImageMemory = VK_NULL_HANDLE;
    }

    if(pImage->Handle != VK_NULL_HANDLE)
    {
        vkDestroyImage(RenderDevice->LogicalDevice, pImage->Handle, nullptr);
        pImage->Handle = VK_NULL_HANDLE;
    }
}

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
