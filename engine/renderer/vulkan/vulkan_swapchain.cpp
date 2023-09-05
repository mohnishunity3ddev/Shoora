#include "vulkan_swapchain.h"
#include "platform/platform.h"
#include "vulkan_image.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_pipeline.h"

b32
CheckSupportedPresentModes(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                           VkPresentModeKHR DesiredPresentMode)
{
    b32 Result = false;
    u32 AvailablePresentModeCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(RenderDevice->PhysicalDevice,
                                                       Swapchain->Surface,
                                                       &AvailablePresentModeCount, 0));

    ASSERT(AvailablePresentModeCount > 0 && AvailablePresentModeCount <= 16);

    VkPresentModeKHR SupportedPresentModes[16];
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(RenderDevice->PhysicalDevice,
                                                       Swapchain->Surface,
                                                       &AvailablePresentModeCount, SupportedPresentModes));

    for(u32 Index = 0;
        Index < AvailablePresentModeCount;
        ++Index)
    {
        VkPresentModeKHR AvlPresentMode = SupportedPresentModes[Index];
        if(AvlPresentMode == DesiredPresentMode)
        {
            Result = true;
            break;
        }
    }

    return Result;
}

inline void
GetSurfaceCapabilities(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                       VkSurfaceCapabilitiesKHR *SurfaceCapabilities)
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(RenderDevice->PhysicalDevice,
                                                       Swapchain->Surface,
                                                       SurfaceCapabilities));
}

void
SelectSwapchainImageCount(shoora_vulkan_swapchain *Swapchain)
{
    VkSurfaceCapabilitiesKHR *SurfaceCapabilities = &Swapchain->SurfaceCapabilities;
    u32 ImageCount = SurfaceCapabilities->minImageCount + 1;

    if(SurfaceCapabilities->maxImageCount > 0)
    {
        LogOutput(LogType_Info, "There is a limit to the number of images supported! Expected if Present Mode is anything other "
                  "than IMMEDIATE MODE!\n");
        if(ImageCount > SurfaceCapabilities->maxImageCount)
        {
            ImageCount = SurfaceCapabilities->maxImageCount;
        }
    }

    if(ImageCount > SHU_MAX_FRAMES_IN_FLIGHT)
    {
        ImageCount = SHU_MAX_FRAMES_IN_FLIGHT;
    }

    Swapchain->ImageCount = ImageCount;
}

void
SelectSwapchainSize(shoora_vulkan_swapchain *Swapchain)
{
    VkSurfaceCapabilitiesKHR *SurfaceCapabilities = &Swapchain->SurfaceCapabilities;

    Swapchain->ImageDimensions = SurfaceCapabilities->currentExtent;
    // Checking if size of the images determine size of the window.
    if(Swapchain->ImageDimensions.width == -1UL)
    {
        Swapchain->ImageDimensions.width = ClampToRange(Swapchain->ImageDimensions.width,
                                                  SurfaceCapabilities->minImageExtent.width,
                                                  SurfaceCapabilities->maxImageExtent.width);
        Swapchain->ImageDimensions.height = ClampToRange(Swapchain->ImageDimensions.height,
                                                   SurfaceCapabilities->minImageExtent.height,
                                                   SurfaceCapabilities->maxImageExtent.height);
    }

    ASSERT((Swapchain->ImageDimensions.width > 0) &&
           (Swapchain->ImageDimensions.width > 0));
}

// Do we use them as render targets, or copy src/dest or sample from them for postprocessing
void
SelectDesiredImageUsage(shoora_vulkan_swapchain *Swapchain, VkImageUsageFlags DesiredImageUsageFlags)
{
    VkSurfaceCapabilitiesKHR *SurfaceCapabilities = &Swapchain->SurfaceCapabilities;
    VkImageUsageFlags ImageUsageFlags = 0;

    u32 NumberOfBitsToCheck = (u32)(sizeof(VkImageUsageFlags) * 8);
    SET_FLAG_BITS_IF_EQUAL(ImageUsageFlags, DesiredImageUsageFlags,
                           SurfaceCapabilities->supportedUsageFlags, NumberOfBitsToCheck);

    if(ImageUsageFlags != DesiredImageUsageFlags)
    {
        // TODO)) Dont ASSERT this as this is non-essential. Figure out what to set this to.
        LogOutput(LogType_Warn, "All Desired Image Usages are not supported!\n");
        // ImageUsageFlags = SurfaceCapabilities->supportedUsageFlags;
    }

    Swapchain->ImageUsageFlags = ImageUsageFlags;
}

// For Pre-Transform an image to landscape/Potrait before presenting it(In case of mobiles)
void
SelectImageTransforms(shoora_vulkan_swapchain *Swapchain, VkSurfaceTransformFlagBitsKHR DesiredTransformFlagBits)
{
    VkSurfaceCapabilitiesKHR *SurfaceCapabilities = &Swapchain->SurfaceCapabilities;
    if (SurfaceCapabilities->supportedTransforms & DesiredTransformFlagBits)
    {
        Swapchain->TransformFlagBits = DesiredTransformFlagBits;
    }
    else
    {
        Swapchain->TransformFlagBits = SurfaceCapabilities->currentTransform;
    }

    // TODO)): Check if this is better. It only sets transforms to the ones that are supported even if they are a
    // subset of the DesiredTransform.

    // VkSurfaceTransformFlagsKHR SurfaceTransforms = SurfaceCapabilities->supportedTransforms;
    // VkSurfaceTransformFlagsKHR Transforms = 0;
    // SET_FLAG_BITS_IF_EQUAL(Transforms, DesiredTransforms, SurfaceTransforms,
    //                        (u32)(sizeof(VkSurfaceTransformFlagsKHR)*8));
    // if(Transforms != DesiredTransforms)
    // {
    //     LogOutput("Not all Desired Image Transforms are supported!\n");
    // }
}

inline void
SetSwapchainDepthFormat(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    Swapchain->DepthFormat = GetSuitableDepthAttachmentFormat(RenderDevice);
}

void
SelectImageFormats(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                   VkFormat DesiredImageFormat, VkColorSpaceKHR DesiredColorSpace)
{
    u32 FormatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(RenderDevice->PhysicalDevice, Swapchain->Surface,
                                                  &FormatCount, 0));

    ASSERT(FormatCount > 0 && FormatCount <= 32);

    VkSurfaceFormatKHR SupportedFormats[32];
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(RenderDevice->PhysicalDevice, Swapchain->Surface,
                                                  &FormatCount, SupportedFormats));

    b32 FormatFound = false;
    // Checking to see if we can set the format to anything
    if((FormatCount == 1) &&
       (SupportedFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        Swapchain->SurfaceFormat.colorSpace = DesiredColorSpace;
        Swapchain->SurfaceFormat.format = DesiredImageFormat;
    }
    else if(FormatCount > 1)
    {
        // Check to see if both imageFormat and ColorSpace are supported!
        for(u32 Index = 0;
            Index < FormatCount;
            ++Index)
        {
            VkSurfaceFormatKHR SupportedFormat = SupportedFormats[Index];
            if ((SupportedFormat.format == DesiredImageFormat) &&
                (SupportedFormat.colorSpace == DesiredColorSpace))
            {
                Swapchain->SurfaceFormat.colorSpace = SupportedFormat.colorSpace;
                Swapchain->SurfaceFormat.format = SupportedFormat.format;
                FormatFound = true;
                break;
            }
        }

        if(!FormatFound)
        {
            for(u32 Index = 0;
                Index < FormatCount;
                ++Index)
            {
                VkSurfaceFormatKHR SupportedFormat = SupportedFormats[Index];
                if (SupportedFormat.format == DesiredImageFormat)
                {
                    Swapchain->SurfaceFormat.format = DesiredImageFormat;
                    Swapchain->SurfaceFormat.colorSpace = SupportedFormat.colorSpace;
                    LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the one with "
                              "the same imageFormat.\n");
                    FormatFound = true;
                    break;
                }
            }

            if(!FormatFound)
            {
                Swapchain->SurfaceFormat.format = SupportedFormats[0].format;
                Swapchain->SurfaceFormat.colorSpace = SupportedFormats[0].colorSpace;
                LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the first "
                          "supported one!\n");
                FormatFound = true;
            }
        }
    }

    SetSwapchainDepthFormat(RenderDevice, Swapchain);

    ASSERT(FormatFound);
}

void
PrepareForSwapchainCreation(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                            shoora_vulkan_swapchain_create_info *ShooraSwapchainInfo)
{
    if (!CheckSupportedPresentModes(RenderDevice, Swapchain, ShooraSwapchainInfo->DesiredPresentMode))
    {
        // NOTE: Vulkan Makes sure Atleast FIFO is supported!
        ShooraSwapchainInfo->DesiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    Swapchain->PresentMode = ShooraSwapchainInfo->DesiredPresentMode;

    GetSurfaceCapabilities(RenderDevice, Swapchain, &Swapchain->SurfaceCapabilities);
    SelectSwapchainImageCount(Swapchain);
    SelectSwapchainSize(Swapchain);
    SelectDesiredImageUsage(Swapchain, ShooraSwapchainInfo->DesiredImageUsages);
    SelectImageTransforms(Swapchain, ShooraSwapchainInfo->DesiredTransformFlagBits);
    SelectImageFormats(RenderDevice, Swapchain, ShooraSwapchainInfo->DesiredImageFormat,
                       ShooraSwapchainInfo->DesiredImageColorSpace);
}

void
GetSwapchainImageHandles(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    u32 SwapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(RenderDevice->LogicalDevice, Swapchain->Handle, &SwapchainImageCount, 0));

    // NOTE: Drivers can produce more images that were actually requested during swapchain creation.
    ASSERT((SwapchainImageCount >= Swapchain->ImageCount) &&
           (SwapchainImageCount <= ARRAY_SIZE(Swapchain->Images)));

    VK_CHECK(vkGetSwapchainImagesKHR(RenderDevice->LogicalDevice, Swapchain->Handle,
                                     &SwapchainImageCount, Swapchain->Images));
    LogOutput(LogType_Info, "Got the Swapchain Image Handles!\n");
}

void
CreateSwapchainImageViews(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    for(u32 ImageIndex = 0;
        ImageIndex < Swapchain->ImageCount;
        ++ImageIndex)
    {
        VkImageViewCreateInfo CreateInfo;
        CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        CreateInfo.pNext = nullptr;
        CreateInfo.flags = 0;
        CreateInfo.image = Swapchain->Images[ImageIndex];
        CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        CreateInfo.format = Swapchain->SurfaceFormat.format;
        CreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

        VkImageSubresourceRange SubresourceRange;
        SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        SubresourceRange.baseMipLevel = 0;
        SubresourceRange.levelCount = 1;
        SubresourceRange.baseArrayLayer = 0;
        SubresourceRange.layerCount = 1;

        CreateInfo.subresourceRange = SubresourceRange;

        VK_CHECK(vkCreateImageView(RenderDevice->LogicalDevice, &CreateInfo, nullptr,
                                   &Swapchain->ImageViews[ImageIndex]));
    }

    LogOutput(LogType_Info, "Created all Swapchain Image Views!\n");
}

void
SetupDepthStencil(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    shoora_vulkan_image *pImage = &Swapchain->DepthStencilImage;

    Shu::vec2u Dim = Shu::vec2u{Swapchain->ImageDimensions.width, Swapchain->ImageDimensions.height};

    VkImageAspectFlags Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if(Swapchain->DepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
    {
        Aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    CreateSimpleImage2D(RenderDevice, Dim, Swapchain->DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        Aspect, &pImage->Handle, &pImage->ImageMemory, &pImage->ImageView);
}

void
CreateSwapchainFramebuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain, VkRenderPass RenderPass)
{
    ASSERT(Swapchain->ImageCount <= ARRAY_SIZE(Swapchain->ImageFramebuffers))

    VkImageView Attachments[2];
    Attachments[1] = Swapchain->DepthStencilImage.ImageView;

    for(u32 Index = 0;
        Index < Swapchain->ImageCount;
        ++Index)
    {
        Attachments[0] = Swapchain->ImageViews[Index];

        VkFramebufferCreateInfo CreateInfo;
        CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        CreateInfo.pNext = nullptr;
        CreateInfo.flags = 0;
        CreateInfo.renderPass = RenderPass;
        CreateInfo.attachmentCount = 2;
        CreateInfo.pAttachments = Attachments;
        CreateInfo.width = Swapchain->ImageDimensions.width;
        CreateInfo.height = Swapchain->ImageDimensions.height;
        CreateInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(RenderDevice->LogicalDevice, &CreateInfo, nullptr,
                                     &Swapchain->ImageFramebuffers[Index]));

        LogOutput(LogType_Info, "Created Swapchain Framebuffer[%d]\n", Index);
    }
}

void
DestroySwapchainFramebuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    for(u32 Index = 0;
        Index < Swapchain->ImageCount;
        ++Index)
    {
        VkFramebuffer FrameBuffer = Swapchain->ImageFramebuffers[Index];
        ASSERT(FrameBuffer != VK_NULL_HANDLE);
        vkDestroyFramebuffer(RenderDevice->LogicalDevice, FrameBuffer, nullptr);
    }

    LogOutput(LogType_Warn, "Destroyed Swapchain Framebuffers!\n");
}

void
DestroySwapchainImageViews(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    for(u32 Index = 0;
        Index < Swapchain->ImageCount;
        ++Index)
    {
        vkDestroyImageView(RenderDevice->LogicalDevice, Swapchain->ImageViews[Index], nullptr);
        Swapchain->ImageViews[Index] = VK_NULL_HANDLE;
    }

    LogOutput(LogType_Warn, "Swapchain Image Views are destroyed!\n");
}


void
AllocateDrawCommandBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    VkCommandBuffer BuffersToAllocate[ARRAY_SIZE(Swapchain->DrawCommandBuffers)] = {};

    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        shoora_vulkan_command_buffer_handle *ShuCmdBuffer = &Swapchain->DrawCommandBuffers[Index];
        BuffersToAllocate[Index] = ShuCmdBuffer->Handle;
        ShuCmdBuffer->IsRecording = false;
        ShuCmdBuffer->CommandPool = &RenderDevice->GraphicsCommandPool;
    }

    VkCommandBufferAllocateInfo AllocInfo;
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.pNext = nullptr;
    AllocInfo.commandPool = RenderDevice->GraphicsCommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = ARRAY_SIZE(Swapchain->DrawCommandBuffers);

    VK_CHECK(vkAllocateCommandBuffers(RenderDevice->LogicalDevice, &AllocInfo, BuffersToAllocate));

    for(u32 Index = 0;
        Index < ARRAY_SIZE(Swapchain->DrawCommandBuffers);
        ++Index)
    {
        shoora_vulkan_command_buffer_handle *ShuCmdBuffer = &Swapchain->DrawCommandBuffers[Index];
        ShuCmdBuffer->Handle = BuffersToAllocate[Index];
    }

    LogOutput(LogType_Info, "Draw Command buffers have been allocated!\n");
}

void
FreeDrawCommandBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    u32 CommandBufferCount = ARRAY_SIZE(Swapchain->DrawCommandBuffers);
    VkCommandBuffer BuffersToFree[SHU_MAX_FRAMES_IN_FLIGHT + 1];

    for(u32 Index = 0;
        Index < CommandBufferCount;
        ++Index)
    {
        shoora_vulkan_command_buffer_handle *ShuCmdBuffer = &Swapchain->DrawCommandBuffers[Index];
        // TODO)): Complete command buffer status tracking. Then add this assert afterwards!
        // ASSERT(!ShuCmdBuffer->IsRecording);
        BuffersToFree[Index] = ShuCmdBuffer->Handle;
        ShuCmdBuffer->CommandPool = nullptr;
    }

    vkFreeCommandBuffers(RenderDevice->LogicalDevice, RenderDevice->GraphicsCommandPool, CommandBufferCount,
                         BuffersToFree);

    LogOutput(LogType_Warn, "Draw Command Buffers have been Freed!\n");
}

void
WindowResized(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain, VkRenderPass RenderPass,
              Shu::vec2u ScreenDim)
{
    // TODO)): Will it be better to wait for all device operations to finish? or queue wait will suffice?
    VK_CHECK(vkQueueWaitIdle(GetQueueHandle(RenderDevice, QueueType_Graphics)));

    Swapchain->ImageDimensions = {ScreenDim.x, ScreenDim.y};

    VkSwapchainCreateInfoKHR CreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    CreateInfo.pNext = 0;
    CreateInfo.flags = 0;
    CreateInfo.surface = Swapchain->Surface;
    CreateInfo.minImageCount = Swapchain->ImageCount;
    CreateInfo.imageFormat = Swapchain->SurfaceFormat.format;
    CreateInfo.imageColorSpace = Swapchain->SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Swapchain->ImageDimensions;

    // NOTE: More than 1 if we are doing layered/stereoscopic rendering
    CreateInfo.imageArrayLayers = 1;

    CreateInfo.imageUsage = Swapchain->ImageUsageFlags;
    CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = 0;
    CreateInfo.preTransform = Swapchain->TransformFlagBits;

    // TODO)): Read More about this!
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    CreateInfo.presentMode = Swapchain->PresentMode;
    CreateInfo.clipped = VK_TRUE;

    VkSwapchainKHR OldSwapchain = Swapchain->Handle;
    if(OldSwapchain != VK_NULL_HANDLE)
    {
        CreateInfo.oldSwapchain = OldSwapchain;
    }

    VK_CHECK(vkCreateSwapchainKHR(RenderDevice->LogicalDevice, &CreateInfo, 0, &Swapchain->Handle));

    if(Swapchain->Handle == VK_NULL_HANDLE)
    {
        LogOutput(LogType_Error, "There was a problem creating the swapchain!\n");
    }

    // DestroySwapchainUniformResources(&Context->Device, &Context->Swapchain);
    DestroyImage2D(RenderDevice, &Swapchain->DepthStencilImage);
    DestroySwapchainFramebuffers(RenderDevice, Swapchain);
    DestroySwapchainImageViews(RenderDevice, Swapchain);
    FreeDrawCommandBuffers(RenderDevice, Swapchain);
    vkDestroySwapchainKHR(RenderDevice->LogicalDevice, OldSwapchain, 0);

    LogOutput(LogType_Warn, "Destroyed Old Swapchain!\n");

    GetSwapchainImageHandles(RenderDevice, Swapchain);
    CreateSwapchainImageViews(RenderDevice, Swapchain);
    ASSERT(RenderPass != VK_NULL_HANDLE);
    CreateSimpleImage2D(RenderDevice, ScreenDim, Swapchain->DepthFormat,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                        &Swapchain->DepthStencilImage.Handle, &Swapchain->DepthStencilImage.ImageMemory,
                        &Swapchain->DepthStencilImage.ImageView);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, RenderPass);

    AllocateDrawCommandBuffers(RenderDevice, Swapchain);

    LogOutput(LogType_Info, "Swapchain Created!\n");
}

void
CreateSwapchain(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain, Shu::vec2u ScreenDim,
                shoora_vulkan_swapchain_create_info *ShooraSwapchainInfo)
{
    PrepareForSwapchainCreation(RenderDevice, Swapchain, ShooraSwapchainInfo);

    shoora_vulkan_swapchain *SwapchainInfo = Swapchain;
    SwapchainInfo->ImageDimensions = {ScreenDim.x, ScreenDim.y};

    VkSwapchainCreateInfoKHR CreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    CreateInfo.pNext = 0;
    CreateInfo.flags = 0;
    CreateInfo.surface = SwapchainInfo->Surface;
    CreateInfo.minImageCount = SwapchainInfo->ImageCount;
    CreateInfo.imageFormat = SwapchainInfo->SurfaceFormat.format;
    CreateInfo.imageColorSpace = SwapchainInfo->SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = SwapchainInfo->ImageDimensions;

    // NOTE: More than 1 if we are doing layered/stereoscopic rendering
    CreateInfo.imageArrayLayers = 1;

    CreateInfo.imageUsage = SwapchainInfo->ImageUsageFlags;
    CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = nullptr;
    CreateInfo.preTransform = SwapchainInfo->TransformFlagBits;

    // TODO)): Read More about this!
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    CreateInfo.presentMode = SwapchainInfo->PresentMode;
    CreateInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(RenderDevice->LogicalDevice, &CreateInfo, 0,
                                  &Swapchain->Handle));
    if(Swapchain->Handle == VK_NULL_HANDLE)
    {
        LogOutput(LogType_Error, "There was a problem creating the swapchain!\n");
    }

    Swapchain->SurfaceFormat = SwapchainInfo->SurfaceFormat;

    GetSwapchainImageHandles(RenderDevice, Swapchain);
    CreateSwapchainImageViews(RenderDevice, Swapchain);
    SetupDepthStencil(RenderDevice, Swapchain);
    AllocateDrawCommandBuffers(RenderDevice, Swapchain);

    LogOutput(LogType_Info, "Swapchain Created!\n");
}

void
CreatePresentationSurface(shoora_vulkan_context *Context, VkSurfaceKHR *Surface)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};

    shoora_platform_presentation_surface Win32Surface = {&SurfaceCreateInfo};
    FillVulkanWin32SurfaceCreateInfo(&Win32Surface);

    VK_CHECK(vkCreateWin32SurfaceKHR(Context->Instance, &SurfaceCreateInfo, 0, Surface));
    LogOutput(LogType_Info, "Created Presentation Surface!\n");
#endif
}

void
AcquireNextSwapchainImage(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                          shoora_vulkan_semaphore_handle *SignalSemaphore)
{
    u32 ImageIndex = -1UL;
    ASSERT(SignalSemaphore != VK_NULL_HANDLE);

    // NOTE: We can wait upto 2 seocnds to acquire the new swapchain image, if not throw an error.
    VkResult AcquireResult = vkAcquireNextImageKHR(RenderDevice->LogicalDevice,
                                                   Swapchain->Handle, NANOSECONDS(2),
                                                   SignalSemaphore->Handle, VK_NULL_HANDLE, &ImageIndex);
    if((AcquireResult != VK_SUCCESS) &&
       (AcquireResult != VK_SUBOPTIMAL_KHR))
    {
        LogOutput(LogType_Error, "There was a problem acquiring a swapchain image!\n");
        ImageIndex = -1UL;
    }
    else if (AcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        LogOutput(LogType_Error, "You cannot use the images of this swapchain. Destroy the swapchain and "
                                 "recreate it again!\n");
        ImageIndex = -1UL;
    }
    else if(AcquireResult == VK_SUCCESS || AcquireResult == VK_SUBOPTIMAL_KHR)
    {
        SignalSemaphore->IsSignaled = true;
    }

    ASSERT(ImageIndex >= 0 && ImageIndex < Swapchain->ImageCount);

    Swapchain->CurrentImageIndex = ImageIndex;
}

#if 0
void
PresentImage(shoora_vulkan_context *Context)
{
    VkQueue PresentQueue;

    VkSemaphore RenderingSemaphores[1];
    VkSwapchainKHR Swapchains[1];
    u32 ImageIndices[1];

    // NOTE: We can present multiple images to presentation engine. But only one from a specific swapchain.
    VkPresentInfoKHR PresentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    PresentInfo.pNext = 0;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = RenderingSemaphores;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = ImageIndices;
    PresentInfo.pResults = nullptr;

    VK_CHECK(vkQueuePresentKHR(PresentQueue, &PresentInfo));
}
#endif

void
SetImageSamplerLayoutBindings(VkDescriptorSetLayoutBinding *pSamplerBindings, u32 SamplerCount)
{
    VkDescriptorSetLayoutBinding DefaultBinding = GetDefaultFragSamplerLayoutBinding();

    for(u32 Index = 0;
        Index < SamplerCount;
        ++Index)
    {
        *pSamplerBindings = DefaultBinding;
        pSamplerBindings[Index].binding = Index;
    }
}

void
CreatePushConstantBlock(shoora_vulkan_graphics_pipeline *Pipeline, VkShaderStageFlags ShaderStage, u64 Size, u64 Offset)
{
    VkPushConstantRange PushConstant = {};
    PushConstant.stageFlags = ShaderStage;
    PushConstant.offset = Offset;
    PushConstant.size = Size;

    Pipeline->PushConstant.Ranges[0] = PushConstant;
    ++Pipeline->PushConstant.Count;
}

void
CreateSwapchainUniformResources(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                                size_t VertUniformBufferSize, size_t FragUniformBufferSize,
                                const char **ppImageFilenames, u32 ImageFilenameCount,
                                VkPushConstantRange *PushConstants, u32 PushConstantCount,
                                VkPipelineLayout *pPipelineLayout)
{
    VkDescriptorPoolSize Sizes[3];
    Sizes[0] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHU_MAX_FRAMES_IN_FLIGHT);
    Sizes[1] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                     ARRAY_SIZE(Swapchain->FragImageSamplers));
    Sizes[2] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHU_MAX_FRAMES_IN_FLIGHT);
    CreateDescriptorPool(RenderDevice, ARRAY_SIZE(Sizes), Sizes, 100, &Swapchain->UniformDescriptorPool);

    // TODO)): Create one merged descirptor which encapsulates data for all the uniform buffers we need.
    // NOTE: 1st Descriptor Set(Uniform Buffer used in Vertex Shadder)

    // NOTE: Get all the data the shader is going to use
    CreateUniformBuffers(RenderDevice, Swapchain->UniformBuffers, Swapchain->ImageCount, VertUniformBufferSize);

    auto SetLayoutBinding = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                                          VK_SHADER_STAGE_VERTEX_BIT);
    CreateDescriptorSetLayout(RenderDevice, &SetLayoutBinding, 1, &Swapchain->UniformSetLayout);
    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        AllocateDescriptorSets(RenderDevice, Swapchain->UniformDescriptorPool, 1, &Swapchain->UniformSetLayout,
                               &Swapchain->UniformDescriptorSets[Index]);
        UpdateBufferDescriptorSet(RenderDevice, Swapchain->UniformDescriptorSets[Index], 0,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain->UniformBuffers[Index].Handle,
                                  Swapchain->UniformBuffers[Index].MemSize);
    }

    // NOTE: 2nd Descriptor Set(Image Sampler used in Fragment shader)
    for(u32 Index = 0;
        Index < ImageFilenameCount;
        ++Index)
    {
        CreateCombinedImageSampler(RenderDevice, ppImageFilenames[Index],
                                   &Swapchain->FragImageSamplers[Index]);
    }

    for(u32 Index = ImageFilenameCount;
        Index < ARRAY_SIZE(Swapchain->FragImageSamplers);
        ++Index)
    {
        CreatePlaceholderTextureSampler(RenderDevice, &Swapchain->FragImageSamplers[Index],
                                        DefaultTexType::TexType_WHITE);
    }

#if 0
    CreateCombinedImageSampler(RenderDevice, "images/wall10/wall10.jpg", &Swapchain->FragImageSamplers[0]);
    CreateCombinedImageSampler(RenderDevice, "images/wall10/wall10_NRM.jpg", &Swapchain->FragImageSamplers[1]);
#endif

    // CreateCombinedImageSampler(RenderDevice, "images/wall10/wall10_SPEC.jpg", &Swapchain->FragImageSamplers[2]);
    auto FragSamplerBinding = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3,
                                                            VK_SHADER_STAGE_FRAGMENT_BIT);
    CreateDescriptorSetLayout(RenderDevice, &FragSamplerBinding, 1, &Swapchain->FragSamplersSetLayout);
    AllocateDescriptorSets(RenderDevice, Swapchain->UniformDescriptorPool, 1,
                           &Swapchain->FragSamplersSetLayout, &Swapchain->FragSamplersDescriptorSet);

    VkDescriptorImageInfo ImageInfos[ARRAY_SIZE(Swapchain->FragImageSamplers)];
    for(u32 Index = 0;
        Index < ARRAY_SIZE(Swapchain->FragImageSamplers);
        ++Index)
    {
        VkDescriptorImageInfo *ImageDescriptorInfo = ImageInfos + Index;
        ImageDescriptorInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageDescriptorInfo->imageView = Swapchain->FragImageSamplers[Index].Image.ImageView;
        ImageDescriptorInfo->sampler = Swapchain->FragImageSamplers[Index].Sampler;
    }
    UpdateImageDescriptorSets(RenderDevice, Swapchain->FragSamplersDescriptorSet, 0,
                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ARRAY_SIZE(ImageInfos), ImageInfos);

    // NOTE: 3rd Descriptor Set: Uniform Buffer used in the fragment shader
    CreateUniformBuffers(RenderDevice, Swapchain->FragUniformBuffers, Swapchain->ImageCount,
                         FragUniformBufferSize);
    auto FragUniformBinding = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                                            VK_SHADER_STAGE_FRAGMENT_BIT);
    CreateDescriptorSetLayout(RenderDevice, &FragUniformBinding, 1, &Swapchain->FragUniformsSetLayout);

    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        AllocateDescriptorSets(RenderDevice, Swapchain->UniformDescriptorPool, 1, &Swapchain->FragUniformsSetLayout,
                               &Swapchain->FragUniformsDescriptorSets[Index]);
        UpdateBufferDescriptorSet(RenderDevice, Swapchain->FragUniformsDescriptorSets[Index], 0,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain->FragUniformBuffers[Index].Handle,
                                  Swapchain->FragUniformBuffers[Index].MemSize);
    }

    VkDescriptorSetLayout SetLayouts[3] = {Swapchain->UniformSetLayout, Swapchain->FragSamplersSetLayout,
                                           Swapchain->FragUniformsSetLayout};

    CreatePipelineLayout(RenderDevice, ARRAY_SIZE(SetLayouts), SetLayouts, PushConstantCount, PushConstants,
                         pPipelineLayout);
}

void
DestroySwapchainUniformResources(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    for (u32 Index = 0; Index < Swapchain->ImageCount; ++Index)
    {
        DestroyUniformBuffer(RenderDevice, &Swapchain->UniformBuffers[Index]);
        DestroyUniformBuffer(RenderDevice, &Swapchain->FragUniformBuffers[Index]);
    }

    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, Swapchain->UniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, Swapchain->FragUniformsSetLayout, nullptr);

    for(u32 Index = 0;
        Index < ARRAY_SIZE(Swapchain->FragImageSamplers);
        ++Index)
    {
        vkDestroySampler(RenderDevice->LogicalDevice, Swapchain->FragImageSamplers[Index].Sampler, nullptr);
        DestroyImage2D(RenderDevice, &Swapchain->FragImageSamplers[Index].Image);
    }
    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, Swapchain->FragSamplersSetLayout, nullptr);

    vkDestroyDescriptorPool(RenderDevice->LogicalDevice, Swapchain->UniformDescriptorPool, nullptr);
    // VkDescriptorSet UniformDescriptorSets[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
}

void
DestroyPresentationSurface(shoora_vulkan_context *Context)
{
    vkDestroySurfaceKHR(Context->Instance, Context->Swapchain.Surface, 0);
    LogOutput(LogType_Info, "Destroyed Presentation Surface!\n");
}

void
DestroySwapchain(shoora_vulkan_context *Context)
{
    DestroySwapchainFramebuffers(&Context->Device, &Context->Swapchain);
    DestroySwapchainImageViews(&Context->Device, &Context->Swapchain);
    DestroyImage2D(&Context->Device, &Context->Swapchain.DepthStencilImage);
    vkDestroySwapchainKHR(Context->Device.LogicalDevice, Context->Swapchain.Handle, 0);
    LogOutput(LogType_Warn, "Destroyed Swapchain!\n");
}