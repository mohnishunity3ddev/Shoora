#include "loaders/meshes/mesh_loader.h"
#include "loaders/image/png_loader.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_geometry.h"
#include "vulkan_image.h"
#include "vulkan_imgui.h"
#include "vulkan_input_info.h"
#include "vulkan_pipeline.h"
#include "vulkan_render_pass.h"
#include "vulkan_work_submission.h"
#include "graphics/vulkan_graphics.h"
#include "scene/vulkan_scene.h"

#include <mesh/database/mesh_database.h>
#include <mesh/mesh_utils.h>
#include <physics/body.h>
#include <physics/constraint.h>
#include <physics/force.h>
#include <physics/collision.h>
#include <utils/utils.h>

#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif

#if SHU_USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#define UNLIT_PIPELINE 1
#include <memory.h>
#include <vector>

static shoora_vulkan_context *Context = nullptr;

static b32 isDebug = false;
static b32 WireframeMode = false;
static f32 TestCameraScale = 0.5f;
static b32 GlobalShowContacts = true;

// NOTE: ALso make the same changes to the lighting shader.
// TODO)): Automate this so that changing this automatically makes changes to the shader using shader variation.
#define MATERIAL_VIEWER 0

struct vert_uniform_data
{
    // Shu::mat4f Model;
    Shu::mat4f View;
    Shu::mat4f Projection;
};

struct spotlight_data
{
    b32 IsOn;

    SHU_ALIGN_16 Shu::vec3f Pos;
    SHU_ALIGN_16 Shu::vec3f Color = Shu::Vec3f(1.0f);
    SHU_ALIGN_16 Shu::vec3f Direction;

    float InnerCutoffAngles = 12.5f;
    float OuterCutoffAngles = 15.5f;
    float Intensity = 5.0f;
};

struct point_light_data
{
    SHU_ALIGN_16 Shu::vec3f Pos = Shu::Vec3f(3, 0, 0);
    SHU_ALIGN_16 Shu::vec3f Color = Shu::Vec3f(1, 1, 0);
    float Intensity = 5.0f;
};

struct lighting_shader_uniform_data
{
    SHU_ALIGN_16 point_light_data PointLightData[4];
    SHU_ALIGN_16 spotlight_data SpotlightData;

    SHU_ALIGN_16 Shu::vec3f CamPos = Shu::Vec3f(0, 0, -10);
    SHU_ALIGN_16 Shu::vec3f ObjectColor;

#if MATERIAL_VIEWER
    shoora_material Material;
#endif
};

struct push_const_block
{
    Shu::mat4f Model;
};

struct light_shader_vert_data
{
    Shu::mat4f View;
    Shu::mat4f Projection;

    // Fragment Shader
};

struct light_shader_push_constant_data
{
    Shu::mat4f Model;
    Shu::vec3f Color = {1, 1, 1};
};

struct shoora_render_state
{
    b8 WireframeMode = false;
    f32 WireLineWidth = 10.0f;
    // Shu::vec3f ClearColor = Shu::vec3f{0.043f, 0.259f, 0.259f};
    Shu::vec3f ClearColor = Shu::Vec3f(0.0f);
    Shu::vec3f MeshColorUniform = Shu::vec3f{1.0f, 1.0f, 1.0f};
};

struct shoora_debug_overlay
{
    f32 MsPerFrame = 0.0f;
    u32 Fps = 0;
} DebugOverlay;

static f32 GlobalDeltaTime = 0.0f;
static shoora_render_state GlobalRenderState;
static f32 GlobalUiUpdateTimer = 0.0f;
static const f32 GlobalUiUpdateWaitTime = 1.0f;
static const Shu::mat4f GlobalMat4Identity = Shu::Mat4f(1.0f);
static Shu::vec2f GlobalLastFrameMousePos = Shu::Vec2f(FLT_MAX, FLT_MAX);
static b32 GlobalSetFPSCap = true;
static i32 GlobalSelectedFPSOption = 2;
static vert_uniform_data GlobalVertUniformData = {};
static lighting_shader_uniform_data GlobalFragUniformData = {};
static light_shader_vert_data GlobalLightShaderData;
static light_shader_push_constant_data GlobalLightPushConstantData[4];
static push_const_block GlobalPushConstBlock = {};
static f32 GlobalImGuiDragFloatStep = 0.005f;
static Shu::vec2u GlobalWindowSize = {};

static shoora_scene *Scene;

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
        GlobalWindowSize = {Width, Height};
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

    f32 DesiredWidth = 400;
    ImGui::SetNextWindowPos(ImVec2(GlobalWindowSize.x - DesiredWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(DesiredWidth, FLT_MAX));
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoResize;
    ImGui::Begin("Inspector", nullptr, WindowFlags);
    i32 FPS = -1;
    ImGui::Checkbox("Set FPS Cap", (bool *)&GlobalSetFPSCap);
    Platform_ToggleFPSCap();
    if(GlobalSetFPSCap)
    {
        ImGui::Text("Set FPS:");
        ImGui::RadioButton("30",  &GlobalSelectedFPSOption, 0); ImGui::SameLine();
        ImGui::RadioButton("60",  &GlobalSelectedFPSOption, 1); ImGui::SameLine();
        ImGui::RadioButton("120", &GlobalSelectedFPSOption, 2); ImGui::SameLine();
        ImGui::RadioButton("240", &GlobalSelectedFPSOption, 3);
        switch(GlobalSelectedFPSOption)
        {
            case FpsOptions_30: { Platform_SetFPS(30); } break;
            case FpsOptions_60: { Platform_SetFPS(60); } break;
            case FpsOptions_120: { Platform_SetFPS(120); } break;
            case FpsOptions_240: { Platform_SetFPS(240); } break;

            SHU_INVALID_DEFAULT;
        }
    }

    ImGui::SliderFloat("Test Scale", &TestCameraScale, 0.5f, 100.0f);
    ImGui::Checkbox("Toggle Wireframe", (bool *)&WireframeMode);

    ImGui::Spacing();
    ImGui::Text("Body Count: %d", Scene->GetBodyCount());

#if CREATE_WIREFRAME_PIPELINE
    ImGui::Checkbox("Toggle Wireframe", (bool *)&GlobalRenderState.WireframeMode);
    ImGui::SliderFloat("Wireframe Line Width", &GlobalRenderState.WireLineWidth, 1.0f, 10.0f);
#endif

    ImGui::ColorEdit3("Clear Color", GlobalRenderState.ClearColor.E);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);
    ImGui::Begin("Debug Stats", nullptr, WindowFlags);
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

Shu::vec2f
MouseToWorld(const Shu::vec2f &MousePos)
{
    // NOTE: This is the position where the y position is flipped since windows's mouse pos (0,0) starts from the
    // top left and y increases in the downward direction.
    Shu::vec2f worldPos = Shu::Vec2f(MousePos.x, ((f32)GlobalWindowSize.y) - MousePos.y);

    worldPos += Context->Camera.Pos.xy;
    worldPos -= (0.5f*Context->Camera.GetBounds());

    return worldPos;
}

// TODO)) : This function is not correct right now! Check this if you are using it.
#if 0
Shu::vec2f
WorldToMouse(const Shu::vec2f &WorldPos)
{
    f32 x = WorldPos.x;
    f32 y = WorldPos.y - (f32)GlobalWindowSize.y;

    Shu::vec2f Result;
    Result.x = x;
    Result.y = y;

    return Result;
}
#endif

static b32 msgShown = false;

void
InitScene()
{
    // Bottom Wall (Static Rigidbody)
    Shu::vec2f Window = Shu::Vec2f((f32)GlobalWindowSize.x, (f32)GlobalWindowSize.y);

    Scene->AddSphereBody(Shu::Vec3f(0.0f, 0.0f, 10.0f), colorU32::Proto_Red, 1.0f, 0.0f, 0.5f);
    Scene->AddCubeBody(Shu::Vec3f(1.5f, 0.0f, 10.0f), Shu::Vec3f(1.0f), colorU32::Proto_Orange, 0.0f, 0.5f);
    Scene->AddCubeBody(Shu::Vec3f(0, -5, 10), Shu::Vec3f(50, .25f, 50), colorU32::Proto_Green, 0.0f, 0.5f);
}

void
DrawScene(const VkCommandBuffer &CmdBuffer)
{
    if(WireframeMode && !isDebug && !msgShown)
    {
        LogWarnUnformatted("Wireframe mode is only available in debug builds. Turning off Wireframe mode!\n");
        msgShown = true;
        WireframeMode = false;
    }

    Scene->Draw(WireframeMode);
}

void
CreateUnlitPipeline(shoora_vulkan_context *Context)
{
    shoora_vulkan_device *RenderDevice = &Context->Device;
    shoora_vulkan_swapchain *Swapchain = &Context->Swapchain;

    VkDescriptorPoolSize Sizes[2];
    Sizes[0] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHU_MAX_FRAMES_IN_FLIGHT);
    Sizes[1] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
    CreateDescriptorPool(&Context->Device, ARRAY_SIZE(Sizes), Sizes, 4, &Context->UnlitDescriptorPool);

    CreateUniformBuffers(RenderDevice, Context->FragUnlitBuffers, ARRAY_SIZE(Context->FragUnlitBuffers),
                         sizeof(light_shader_vert_data));

    VkDescriptorSetLayoutBinding Bindings[1];
    Bindings[0] = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    CreateDescriptorSetLayout(RenderDevice, Bindings, ARRAY_SIZE(Bindings), &Context->UnlitSetLayouts[0]);
    for(u32 Index = 0; Index < SHU_MAX_FRAMES_IN_FLIGHT; ++Index)
    {
        AllocateDescriptorSets(RenderDevice, Context->UnlitDescriptorPool, 1, &Context->UnlitSetLayouts[0],
                               &Context->UnlitSets[Index]);
        UpdateBufferDescriptorSet(RenderDevice, Context->UnlitSets[Index], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  Context->FragUnlitBuffers[Index].Handle, 2*sizeof(Shu::mat4f), 0);
    }

    CreateCombinedImageSampler(RenderDevice, "images/proto/Grid_02.png", VK_SAMPLE_COUNT_1_BIT,
                               &Context->UnlitImageSampler);
    auto ImageSamplerBinding = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                             VK_SHADER_STAGE_FRAGMENT_BIT);
    CreateDescriptorSetLayout(RenderDevice, &ImageSamplerBinding, 1, &Context->UnlitSetLayouts[1]);
    AllocateDescriptorSets(RenderDevice, Context->UnlitDescriptorPool, 1, &Context->UnlitSetLayouts[1],
                           &Context->UnlitSamplerSet);
    VkDescriptorImageInfo ImageInfo;
    ImageInfo.sampler = Context->UnlitImageSampler.Sampler;
    ImageInfo.imageView = Context->UnlitImageSampler.Image.ImageView;
    ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    UpdateImageDescriptorSets(RenderDevice, Context->UnlitSamplerSet, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                              1, &ImageInfo);

    VkPushConstantRange PushConstants[1];
    PushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    PushConstants[0].offset = 0;
    PushConstants[0].size = sizeof(light_shader_push_constant_data);

    CreatePipelineLayout(RenderDevice, ARRAY_SIZE(Context->UnlitSetLayouts), Context->UnlitSetLayouts,
                         ARRAY_SIZE(PushConstants), PushConstants, &Context->UnlitPipeline.Layout);
    CreateGraphicsPipeline(Context, "shaders/spirv/unlit.vert.spv", "shaders/spirv/unlit.frag.spv",
                           &Context->UnlitPipeline);
}

void
CleanupUnlitPipeline()
{
    vkDestroyDescriptorPool(Context->Device.LogicalDevice, Context->UnlitDescriptorPool, nullptr);
    DestroyImage2D(&Context->Device, &Context->UnlitImageSampler.Image);
    vkDestroySampler(Context->Device.LogicalDevice, Context->UnlitImageSampler.Sampler, nullptr);
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

    vkDestroyDescriptorSetLayout(Context->Device.LogicalDevice, Context->UnlitSetLayouts[0], nullptr);
    vkDestroyDescriptorSetLayout(Context->Device.LogicalDevice, Context->UnlitSetLayouts[1], nullptr);
    DestroyPipeline(&Context->Device, &Context->UnlitPipeline);
}

void
InitializeLightData()
{
    GlobalFragUniformData.PointLightData[0].Pos = Shu::Vec3f(1.62f, 3.2f, -1.65f);
    GlobalFragUniformData.PointLightData[0].Color = Shu::Vec3f(1.0f, 0.0f, 0.0f);
    GlobalFragUniformData.PointLightData[0].Intensity = 2.575f;

    GlobalFragUniformData.PointLightData[1].Pos = Shu::Vec3f(1.615f, 0.0f, -5.105f);
    GlobalFragUniformData.PointLightData[1].Color = Shu::Vec3f(0.0f, 1.0f, 0.0f);
    GlobalFragUniformData.PointLightData[1].Intensity = 3.535f;

    GlobalFragUniformData.PointLightData[2].Pos = Shu::Vec3f(2.47f, 3.27f, -13.645f);
    GlobalFragUniformData.PointLightData[2].Color = Shu::Vec3f(0.0f, 0.0f, 1.0f);
    GlobalFragUniformData.PointLightData[2].Intensity = 2.575f;

    GlobalFragUniformData.PointLightData[3].Pos = Shu::Vec3f(-2.575f, 0.785f, -11.24f);
    GlobalFragUniformData.PointLightData[3].Color = Shu::Vec3f(1.0f, 0.0f, 1.0f);
    GlobalFragUniformData.PointLightData[3].Intensity = 2.575f;

    GlobalFragUniformData.SpotlightData.InnerCutoffAngles = 10.0f;
    GlobalFragUniformData.SpotlightData.OuterCutoffAngles = 14.367f;
    GlobalFragUniformData.SpotlightData.Color = Shu::Vec3f(30.0f / 255.0f, 194.0 / 255.0f, 165.0 / 255.0f);
    GlobalFragUniformData.SpotlightData.Intensity = 5.0f;
}

void
InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo)
{
#if _SHU_DEBUG
    isDebug = true;
#endif

    GlobalWindowSize = {AppInfo->WindowWidth, AppInfo->WindowHeight};
    InitializeLightData();

    VK_CHECK(volkInitialize());

    ShuraInstanceCreateInfo.AppName = AppInfo->AppName;
#if _SHU_DEBUG
    ShuraInstanceCreateInfo.ppRequiredInstanceLayers = RequiredInstanceLayers;
    ShuraInstanceCreateInfo.RequiredInstanceLayerCount = ARRAY_SIZE(RequiredInstanceLayers);
#endif

    CreateVulkanInstance(VulkanContext, &ShuraInstanceCreateInfo);
    volkLoadInstance(VulkanContext->Instance);

#ifdef _SHU_DEBUG
    SetupDebugCallbacks(VulkanContext, DebugCreateInfo);
#endif

    shoora_vulkan_device *RenderDevice = &VulkanContext->Device;
    shoora_vulkan_swapchain *Swapchain = &VulkanContext->Swapchain;

    CreatePresentationSurface(VulkanContext, &Swapchain->Surface);
    CreateDeviceAndQueues(VulkanContext, &DeviceCreateInfo);
    volkLoadDevice(RenderDevice->LogicalDevice);
    CreateCommandPools(RenderDevice);

#if 0
    GenerateNormals(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));
    GenerateTangentInformation(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));
#endif

    Shu::vec2u ScreenDim = Shu::vec2u{AppInfo->WindowWidth, AppInfo->WindowHeight};
    CreateSwapchain(&VulkanContext->Device, &VulkanContext->Swapchain, ScreenDim, &SwapchainInfo);
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    SetupCamera(&VulkanContext->Camera, shoora_projection::PROJECTION_PERSPECTIVE, 0.1f, 100.0f, 16.0f / 9.0f,
                GlobalWindowSize.y, 45.0f /*,  Shu::Vec3f(GlobalWindowSize.x/2, GlobalWindowSize.y/2, 0.0f) */);

    // NOTE: Mesh Database setup
    shoora_mesh_database::MeshDbBegin(RenderDevice);
    Shu::vec3f Poly1[] = {{0, 10, 1}, {10, 3, 1}, {10, -5, 1}, {-10, -5, 1}, {-10, 3, 1}};
    shoora_mesh_database::AddPolygonMeshToDb(Poly1, ARRAY_SIZE(Poly1));
    Shu::vec3f Poly2[] = {{20, 60, 1}, {40, 20, 1}, {20, -60, 1}, {-20, -60, 1}, {-40, 20, 1}};
    shoora_mesh_database::AddPolygonMeshToDb(Poly2, ARRAY_SIZE(Poly2));
    shoora_mesh_database::MeshDbEnd();

#if RENDER_SPONZA
    // NOTE: This is for Sponza Scene Gemoetry. Loading all meshes, textures and everything else related to it!.
    SetupGeometry(RenderDevice, &VulkanContext->Geometry, &VulkanContext->Camera, GlobalVertUniformData.Projection,
                  VulkanContext->GraphicsRenderPass, "meshes/sponza/Sponza.gltf", "shaders/spirv/sponza.vert.spv",
                  "shaders/spirv/sponza.frag.spv");
    const char *ppTexturePaths[] =
    {
        "images/cobblestone.png",
        "images/cobblestone_NRM.png",
        "images/cobblestone_SPEC.png",
    };
    CreatePushConstantBlock(&VulkanContext->GraphicsPipeline, VK_SHADER_STAGE_VERTEX_BIT, sizeof(push_const_block), 0);
    CreateSwapchainUniformResources(RenderDevice, Swapchain, sizeof(vert_uniform_data),
                                    sizeof(lighting_shader_uniform_data), ppTexturePaths,
                                    ARRAY_SIZE(ppTexturePaths),
                                    VulkanContext->GraphicsPipeline.PushConstant.Ranges,
                                    VulkanContext->GraphicsPipeline.PushConstant.Count,
                                    &VulkanContext->GraphicsPipeline.Layout);
    CreateGraphicsPipeline(VulkanContext, "shaders/spirv/blinn-phong.vert.spv",
                           "shaders/spirv/blinn-phong.frag.spv", &VulkanContext->GraphicsPipeline);
#endif

    // Wireframe
#if CREATE_WIREFRAME_PIPELINE
    CreatePipelineLayout(RenderDevice, 1, &VulkanContext->Swapchain.UniformSetLayout, 0, nullptr,
                         &VulkanContext->WireframePipeline.Layout);
    CreateWireframePipeline(VulkanContext, "shaders/spirv/wireframe.vert.spv", "shaders/spirv/wireframe.frag.spv");
#endif

    // Unlit Pipeline
    CreateUnlitPipeline(VulkanContext);

    CreateSynchronizationPrimitives(&VulkanContext->Device, &VulkanContext->SyncHandles);
    PrepareImGui(RenderDevice, &VulkanContext->ImContext, ScreenDim, VulkanContext->GraphicsRenderPass);

    AppInfo->WindowResizeCallback = &WindowResizedCallback;

#if MATERIAL_VIEWER
    GetMaterial(MaterialType::BRONZE, &FragUniformData.Material);
#endif

    VulkanContext->CurrentFrame = 0;
    VulkanContext->IsInitialized = true;
    VulkanContext->FrameCounter = 0;

    GlobalUiUpdateTimer = 0.0f;

    Context = VulkanContext;

    shoora_graphics::UpdatePipelineLayout(Context->UnlitPipeline.Layout);

    Scene = new shoora_scene();
    InitScene();
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
    Shu::mat4f Model = GlobalMat4Identity;
    Shu::Scale(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f));
    Shu::RotateGimbalLock(Model, Shu::Vec3f(1.0f, 1.0f, 1.0f), Angle*AngleSpeed);
    Shu::Translate(Model, Shu::Vec3f(0.0f, 0.0f, 0.0f));
    GlobalPushConstBlock.Model = Model;

    Shu::mat4f View = GlobalMat4Identity;
    View = Context->Camera.GetViewMatrix(View);
    GlobalVertUniformData.View = View;

    Shu::mat4f Projection = Context->Camera.GetProjectionMatrix();
    GlobalVertUniformData.Projection = Projection;

    GlobalLightShaderData.View = View;
    GlobalLightShaderData.Projection = Projection;
    memcpy(Context->FragUnlitBuffers[ImageIndex].pMapped, &GlobalLightShaderData, sizeof(light_shader_vert_data));

    GlobalFragUniformData.ObjectColor = GlobalRenderState.MeshColorUniform;
    GlobalFragUniformData.CamPos = Context->Camera.Pos;

    GlobalFragUniformData.SpotlightData.Direction = Shu::Normalize(Context->Camera.Front);
    GlobalFragUniformData.SpotlightData.Pos = Context->Camera.Pos;
}

void
GetMousePosDelta(f32 CurrentMouseDeltaX, f32 CurrentMouseDeltaY, f32 *outMouseDeltaX, f32 *outMouseDeltaY)
{
    *outMouseDeltaX = CurrentMouseDeltaX - GlobalLastFrameMousePos.x;
    *outMouseDeltaY = CurrentMouseDeltaY - GlobalLastFrameMousePos.y;
    GlobalLastFrameMousePos.x = CurrentMouseDeltaX;
    GlobalLastFrameMousePos.y = CurrentMouseDeltaY;
}

void
DrawFrameInVulkan(shoora_platform_frame_packet *FramePacket)
{
    if(Platform_GetKeyInputState(VirtualKeyCodes::SU_LEFTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        int x = 0;
    }

    // VK_CHECK(vkQueueWaitIdle(Context->Device.GraphicsQueue));
    Shu::vec2f CurrentMousePos = Shu::Vec2f(FramePacket->MouseXPos, FramePacket->MouseYPos);
    Shu::vec2f CurrentMouseWorldPos = MouseToWorld(CurrentMousePos);

    Context->Camera.UpdateWindowSize(Shu::Vec2f(GlobalWindowSize.x, GlobalWindowSize.y));

    ASSERT(Context != nullptr);
    ASSERT(FramePacket->DeltaTime > 0.0f);
    if(!Context->IsInitialized || Context->CurrentFrame >= SHU_MAX_FRAMES_IN_FLIGHT)
    {
        Platform_ExitApplication("[RENDERER]: Either the render is not initialized or there was some "
                                 "problem with the current frame counter.\n");
    }

    GlobalDeltaTime = FramePacket->DeltaTime;
    GlobalUiUpdateTimer += GlobalDeltaTime;
    if(GlobalUiUpdateTimer >= GlobalUiUpdateWaitTime)
    {
        DebugOverlay.Fps = FramePacket->Fps;
        DebugOverlay.MsPerFrame = 1000.0f*GlobalDeltaTime;
        GlobalUiUpdateTimer = 0.0f;
    }

    b32 LMBDown = Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, KeyState::SHU_KEYSTATE_DOWN);
#if 0
    if(Platform_GetKeyInputState(SU_RIGHTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        Scene->AddCircleBody(CurrentMouseWorldPos, colorU32::White, 25, 1.0, 0.5f);
    }
    if(Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        Scene->AddBoxBody(CurrentMouseWorldPos, colorU32::White, 50, 50, 1.0, 0.5f);
        // Scene->AddPolygonBody(1, CurrentMouseWorldPos, colorU32::White, 1.0f, 1.0f, 0.0f, 1.0f);
    }
    if(Platform_GetKeyInputState(SU_SPACE, KeyState::SHU_KEYSTATE_PRESS))
    {
        GlobalShowContacts = !GlobalShowContacts;
    }
#endif

    if(Platform_GetKeyInputState(SU_RIGHTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        GlobalLastFrameMousePos = {FramePacket->MouseXPos, FramePacket->MouseYPos};
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

    if(Platform_GetKeyInputState('F', KeyState::SHU_KEYSTATE_PRESS))
    {
        GlobalFragUniformData.SpotlightData.IsOn = !GlobalFragUniformData.SpotlightData.IsOn;
    }

    shoora_vulkan_fence_handle *pCurrentFrameFence = GetCurrentFrameFencePtr(&Context->SyncHandles,
                                                                             Context->CurrentFrame);
    auto *pCurrentFrameImageAvlSemaphore = GetImageAvailableSemaphorePtr(&Context->SyncHandles,
                                                                         Context->CurrentFrame);

    auto *pCurrentFramePresentCompleteSemaphore = GetRenderFinishedSemaphorePtr(&Context->SyncHandles,
                                                                                Context->CurrentFrame);

    VK_CHECK(vkWaitForFences(Context->Device.LogicalDevice, 1, &pCurrentFrameFence->Handle, VK_TRUE,
                             SHU_DEFAULT_FENCE_TIMEOUT));

    AcquireNextSwapchainImage(&Context->Device, &Context->Swapchain, pCurrentFrameImageAvlSemaphore);

    u32 ImageIndex = Context->Swapchain.CurrentImageIndex;
    shoora_vulkan_command_buffer_handle *pDrawCmdBuffer = &Context->Swapchain.DrawCommandBuffers[ImageIndex];
    VkCommandBuffer DrawCmdBuffer = pDrawCmdBuffer->Handle;

    shoora_graphics::UpdateCmdBuffer(DrawCmdBuffer);

    // RenderState.MeshColor = Vec3(1, 1, 0);
    WriteUniformData(ImageIndex, FramePacket->DeltaTime);

    VK_CHECK(vkResetFences(Context->Device.LogicalDevice, 1, &pCurrentFrameFence->Handle));
    VK_CHECK(vkResetCommandBuffer(DrawCmdBuffer, 0));

    VkCommandBufferBeginInfo DrawCmdBufferBeginInfo = {};
    DrawCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkClearValue ClearValues[2] = {};
    ClearValues[0].color =
    {
        {GlobalRenderState.ClearColor.r, GlobalRenderState.ClearColor.g, GlobalRenderState.ClearColor.b, 1.0f}
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

        // NOTE: This is the call which draws the sponza scene
        // DrawGeometry(DrawCmdBuffer, &Context->Geometry);

#if CUBE_SCENE
        VkDescriptorSet DescriptorSets[] = {Context->Swapchain.UniformDescriptorSets[ImageIndex],
                                            Context->Swapchain.FragSamplersDescriptorSet,
                                            Context->Swapchain.FragUniformsDescriptorSets[ImageIndex]};
        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Context->GraphicsPipeline.Layout, 0, ARRAY_SIZE(DescriptorSets),
                                DescriptorSets, 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->GraphicsPipeline.Handle);

        VkDeviceSize offsets[1] = {0};
        shoora_vulkan_vertex_buffers *VertBuffers = &Context->Sponza.VertBuffers;
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &VertBuffers->VertexBuffer.Handle, offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, VertBuffers->IndexBuffer.Handle, 0,
                             VK_INDEX_TYPE_UINT32);

        RenderCubes(DrawCmdBuffer);
#endif
#if CREATE_WIREFRAME_PIPELINE
        if(GlobalRenderState.WireframeMode)
        {
            vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->WireframePipeline.Handle);
            vkCmdSetLineWidth(DrawCmdBuffer, GlobalRenderState.WireLineWidth);
            vkCmdDrawIndexed(DrawCmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);
        }
#endif

#if UNLIT_PIPELINE
        VkDescriptorSet unlitSets[2] = {Context->UnlitSets[ImageIndex], Context->UnlitSamplerSet};
        vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Layout, 0,
                                ARRAY_SIZE(unlitSets), unlitSets, 0, nullptr);
        vkCmdBindPipeline(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Handle);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, shoora_mesh_database::GetVertexBufferHandlePtr(), offsets);
        vkCmdBindIndexBuffer(DrawCmdBuffer, shoora_mesh_database::GetIndexBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

        Scene->UpdateInput(CurrentMouseWorldPos);
        Scene->PhysicsUpdate(GlobalDeltaTime, GlobalShowContacts);
        DrawScene(DrawCmdBuffer);
#endif

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
    delete Scene;
    ImGuiCleanup(RenderDevice, &Context->ImContext);
    CleanupGeometry(RenderDevice, &Context->Geometry);

    CleanupUnlitPipeline();
    shoora_mesh_database::Destroy();
    DestroyUnlitPipelineResources();
    DestroySwapchainUniformResources(RenderDevice, &Context->Swapchain);

    DestroyAllSynchronizationPrimitives(RenderDevice, &Context->SyncHandles);
    DestroyVertexBuffer(RenderDevice, &Context->Geometry.VertBuffers.VertexBuffer,
                        &Context->Geometry.VertBuffers.IndexBuffer);
    DestroyPipeline(RenderDevice, &Context->GraphicsPipeline);
#if CREATE_WIREFRAME_PIPELINE
    DestroyPipeline(RenderDevice, &Context->WireframePipeline);
#endif
    DestroyRenderPass(RenderDevice, Context->GraphicsRenderPass);
    DestroySwapchain(Context);
    DestroyPresentationSurface(Context);
    DestroyLogicalDevice(RenderDevice);
#ifdef _SHU_DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}