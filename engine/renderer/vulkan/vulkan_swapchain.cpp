#include "vulkan_swapchain.h"
#include "platform/platform.h"

b32
CheckSupportedPresentModes(const shoora_vulkan_context *Context, VkPresentModeKHR DesiredPresentMode)
{
    b32 Result = false;
    u32 AvailablePresentModeCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(Context->Device.PhysicalDevice,
                                                       Context->Swapchain.Surface,
                                                       &AvailablePresentModeCount, 0));

    ASSERT(AvailablePresentModeCount > 0 && AvailablePresentModeCount <= 16);

    VkPresentModeKHR SupportedPresentModes[16];
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(Context->Device.PhysicalDevice,
                                                       Context->Swapchain.Surface,
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
GetSurfaceCapabilities(shoora_vulkan_context *Context, VkSurfaceCapabilitiesKHR *SurfaceCapabilities)
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context->Device.PhysicalDevice,
                                                       Context->Swapchain.Surface,
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

void
SelectImageFormats(shoora_vulkan_context *Context, VkFormat DesiredImageFormat, VkColorSpaceKHR DesiredColorSpace)
{
    u32 FormatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(Context->Device.PhysicalDevice, Context->Swapchain.Surface,
                                                  &FormatCount, 0));

    ASSERT(FormatCount > 0 && FormatCount <= 32);

    VkSurfaceFormatKHR SupportedFormats[32];
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(Context->Device.PhysicalDevice, Context->Swapchain.Surface,
                                                  &FormatCount, SupportedFormats));

    b32 FormatFound = false;
    // Checking to see if we can set the format to anything
    if((FormatCount == 1) &&
       (SupportedFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        Context->Swapchain.SurfaceFormat.colorSpace = DesiredColorSpace;
        Context->Swapchain.SurfaceFormat.format = DesiredImageFormat;
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
                Context->Swapchain.SurfaceFormat.colorSpace = SupportedFormat.colorSpace;
                Context->Swapchain.SurfaceFormat.format = SupportedFormat.format;
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
                    Context->Swapchain.SurfaceFormat.format = DesiredImageFormat;
                    Context->Swapchain.SurfaceFormat.colorSpace = SupportedFormat.colorSpace;
                    LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the one with "
                              "the same imageFormat.\n");
                    FormatFound = true;
                    break;
                }
            }

            if(!FormatFound)
            {
                Context->Swapchain.SurfaceFormat.format = SupportedFormats[0].format;
                Context->Swapchain.SurfaceFormat.colorSpace = SupportedFormats[0].colorSpace;
                LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the first "
                          "supported one!\n");
                FormatFound = true;
            }
        }
    }

    ASSERT(FormatFound);
}

void
PrepareForSwapchainCreation(shoora_vulkan_context *Context,
                            shoora_vulkan_swapchain_create_info *ShuraSwapchainInfo)
{
    if (!CheckSupportedPresentModes(Context, ShuraSwapchainInfo->DesiredPresentMode))
    {
        // NOTE: Vulkan Makes sure Atleast FIFO is supported!
        ShuraSwapchainInfo->DesiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    Context->Swapchain.PresentMode = ShuraSwapchainInfo->DesiredPresentMode;

    GetSurfaceCapabilities(Context, &Context->Swapchain.SurfaceCapabilities);
    SelectSwapchainImageCount(&Context->Swapchain);
    SelectSwapchainSize(&Context->Swapchain);
    SelectDesiredImageUsage(&Context->Swapchain, ShuraSwapchainInfo->DesiredImageUsages);
    SelectImageTransforms(&Context->Swapchain, ShuraSwapchainInfo->DesiredTransformFlagBits);
    SelectImageFormats(Context, ShuraSwapchainInfo->DesiredImageFormat,
                       ShuraSwapchainInfo->DesiredImageColorSpace);
}

void
GetSwapchainImageHandles(shoora_vulkan_context *Context)
{
    u32 SwapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle,
                                     &SwapchainImageCount, 0));

    // NOTE: Drivers can produce more images that were actually requested during swapchain creation.
    ASSERT((SwapchainImageCount >= Context->Swapchain.ImageCount) &&
           (SwapchainImageCount <= ARRAY_SIZE(Context->Swapchain.Images)));

    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle,
                                     &SwapchainImageCount, Context->Swapchain.Images));
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
CreateSwapchainFramebuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    // for(u32 ImageIndex = 0;
    //     ImageIndex < Swapchain->ImageCount;
    //     ++ImageIndex)
    // {
    //     VkFramebufferCreateInfo CreateInfo;
    //     CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //     CreateInfo.pNext = nullptr;
    //     CreateInfo.flags = 0;
    //     CreateInfo.renderPass = ;
    //     CreateInfo.attachmentCount = ;
    //     CreateInfo.pAttachments = ;
    //     CreateInfo.width = ;
    //     CreateInfo.height = ;
    //     CreateInfo.layers = ;

    //     VK_CHECK(vkCreateFramebuffer(RenderDevice->LogicalDevice, &CreateInfo, nullptr,
    //                                  &Swapchain->Framebuffers[ImageIndex]));
    // }
}

void
DestroySwapchainImageViews(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain)
{
    for(u32 Index = 0;
        Index < Swapchain->ImageCount;
        ++Index)
    {
        vkDestroyImageView(RenderDevice->LogicalDevice, Swapchain->ImageViews[Index], nullptr);
    }

    LogOutput(LogType_Warn, "Swapchain Image Views are destroyed!\n");
}

void
CreateSwapchain(shoora_vulkan_context *Context,
                shoora_vulkan_swapchain_create_info *ShuraSwapchainInfo)
{
    if(Context->Swapchain.SwapchainHandle == VK_NULL_HANDLE)
    {
        ASSERT(ShuraSwapchainInfo != nullptr);
        PrepareForSwapchainCreation(Context, ShuraSwapchainInfo);
    }
    else
    {
        DestroySwapchainImageViews(&Context->Device, &Context->Swapchain);
        // TODO)): Destroy Swapchain Framebuffers
    }

    shoora_vulkan_swapchain *SwapchainInfo = &Context->Swapchain;
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
    CreateInfo.pQueueFamilyIndices = 0;
    CreateInfo.preTransform = SwapchainInfo->TransformFlagBits;

    // TODO)): Read More about this!
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    CreateInfo.presentMode = SwapchainInfo->PresentMode;
    CreateInfo.clipped = VK_TRUE;

    VkSwapchainKHR OldSwapchain = Context->Swapchain.SwapchainHandle;
    CreateInfo.oldSwapchain = OldSwapchain;

    VK_CHECK(vkCreateSwapchainKHR(Context->Device.LogicalDevice, &CreateInfo, 0,
                                  &Context->Swapchain.SwapchainHandle));
    Context->Swapchain.SurfaceFormat = SwapchainInfo->SurfaceFormat;

    if(Context->Swapchain.SwapchainHandle == VK_NULL_HANDLE)
    {
        LogOutput(LogType_Error, "There was a problem creating the swapchain!\n");
    }

    if(OldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(Context->Device.LogicalDevice, OldSwapchain, 0);
        LogOutput(LogType_Warn, "Destroyed Old Swapchain!\n");
    }

    LogOutput(LogType_Info, "Swapchain Created!\n");
    GetSwapchainImageHandles(Context);
    CreateSwapchainImageViews(&Context->Device, &Context->Swapchain);
    // TODO)): Have to create a renderpass to create these framebuffers
    CreateSwapchainFramebuffers(&Context->Device, &Context->Swapchain);
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

u32
AcquireNextSwapchainImage(shoora_vulkan_context *Context, u32 SemaphoreInternalIndex, u32 FenceInternalIndex)
{
    u32 ImageIndex = 0;
    VkSemaphore SemaphoreHandle = Context->SyncHandles.Semaphores[SemaphoreInternalIndex].Handle;
    VkFence FenceHandle = Context->SyncHandles.Fences[FenceInternalIndex].Handle;

    // NOTE: We can wait upto 2 seocnds to acquire the new swapchain image, if not throw an error.
    VkResult AcquireResult = vkAcquireNextImageKHR(Context->Device.LogicalDevice,
                                                   Context->Swapchain.SwapchainHandle, NANOSECONDS(2),
                                                   SemaphoreHandle, FenceHandle, &ImageIndex);
    if((AcquireResult != VK_SUCCESS) &&
       (AcquireResult != VK_SUBOPTIMAL_KHR))
    {
        LogOutput(LogType_Error, "There was a problem acquiring a swapchain image!\n");

        if(AcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            LogOutput(LogType_Error, "You cannot use the images of this swapchain. Destroy the swapchain and "
                                     "recreate it again!\n");
            return -1;
        }
    }

    ASSERT(ImageIndex >= 0 && ImageIndex < Context->Swapchain.ImageCount);
    return ImageIndex;
}

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

void
DestroyPresentationSurface(shoora_vulkan_context *Context)
{
    vkDestroySurfaceKHR(Context->Instance, Context->Swapchain.Surface, 0);
    LogOutput(LogType_Info, "Destroyed Presentation Surface!\n");
}

void
DestroySwapchain(shoora_vulkan_context *Context)
{
    DestroySwapchainImageViews(&Context->Device, &Context->Swapchain);
    vkDestroySwapchainKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle, 0);
    LogOutput(LogType_Warn, "Destroyed Swapchain!\n");
}
