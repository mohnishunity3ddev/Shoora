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
    CreateDeviceNQueuesNCommandPools(Context, &DeviceCreateInfo);
    volkLoadDevice(Context->Device.LogicalDevice);

    CreateSwapchain(Context, &SwapchainInfo);

    AllocateCommandBuffers(Context, Shu_BufferAllocInfos, ARRAY_SIZE(Shu_BufferAllocInfos));

    shura_vulkan_command_buffer *CmdBuffer = &Context->CommandBuffers[QueueType_Graphics];
    u32 CmdBufferIndex = CmdBuffer->BufferCount - 1;
    BeginCommandBuffer(CmdBuffer, CmdBufferIndex, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    EndCommandBuffer(CmdBuffer, CmdBufferIndex);
}

void
DestroyVulkanRenderer(shura_vulkan_context *Context)
{
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(&Context->Device);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}
