#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include <memory.h>

static shoora_vulkan_context *Context = nullptr;
// NOTE: Triangle
// static shoora_vertex_info Vertices[] =
// {
//     {.VertexPos = Vec2( 0.0f,  0.5f), .VertexColor = Vec3(1, 0, 0)},
//     {.VertexPos = Vec2( 0.5f, -0.5f), .VertexColor = Vec3(0, 1, 0)},
//     {.VertexPos = Vec2(-0.5f, -0.5f), .VertexColor = Vec3(0, 0, 1)}
// };
// static u32 Indices[] = {0, 1, 2, 0, 1, 3};

// NOTE: Rectangle
static shoora_vertex_info Vertices[] =
{
    {.VertexPos = Vec2( 0.5f,  0.5f), .VertexColor = Vec3(1, 1, 0)},
    {.VertexPos = Vec2( 0.5f, -0.5f), .VertexColor = Vec3(0, 1, .32f)},
    {.VertexPos = Vec2(-0.5f, -0.5f), .VertexColor = Vec3(0.32f, 1, 1)},
    {.VertexPos = Vec2(-0.5f,  0.5f), .VertexColor = Vec3(0.32f, 0.21f, 0.66f)},
};
static u32 Indices[] = {0, 1, 2, 0, 2, 3};
struct uniform_data
{
    vec3 Color;
};
uniform_data UniformData = {};
static b32 WireframeMode = true;

static exit_application *QuitApplication;

void WindowResizedCallback(u32 Width, u32 Height)
{
    LogOutput(LogType_Debug, "Window Resized to {%d, %d}\n", Width, Height);

    if(Context && (Width > 0 && Height > 0))
    {
        ASSERT(Context->IsInitialized);
        CreateSwapchain(Context, Width, Height);
    }
}

void
InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo)
{
    VK_CHECK(volkInitialize());

    ShuraInstanceCreateInfo.AppName = AppInfo->AppName;

    CreateVulkanInstance(VulkanContext, &ShuraInstanceCreateInfo);
    volkLoadInstance(VulkanContext->Instance);

#ifdef _DEBUG
    SetupDebugCallbacks(VulkanContext, DebugCreateInfo);
#endif

    shoora_vulkan_device *RenderDevice = &VulkanContext->Device;
    shoora_vulkan_swapchain *Swapchain = &VulkanContext->Swapchain;

    CreatePresentationSurface(VulkanContext, &Swapchain->Surface);
    CreateDeviceAndQueues(VulkanContext, &DeviceCreateInfo);
    volkLoadDevice(RenderDevice->LogicalDevice);
    CreateCommandPools(RenderDevice);

    CreateSwapchain(VulkanContext, AppInfo->WindowWidth, AppInfo->WindowHeight, &SwapchainInfo, sizeof(uniform_data));
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    CreateVertexBuffer(RenderDevice, Vertices, ARRAY_SIZE(Vertices), Indices, ARRAY_SIZE(Indices),
                       &VulkanContext->VertexBuffer, &VulkanContext->IndexBuffer);

    CreateSwapchainUniformResources(RenderDevice, Swapchain, sizeof(uniform_data), VulkanContext->Pipeline.GraphicsPipelineLayout);
    CreateGraphicsPipeline(VulkanContext, "shaders/spirv/triangle.vert.spv", "shaders/spirv/triangle.frag.spv",
                           &Swapchain->UniformPipelineLayout);
    CreateWireframePipeline(VulkanContext, "shaders/spirv/wireframe.vert.spv", "shaders/spirv/wireframe.frag.spv");
    CreateSynchronizationPrimitives(&VulkanContext->Device, &VulkanContext->SyncHandles);

    AppInfo->WindowResizeCallback = &WindowResizedCallback;
    QuitApplication = AppInfo->ExitApplication;
    ASSERT(QuitApplication);

    VulkanContext->CurrentFrame = 0;
    VulkanContext->IsInitialized = true;
    VulkanContext->FrameCounter = 0;

    Context = VulkanContext;
}

void
AdvanceToNextFrame()
{
    ASSERT(Context != nullptr);

    if(++Context->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        Context->CurrentFrame = 0;
    }

    ++Context->FrameCounter;
}

void DrawFrameInVulkan()
{
    ASSERT(Context != nullptr);
    if(!Context->IsInitialized || Context->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        QuitApplication("[RENDERER]: Either the render is not initialized or there was some "
                        "problem with the current "
                        "frame counter.\n");
    }

    shoora_vulkan_fence_handle *pCurrentFrameFence = GetCurrentFrameFencePtr(&Context->SyncHandles,
                                                                             Context->CurrentFrame);
    shoora_vulkan_semaphore_handle *pCurrentFrameImageAvlSemaphore =
        GetImageAvailableSemaphorePtr(&Context->SyncHandles, Context->CurrentFrame);

    shoora_vulkan_semaphore_handle *pCurrentFramePresentCompleteSemaphore =
        GetRenderFinishedSemaphorePtr(&Context->SyncHandles, Context->CurrentFrame);

    VK_CHECK(vkWaitForFences(Context->Device.LogicalDevice, 1, &pCurrentFrameFence->Handle, VK_TRUE,
                             SHU_DEFAULT_FENCE_TIMEOUT));

    AcquireNextSwapchainImage(&Context->Device, &Context->Swapchain, pCurrentFrameImageAvlSemaphore);

    u32 ImageIndex = Context->Swapchain.CurrentImageIndex;
    shoora_vulkan_command_buffer_handle *pDrawCmdBuffer =
        &Context->Swapchain.DrawCommandBuffers[ImageIndex];
    VkCommandBuffer DrawCmdBuffer = pDrawCmdBuffer->Handle;

    // f32 RedColor = (f32)(Context->FrameCounter % 600) / 600.0f;
    // f32 GreenColor = (f32)(Context->FrameCounter % 1200) / 1200.0f;
    // f32 BlueColor = (f32)(Context->FrameCounter % 900) / 900.0f;

    UniformData.Color = Vec3(1, 1, 0);
    memcpy(Context->Swapchain.UniformBuffers[ImageIndex].pMapped, &UniformData,
           sizeof(UniformData));

    VK_CHECK(vkResetFences(Context->Device.LogicalDevice, 1, &pCurrentFrameFence->Handle));
    VK_CHECK(vkResetCommandBuffer(DrawCmdBuffer, 0));

    VkCommandBufferBeginInfo DrawCmdBufferBeginInfo = {};
    DrawCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkClearValue ClearValues[1];
    ClearValues[0].color = { {0.0f, 0.0f, 0.2f, 1.0f} };
    VkRect2D RenderArea = {};
    RenderArea.offset = {0, 0};
    RenderArea.extent = Context->Swapchain.ImageDimensions;
    VkRenderPassBeginInfo RenderPassBeginInfo = {};
    RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInfo.pNext = nullptr;
    RenderPassBeginInfo.renderPass = Context->GraphicsRenderPass;
    RenderPassBeginInfo.framebuffer = Context->Swapchain.ImageFramebuffers[ImageIndex];
    RenderPassBeginInfo.renderArea = RenderArea;
    RenderPassBeginInfo.clearValueCount = 1;
    RenderPassBeginInfo.pClearValues = ClearValues;
    VK_CHECK(vkBeginCommandBuffer(DrawCmdBuffer, &DrawCmdBufferBeginInfo));
        pDrawCmdBuffer->IsRecording = true;
        vkCmdBeginRenderPass(DrawCmdBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport Viewport = {};
        Viewport.width = (f32)RenderArea.extent.width;
        Viewport.height = -(f32)RenderArea.extent.height;
        Viewport.x = 0;
        Viewport.y = RenderArea.extent.height;
        vkCmdSetViewport(DrawCmdBuffer, 0, 1, &Viewport);
        VkRect2D Scissor = {};
        Scissor.extent = Context->Swapchain.ImageDimensions;
        vkCmdSetScissor(DrawCmdBuffer, 0, 1, &Scissor);

        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Context->Swapchain.UniformPipelineLayout, 0, 1,
                                &Context->Swapchain.UniformDescriptorSets[ImageIndex], 0,
                                nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->Pipeline.GraphicsPipeline);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &Context->VertexBuffer.Handle, offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, Context->IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(Indices), 1, 0, 0, 1);

        if(WireframeMode)
        {
            vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            Context->Pipeline.WireframeGraphicsPipeline);
            vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(Indices), 1, 0, 0, 1);
        }

        vkCmdEndRenderPass(DrawCmdBuffer);
    VK_CHECK(vkEndCommandBuffer(DrawCmdBuffer));

    //? Submit DrawCommandBuffer
    // NOTE: Wait for the ImageAvailableSemaphore To Be Signaled at this stage.
    VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.pNext = nullptr;
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = &pCurrentFrameImageAvlSemaphore->Handle;
    SubmitInfo.pWaitDstStageMask = &WaitStageMask;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &DrawCmdBuffer;
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = &pCurrentFramePresentCompleteSemaphore->Handle;
    VkQueue GraphicsQueue = GetQueueHandle(&Context->Device, QueueType_Graphics);
    VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, pCurrentFrameFence->Handle));

    //? Command Buffer should finish rendering. Present the Swapchain Image to the presentation engine
    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.pNext = nullptr;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &pCurrentFramePresentCompleteSemaphore->Handle;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &Context->Swapchain.SwapchainHandle;
    PresentInfo.pImageIndices = &Context->Swapchain.CurrentImageIndex;
    VK_CHECK(vkQueuePresentKHR(GraphicsQueue, &PresentInfo));

    AdvanceToNextFrame();
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    VK_CHECK(vkDeviceWaitIdle(Context->Device.LogicalDevice));
    shoora_vulkan_device *RenderDevice = &Context->Device;

    DestroySwapchainUniformResources(RenderDevice, &Context->Swapchain);

    DestroyAllSynchronizationPrimitives(RenderDevice, &Context->SyncHandles);
    DestroyVertexBuffer(RenderDevice, &Context->VertexBuffer, &Context->IndexBuffer);
    // DestroyAllSemaphores(Context);
    // DestroyAllFences(Context);
    DestroyPipelines(RenderDevice, &Context->Pipeline);
    DestroyRenderPass(RenderDevice, Context->GraphicsRenderPass);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(RenderDevice);
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}