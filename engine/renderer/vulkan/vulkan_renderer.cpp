#include "vulkan_input_info.h"
#include "vulkan_work_submission.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_imgui.h"
#include "loaders/image/png_loader.h"
#include "vulkan_image.h"
#include "../material/material.h"

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
    {.Pos = Shu::vec3f{ 0.0f,  0.5f, 0.0f}, .Color = Shu::vec3f{1, 0, 0}},
    {.Pos = Shu::vec3f{ 0.5f, -0.5f, 0.0f}, .Color = Shu::vec3f{0, 1, 0}},
    {.Pos = Shu::vec3f{-0.5f, -0.5f, 0.0f}, .Color = Shu::vec3f{0, 0, 1}}
};
static u32 TriangleIndices[] = {0, 1, 2};

// NOTE: Rectangle
static shoora_vertex_info RectVertices[] =
{
    {.Pos = Shu::vec3f{ 1.0f,  1.0f, 0.0f}, .Color = Shu::vec3f{1, 0, 0}, .UV = Shu::vec2f{1, 1}},
    {.Pos = Shu::vec3f{ 1.0f, -1.0f, 0.0f}, .Color = Shu::vec3f{0, 1, 0}, .UV = Shu::vec2f{1, 0}},
    {.Pos = Shu::vec3f{-1.0f, -1.0f, 0.0f}, .Color = Shu::vec3f{0, 0, 1}, .UV = Shu::vec2f{0, 0}},
    {.Pos = Shu::vec3f{-1.0f,  1.0f, 0.0f}, .Color = Shu::vec3f{0, 0, 0}, .UV = Shu::vec2f{0, 1}},
};
static u32 RectIndices[] = {0, 1, 2, 0, 2, 3};

static Shu::vec3f CubeVertexPositions[] =
{
    Shu::vec3f{ 1.0f,  1.0f,  -1.0f},   // Top-Right
    Shu::vec3f{ 1.0f, -1.0f,  -1.0f},   // Bottom-Right
    Shu::vec3f{-1.0f, -1.0f,  -1.0f},   // Bottom-Left
    Shu::vec3f{-1.0f,  1.0f,  -1.0f},   // Top-Left
    Shu::vec3f{ 1.0f,  1.0f,   1.0f},   // Top-Right
    Shu::vec3f{ 1.0f, -1.0f,   1.0f},   // Bottom-Right
    Shu::vec3f{-1.0f, -1.0f,   1.0f},   // Bottom-Left
    Shu::vec3f{-1.0f,  1.0f,   1.0f}    // Top-Left
};

// NOTE: Cube
static shoora_vertex_info CubeVertices[] =
{
    // Front Face
    {.Pos = CubeVertexPositions[0], /* .Normal = Shu::vec3f{ 0,  0, -1}, */ .UV = Shu::vec2f{1, 1}}, // 0
    {.Pos = CubeVertexPositions[1], /* .Normal = Shu::vec3f{ 0,  0, -1}, */ .UV = Shu::vec2f{1, 0}}, // 1
    {.Pos = CubeVertexPositions[2], /* .Normal = Shu::vec3f{ 0,  0, -1}, */ .UV = Shu::vec2f{0, 0}}, // 2
    {.Pos = CubeVertexPositions[3], /* .Normal = Shu::vec3f{ 0,  0, -1}, */ .UV = Shu::vec2f{0, 1}}, // 3
    // Right Face
    {.Pos = CubeVertexPositions[0], /* .Normal = Shu::vec3f{ 1,  0,  0}, */ .UV = Shu::vec2f{0, 1}}, // 4
    {.Pos = CubeVertexPositions[1], /* .Normal = Shu::vec3f{ 1,  0,  0}, */ .UV = Shu::vec2f{0, 0}}, // 5
    {.Pos = CubeVertexPositions[5], /* .Normal = Shu::vec3f{ 1,  0,  0}, */ .UV = Shu::vec2f{1, 0}}, // 6
    {.Pos = CubeVertexPositions[4], /* .Normal = Shu::vec3f{ 1,  0,  0}, */ .UV = Shu::vec2f{1, 1}}, // 7
    // Back Face
    {.Pos = CubeVertexPositions[7], /* .Normal = Shu::vec3f{ 0,  0,  1}, */ .UV = Shu::vec2f{1, 1}}, // 8
    {.Pos = CubeVertexPositions[6], /* .Normal = Shu::vec3f{ 0,  0,  1}, */ .UV = Shu::vec2f{1, 0}}, // 9
    {.Pos = CubeVertexPositions[5], /* .Normal = Shu::vec3f{ 0,  0,  1}, */ .UV = Shu::vec2f{0, 0}}, // 10
    {.Pos = CubeVertexPositions[4], /* .Normal = Shu::vec3f{ 0,  0,  1}, */ .UV = Shu::vec2f{0, 1}}, // 11
    // Left Face
    {.Pos = CubeVertexPositions[6], /* .Normal = Shu::vec3f{-1,  0,  0}, */ .UV = Shu::vec2f{0, 0}}, // 12
    {.Pos = CubeVertexPositions[2], /* .Normal = Shu::vec3f{-1,  0,  0}, */ .UV = Shu::vec2f{1, 0}}, // 13
    {.Pos = CubeVertexPositions[3], /* .Normal = Shu::vec3f{-1,  0,  0}, */ .UV = Shu::vec2f{1, 1}}, // 14
    {.Pos = CubeVertexPositions[7], /* .Normal = Shu::vec3f{-1,  0,  0}, */ .UV = Shu::vec2f{0, 1}}, // 15
    // Top Face
    {.Pos = CubeVertexPositions[3], /* .Normal = Shu::vec3f{ 0,  1,  0}, */ .UV = Shu::vec2f{0, 0}}, // 16
    {.Pos = CubeVertexPositions[0], /* .Normal = Shu::vec3f{ 0,  1,  0}, */ .UV = Shu::vec2f{1, 0}}, // 17
    {.Pos = CubeVertexPositions[4], /* .Normal = Shu::vec3f{ 0,  1,  0}, */ .UV = Shu::vec2f{1, 1}}, // 18
    {.Pos = CubeVertexPositions[7], /* .Normal = Shu::vec3f{ 0,  1,  0}, */ .UV = Shu::vec2f{0, 1}}, // 19
    // Bottom Face
    {.Pos = CubeVertexPositions[2], /* .Normal = Shu::vec3f{ 0, -1,  0}, */ .UV = Shu::vec2f{0, 0}}, // 20
    {.Pos = CubeVertexPositions[1], /* .Normal = Shu::vec3f{ 0, -1,  0}, */ .UV = Shu::vec2f{1, 0}}, // 21
    {.Pos = CubeVertexPositions[5], /* .Normal = Shu::vec3f{ 0, -1,  0}, */ .UV = Shu::vec2f{1, 1}}, // 22
    {.Pos = CubeVertexPositions[6], /* .Normal = Shu::vec3f{ 0, -1,  0}, */ .UV = Shu::vec2f{0, 1}}, // 23
};
static u32 CubeIndices[] = { 0,  1,  2,  0,  2,  3,                      // Front Face
                             4,  7,  6,  4,  6,  5,                      // Right Face
                             9, 10,  8,  8, 10, 11,                      // Back Face
                            14, 13, 12, 15, 14, 12,                      // Left Face
                            17, 16, 19, 17, 19, 18,                      // Top Face
                            20, 21, 23, 21, 22, 23};                     // Bottom Face

void
GenerateNormals(shoora_vertex_info *VertexInfo, u32 *Indices, u32 IndexCount)
{
    for(u32 Index = 0;
        Index < IndexCount;
        Index += 3)
    {
        u32 I0 = Indices[Index + 0];
        u32 I1 = Indices[Index + 1];
        u32 I2 = Indices[Index + 2];

        Shu::vec3f Edge1 = VertexInfo[I1].Pos - VertexInfo[I0].Pos;
        Shu::vec3f Edge2 = VertexInfo[I2].Pos - VertexInfo[I0].Pos;

        Shu::vec3f Normal = Shu::Normalize(Shu::Cross(Edge1, Edge2));

        VertexInfo[I0].Normal = Normal;
        VertexInfo[I1].Normal = Normal;
        VertexInfo[I2].Normal = Normal;
    }
}

void
GenerateTangentInformation(shoora_vertex_info *VertexInfo, u32 *Indices, u32 IndexCount)
{
    for(u32 Index = 0;
        Index < IndexCount;
        Index += 3)
    {
        u32 I0 = Indices[Index + 0];
        u32 I1 = Indices[Index + 1];
        u32 I2 = Indices[Index + 2];

        shoora_vertex_info *V0 = VertexInfo + I0;
        shoora_vertex_info *V1 = VertexInfo + I1;
        shoora_vertex_info *V2 = VertexInfo + I2;

        Shu::vec3f Edge1 = V1->Pos - V0->Pos;
        Shu::vec3f Edge2 = V2->Pos - V0->Pos;

        f32 DeltaU1 = V1->UV.x - V0->UV.x;
        f32 DeltaV1 = V1->UV.y - V0->UV.y;
        f32 DeltaU2 = V2->UV.x - V0->UV.x;
        f32 DeltaV2 = V2->UV.y - V0->UV.y;
        f32 Denominator = DeltaU1*DeltaV2 - DeltaV1*DeltaU2;
        f32 OneByDenominator = 1.0f / Denominator;
        ASSERT(Denominator != 0.0f);

        Shu::vec3f Tangent;
        Tangent.x = OneByDenominator * (DeltaV2*Edge1.x - DeltaU2*Edge2.x);
        Tangent.y = OneByDenominator * (DeltaV2*Edge1.y - DeltaU2*Edge2.y);
        Tangent.z = OneByDenominator * (DeltaV2*Edge1.z - DeltaU2*Edge2.z);
        Tangent = Shu::Normalize(Tangent);

        V0->Tangent = Tangent;
        V1->Tangent = Tangent;
        V2->Tangent = Tangent;

#if CALCULATE_BITANGENT
        Shu::vec3f BiTangent;
        Tangent.x = OneByDenominator * (DeltaU1*Edge2.x - DeltaV1*Edge1.x);
        Tangent.y = OneByDenominator * (DeltaU1*Edge2.y - DeltaV1*Edge1.y);
        Tangent.z = OneByDenominator * (DeltaU1*Edge2.z - DeltaV1*Edge1.z);
        BiTangent = Shu::Normalize(BiTangent);
        // Orthogonalize the tangent and the bitangent.
        Shu::vec3f TangentProj = BiTangent*Dot(Tangent, BiTangent);
        Tangent -= TangentProj;
        f32 D = Dot(Tangent, BiTangent);
        ASSERT(ABSOLUTE(D) <= FLT_EPSILON);

        V0->BiTangent = BiTangent;
        V1->BiTangent = BiTangent;
        V2->BiTangent = BiTangent;
#endif
    }
}

// NOTE: ALso make the same changes to the lighting shader.
// TODO)): Automate this so that changing this automatically makes changes to the shader using shader variation.
#define MATERIAL_VIEWER 0
struct vert_uniform_data
{
    Shu::mat4f Model;
    Shu::mat4f View;
    Shu::mat4f Projection;
};
struct lighting_shader_uniform_data
{
    SHU_ALIGN_16 Shu::vec3f LightPos = Shu::Vec3f(3, 0, 0);
    SHU_ALIGN_16 Shu::vec3f LightColor = Shu::Vec3f(1, 1, 0);
    SHU_ALIGN_16 Shu::vec3f CamPos = Shu::Vec3f(0, 0, -10);
    SHU_ALIGN_16 Shu::vec3f ObjectColor;

#if MATERIAL_VIEWER
    shoora_material Material;
#endif
};
struct light_shader_data
{
    // Vertex Uniform Buffer
    Shu::mat4f Model;
    Shu::mat4f View;
    Shu::mat4f Projection;

    // Fragment Uniform Buffer
    SHU_ALIGN_16 Shu::vec3f Color = Shu::vec3f{1, 1, 1};
};
static light_shader_data LightShaderData = {};
struct shoora_render_state
{
    light_shader_data *LightData = &LightShaderData;

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
static vert_uniform_data VertUniformData = {};
static lighting_shader_uniform_data FragUniformData = {};
static f32 ImGuiDragFloatStep = 0.005f;

void
WindowResizedCallback(u32 Width, u32 Height)
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
    ImGui::Spacing();
    ImGui::TextUnformatted("Object Data:");
    // ImGui::ColorEdit3("Mesh Color", RenderState.MeshColorUniform.E);
#if MATERIAL_VIEWER
    const char *MaterialNames[] =
    {
        "Emerald", "Jade", "Obsidian", "Pearl", "Ruby", "Turquoise", "Brass", "Bronze", "Chrome", "Copper",
        "Gold", "Silver", "Black Plastic", "Cyan Plastic", "Green Plastic", "Red Plastic", "White Plastic",
        "Yellow Plastic", "Black Rubber", "Cyan Rubber", "Green Rubber", "Red Rubber", "White Rubber",
        "Yellow Rubber",
    };
    static int CurrentItem = 0;
    if(ImGui::Combo("Select Material", &CurrentItem, MaterialNames, ARRAY_SIZE(MaterialNames)))
    {
        GetMaterial((MaterialType)CurrentItem, &FragUniformData.Material);
    }
#endif
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(800, 500), 1 << 2);
    ImGui::SetNextWindowSize(ImVec2(400, 400), 1 << 2);
    ImGui::Begin("Light Data");
    ImGui::DragFloat3("Light Position", FragUniformData.LightPos.E, ImGuiDragFloatStep);
    ImGui::ColorEdit3("Light Color", RenderState.LightData->Color.E);
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
CreateUnlitPipeline(shoora_vulkan_context *Context)
{
    shoora_vulkan_device *RenderDevice = &Context->Device;
    shoora_vulkan_swapchain *Swapchain = &Context->Swapchain;

    CreateUniformBuffers(RenderDevice, Context->FragUnlitBuffers, ARRAY_SIZE(Context->FragUnlitBuffers),
                         sizeof(light_shader_data));

    VkDescriptorSetLayoutBinding Bindings[2];
    // NOTE: So, This descriptor's data has already been computed and is being used in other pipelines
    // This is the one which contains Model, View, Projection Matrices data.
    Bindings[0] = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                                VK_SHADER_STAGE_VERTEX_BIT);
    Bindings[1] = GetDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                                VK_SHADER_STAGE_FRAGMENT_BIT);
    CreateDescriptorSetLayout(RenderDevice, Bindings, ARRAY_SIZE(Bindings), &Context->UnlitSetLayout);

    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        AllocateDescriptorSets(RenderDevice, Swapchain->UniformDescriptorPool, 1, &Context->UnlitSetLayout,
                               &Context->UnlitSets[Index]);
        // NOTE: MVP Matrices Data - three 4x4 Matrices
        UpdateBufferDescriptorSet(RenderDevice, Context->UnlitSets[Index], 0,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Context->FragUnlitBuffers[Index].Handle,
                                  3*sizeof(Shu::mat4f), 0);
        // NOTE: Light Fragment Data - one vec3 - Color
        UpdateBufferDescriptorSet(RenderDevice, Context->UnlitSets[Index], 1,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Context->FragUnlitBuffers[Index].Handle,
                                  sizeof(Shu::vec3f), OFFSET_OF(light_shader_data, Color));
    }

    CreatePipelineLayout(RenderDevice, 1, &Context->UnlitSetLayout, 0, nullptr, &Context->UnlitPipeline.Layout);
    CreateGraphicsPipeline(Context, "shaders/spirv/unlit.vert.spv", "shaders/spirv/unlit.frag.spv",
                           &Context->UnlitPipeline);
}

void
DestroyUnlitPipelineResources()
{

    for(u32 Index = 0;
        Index < ARRAY_SIZE(Context->FragUnlitBuffers);
        ++Index)
    {
        DestroyUniformBuffer(&Context->Device, &Context->FragUnlitBuffers[Index]);
    }
    vkDestroyDescriptorSetLayout(Context->Device.LogicalDevice, Context->UnlitSetLayout, nullptr);
    DestroyPipeline(&Context->Device, &Context->UnlitPipeline);
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

    GenerateNormals(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));
    GenerateTangentInformation(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));

    Shu::vec2u ScreenDim = Shu::vec2u{AppInfo->WindowWidth, AppInfo->WindowHeight};
    CreateSwapchain(&VulkanContext->Device, &VulkanContext->Swapchain, ScreenDim, &SwapchainInfo);
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    CreateVertexBuffer(RenderDevice, CubeVertices, ARRAY_SIZE(CubeVertices), CubeIndices, ARRAY_SIZE(CubeIndices),
                       &VulkanContext->VertexBuffer, &VulkanContext->IndexBuffer);

    const char *ppTexturePaths[] =
    {
        "images/brickwall/brickwall.jpg",
        "images/brickwall/brickwall_NRM.jpg"
    };
    CreateSwapchainUniformResources(RenderDevice, Swapchain, sizeof(vert_uniform_data), sizeof(lighting_shader_uniform_data),
                                    ppTexturePaths, ARRAY_SIZE(ppTexturePaths), &VulkanContext->GraphicsPipeline.Layout);
    CreateGraphicsPipeline(VulkanContext, "shaders/spirv/blinn-phong.vert.spv", "shaders/spirv/blinn-phong.frag.spv",
                           &VulkanContext->GraphicsPipeline);

    // Wireframe
    CreatePipelineLayout(RenderDevice, 1, &VulkanContext->Swapchain.UniformSetLayout, 0, nullptr,
                         &VulkanContext->WireframePipeline.Layout);
    CreateWireframePipeline(VulkanContext, "shaders/spirv/wireframe.vert.spv", "shaders/spirv/wireframe.frag.spv");

    // Unlit Pipeline
    CreateUnlitPipeline(VulkanContext);

    CreateSynchronizationPrimitives(&VulkanContext->Device, &VulkanContext->SyncHandles);

    PrepareImGui(RenderDevice, &VulkanContext->ImContext, ScreenDim, VulkanContext->GraphicsRenderPass);

    SetupCamera(&VulkanContext->Camera, Shu::Vec3f(0, 0, -10.0f), Shu::Vec3f(0, 1, 0));

    AppInfo->WindowResizeCallback = &WindowResizedCallback;


#if MATERIAL_VIEWER
    GetMaterial(MaterialType::BRONZE, &FragUniformData.Material);
#endif

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
static const f32 AngleSpeed = 25.0f;

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
    // Shu::RotateGimbalLock(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f), Angle*AngleSpeed);
    Shu::Translate(Model, Shu::Vec3f(0.0f, 0.0f, 0.0f));
    VertUniformData.Model = Model;

    Shu::mat4f View = Mat4Identity;
    View = Context->Camera.GetViewMatrix(View);
    VertUniformData.View = View;

    Shu::mat4f Projection = Shu::Perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    VertUniformData.Projection = Projection;
#endif
    memcpy(Context->Swapchain.UniformBuffers[ImageIndex].pMapped, &VertUniformData, sizeof(vert_uniform_data));

    LightShaderData.Model = Mat4Identity;
    Shu::Scale(LightShaderData.Model, Shu::Vec3f(0.25f));
    Shu::Translate(LightShaderData.Model, FragUniformData.LightPos);
    LightShaderData.View = View;
    LightShaderData.Projection = Projection;
    memcpy(Context->FragUnlitBuffers[ImageIndex].pMapped, &LightShaderData, sizeof(light_shader_data));

    // Light Position is set directly in ImGui
    FragUniformData.LightColor = RenderState.LightData->Color;
    FragUniformData.ObjectColor = RenderState.MeshColorUniform;
    FragUniformData.CamPos = Context->Camera.Pos;
    memcpy(Context->Swapchain.FragUniformBuffers[ImageIndex].pMapped, &FragUniformData, sizeof(lighting_shader_uniform_data));
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
                                Context->GraphicsPipeline.Layout, 0, ARRAY_SIZE(DescriptorSets),
                                DescriptorSets, 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->GraphicsPipeline.Handle);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &Context->VertexBuffer.Handle, offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, Context->IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);

        if(RenderState.WireframeMode)
        {
            vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->WireframePipeline.Handle);
            vkCmdSetLineWidth(DrawCmdBuffer, RenderState.WireLineWidth);
            vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);
        }

        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Layout, 0,
                                1, &Context->UnlitSets[ImageIndex], 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Handle);
        vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);

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

    DestroyUnlitPipelineResources();
    DestroySwapchainUniformResources(RenderDevice, &Context->Swapchain);

    DestroyAllSynchronizationPrimitives(RenderDevice, &Context->SyncHandles);
    DestroyVertexBuffer(RenderDevice, &Context->VertexBuffer, &Context->IndexBuffer);
    DestroyPipeline(RenderDevice, &Context->GraphicsPipeline);
    DestroyPipeline(RenderDevice, &Context->WireframePipeline);
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