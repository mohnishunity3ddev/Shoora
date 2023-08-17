#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"

static shoora_vulkan_context *VulkanContext = nullptr;

void WindowResizedCallback(u32 Width, u32 Height)
{
    LogOutput(LogType_Debug, "Window Resized to {%d, %d}\n", Width, Height);

    if(VulkanContext && (Width > 0 && Height > 0))
    {
        CreateSwapchain(VulkanContext, Width, Height);
    }
}

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
    CreateDeviceAndQueues(Context, &DeviceCreateInfo);
    volkLoadDevice(Context->Device.LogicalDevice);

    CreateSwapchain(Context, AppInfo->WindowWidth, AppInfo->WindowHeight, &SwapchainInfo);
    CreateRenderPass(&Context->Device, &Context->Swapchain, &Context->RenderPass);
    CreateGraphicsPipeline(Context, "shaders/spirv/triangle.vert.spv", "shaders/spirv/triangle.frag.spv");
    CreateSwapchainFramebuffers(&Context->Device, &Context->Swapchain, Context->RenderPass);

    CreateCommandPools(&Context->Device);

    AppInfo->WindowResizeCallback = &WindowResizedCallback;
    VulkanContext = Context;
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    DeviceWaitIdle(Context->Device.LogicalDevice);

    // DestroyAllSemaphores(Context);
    // DestroyAllFences(Context);
    DestroyPipeline(&Context->Device, &Context->Pipeline);
    DestroyRenderPass(&Context->Device, Context->RenderPass);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(&Context->Device);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}