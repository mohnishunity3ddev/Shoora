#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"

static shoora_vulkan_context *VulkanContext = nullptr;
static shoora_vertex_info Vertices[] =
{
    {.VertexPos = Vec2(0, 0), .VertexColor = Vec3(1,1,1)},
    {.VertexPos = Vec2(0, 0), .VertexColor = Vec3(1,1,1)},
    {.VertexPos = Vec2(0, 0), .VertexColor = Vec3(1,1,1)}
};
static u32 Indices[] = {0, 1, 2};
static exit_application *QuitApplication;

void WindowResizedCallback(u32 Width, u32 Height)
{
    LogOutput(LogType_Debug, "Window Resized to {%d, %d}\n", Width, Height);

    if(VulkanContext && (Width > 0 && Height > 0))
    {
        ASSERT(VulkanContext->IsInitialized);
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
    CreateCommandPools(&Context->Device);

    CreateSwapchain(Context, AppInfo->WindowWidth, AppInfo->WindowHeight, &SwapchainInfo);
    CreateRenderPass(&Context->Device, &Context->Swapchain, &Context->GraphicsRenderPass);
    CreateSwapchainFramebuffers(&Context->Device, &Context->Swapchain, Context->GraphicsRenderPass);

    CreateGraphicsPipeline(Context, "shaders/spirv/triangle.vert.spv", "shaders/spirv/triangle.frag.spv");
    CreateVertexBuffer(&Context->Device, Vertices, ARRAY_SIZE(Vertices), Indices, ARRAY_SIZE(Indices),
                       &Context->VertexBuffer, &Context->IndexBuffer);
    CreateSynchronizationPrimitives(&Context->Device, &Context->SyncHandles);

    AppInfo->WindowResizeCallback = &WindowResizedCallback;
    QuitApplication = AppInfo->ExitApplication;
    ASSERT(QuitApplication);

    Context->CurrentFrame = 3;
    Context->IsInitialized = true;
    VulkanContext = Context;
}

void DrawFrameInVulkan()
{
    if(!VulkanContext->IsInitialized || VulkanContext->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        QuitApplication("[RENDERER]: Either the render is not initialized or there was some "
                        "problem with the current "
                        "frame counter.\n");
    }

    
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    VK_CHECK(vkDeviceWaitIdle(Context->Device.LogicalDevice));

    DestroyAllSynchronizationPrimitives(&Context->Device, &Context->SyncHandles);
    DestroyVertexBuffer(&Context->Device, &Context->VertexBuffer, &Context->IndexBuffer);
    // DestroyAllSemaphores(Context);
    // DestroyAllFences(Context);
    DestroyPipeline(&Context->Device, &Context->Pipeline);
    DestroyRenderPass(&Context->Device, Context->GraphicsRenderPass);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(&Context->Device);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}