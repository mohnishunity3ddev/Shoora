#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"

void
InitializeVulkanRenderer(shoora_vulkan_context *Context, shoora_app_info *AppInfo)
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

    shoora_vulkan_command_buffer *CmdBufferGroup = GetCommandBufferGroupForQueue(Context, QueueType_Graphics);
    u32 CmdBufferInternalIndex = CmdBufferGroup->BufferCount - 1;

    ResetAllCommandPools(&Context->Device, true);

    BeginCommandBuffer(CmdBufferGroup, CmdBufferInternalIndex, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    EndCommandBuffer(CmdBufferGroup, CmdBufferInternalIndex);

    CreateAllSemaphores(Context);
    CreateAllFences(Context);
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    DeviceWaitIdle(Context->Device.LogicalDevice);

    DestroyAllSemaphores(Context);
    DestroyAllFences(Context);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(&Context->Device);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}
