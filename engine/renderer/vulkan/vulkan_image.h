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

struct shoora_image_create_info
{
    VkFormat Format;

};

void Create2DVulkanImageForSampling(shoora_vulkan_device *RenderDevice, u32 ImageWidth, u32 ImageHeight,
                                    VkFormat ImageFormat, VkImageUsageFlags UsageFlags, b32 GenerateMipMaps,
                                    VkImage *ImageHandle);
void GenerateMipMaps(const char *InputFilename, const char *OutputFilename, i32 MipLevelCount, i32 Quality,
                     u64 *MipOffsets, b32 CapImageToFullHD = false);

#define VULKAN_IMAGE_H
#endif // VULKAN_IMAGE_H