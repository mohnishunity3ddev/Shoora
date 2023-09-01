#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_imgui.h"
#include "loaders/image/png_loader.h"

#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif

#if SHU_USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include <memory.h>

static shoora_vulkan_context *Context = nullptr;
// NOTE: Triangle
static shoora_vertex_info TriangleVertices[] =
{
    {.VertexPos = Shu::vec3f{ 0.0f,  0.5f, 0.0f}, .VertexColor = Shu::vec3f{1, 0, 0}},
    {.VertexPos = Shu::vec3f{ 0.5f, -0.5f, 0.0f}, .VertexColor = Shu::vec3f{0, 1, 0}},
    {.VertexPos = Shu::vec3f{-0.5f, -0.5f, 0.0f}, .VertexColor = Shu::vec3f{0, 0, 1}}
};
static u32 TriangleIndices[] = {0, 1, 2};

// NOTE: Rectangle
static shoora_vertex_info RectVertices[] =
{
    {.VertexPos = Shu::vec3f{ 1.0f,  1.0f, 0.0f}, .VertexColor = Shu::vec3f{1, 0, 0}, .VertexUV = Shu::vec2f{1, 1}},
    {.VertexPos = Shu::vec3f{ 1.0f, -1.0f, 0.0f}, .VertexColor = Shu::vec3f{0, 1, 0}, .VertexUV = Shu::vec2f{1, 0}},
    {.VertexPos = Shu::vec3f{-1.0f, -1.0f, 0.0f}, .VertexColor = Shu::vec3f{0, 0, 1}, .VertexUV = Shu::vec2f{0, 0}},
    {.VertexPos = Shu::vec3f{-1.0f,  1.0f, 0.0f}, .VertexColor = Shu::vec3f{0, 0, 0}, .VertexUV = Shu::vec2f{0, 1}},
};
static u32 RectIndices[] = {0, 1, 2, 0, 2, 3};

static Shu::vec3f CubeVertexPositions[] =
{
    Shu::vec3f{ 0.5f,  0.5f,  -0.5f},   // Top-Right
    Shu::vec3f{ 0.5f, -0.5f,  -0.5f},   // Bottom-Right
    Shu::vec3f{-0.5f, -0.5f,  -0.5f},   // Bottom-Left
    Shu::vec3f{-0.5f,  0.5f,  -0.5f},   // Top-Left
    Shu::vec3f{ 0.5f,  0.5f,   0.5f},   // Top-Right
    Shu::vec3f{ 0.5f, -0.5f,   0.5f},   // Bottom-Right
    Shu::vec3f{-0.5f, -0.5f,   0.5f},   // Bottom-Left
    Shu::vec3f{-0.5f,  0.5f,   0.5f}    // Top-Left
};

// NOTE: Cube
static shoora_vertex_info CubeVertices[] =
{
    // Front Face
    {.VertexPos = CubeVertexPositions[0], .VertexUV = Shu::vec2f{1, 1}}, // 0
    {.VertexPos = CubeVertexPositions[1], .VertexUV = Shu::vec2f{1, 0}}, // 1
    {.VertexPos = CubeVertexPositions[2], .VertexUV = Shu::vec2f{0, 0}}, // 2
    {.VertexPos = CubeVertexPositions[3], .VertexUV = Shu::vec2f{0, 1}}, // 3
    // Right Face
    {.VertexPos = CubeVertexPositions[0], .VertexUV = Shu::vec2f{0, 1}}, // 4
    {.VertexPos = CubeVertexPositions[1], .VertexUV = Shu::vec2f{0, 0}}, // 5
    {.VertexPos = CubeVertexPositions[5], .VertexUV = Shu::vec2f{1, 0}}, // 6
    {.VertexPos = CubeVertexPositions[4], .VertexUV = Shu::vec2f{1, 1}}, // 7
    // Back Face
    {.VertexPos = CubeVertexPositions[7], .VertexUV = Shu::vec2f{1, 1}}, // 8
    {.VertexPos = CubeVertexPositions[6], .VertexUV = Shu::vec2f{1, 0}}, // 9
    {.VertexPos = CubeVertexPositions[5], .VertexUV = Shu::vec2f{0, 0}}, // 10
    {.VertexPos = CubeVertexPositions[4], .VertexUV = Shu::vec2f{0, 1}}, // 11
    // Left Face
    {.VertexPos = CubeVertexPositions[6], .VertexUV = Shu::vec2f{0, 0}}, // 12
    {.VertexPos = CubeVertexPositions[2], .VertexUV = Shu::vec2f{1, 0}}, // 13
    {.VertexPos = CubeVertexPositions[3], .VertexUV = Shu::vec2f{1, 1}}, // 14
    {.VertexPos = CubeVertexPositions[7], .VertexUV = Shu::vec2f{0, 1}}, // 15
    // Top Face
    {.VertexPos = CubeVertexPositions[3], .VertexUV = Shu::vec2f{0, 0}}, // 16
    {.VertexPos = CubeVertexPositions[0], .VertexUV = Shu::vec2f{1, 0}}, // 17
    {.VertexPos = CubeVertexPositions[4], .VertexUV = Shu::vec2f{1, 1}}, // 18
    {.VertexPos = CubeVertexPositions[7], .VertexUV = Shu::vec2f{0, 1}}, // 19
    // Top Face
    {.VertexPos = CubeVertexPositions[2], .VertexUV = Shu::vec2f{0, 0}}, // 20
    {.VertexPos = CubeVertexPositions[1], .VertexUV = Shu::vec2f{1, 0}}, // 21
    {.VertexPos = CubeVertexPositions[5], .VertexUV = Shu::vec2f{1, 1}}, // 22
    {.VertexPos = CubeVertexPositions[6], .VertexUV = Shu::vec2f{0, 1}}, // 23
};
static u32 CubeIndices[] = { 0,  1,  2,  0,  2,  3,                      // Front Face
                             4,  7,  6,  4,  6,  5,                      // Right Face
                             9, 10,  8,  8, 10, 11,                      // Back Face
                            14, 13, 12, 15, 14, 12,                      // Left Face
                            17, 16, 19, 17, 19, 18,                      // Top Face
                            20, 21, 23, 21, 22, 23};                     // Bottom Face

struct vert_uniform_data
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
    const u32 Padding00 = 0;
};
struct frag_uniform_data
{
    Shu::vec3f DirLightDirection;
    const u32 Padding00 = 0;
    Shu::vec3f DirLightColor;
    const u32 Padding01 = 0;
};
static vert_uniform_data VertUniformData = {};
static frag_uniform_data FragUniformData = {};

struct shoora_render_state
{
    Shu::vec3f DirLightDirection = Shu::Vec3f(1, 0, 0);
    Shu::vec3f DirLightColor = Shu::Vec3f(1, 1, 1);

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
static const f32 UiUpdateWaitTime = 1.0f;
static const Shu::mat4f Mat4Identity = Shu::Mat4f(1.0f);
static Shu::vec2f LastFrameMousePos = Shu::Vec2f(FLT_MAX, FLT_MAX);
static b32 SetFPSCap = true;
static i32 SelectedFPSOption = 2;

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

enum FpsOptions
{
    FpsOptions_30 = 0,
    FpsOptions_60 = 1,
    FpsOptions_120 = 2,
    FpsOptions_240 = 3,
};

void
ImGuiNewFrame()
{
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    ImGui::SetNextWindowPos(ImVec2(800, 100), 1 << 2);
    ImGui::SetNextWindowSize(ImVec2(400, 400), 1 << 2);

    ImGui::Begin("Inspector");

    i32 FPS = -1;
    ImGui::Checkbox("Set FPS Cap", (bool *)&SetFPSCap);
    Platform_ToggleFPSCap();
    if(SetFPSCap)
    {
        ImGui::Text("Set FPS:");
        ImGui::RadioButton("30",  &SelectedFPSOption, 0); ImGui::SameLine();
        ImGui::RadioButton("60",  &SelectedFPSOption, 1); ImGui::SameLine();
        ImGui::RadioButton("120", &SelectedFPSOption, 2); ImGui::SameLine();
        ImGui::RadioButton("240", &SelectedFPSOption, 3);
        switch(SelectedFPSOption)
        {
            case FpsOptions_30:
            {
                Platform_SetFPS(30);
            } break;
            case FpsOptions_60:
            {
                Platform_SetFPS(60);
            } break;
            case FpsOptions_120:
            {
                Platform_SetFPS(120);
            } break;
            case FpsOptions_240:
            {
                Platform_SetFPS(240);
            } break;

            SHU_INVALID_DEFAULT;
        }
    }

    ImGui::Checkbox("Toggle Wireframe", (bool *)&RenderState.WireframeMode);
    ImGui::SliderFloat("Wireframe Line Width", &RenderState.WireLineWidth, 1.0f, 10.0f);
    ImGui::ColorEdit3("Clear Color", RenderState.ClearColor.E);
    ImGui::ColorEdit3("Rectangle Color", RenderState.MeshColorUniform.E);

    ImGui::DragFloat3("Direction Light Direction", RenderState.DirLightDirection.E);
    ImGui::ColorEdit3("Direction Light Color", RenderState.DirLightColor.E);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(0, 0), 1 << 2);
    ImGui::SetNextWindowSize(ImVec2(400, 400), 1 << 2);
    ImGui::Begin("Debug Stats");
    ImGui::Text("Device: %s", Context->Device.DeviceProperties.deviceName);
    ImGui::Text("FPS: %u", DebugOverlay.Fps);
    ImGui::Text("MS Per Frame: %f", DebugOverlay.MsPerFrame);
    ImGui::BeginDisabled();
    ImGui::Text("Camera: {%.2f, %.2f, %.2f}", Context->Camera.Pos.x, Context->Camera.Pos.y, Context->Camera.Pos.z);
    ImGui::EndDisabled();
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

    CreateVertexBuffer(RenderDevice, CubeVertices, ARRAY_SIZE(CubeVertices), CubeIndices, ARRAY_SIZE(CubeIndices),
                       &VulkanContext->VertexBuffer, &VulkanContext->IndexBuffer);

    CreateSwapchainUniformResources(RenderDevice, Swapchain, sizeof(vert_uniform_data), sizeof(frag_uniform_data),
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
    // Model = glm::rotate(Model, Angle*AngleSpeed/50.0f, glm::vec3(0.0, 0.0f, 1.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));
    UniformData.Model = Model;

    glm::mat4 View = glm::mat4(1.0f);
    View = Context->Camera.GetViewMatrix(View);
    UniformData.View = View;

    glm::mat4 Projection = glm::mat4(1.0f);
    Projection = glm::perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    UniformData.Projection = Projection;
#else
    Shu::mat4f Model = Mat4Identity;
    Shu::Scale(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f));
    // Shu::RotateGimbalLock(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f), Angle * AngleSpeed);
    Shu::Translate(Model, Shu::Vec3f(0.0f, 0.0f, 0.0f));
    VertUniformData.Model = Model;

    Shu::mat4f View = Mat4Identity;
    View = Context->Camera.GetViewMatrix(View);
    VertUniformData.View = View;

    Shu::mat4f Projection = Shu::Perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    VertUniformData.Projection = Projection;
#endif

    VertUniformData.Color = RenderState.MeshColorUniform;
    memcpy(Context->Swapchain.UniformBuffers[ImageIndex].pMapped, &VertUniformData, sizeof(vert_uniform_data));

    FragUniformData.DirLightDirection = RenderState.DirLightDirection;
    FragUniformData.DirLightColor = RenderState.DirLightColor;
    memcpy(Context->Swapchain.FragUniformBuffers[ImageIndex].pMapped, &FragUniformData, sizeof(frag_uniform_data));
}

void
GetMousePosDelta(f32 CurrentMouseDeltaX, f32 CurrentMouseDeltaY, f32 *outMouseDeltaX, f32 *outMouseDeltaY)
{
    *outMouseDeltaX = CurrentMouseDeltaX - LastFrameMousePos.x;
    *outMouseDeltaY = CurrentMouseDeltaY - LastFrameMousePos.y;
    LastFrameMousePos.x = CurrentMouseDeltaX;
    LastFrameMousePos.y = CurrentMouseDeltaY;
}

void
DrawFrameInVulkan(shoora_platform_frame_packet *FramePacket)
{
    // VK_CHECK(vkQueueWaitIdle(Context->Device.GraphicsQueue));
    ASSERT(Context != nullptr);
    ASSERT(FramePacket->DeltaTime > 0.0f);
    if(!Context->IsInitialized || Context->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        Platform_ExitApplication("[RENDERER]: Either the render is not initialized or there was some "
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

    b32 LMBDown = Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, KeyState::SHU_KEYSTATE_DOWN);

    if(Platform_GetKeyInputState(SU_RIGHTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        LastFrameMousePos = {FramePacket->MouseXPos, FramePacket->MouseYPos};
    }

    if(Platform_GetKeyInputState(SU_RIGHTMOUSEBUTTON, KeyState::SHU_KEYSTATE_DOWN))
    {
        shoora_camera_input CameraInput = {};
        CameraInput.DeltaTime = FramePacket->DeltaTime;
        CameraInput.MoveForwards    = Platform_GetKeyInputState('W',          KeyState::SHU_KEYSTATE_DOWN);
        CameraInput.MoveLeft        = Platform_GetKeyInputState('A',          KeyState::SHU_KEYSTATE_DOWN);
        CameraInput.MoveBackwards   = Platform_GetKeyInputState('S',          KeyState::SHU_KEYSTATE_DOWN);
        CameraInput.MoveRight       = Platform_GetKeyInputState('D',          KeyState::SHU_KEYSTATE_DOWN);
        CameraInput.MoveFaster      = Platform_GetKeyInputState(SU_LEFTSHIFT, KeyState::SHU_KEYSTATE_DOWN);

        GetMousePosDelta(FramePacket->MouseXPos, FramePacket->MouseYPos, &CameraInput.MouseDeltaX,
                         &CameraInput.MouseDeltaY);
        Context->Camera.HandleInput(&CameraInput);
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
    ClearValues[1].depthStencil = { .depth = 1.0f, .stencil = 0};
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
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;

        vkCmdSetViewport(DrawCmdBuffer, 0, 1, &Viewport);
        VkRect2D Scissor = {};
        Scissor.extent = Context->Swapchain.ImageDimensions;
        vkCmdSetScissor(DrawCmdBuffer, 0, 1, &Scissor);

        VkDescriptorSet DescriptorSets[] = {Context->Swapchain.UniformDescriptorSets[ImageIndex],
                                            Context->Swapchain.FragSamplersDescriptorSet,
                                            Context->Swapchain.FragUniformsDescriptorSets[ImageIndex]};
        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Context->Pipeline.GraphicsPipelineLayout, 0, ARRAY_SIZE(DescriptorSets),
                                DescriptorSets, 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->Pipeline.GraphicsPipeline);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &Context->VertexBuffer.Handle, offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, Context->IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);

        if(RenderState.WireframeMode)
        {
            vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              Context->Pipeline.WireframeGraphicsPipeline);
            vkCmdSetLineWidth(DrawCmdBuffer, RenderState.WireLineWidth);
            vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);
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
    ImGuiUpdateInputState(FramePacket->MouseXPos, FramePacket->MouseYPos, LMBDown);
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