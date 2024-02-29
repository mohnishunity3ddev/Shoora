#if !defined(VULKAN_IMAGE_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"


struct shoora_image_transition
{
    VkImage Image;
    VkAccessFlags CurrentAccess;
    VkAccessFlags NewAccess;
    VkImageLayout CurrentLayout;
    VkImageLayout NewLayout;
    // NOTE: To transfer ownership to a new queue. Only possible if the image's sharing mode was set to exclusive
    // during creation. Set this to QUEUE_FAMILY_IGNORED for both when we dont want to transfer the ownership.
    u32 CurrentQueueFamily;
    // NOTE: Index of the new queue family that will be referencing the image after the barrier
    u32 NewQueueFamily;
    // NOTE: Image's Usage Context. Color/Depth/Stencil contexts.
    VkImageAspectFlags Aspect;
};

enum DefaultTexType
{
    TexType_BLACK,
    TexType_WHITE,

    TexType_MAX_COUNT
};

void CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, shu::vec2u Dim, VkSampleCountFlagBits NumSamples,
                         VkFormat Format, VkImageUsageFlags Usage, VkImageAspectFlags Aspect, VkImage *pImage,
                         VkDeviceMemory *pMemory, VkImageView *pView);

void CreateSimpleImage2D(shoora_vulkan_device *RenderDevice, shu::vec2u Dim, VkSampleCountFlagBits NumSamples,
                         VkFormat Format, VkImageUsageFlags Usage, VkImageAspectFlags Aspect,
                         shoora_vulkan_image *pImage);

void CreateImageView2D(shoora_vulkan_device *RenderDevice, VkImage Image, VkFormat Format,
                       VkImageAspectFlags Aspect, VkImageView *pImageView);

void CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, const char *ImageFilename,
                                VkSampleCountFlagBits NumSamples, shoora_vulkan_image_sampler *pImageSampler);
void CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice, shoora_image_data *ImageData,
                                VkSampleCountFlagBits NumSamples, shoora_vulkan_image_sampler *pImageSampler);

void SetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageAspectFlags Aspect,
                    VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkPipelineStageFlags SrcStage,
                    VkPipelineStageFlags DstStage);

void CreateSampler2D(shoora_vulkan_device *RenderDevice, VkFilter Filter, VkSamplerMipmapMode MipmapMode,
                     VkSamplerAddressMode AddressMode, VkBorderColor BorderColor, VkSampler *pSampler);

VkFormat GetSuitableDepthAttachmentFormat(shoora_vulkan_device *RenderDevice);

void DestroyImage2D(shoora_vulkan_device *RenderDevice, shoora_vulkan_image *pImage);

void CreatePlaceholderTextureSampler(shoora_vulkan_device *RenderDevice, shoora_vulkan_image_sampler *Sampler,
                                     DefaultTexType Type);

#define VULKAN_IMAGE_H
#endif // VULKAN_IMAGE_H