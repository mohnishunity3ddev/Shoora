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

#include <mesh/primitive/geometry_primitive.h>
#include <physics/body.h>
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

#define NUM_BODIES 10
static b32 MouseTracking = false;
static Shu::vec2f MouseInitialDownPos = Shu::Vec2f(0);
static shoora_body *BodyToMove = nullptr;
static shoora_body Bodies[8192];
static u32 BodyCount = 0;
static f32 BodyMass = 1.5f;
static f32 PrevBodyMass = BodyMass;
static f32 BodyRadius = 50.0f;
static f32 PrevBodyRadius = BodyRadius;

static b32 isDebug = false;
static b32 WireframeMode = true;
static f32 TestCameraScale = 0.5f;

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
    ImGui::SliderFloat("Bodies Mass", &BodyMass, 0.1f, 10.0f);
    ImGui::SliderFloat("Bodies Radius", &BodyRadius, 1.0f, 100.0f);
    ImGui::SliderFloat("Test Scale", &TestCameraScale, 0.5f, 100.0f);
    ImGui::Checkbox("Toggle Wireframe", (bool *)&WireframeMode);


#if CREATE_WIREFRAME_PIPELINE
    ImGui::Checkbox("Toggle Wireframe", (bool *)&GlobalRenderState.WireframeMode);
    ImGui::SliderFloat("Wireframe Line Width", &GlobalRenderState.WireLineWidth, 1.0f, 10.0f);
#endif

    ImGui::ColorEdit3("Clear Color", GlobalRenderState.ClearColor.E);

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

#if 0
    ImVec2 SecondWindowPos = ImVec2(GlobalWindowSize.x - DesiredWidth, 400);
    ImVec2 SecondWindowSize = ImVec2(0, 0);
    ImGui::SetNextWindowPos(SecondWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(SecondWindowSize, ImGuiCond_Always);
    ImGui::Begin("Light Data", nullptr, WindowFlags);

    if(ImGui::CollapsingHeader("Point Light Data"))
    {
        if(ImGui::CollapsingHeader("Point Light 01"))
        {
            ImGui::DragFloat3("Position 01", GlobalFragUniformData.PointLightData[0].Pos.E, GlobalImGuiDragFloatStep);
            ImGui::ColorEdit3("Color 01", GlobalFragUniformData.PointLightData[0].Color.E);
            ImGui::SliderFloat("Intensity 01", &GlobalFragUniformData.PointLightData[0].Intensity, 0.3f, 10.0f);
        }

        if(ImGui::CollapsingHeader("Point Light 02"))
        {
            ImGui::DragFloat3("Position 02", GlobalFragUniformData.PointLightData[1].Pos.E, GlobalImGuiDragFloatStep);
            ImGui::ColorEdit3("Color 02", GlobalFragUniformData.PointLightData[1].Color.E);
            ImGui::SliderFloat("Intensity 02", &GlobalFragUniformData.PointLightData[1].Intensity, 0.3f, 10.0f);
        }

        if(ImGui::CollapsingHeader("Point Light 03"))
        {
            ImGui::DragFloat3("Position 03", GlobalFragUniformData.PointLightData[2].Pos.E, GlobalImGuiDragFloatStep);
            ImGui::ColorEdit3("Color 03", GlobalFragUniformData.PointLightData[2].Color.E);
            ImGui::SliderFloat("Intensity 03", &GlobalFragUniformData.PointLightData[2].Intensity, 0.3f, 10.0f);
        }

        if(ImGui::CollapsingHeader("Point Light 04"))
        {
            ImGui::DragFloat3("Position 04", GlobalFragUniformData.PointLightData[3].Pos.E, GlobalImGuiDragFloatStep);
            ImGui::ColorEdit3("Color 04", GlobalFragUniformData.PointLightData[3].Color.E);
            ImGui::SliderFloat("Intensity 04", &GlobalFragUniformData.PointLightData[3].Intensity, 0.3f, 10.0f);
        }
    }

    if(ImGui::CollapsingHeader("Spot Light"))
    {
        ImGui::SliderFloat("Inner Cutoff", &GlobalFragUniformData.SpotlightData.InnerCutoffAngles, 10.0f, 45.0f);
        ImGui::SliderFloat("Outer Cutoff", &GlobalFragUniformData.SpotlightData.OuterCutoffAngles,
                           GlobalFragUniformData.SpotlightData.InnerCutoffAngles, 45.0f);
        ImGui::ColorEdit3("Color", GlobalFragUniformData.SpotlightData.Color.E);
        ImGui::SliderFloat("Intensity", &GlobalFragUniformData.SpotlightData.Intensity, 0.3f, 100.0f);
    }
    ImGui::End();
#endif

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

struct unlit_shader_data
{
    Shu::mat4f Model;
    Shu::vec3f Color = {1, 1, 1};
};

void
AddCircle(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution)
{
    shoora_body *Body = &Bodies[BodyCount++];
    Body->Initialize(GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_circle>(Radius));
}

void
AddBox(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution)
{
    shoora_body *Body = &Bodies[BodyCount++];
    Body->Initialize(GetColor(ColorU32), Pos, Mass, Restitution,
                     std::make_unique<shoora_shape_box>(Width, Height));
}

void
AddTriangle(const Shu::vec2f Pos, u32 ColorU32, f32 Base, f32 Height, f32 Mass, f32 Restitution)
{
    shoora_body *Body = &Bodies[BodyCount++];
    Body->Initialize(GetColor(ColorU32), Pos, Mass, Restitution,
                     std::make_unique<shoora_shape_triangle>(Base, Height));
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

void
AddImpulseToBody(shoora_body *Body, const Shu::vec2f InImpulse)
{
    Shu::vec2f Impulse = InImpulse*5.0f;
    Body->Velocity += Shu::Vec3f(Impulse, 0.0f);
}

inline void
UpdateBodies(const VkCommandBuffer &CmdBuffer, const VkPipelineLayout &PipelineLayout, f32 dt)
{
    for(i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        ASSERT(BodyIndex < BodyCount);
        shoora_body *Body = Bodies + BodyIndex;

        // NOTE: If I am debugging, the frametime is going to be huge. So hence, clamping here.
#if _SHU_DEBUG
        if(dt > 1.0f/29.0f) { dt = 1.0f / 29.0f; }
#endif

#if 0 // Weight Force
        Shu::vec2f WeightForce = Shu::Vec2f(0.0f, -9.8f*SHU_PIXELS_PER_METER*Body->Mass);
        Body->AddForce(WeightForce);
#endif

#if 0 // Push Force through Keys
        Shu::vec2f PushForce = Shu::Vec2f(1.0f, 1.0f)*(SHU_PIXELS_PER_METER*100);
        Shu::vec2f BodyForce = Shu::vec2f::Zero();
        if(Platform_GetKeyInputState(SU_UPARROW, KeyState::SHU_KEYSTATE_DOWN)) { BodyForce.y += PushForce.y; }
        if(Platform_GetKeyInputState(SU_DOWNARROW, KeyState::SHU_KEYSTATE_DOWN)) { BodyForce.y -= PushForce.y; }
        if(Platform_GetKeyInputState(SU_LEFTARROW, KeyState::SHU_KEYSTATE_DOWN)) { BodyForce.x -= PushForce.x; }
        if(Platform_GetKeyInputState(SU_RIGHTARROW, KeyState::SHU_KEYSTATE_DOWN)) { BodyForce.x += PushForce.x; }
        Body->AddForce(BodyForce);
#endif

#if 0 // Drag Force
        Shu::vec2f DragForce = force::GenerateDragForce(Body, 0.03f);
        Body->AddForce(DragForce);
#endif

#if 0 // Wind Force
        Shu::vec2f Wind = Shu::Vec2f(20.0f * SHU_PIXELS_PER_METER, 0.0f);
        Body->AddForce(Wind);
#endif

#if 0 // Gravitation Force
        if((BodyIndex+1) < BodyCount)
        {
            shoora_Body *NextBody = Bodies + (BodyIndex+1);
            Shu::vec2f GravitationalForce = force::GenerateGravitationalForce(Body, NextBody, 100.0f, 5.0f, 100.0f);
            Body->AddForce(GravitationalForce);
            NextBody->AddForce(GravitationalForce*-1.0f);
        }
#endif

#if 0 // Friction Force
        Shu::vec2f FrictionForce = force::GenerateFrictionForce(Body, 10.0f*SHU_PIXELS_PER_METER);
        Body->AddForce(FrictionForce);
#endif

        Body->IntegrateLinear(dt);

        for (i32 i = 0; i < BodyCount; ++i)
        {
            Bodies[i].IsColliding = false;
        }

        // NOTE: Check for collision with the rest of the rigidbodies present in the scene.
        for(i32 i = 0; i < (BodyCount-1); ++i)
        {
            shoora_body *A = Bodies + i;
            for(i32 j = (i+1); j < BodyCount; ++j)
            {
                shoora_body *B = Bodies + j;
                contact Contact;
                if(collision2d::IsColliding(A, B, Contact))
                {
                    // NOTE: Visualizing the Collision Contact Info.
                    DrawCircle(CmdBuffer, PipelineLayout, Contact.Start.xy, 3, 0xff00ffff);
                    DrawCircle(CmdBuffer, PipelineLayout, Contact.End.xy, 3, 0xffff0000);
                    Shu::vec2f ContactNormalLineEnd = Shu::Vec2f(Contact.Start.x + Contact.Normal.x*15.0f,
                                                                 Contact.Start.y + Contact.Normal.y*15.0f);
                    DrawLine(CmdBuffer, PipelineLayout, Contact.Start.xy, ContactNormalLineEnd, 0xffff00ff, 2);

                    Contact.ResolveCollision();

                    A->IsColliding = true;
                    B->IsColliding = true;
                }
            }
        }

        Shu::rect2d Rect = Context->Camera.GetRect();
        f32 DampFactor = -1.0f;
        Body->KeepInView(Rect, DampFactor);
    }
}

void
DrawBodyWireframe(const VkCommandBuffer cmdBuffer, shoora_body *body, const Shu::mat4f &model, f32 thickness,
                  u32 color)
{
    shoora_mesh_filter *mesh = &body->Shape->Primitive->MeshFilter;
    shoora_primitive_type Type = body->Shape->Primitive->PrimitiveType;
    if (Type == shoora_primitive_type::CIRCLE)
    {
        for (i32 i = 1; i < mesh->VertexCount; ++i)
        {
            Shu::vec3f pos0 = mesh->Vertices[i - 1].Pos;
            Shu::vec3f pos1 = mesh->Vertices[i].Pos;

            Shu::vec2f p0 = (model * pos0).xy;
            Shu::vec2f p1 = (model * pos1).xy;
            DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, 2.5f);
        }

        Shu::vec2f p0 = (model * mesh->Vertices[mesh->VertexCount - 1].Pos).xy;
        Shu::vec2f p1 = (model * mesh->Vertices[1].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
    }
    else if(Type == shoora_primitive_type::RECT_2D)
    {
        ASSERT(mesh->VertexCount == 4);

        Shu::vec2f p0 = (model * mesh->Vertices[2].Pos).xy;
        Shu::vec2f p1 = (model * mesh->Vertices[1].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
        p0 = (model * mesh->Vertices[1].Pos).xy;
        p1 = (model * mesh->Vertices[0].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
        p0 = (model * mesh->Vertices[0].Pos).xy;
        p1 = (model * mesh->Vertices[3].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
        p0 = (model * mesh->Vertices[3].Pos).xy;
        p1 = (model * mesh->Vertices[2].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
    }
    else if (Type == shoora_primitive_type::TRIANGLE)
    {
        ASSERT(mesh->VertexCount == 3);

        Shu::vec2f p0 = (model * mesh->Vertices[1].Pos).xy;
        Shu::vec2f p1 = (model * mesh->Vertices[0].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
        p0 = (model * mesh->Vertices[2].Pos).xy;
        p1 = (model * mesh->Vertices[1].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
        p0 = (model * mesh->Vertices[0].Pos).xy;
        p1 = (model * mesh->Vertices[2].Pos).xy;
        DrawLine(cmdBuffer, Context->UnlitPipeline.Layout, p0, p1, color, thickness);
    }
}

void
DrawBodies(const VkCommandBuffer &CmdBuffer, const VkPipelineLayout &PipelineLayout, f32 DeltaTime, b32 Wireframe)
{
    auto camRect = Context->Camera.GetRect();
    auto left = Shu::Vec2f(camRect.x - (camRect.width / 2), camRect.y);
    auto right = Shu::Vec2f(camRect.x + (camRect.width / 2), camRect.y);
    DrawLine(CmdBuffer, PipelineLayout, left, right, 0xff313131, 1.0f);
    auto top = Shu::Vec2f(camRect.x, camRect.y + (camRect.height / 2));
    auto bottom = Shu::Vec2f(camRect.x, camRect.y - (camRect.height / 2));
    DrawLine(CmdBuffer, PipelineLayout, top, bottom, 0xff313131, 1.0f);


    std::vector<Shu::vec3f> vertices[2];
    for (i32 i = 0; i < 2; ++i)
    {
        shoora_body *body = Bodies + i;
        auto *meshFilter = &shoora_primitive_collection::GetPrimitive(body->Shape->Type)->MeshFilter;
        vertices[i].reserve(meshFilter->VertexCount);
        auto model = Shu::TRS(body->Position, body->Scale, body->RotationRadians * RAD_TO_DEG, Shu::Vec3f(0, 0, 1));
        for (i32 j = 0; j < meshFilter->VertexCount; ++j)
        {
            vertices[i].emplace_back((model * meshFilter->Vertices[j].Pos).xyz);
        }
    }

    std::vector<Shu::vec3f> minkowksiVertices;
    minkowksiVertices.reserve(vertices[0].size() * vertices[1].size());
    u32 colors[4] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffff00};
    for (i32 i = 0; i < vertices[1].size(); ++i)
    {
        auto vb = vertices[1][i];
        for (i32 j = 0; j < vertices[0].size(); ++j)
        {
            auto o = Shu::Vec3f(0.0f);
            auto line = vertices[0][j] - vb;
            line = Shu::Normalize(line) * (line.Magnitude());
            auto va = o + line;

            DrawLine(CmdBuffer, PipelineLayout, o.xy, va.xy, colors[i], 1.4f);
            DrawLine(CmdBuffer, PipelineLayout, vb.xy, vertices[0][j].xy, colors[i], .4f);
            DrawCircle(CmdBuffer, PipelineLayout, va.xy, 5.0f, 0xffff0000);
            minkowksiVertices.emplace_back(va);
        }
    }


    if (Wireframe && !isDebug)
    {
        LogWarnUnformatted("Wireframe mode is only available in debug builds. Turning off Wireframe mode!\n");
        Wireframe = false;
    }

    for(u32 BodyIndex = 0;
        BodyIndex < BodyCount;
        ++BodyIndex)
    {
        shoora_body *Body = Bodies + BodyIndex;

        u32 ColorU32 = Body->IsColliding ? 0xffff0000 : 0xffffffff;
        Shu::vec3f Color = GetColor(ColorU32);

        Shu::mat4f Model = Shu::TRS(Body->Position, Body->Scale, Body->RotationRadians*RAD_TO_DEG, Shu::Vec3f(0, 0, 1));
        unlit_shader_data Value = {.Model = Model, .Color = Color};

        if(!Wireframe)
        {
            vkCmdPushConstants(CmdBuffer, Context->UnlitPipeline.Layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(unlit_shader_data), &Value);
            Body->Shape->Draw(CmdBuffer);
        }
        else
        {
            DrawBodyWireframe(CmdBuffer, Body, Model, 2.5f, ColorU32);
        }
    }
}

void
UpdateBodiesOnInput(VkCommandBuffer CmdBuffer, const Shu::vec2f &CurrentMousePos,
                    const Shu::vec2f &CurrentMouseWorldPos)
{
    for(u32 BodyIndex = 0;
        BodyIndex < BodyCount;
        ++BodyIndex)
    {
        shoora_body *Body = Bodies + BodyIndex;

        if(Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, SHU_KEYSTATE_DOWN))
        {
            if(!MouseTracking)
            {
                if(Body->CheckIfClicked(MouseToWorld(CurrentMousePos)))
                {
                    MouseTracking = true;
                    MouseInitialDownPos = Shu::ToVec2(Body->Position);
                    BodyToMove = Body;
                }
            }
            else if(BodyToMove != nullptr)
            {
                BodyToMove->Position = Shu::Vec3f(CurrentMouseWorldPos, BodyToMove->Position.z);
            }
        }

#if 0
        if(MouseTracking)
        {
            DrawLine(CmdBuffer, Context->UnlitPipeline.Layout, MouseInitialDownPos, CurrentMouseWorldPos,
                     0xffff0000, 3.0f);
        }
#endif
    }

    if (MouseTracking && Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, SHU_KEYSTATE_RELEASE))
    {
        ASSERT(BodyToMove != nullptr);
#if 0
        Shu::vec2f Force = (MouseInitialDownPos - CurrentMouseWorldPos);
        AddImpulseToBody(BodyToMove, Force);
#endif
        MouseTracking = false;
        BodyToMove = nullptr;
    }
}

void
InitScene()
{
    // AddCircle(Shu::Vec2f(100.0f, 100.0f), 0xff00ffff, 200, 0, 1.0f);
    // AddCircle(Shu::Vec2f(500.0f, 100.0f), 0xff00ffff, 50, 1, 0.8f);
    // AddCircle(Shu::Vec2f(500.0f, 100.0f), 0xff00ffff, 50, 1, 0.8f);
    // AddCircle(Shu::Vec2f(500.0f, 100.0f), 0xff00ffff, 50, 1, 0.8f);
    // AddCircle(Shu::Vec2f(500.0f, 100.0f), 0xff00ffff, 50, 1, 0.8f);
    // AddCircle(Shu::Vec2f(500.0f, 100.0f), 0xff00ffff, 50, 1, 0.8f);

    AddBox(Shu::Vec2f(100, 100), 0xffffffff, 100, 200, 1.0f, 1.0f);
    Bodies[0].RotationRadians = 45.0f*DEG_TO_RAD;

    AddBox(Shu::Vec2f(300, 100), 0xffffffff, 100, 50, 1.0f, 1.0f);
    Bodies[1].RotationRadians = -30.0f*DEG_TO_RAD;
}

void
DrawScene(const VkCommandBuffer &CmdBuffer, const VkPipelineLayout &PipelineLayout, u32 SwapchainImageIndex,
          const Shu::vec2f CurrentMousePos, const Shu::vec2f CurrentMouseWorldPos, f32 DeltaTime)
{
    vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Layout, 0, 1,
                            &Context->UnlitSets[SwapchainImageIndex], 0, nullptr);
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->UnlitPipeline.Handle);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(CmdBuffer, 0, 1, shoora_primitive_collection::GetVertexBufferHandlePtr(), offsets);
    vkCmdBindIndexBuffer(CmdBuffer, shoora_primitive_collection::GetIndexBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

    UpdateBodiesOnInput(CmdBuffer, CurrentMousePos, CurrentMouseWorldPos);
    UpdateBodies(CmdBuffer, PipelineLayout, DeltaTime);
    DrawBodies(CmdBuffer, PipelineLayout, GlobalDeltaTime, WireframeMode);
}

void
CreateUnlitPipeline(shoora_vulkan_context *Context)
{
    shoora_vulkan_device *RenderDevice = &Context->Device;
    shoora_vulkan_swapchain *Swapchain = &Context->Swapchain;

    VkDescriptorPoolSize Sizes[1];
    Sizes[0] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHU_MAX_FRAMES_IN_FLIGHT);
    CreateDescriptorPool(&Context->Device, ARRAY_SIZE(Sizes), Sizes, 4, &Context->UnlitDescriptorPool);

    CreateUniformBuffers(RenderDevice, Context->FragUnlitBuffers, ARRAY_SIZE(Context->FragUnlitBuffers),
                         sizeof(light_shader_vert_data));

    VkDescriptorSetLayoutBinding Bindings[1];
    // NOTE: So, This descriptor's data has already been computed and is being used in other pipelines
    // This is the one which contains Model, View, Projection Matrices data.
    Bindings[0] = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                                VK_SHADER_STAGE_VERTEX_BIT);
    CreateDescriptorSetLayout(RenderDevice, Bindings, ARRAY_SIZE(Bindings), &Context->UnlitSetLayout);

    for(u32 Index = 0;
        Index < SHU_MAX_FRAMES_IN_FLIGHT;
        ++Index)
    {
        AllocateDescriptorSets(RenderDevice, Context->UnlitDescriptorPool, 1, &Context->UnlitSetLayout,
                               &Context->UnlitSets[Index]);
        // NOTE: MVP Matrices Data - three 4x4 Matrices
        UpdateBufferDescriptorSet(RenderDevice, Context->UnlitSets[Index], 0,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Context->FragUnlitBuffers[Index].Handle,
                                  2*sizeof(Shu::mat4f), 0);
        // NOTE: Light Fragment Data - one vec3 - Color
        // UpdateBufferDescriptorSet(RenderDevice, Context->UnlitSets[Index], 1,
        //                           VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Context->FragUnlitBuffers[Index].Handle,
        //                           sizeof(Shu::vec3f), OFFSET_OF(light_shader_vert_data, Color));
    }

    VkPushConstantRange PushConstants[1];
    PushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    PushConstants[0].offset = 0;
    PushConstants[0].size = sizeof(light_shader_push_constant_data);

    CreatePipelineLayout(RenderDevice, 1, &Context->UnlitSetLayout, ARRAY_SIZE(PushConstants), PushConstants,
                         &Context->UnlitPipeline.Layout);
    CreateGraphicsPipeline(Context, "shaders/spirv/unlit.vert.spv", "shaders/spirv/unlit.frag.spv",
                           &Context->UnlitPipeline);

    InitScene();
}

void
CleanupUnlitPipeline()
{
    vkDestroyDescriptorPool(Context->Device.LogicalDevice, Context->UnlitDescriptorPool, nullptr);
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

#if 0
    GenerateNormals(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));
    GenerateTangentInformation(CubeVertices, CubeIndices, ARRAY_SIZE(CubeIndices));
#endif

    Shu::vec2u ScreenDim = Shu::vec2u{AppInfo->WindowWidth, AppInfo->WindowHeight};
    CreateSwapchain(&VulkanContext->Device, &VulkanContext->Swapchain, ScreenDim, &SwapchainInfo);
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    SetupCamera(&VulkanContext->Camera, shoora_projection::PROJECTION_ORTHOGRAPHIC, 0.1f, 100.0f, 16.0f / 9.0f,
                GlobalWindowSize.y, 45.0f /*,  Shu::Vec3f(GlobalWindowSize.x/2, GlobalWindowSize.y/2, 0.0f) */);

    shoora_primitive_collection::Initialize(RenderDevice, 40);

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
#endif
    // memcpy(Context->Swapchain.UniformBuffers[ImageIndex].pMapped, &GlobalVertUniformData,
    //        sizeof(vert_uniform_data));

    GlobalLightShaderData.View = View;
    GlobalLightShaderData.Projection = Projection;
    memcpy(Context->FragUnlitBuffers[ImageIndex].pMapped, &GlobalLightShaderData, sizeof(light_shader_vert_data));

    // Light Position is set directly in ImGui
    // GlobalFragUniformData.PointLightData.Color = ;
    GlobalFragUniformData.ObjectColor = GlobalRenderState.MeshColorUniform;
    GlobalFragUniformData.CamPos = Context->Camera.Pos;

    GlobalFragUniformData.SpotlightData.Direction = Shu::Normalize(Context->Camera.Front);
    GlobalFragUniformData.SpotlightData.Pos = Context->Camera.Pos;

    // memcpy(Context->Swapchain.FragUniformBuffers[ImageIndex].pMapped, &GlobalFragUniformData,
    //        sizeof(lighting_shader_uniform_data));

    // UpdateGeometryUniformBuffers(&Context->Geometry, &Context->Camera, GlobalVertUniformData.Projection);
}

void
GetMousePosDelta(f32 CurrentMouseDeltaX, f32 CurrentMouseDeltaY, f32 *outMouseDeltaX, f32 *outMouseDeltaY)
{
    *outMouseDeltaX = CurrentMouseDeltaX - GlobalLastFrameMousePos.x;
    *outMouseDeltaY = CurrentMouseDeltaY - GlobalLastFrameMousePos.y;
    GlobalLastFrameMousePos.x = CurrentMouseDeltaX;
    GlobalLastFrameMousePos.y = CurrentMouseDeltaY;
}

#if 0
void
RenderCubes(VkCommandBuffer CmdBuffer)
{
    static f32 LocalStaticAngle = 0.0f;
    LocalStaticAngle += GlobalDeltaTime;
    if(LocalStaticAngle >= 360.0f)
    {
        LocalStaticAngle = 360.0f;
    }
    for(u32 Index = 0;
        Index < ARRAY_SIZE(CubePositions);
        ++Index)
    {
        Shu::mat4f Model = GlobalMat4Identity;
        Shu::Scale(Model, Shu::Vec3f(0.5f, 0.5f, 0.5f));
        f32 LocalAngle = LocalStaticAngle*(Index + 1)*0.05f;
        Shu::RotateGimbalLock(Model, Shu::Vec3f(0.5f*(Index + 1), 0.3f*(Index + 1), 1.0f*(Index + 1)),
                              LocalAngle*AngleSpeed);
        Shu::Translate(Model, CubePositions[Index]);
        GlobalPushConstBlock.Model = Model;
        vkCmdPushConstants(CmdBuffer, Context->GraphicsPipeline.Layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(push_const_block), &GlobalPushConstBlock);
        vkCmdDrawIndexed(CmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);
    }
}
void
RenderLightCubes(VkCommandBuffer CmdBuffer)
{
    for(u32 Index = 0;
        Index < ARRAY_SIZE(GlobalLightPushConstantData);
        ++Index)
    {
        Shu::mat4f *pModel = &GlobalLightPushConstantData[Index].Model;
        *pModel = GlobalMat4Identity;
        Shu::Scale(*pModel, Shu::Vec3f(0.05f));
        Shu::Translate(*pModel, GlobalFragUniformData.PointLightData[Index].Pos);
        GlobalLightPushConstantData[Index].Color = GlobalFragUniformData.PointLightData[Index].Color;
        vkCmdPushConstants(CmdBuffer, Context->UnlitPipeline.Layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(light_shader_push_constant_data), &GlobalLightPushConstantData[Index]);
        vkCmdDrawIndexed(CmdBuffer, ARRAY_SIZE(CubeIndices), 1, 0, 0, 1);
    }
}
#endif

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
        DrawScene(DrawCmdBuffer, Context->UnlitPipeline.Layout, ImageIndex, CurrentMousePos, CurrentMouseWorldPos,
                  GlobalDeltaTime);
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

    ImGuiCleanup(RenderDevice, &Context->ImContext);
    CleanupGeometry(RenderDevice, &Context->Geometry);

    CleanupUnlitPipeline();
    shoora_primitive_collection::Destroy();
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
#ifdef _DEBUG
    DestroyDebugUtilHandles(Context);
#endif
    DestroyVulkanInstance(Context);
    LogOutput(LogType_Info, "Destroyed Vulkan Renderer!\n");
}