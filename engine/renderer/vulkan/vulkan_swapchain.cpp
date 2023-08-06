#include "vulkan_swapchain.h"
#include "platform/platform.h"

b32
CheckSupportedPresentModes(const shura_vulkan_context *Context, VkPresentModeKHR DesiredPresentMode)
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
GetSurfaceCapabilities(shura_vulkan_context *Context, VkSurfaceCapabilitiesKHR *SurfaceCapabilities)
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context->Device.PhysicalDevice,
                                                       Context->Swapchain.Surface,
                                                       SurfaceCapabilities));
}

void
SelectSwapchainImageCount(shura_vulkan_swapchain *Swapchain)
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

    Swapchain->SwapchainImageCount = ImageCount;
}

void
SelectSwapchainSize(shura_vulkan_swapchain *Swapchain)
{
    VkSurfaceCapabilitiesKHR *SurfaceCapabilities = &Swapchain->SurfaceCapabilities;

    Swapchain->ImageSize = SurfaceCapabilities->currentExtent;
    // Checking if size of the images determine size of the window.
    if(Swapchain->ImageSize.width == -1UL)
    {
        Swapchain->ImageSize.width = ClampToRange(Swapchain->ImageSize.width,
                                                  SurfaceCapabilities->minImageExtent.width,
                                                  SurfaceCapabilities->maxImageExtent.width);
        Swapchain->ImageSize.height = ClampToRange(Swapchain->ImageSize.height,
                                                   SurfaceCapabilities->minImageExtent.height,
                                                   SurfaceCapabilities->maxImageExtent.height);
    }

    ASSERT((Swapchain->ImageSize.width > 0) &&
           (Swapchain->ImageSize.width > 0));
}

// Do we use them as render targets, or copy src/dest or sample from them for postprocessing
void
SelectDesiredImageUsage(shura_vulkan_swapchain *Swapchain, VkImageUsageFlags DesiredImageUsageFlags)
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
SelectImageTransforms(shura_vulkan_swapchain *Swapchain, VkSurfaceTransformFlagBitsKHR DesiredTransformFlagBits)
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
SelectImageFormats(shura_vulkan_context *Context, VkFormat DesiredImageFormat, VkColorSpaceKHR DesiredColorSpace)
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
        Context->Swapchain.ImageFormat.colorSpace = DesiredColorSpace;
        Context->Swapchain.ImageFormat.format = DesiredImageFormat;
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
                Context->Swapchain.ImageFormat.colorSpace = SupportedFormat.colorSpace;
                Context->Swapchain.ImageFormat.format = SupportedFormat.format;
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
                    Context->Swapchain.ImageFormat.format = DesiredImageFormat;
                    Context->Swapchain.ImageFormat.colorSpace = SupportedFormat.colorSpace;
                    LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the one with "
                              "the same imageFormat.\n");
                    FormatFound = true;
                    break;
                }
            }

            if(!FormatFound)
            {
                Context->Swapchain.ImageFormat.format = SupportedFormats[0].format;
                Context->Swapchain.ImageFormat.colorSpace = SupportedFormats[0].colorSpace;
                LogOutput(LogType_Warn, "Desired ColorSpace ImageFormat Combination was not found. Selecting the first "
                          "supported one!\n");
                FormatFound = true;
            }
        }
    }

    ASSERT(FormatFound);
}

void
PrepareForSwapchainCreation(shura_vulkan_context *Context,
                            shura_vulkan_swapchain_create_info *ShuraSwapchainInfo)
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
GetSwapchainImageHandles(shura_vulkan_context *Context)
{
    u32 SwapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle,
                                     &SwapchainImageCount, 0));

    // NOTE: Drivers can produce more images that were actually requested during swapchain creation.
    ASSERT((SwapchainImageCount >= Context->Swapchain.SwapchainImageCount) &&
           (SwapchainImageCount <= ARRAY_SIZE(Context->Swapchain.SwapchainImages)));

    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle,
                                     &SwapchainImageCount, Context->Swapchain.SwapchainImages));
    LogOutput(LogType_Info, "Got the Swapchain Image Handles!\n");
}

void
CreateSwapchain(shura_vulkan_context *Context,
                shura_vulkan_swapchain_create_info *ShuraSwapchainInfo)
{
    PrepareForSwapchainCreation(Context, ShuraSwapchainInfo);
    shura_vulkan_swapchain *SwapchainInfo = &Context->Swapchain;

    VkSwapchainCreateInfoKHR CreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    CreateInfo.pNext = 0;
    CreateInfo.flags = 0;
    CreateInfo.surface = SwapchainInfo->Surface;
    CreateInfo.minImageCount = SwapchainInfo->SwapchainImageCount;
    CreateInfo.imageFormat = SwapchainInfo->ImageFormat.format;
    CreateInfo.imageColorSpace = SwapchainInfo->ImageFormat.colorSpace;
    CreateInfo.imageExtent = SwapchainInfo->ImageSize;

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
    if(Context->Swapchain.SwapchainHandle == VK_NULL_HANDLE)
    {
        LogOutput(LogType_Error, "There was a problem creating the swapchain!\n");
    }

    if(OldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(Context->Device.LogicalDevice, OldSwapchain, 0);
    }

    LogOutput(LogType_Info, "Swapchain Created!\n");
    GetSwapchainImageHandles(Context);
}

void
CreatePresentationSurface(shura_vulkan_context *Context, VkSurfaceKHR *Surface)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};

    shura_platform_presentation_surface Win32Surface = {&SurfaceCreateInfo};
    FillVulkanWin32SurfaceCreateInfo(&Win32Surface);

    VK_CHECK(vkCreateWin32SurfaceKHR(Context->Instance, &SurfaceCreateInfo, 0, Surface));
    LogOutput(LogType_Info, "Created Presentation Surface!\n");
#endif
}

u32
AcquireNextSwapchainImage(shura_vulkan_context *Context)
{
    u32 ImageIndex = 0;

    // NOTE: We can wait upto 2 seocnds to acquire the new swapchain image, if not throw an error.
    VkResult AcquireResult = vkAcquireNextImageKHR(Context->Device.LogicalDevice,
                                                   Context->Swapchain.SwapchainHandle, NANOSECONDS(2),
                                                   Context->Semaphore, Context->Fence, &ImageIndex);
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

    ASSERT(ImageIndex >= 0 && ImageIndex < Context->Swapchain.SwapchainImageCount);
    return ImageIndex;
}

void
PresentImage(shura_vulkan_context *Context)
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
DestroyPresentationSurface(shura_vulkan_context *Context)
{
    vkDestroySurfaceKHR(Context->Instance, Context->Swapchain.Surface, 0);
    LogOutput(LogType_Info, "Destroyed Presentation Surface!\n");
}

void
DestroySwapchain(shura_vulkan_context *Context)
{
    vkDestroySwapchainKHR(Context->Device.LogicalDevice, Context->Swapchain.SwapchainHandle, 0);
    LogOutput(LogType_Info, "Destroyed Swapchain!\n");
}
