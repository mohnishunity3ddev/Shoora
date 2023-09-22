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
CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, Shu::vec2u Dim, VkSampleCountFlagBits NumSamples,
                    VkFormat Format, VkImageUsageFlags Usage, VkImageAspectFlags Aspect, VkImage *pImage,
                    VkDeviceMemory *pMemory, VkImageView *pView)
{
    VkImageCreateInfo ImageInfo = {};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.mipLevels = 1;
    ImageInfo.arrayLayers = 1;
    ImageInfo.samples = NumSamples;
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
CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, Shu::vec2u Dim, VkSampleCountFlagBits NumSamples,
                    VkFormat Format, VkImageUsageFlags Usage, VkImageAspectFlags Aspect,
                    shoora_vulkan_image *pImage)
{
    CreateSimpleImage2D(RenderDevice, Dim, NumSamples, Format, Usage, Aspect, &pImage->Handle,
                        &pImage->ImageMemory, &pImage->ImageView);
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
CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, shoora_image_data ImageData,
                           VkSampleCountFlagBits NumSamples,
                           shoora_vulkan_image_sampler *pImageSampler)
{
    // LogInfoUnformatted("Creating Image Sampler!\n");

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

    CreateSimpleImage2D(RenderDevice, Shu::vec2u{(u32)ImageData.Dim.w, (u32)ImageData.Dim.h}, NumSamples,
                        ImageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT, &pImageSampler->Image);

    // Create buffer to store store font data.
    shoora_vulkan_buffer StagingBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                      VK_SHARING_MODE_EXCLUSIVE,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      ImageData.Data, ImageData.TotalSize);
    // LogInfoUnformatted("Created Staging Buffer!\n");

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
    // FreeImageData(&ImageData);

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
    // LogInfoUnformatted("Created Image Sampler!\n");
}

void
CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, shoora_image_data *ImageData,
                           VkSampleCountFlagBits NumSamples,
                           shoora_vulkan_image_sampler *pImageSampler)
{
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

    CreateSimpleImage2D(RenderDevice, Shu::vec2u{(u32)ImageData->Dim.w, (u32)ImageData->Dim.h}, NumSamples,
                        ImageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT, &pImageSampler->Image);

    // Create buffer to store store font data.
    shoora_vulkan_buffer StagingBuffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                      VK_SHARING_MODE_EXCLUSIVE,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      ImageData->Data, ImageData->TotalSize);
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
    BufferCopy.imageExtent.width = ImageData->Dim.w;
    BufferCopy.imageExtent.height = ImageData->Dim.h;
    BufferCopy.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(CopyCmdBuffer, StagingBuffer.Handle, pImageSampler->Image.Handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferCopy);
    FreeImageData(ImageData);

    pImageSampler->Image.ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Prepare for shader read
    SetImageLayout(CopyCmdBuffer, pImageSampler->Image.Handle, VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    FlushTransientCommandBuffer(RenderDevice, CopyCmdBuffer, RenderDevice->GraphicsQueue,
                                RenderDevice->GraphicsCommandPool, true);
    DestroyBuffer(RenderDevice, &StagingBuffer);

    // Descriptor Set Stuff
    CreateSampler2D(RenderDevice, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                    &pImageSampler->Sampler);
    // LogInfoUnformatted("Created Image Sabumpler!\n");
}

void
CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, const char *ImageFilename,
                           VkSampleCountFlagBits NumSamples,
                           shoora_vulkan_image_sampler *pImageSampler)
{
    LogInfoUnformatted("Creating Image Sampler!\n");
    shoora_image_data ImageData = LoadImageFile(ImageFilename);

    CreateCombinedImageSampler(RenderDevice, &ImageData, NumSamples, pImageSampler);
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

void
CreatePlaceholderTextureSampler(shoora_vulkan_device *RenderDevice, shoora_vulkan_image_sampler *Sampler,
                                DefaultTexType Type)
{
    ASSERT(Type < DefaultTexType::TexType_MAX_COUNT);

    u8 PixelColor[4];
    if(Type == DefaultTexType::TexType_WHITE)
    {
        PixelColor[0] = PixelColor[1] = PixelColor[2] = PixelColor[3] = 255;
    }
    else if(Type == DefaultTexType::TexType_BLACK)
    {
        PixelColor[0] = PixelColor[1] = PixelColor[2] = 0;
        PixelColor[3] = 255;
    }

    shoora_image_data ImageData = {};
    ImageData.Dim = Shu::vec2i{1, 1};
    ImageData.NumChannels = 4;
    ImageData.TotalSize = 4*sizeof(u8);
    ImageData.Data = (u8 *)&PixelColor;

    CreateCombinedImageSampler(RenderDevice, ImageData, VK_SAMPLE_COUNT_1_BIT, Sampler);
}