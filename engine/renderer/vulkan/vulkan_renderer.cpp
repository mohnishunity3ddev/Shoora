#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_imgui.h"
#include "loaders/image/png_loader.h"

#define SHU_USE_GLM 0
#if SHU_USE_GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include <memory.h>

static shoora_vulkan_context *Context = nullptr;
// NOTE: Triangle
static shoora_vertex_info TriangleVertices[] =
{
    {.VertexPos = Shu::vec2f{ 0.0f,  0.5f}, .VertexColor = Shu::vec3f{1, 0, 0}},
    {.VertexPos = Shu::vec2f{ 0.5f, -0.5f}, .VertexColor = Shu::vec3f{0, 1, 0}},
    {.VertexPos = Shu::vec2f{-0.5f, -0.5f}, .VertexColor = Shu::vec3f{0, 0, 1}}
};
static u32 TriangleIndices[] = {0, 1, 2};

// NOTE: Rectangle
static shoora_vertex_info RectVertices[] =
{
    {.VertexPos = Shu::vec2f{ 1.0f,  1.0f}, .VertexColor = Shu::vec3f{1, 0, 0}, .VertexUV = Shu::vec2f{1, 1}},
    {.VertexPos = Shu::vec2f{ 1.0f, -1.0f}, .VertexColor = Shu::vec3f{0, 1, 0}, .VertexUV = Shu::vec2f{1, 0}},
    {.VertexPos = Shu::vec2f{-1.0f, -1.0f}, .VertexColor = Shu::vec3f{0, 0, 1}, .VertexUV = Shu::vec2f{0, 0}},
    {.VertexPos = Shu::vec2f{-1.0f,  1.0f}, .VertexColor = Shu::vec3f{0, 0, 0}, .VertexUV = Shu::vec2f{0, 1}},
};
static u32 RectIndices[] = {0, 1, 2, 0, 2, 3};

struct uniform_data
{
#if SHU_USE_GLM
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Projection;
#else
    Shu::mat4f Model;
    Shu::mat4f View;
    Shu::mat4f Projection;
#endif

    Shu::vec3f Color;
};

static uniform_data UniformData = {};

struct shoora_render_state
{
    b8 WireframeMode = false;
    f32 WireLineWidth = 10.0f;
    Shu::vec3f ClearColor = Shu::vec3f{0.2f, 0.3f, 0.3f};
    Shu::vec3f MeshColorUniform = Shu::vec3f{1.0f, 1.0f, 1.0f};
};
struct shoora_debug_overlay
{
    f32 MsPerFrame = 0.0f;
    u32 Fps = 0;
} DebugOverlay;
static f32 DeltaTime = 0.0f;
static shoora_render_state RenderState;
static f32 UiUpdateTimer = 0.0f;
static const f32 UiUpdateWaitTime = 1;
static const Shu::mat4f Mat4Identity = Shu::Mat4f(1.0f);
static Shu::vec2f LastFrameMousePos = Shu::Vec2f(FLT_MAX, FLT_MAX);
static exit_application *QuitApplication;

void WindowResizedCallback(u32 Width, u32 Height)
{
    LogOutput(LogType_Debug, "Window Resized to {%d, %d}\n", Width, Height);

    if(Context && (Width > 0 && Height > 0))
    {
        ASSERT(Context->IsInitialized);
        WindowResized(&Context->Device, &Context->Swapchain, Context->GraphicsRenderPass,
                      Shu::vec2u{Width, Height});
        ImGuiUpdateWindowSize(Shu::vec2u{Width, Height});
    }
}

void
ImGuiNewFrame()
{
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    ImGui::SetNextWindowPos(ImVec2(800, 100), 1 << 2);
    ImGui::SetNextWindowSize(ImVec2(400, 400), 1 << 2);

    ImGui::Begin("Inspector");
    ImGui::Checkbox("Toggle Wireframe", (bool *)&RenderState.WireframeMode);
    ImGui::SliderFloat("Wireframe Line Width", &RenderState.WireLineWidth, 1.0f, 10.0f);
    ImGui::ColorEdit3("Clear Color", RenderState.ClearColor.E);

    ImGui::ColorEdit3("Rectangle Color", RenderState.MeshColorUniform.E);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(0, 0), 1 << 2);
    ImGui::SetNextWindowSize(ImVec2(400, 400), 1 << 2);
    ImGui::Begin("Debug Stats");
    ImGui::Text("Device: %s", Context->Device.DeviceProperties.deviceName);
    ImGui::Text("FPS: %u", DebugOverlay.Fps);
    ImGui::Text("MS Per Frame: %f", DebugOverlay.MsPerFrame);
#if SHU_USE_GLM
    ImGui::TextUnformatted("Using GLM!");
#endif
    ImGui::End();

    ImGui::Render();
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

    Shu::vec2u ScreenDim = Shu::vec2u{AppInfo->WindowWidth, AppInfo->WindowHeight};
    CreateSwapchain(&VulkanContext->Device, &VulkanContext->Swapchain, ScreenDim, &SwapchainInfo);
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    CreateVertexBuffer(RenderDevice, RectVertices, ARRAY_SIZE(RectVertices), RectIndices, ARRAY_SIZE(RectIndices),
                       &VulkanContext->VertexBuffer, &VulkanContext->IndexBuffer);

    CreateSwapchainUniformResources(RenderDevice, Swapchain, sizeof(uniform_data),
                                    &VulkanContext->Pipeline.GraphicsPipelineLayout);
    CreateGraphicsPipeline(VulkanContext, "shaders/spirv/triangle.vert.spv", "shaders/spirv/triangle.frag.spv",
                           &VulkanContext->Pipeline);

    // Wireframe
    CreatePipelineLayout(RenderDevice, 1, &VulkanContext->Swapchain.UniformSetLayout, 0, nullptr,
                         &VulkanContext->Pipeline.WireframePipelineLayout);
    CreateWireframePipeline(VulkanContext, "shaders/spirv/wireframe.vert.spv", "shaders/spirv/wireframe.frag.spv");

    CreateSynchronizationPrimitives(&VulkanContext->Device, &VulkanContext->SyncHandles);

    PrepareImGui(RenderDevice, &VulkanContext->ImContext, ScreenDim, VulkanContext->GraphicsRenderPass);

    SetupCamera(&VulkanContext->Camera, Shu::Vec3f(0, 0, -10.0f), Shu::Vec3f(0, 1, 0));

    AppInfo->WindowResizeCallback = &WindowResizedCallback;
    QuitApplication = AppInfo->ExitApplication;
    ASSERT(QuitApplication);

    VulkanContext->CurrentFrame = 0;
    VulkanContext->IsInitialized = true;
    VulkanContext->FrameCounter = 0;

    UiUpdateTimer = 0.0f;

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

static f32 Angle = 0.0f;
static const f32 AngleSpeed = 50.0f;

static f32 Yaw = 0.0f;
static f32 CamXPosDelta = 0.0f;
static i32 DeltaSign = 1;

void
WriteUniformData(u32 ImageIndex, f32 Delta)
{
    Angle += Delta;
    if(Angle > 360.0f)
    {
        Angle = 0.0f;
    }

#if SHU_USE_GLM
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(1.0f, 1.0f, 1.0f));
    Model = glm::rotate(Model, Angle*AngleSpeed/50.0f, glm::vec3(0.0, 0.0f, 1.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));
    UniformData.Model = Model;

    Context->Camera.Pos.z += Delta;
    Context->Camera.UpdateCameraVectors();

    f32 ZPos = Context->Camera.Pos.z;
    glm::mat4 View = glm::mat4(1.0f);
    View = glm::lookAt(glm::vec3(0.0f, 0.0f, ZPos), glm::vec3(0.0f, 0.0f, ZPos + 1.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));
    UniformData.View = View;

    glm::mat4 Projection = glm::mat4(1.0f);
    Projection = glm::perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    UniformData.Projection = Projection;
#else
    Shu::mat4f Model = Mat4Identity;
    Shu::Scale(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f));
    Shu::RotateGimbalLock(Model, Shu::Vec3f(0.0f, 0.0f, 1.0f), Angle * AngleSpeed);
    Shu::Translate(Model, Shu::Vec3f(0.0f, 0.0f, 0.0f));
    UniformData.Model = Model;

    Shu::mat4f View = Mat4Identity;
    View = Context->Camera.GetViewMatrix(View);
    UniformData.View = View;

    Shu::mat4f Projection = Shu::Perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    UniformData.Projection = Projection;
#endif

    UniformData.Color = RenderState.MeshColorUniform;
    memcpy(Context->Swapchain.UniformBuffers[ImageIndex].pMapped, &UniformData, sizeof(uniform_data));
}

void
GetMousePosDelta(f32 CurrentMouseDeltaX, f32 CurrentMouseDeltaY, Shu::vec2f *MousePosDelta)
{
    MousePosDelta->x = CurrentMouseDeltaX - LastFrameMousePos.x;
    MousePosDelta->y = CurrentMouseDeltaY - LastFrameMousePos.y;
    LastFrameMousePos.x = CurrentMouseDeltaX;
    LastFrameMousePos.y = CurrentMouseDeltaY;
}

void DrawFrameInVulkan(shoora_platform_frame_packet *FramePacket)
{
    ASSERT(Context != nullptr);
    ASSERT(FramePacket->DeltaTime > 0.0f);
    if(!Context->IsInitialized || Context->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        QuitApplication("[RENDERER]: Either the render is not initialized or there was some "
                        "problem with the current "
                        "frame counter.\n");
    }

    DeltaTime = FramePacket->DeltaTime;
    UiUpdateTimer += DeltaTime;
    if(UiUpdateTimer >= UiUpdateWaitTime)
    {
        DebugOverlay.Fps = FramePacket->Fps;
        DebugOverlay.MsPerFrame = 1000.0f*DeltaTime;
        UiUpdateTimer = 0.0f;
    }

    if(Context->FrameCounter == 0)
    {
        // 1st Frame
        LastFrameMousePos = {FramePacket->MouseXPos, FramePacket->MouseYPos};
    }
    Shu::vec2f MouseDelta;
    GetMousePosDelta(FramePacket->MouseXPos, FramePacket->MouseYPos, &MouseDelta);
    Context->Camera.HandleInput(MouseDelta.x, MouseDelta.y);

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
    shoora_vulkan_command_buffer_handle *pDrawCmdBuffer = &Context->Swapchain.DrawCommandBuffers[ImageIndex];
    VkCommandBuffer DrawCmdBuffer = pDrawCmdBuffer->Handle;

    // RenderState.MeshColor = Vec3(1, 1, 0);
    WriteUniformData(ImageIndex, FramePacket->DeltaTime);

    VK_CHECK(vkResetFences(Context->Device.LogicalDevice, 1, &pCurrentFrameFence->Handle));
    VK_CHECK(vkResetCommandBuffer(DrawCmdBuffer, 0));

    VkCommandBufferBeginInfo DrawCmdBufferBeginInfo = {};
    DrawCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkClearValue ClearValues[2] = {};
    ClearValues[0].color =
    {
        {RenderState.ClearColor.r, RenderState.ClearColor.g, RenderState.ClearColor.b, 1.0f}
    };
    ClearValues[1].depthStencil = { .depth = 0.0f, .stencil = 0};
    VkRect2D RenderArea = {};
    RenderArea.offset = {0, 0};
    RenderArea.extent = Context->Swapchain.ImageDimensions;

    ImGuiNewFrame();
    ImGuiUpdateBuffers(&Context->Device, &Context->ImContext);

    VkRenderPassBeginInfo RenderPassBeginInfo = {};
    RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInfo.pNext = nullptr;
    RenderPassBeginInfo.renderPass = Context->GraphicsRenderPass;
    RenderPassBeginInfo.framebuffer = Context->Swapchain.ImageFramebuffers[ImageIndex];
    RenderPassBeginInfo.renderArea = RenderArea;
    RenderPassBeginInfo.clearValueCount = ARRAY_SIZE(ClearValues);
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

        VkDescriptorSet DescriptorSets[] = {Context->Swapchain.UniformDescriptorSets[ImageIndex],
                                            Context->Swapchain.SampledImageDescriptorSet};
        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Context->Pipeline.GraphicsPipelineLayout, 0, ARRAY_SIZE(DescriptorSets),
                                DescriptorSets, 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->Pipeline.GraphicsPipeline);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &Context->VertexBuffer.Handle, offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, Context->IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(RectIndices), 1, 0, 0, 1);

        if(RenderState.WireframeMode)
        {
            vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              Context->Pipeline.WireframeGraphicsPipeline);
            vkCmdSetLineWidth(DrawCmdBuffer, RenderState.WireLineWidth);
            vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(RectIndices), 1, 0, 0, 1);
        }

        ImGuiDrawFrame(DrawCmdBuffer, &Context->ImContext);

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
    PresentInfo.pSwapchains = &Context->Swapchain.Handle;
    PresentInfo.pImageIndices = &Context->Swapchain.CurrentImageIndex;
    VK_CHECK(vkQueuePresentKHR(GraphicsQueue, &PresentInfo));

    AdvanceToNextFrame();
    ImGuiUpdateInputState(FramePacket);
    VK_CHECK(vkQueueWaitIdle(Context->Device.GraphicsQueue));
}

void
DestroyVulkanRenderer(shoora_vulkan_context *Context)
{
    VK_CHECK(vkDeviceWaitIdle(Context->Device.LogicalDevice));
    shoora_vulkan_device *RenderDevice = &Context->Device;

    ImGuiCleanup(RenderDevice, &Context->ImContext);
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