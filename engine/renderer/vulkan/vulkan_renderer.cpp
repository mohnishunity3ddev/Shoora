#include "vulkan_input_info.h"


void
InitializeVulkanRenderer(shura_vulkan_context *Context, shura_app_info *AppInfo)
{
    VK_CHECK(volkInitialize());

    ShuraInstanceCreateInfo.AppName = AppInfo->AppName;
    CreateVulkanInstance(Context, &ShuraInstanceCreateInfo);
    volkLoadInstance(Context->Instance);

#ifdef _DEBUG
    SetupDebugCallbacks(Context, DebugCreateInfo);
#endif

    CreatePresentationSurface(Context, &Context->Swapchain.Surface);
    CreateLogicalDeviceAndGetQueues(Context, &DeviceCreateInfo);
    volkLoadDevice(Context->Device.LogicalDevice);

    CreateSwapchain(Context, &SwapchainInfo);
}

void
DestroyVulkanRenderer(shura_vulkan_context *Context)
{
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(Context);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput("Destroyed Vulkan Renderer!\n");
}
