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
#include <physics/bounds.h>
#include <utils/utils.h>
#include <utils/random/random.h>

#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif

#if SHU_USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#define UNLIT_PIPELINE 1

static shoora_vulkan_context *Context = nullptr;

static b32 isDebug = false;
static b32 WireframeMode = false;
static f32 TestCameraScale = 0.5f;
static b32 GlobalDebugMode = false;
static b32 GlobalPausePhysics = false;
void Shu_DebugBreak() { GlobalPausePhysics = !GlobalPausePhysics; }

static platform_work_queue *GlobalJobQueue;

// NOTE: ALso make the same changes to the lighting shader.
// TODO)): Automate this so that changing this automatically makes changes to the shader using shader variation.
#define MATERIAL_VIEWER 0

struct vert_uniform_data
{
    shu::mat4f View;
    // Shu::mat4f Model;
    shu::mat4f Projection;
};

struct spotlight_data
{
    b32 IsOn;

    SHU_ALIGN_16 shu::vec3f Pos;
    SHU_ALIGN_16 shu::vec3f Color = shu::Vec3f(1.0f);
    SHU_ALIGN_16 shu::vec3f Direction;

    float InnerCutoffAngles = 12.5f;
    float OuterCutoffAngles = 15.5f;
    float Intensity = 5.0f;
};

struct point_light_data
{
    SHU_ALIGN_16 shu::vec3f Pos = shu::Vec3f(3, 0, 0);
    SHU_ALIGN_16 shu::vec3f Color = shu::Vec3f(1, 1, 0);
    float Intensity = 5.0f;
};

struct lighting_shader_uniform_data
{
    SHU_ALIGN_16 point_light_data PointLightData[4];
    SHU_ALIGN_16 spotlight_data SpotlightData;

    SHU_ALIGN_16 shu::vec3f CamPos = shu::Vec3f(0, 0, -10);
    SHU_ALIGN_16 shu::vec3f ObjectColor;

#if MATERIAL_VIEWER
    shoora_material Material;
#endif
};

struct push_const_block
{
    shu::mat4f Model;
};

struct light_shader_vert_data
{
    shu::mat4f View;
    shu::mat4f Projection;
    // Fragment Shader
};

struct light_shader_push_constant_data
{
    shu::mat4f Model;
    shu::vec3f Color = {1, 1, 1};
};

struct shoora_render_state
{
    b8 WireframeMode = false;
    f32 WireLineWidth = 10.0f;
    // shu::vec3f ClearColor = shu::vec3f{0.043f, 0.259f, 0.259f};
    shu::vec3f ClearColor = shu::Vec3f(0.0f);
    shu::vec3f MeshColorUniform = shu::vec3f{1.0f, 1.0f, 1.0f};
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
static const shu::mat4f GlobalMat4Identity = shu::Mat4f(1.0f);
static shu::vec2f GlobalLastFrameMousePos = shu::Vec2f(FLT_MAX, FLT_MAX);
static b32 GlobalSetFPSCap = true;
static i32 GlobalSelectedFPSOption = 1;
static vert_uniform_data GlobalVertUniformData = {};
static lighting_shader_uniform_data GlobalFragUniformData = {};
static light_shader_vert_data GlobalLightShaderData;
static light_shader_push_constant_data GlobalLightPushConstantData[4];
static push_const_block GlobalPushConstBlock = {};
static f32 GlobalImGuiDragFloatStep = 0.005f;
static shu::vec2u GlobalWindowSize = {};

static shoora_scene *Scene;

void
WindowResizedCallback(u32 Width, u32 Height)
{
    LogOutput(LogType_Debug, "Window Resized to {%d, %d}\n", Width, Height);

    if(Context && (Width > 0 && Height > 0))
    {
        ASSERT(Context->IsInitialized);
        WindowResized(&Context->Device, &Context->Swapchain, Context->GraphicsRenderPass,
                      shu::vec2u{Width, Height});
        ImGuiUpdateWindowSize(shu::vec2u{Width, Height});
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

#define GJK_STEPTHROUGH 0
#if GJK_STEPTHROUGH
static shu::vec3f diamondEulerAngles = shu::Vec3f(0.0f);
#endif

static shu::vec3f Pos = shu::Vec3f(0, 2.5f, 0);
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
    ImGui::DragFloat3("Pos", Pos.E, 1.0f, -100, 100);

#if GJK_STEPTHROUGH
    shoora_body *a = &Scene->Bodies[0];
    shoora_body *b = &Scene->Bodies[1];
    ImGui::DragFloat3("Diamond Position", a->Position.E, .05f, -100, 100);
    ImGui::DragFloat3("Diamond Rotation", diamondEulerAngles.E, .5f, -500, 500);
    ImGui::DragFloat3("Floor Position", b->Position.E, .05f, -100, 100);
#endif

    ImGui::SliderFloat("Test Scale", &TestCameraScale, 0.5f, 100.0f);
    ImGui::Checkbox("Toggle Wireframe", (bool *)&WireframeMode);

    ImGui::Spacing();
    ImGui::Text("Body Count: %d", Scene->GetBodyCount());

    if(Scene->GetBodyCount() > 1)
    {
        const shoora_body *b = &Scene->Bodies[1];
        const shu::vec3f v = b->LinearVelocity;
        const shu::vec3f w = b->AngularVelocity;
        ImGui::Text("B Linear Vel: [%.3f, %.3f, %.3f].", v.x, v.y, v.z);
        ImGui::Text("B Angular Vel: [%.3f, %.3f, %.3f].", w.x, w.y, w.z);
    }

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

shu::vec2f
MouseToWorld(const shu::vec2f &MousePos)
{
    // NOTE: This is the position where the y position is flipped since windows's mouse pos (0,0) starts from the
    // top left and y increases in the downward direction.
    shu::vec2f worldPos = shu::Vec2f(MousePos.x, ((f32)GlobalWindowSize.y) - MousePos.y);

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
static const f32 width = 50;
static const f32 height = 1;
static const f32 depth = 25;

shu::vec3f g_boxGround[] =
{
    shu::Vec3f(-width,  height*.5f, -depth),
    shu::Vec3f( width,  height*.5f, -depth),
    shu::Vec3f(-width,  height*.5f,  depth),
    shu::Vec3f( width,  height*.5f,  depth),
    shu::Vec3f(-width, -height*.5f, -depth),
    shu::Vec3f( width, -height*.5f, -depth),
    shu::Vec3f(-width, -height*.5f,  depth),
    shu::Vec3f( width, -height*.5f,  depth)
};

shu::vec3f g_boxWall0[] =
{
    shu::Vec3f(-1,  -2.5f, -depth),
    shu::Vec3f( 1,  -2.5f, -depth),
    shu::Vec3f(-1,  -2.5f,  depth),
    shu::Vec3f( 1,  -2.5f,  depth),
    shu::Vec3f(-1,   2.5f, -depth),
    shu::Vec3f( 1,   2.5f, -depth),
    shu::Vec3f(-1,   2.5f,  depth),
    shu::Vec3f( 1,   2.5f,  depth)
};

shu::vec3f g_boxWall1[] =
{
    shu::Vec3f(-width,  -2.5f, -1),
    shu::Vec3f( width,  -2.5f, -1),
    shu::Vec3f(-width,  -2.5f,  1),
    shu::Vec3f( width,  -2.5f,  1),
    shu::Vec3f(-width,   2.5f, -1),
    shu::Vec3f( width,   2.5f, -1),
    shu::Vec3f(-width,   2.5f,  1),
    shu::Vec3f( width,   2.5f,  1)
};

shu::vec3f g_boxUnit[] =
{
    shu::Vec3f(-1, -1, -1),
    shu::Vec3f( 1, -1, -1),
    shu::Vec3f(-1,  1, -1),
    shu::Vec3f( 1,  1, -1),
    shu::Vec3f(-1, -1,  1),
    shu::Vec3f( 1, -1,  1),
    shu::Vec3f(-1,  1,  1),
    shu::Vec3f( 1,  1,  1)
};

static const float t = 0.25f;
shu::vec3f g_boxSmall[] =
{

    shu::Vec3f(-t, -t, -t),
    shu::Vec3f( t, -t, -t),
    shu::Vec3f(-t,  t, -t),
    shu::Vec3f( t,  t, -t),
    shu::Vec3f(-t, -t,  t),
    shu::Vec3f( t, -t,  t),
    shu::Vec3f(-t,  t,  t),
    shu::Vec3f( t,  t,  t)
};

static const float l = 3.0f;
shu::vec3f g_boxBeam[] =
{
    shu::Vec3f(-l, -t, -t),
    shu::Vec3f( l, -t, -t),
    shu::Vec3f(-l,  t, -t),
    shu::Vec3f( l,  t, -t),
    shu::Vec3f(-l, -t,  t),
    shu::Vec3f( l, -t,  t),
    shu::Vec3f(-l,  t,  t),
    shu::Vec3f( l,  t,  t)
};


shu::vec3f g_diamond[7*8];
void
DrawDebugDiamond()
{
#if 1
    shoora_graphics::DrawSphere(g_diamond[0], .025f, colorU32::Red);
    for (i32 i = 1; i < ARRAY_SIZE(g_diamond); ++i)
    {
        u32 col = colorU32::Red;
        shoora_graphics::DrawSphere(g_diamond[i], .025f, col);
        shoora_graphics::DrawLine3D(g_diamond[i - 1], g_diamond[i], colorU32::Proto_Yellow, .01f);
    }
    shoora_graphics::DrawLine3D(g_diamond[ARRAY_SIZE(g_diamond) - 1], g_diamond[0], colorU32::Proto_Yellow, .01f);
#else
    auto o = Shu::Vec3f(0.0f);
    shoora_graphics::DrawLine3D(o, Shu::Vec3f(1, 0, 0), colorU32::Red, .01f);
    shoora_graphics::DrawLine3D(o, Shu::Vec3f(0, 1, 0), colorU32::Green, .01f);
    shoora_graphics::DrawLine3D(o, Shu::Vec3f(0, 0, 1), colorU32::Blue, .01f);
    shoora_graphics::DrawSphere(o, .025f, 0);
    i32 s = 0;
    for (i32 i = s; i < s+7; ++i)
    {
        u32 col = colorU32::Proto_Red;
        if(i == s+1) col = colorU32::Proto_Blue;
        else if(i == s+2) col = colorU32::Proto_Green;
        else if(i == s+3) col = colorU32::Yellow;
        else if(i == s+4) col = colorU32::White;
        else if(i == s+5) col = colorU32::Proto_Yellow;
        else if(i == s+6) col = colorU32::Magenta;
        shoora_graphics::DrawSphere(g_diamond[i], .025f, col);
    }
    for (i32 j = 0; j < 7; ++j)
    {
        s += 7;
        for (i32 i = s; i < s+7; ++i)
        {
            u32 col = colorU32::Proto_Red;
            if(i == s+1) col = colorU32::Proto_Blue;
            else if(i == s+2) col = colorU32::Proto_Green;
            else if(i == s+3) col = colorU32::Yellow;
            else if(i == s+4) col = colorU32::White;
            else if(i == s+5) col = colorU32::Proto_Yellow;
            else if(i == s+6) col = colorU32::Magenta;
            shoora_graphics::DrawSphere(g_diamond[i], .025f, col);
        }
    }
#endif
}

#define BUILD_CONVEX_THREADED 0

#if BUILD_CONVEX_THREADED
#include <physics/shape/shape_convex.h>
void
OnConvexBodyReady(shoora_shape_convex *Convex, memory_arena *Arena = nullptr)
{
    shoora_body body = {};
    body.Position = shu::Vec3f(0, 0, 0);
    // body.Scale = shu::Vec3f(1, 1, 1);
    // body.Rotation = shu::QuatAngleAxisDeg(45, shu::Vec3f(1, 1, 1));
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0, 0, 0);
    body.InvMass = 0.0f;
    body.Mass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    body.Shape = Convex;
    body.InertiaTensor = (body.Shape->InertiaTensor() * body.Mass);
    body.InverseInertiaTensor = body.InertiaTensor.IsZero() ? shu::Mat3f(0.0f) : body.InertiaTensor.Inverse();
    body.Scale = body.Shape->GetDim();
    body.SumForces = body.Acceleration = shu::vec3f::Zero();
    body.SumTorques = 0.0f;

    LogInfoUnformatted("\n\n");

    if (Arena == nullptr)
    {
        Arena = GetArena(MEMTYPE_GLOBAL);
    }
    shoora_vertex_info *Vertices = (shoora_vertex_info *)ShuAllocate_(Arena,
                                                                      sizeof(shoora_vertex_info) * Convex->NumPoints);
    for (i32 i = 0; i < Convex->NumHullPoints; ++i)
    {
        Vertices[i].Pos = Convex->HullPoints[i];
        Vertices[i].Normal = shu::vec3f::Zero();
        Vertices[i].Color = shu::Vec3f(i) / (f32)Convex->NumHullPoints;
    }

    // NOTE: Normals Calculation
    for (i32 i = 0; i < Convex->NumHullIndices; i += 3)
    {
        // counter clockwise normal calculation
        u32 vIndex0 = Convex->HullIndices[i + 0];
        u32 vIndex1 = Convex->HullIndices[i + 1];
        u32 vIndex2 = Convex->HullIndices[i + 2];

        shu::vec3f a = Vertices[vIndex0].Pos;
        shu::vec3f b = Vertices[vIndex1].Pos;
        shu::vec3f c = Vertices[vIndex2].Pos;

        shu::vec3f ba = (b - a);
        shu::vec3f ca = (c - a);
        shu::vec3f normal = shu::Normalize(shu::Cross(ba, ca));

        Vertices[vIndex0].Normal += normal;
        Vertices[vIndex1].Normal += normal;
        Vertices[vIndex2].Normal += normal;
    }
    for (i32 i = 0; i < Convex->NumHullPoints; ++i)
    {
        Vertices[i].Normal = shu::Normalize(Vertices[i].Normal);
    }

    // for(i32 i = 0; i < Convex->NumHullPoints; ++i)
    // {
    //     LogInfo("Pos: {%f, %f, %f}.\n", Convex->HullPoints[i].x, Convex->HullPoints[i].y, Convex->HullPoints[i].z);
    //     LogInfo("Normal: {%f, %f, %f}.\n", Vertices[i].Normal.x, Vertices[i].Normal.y, Vertices[i].Normal.z);
    // }

    CreateVertexBuffers(&Context->Device, Vertices, Convex->NumPoints, Convex->HullIndices, Convex->NumHullIndices,
                        &Convex->VertexBuffer, &Convex->IndexBuffer);

    // TODO: Make AddBody Thread Safe.
    shoora_body *ConvexBody = Scene->AddBody(std::move(body));
}
#endif

void
FillDiamond()
{
    shu::vec3f pts[4 + 4];
    pts[0] = shu::Vec3f( 0.1f,  0.0f, -1.0f);
    pts[1] = shu::Vec3f( 1.0f,  0.0f,  0.0f);
    pts[2] = shu::Vec3f( 1.0f,  0.0f,  0.1f);
    pts[3] = shu::Vec3f( 0.4f,  0.0f,  0.4f);

    // 22.5 degree rotation around the z-axis.
    const shu::quat quatHalf = shu::QuatAngleAxisRad(2.0f * SHU_PI_BY_2 * 0.125f, shu::Vec3f(0, 0, 1));

    pts[4] = shu::Vec3f(0.8f, 0.0f, 0.3f);
    pts[4] = shu::QuatRotateVec(quatHalf, pts[4]);
    pts[5] = shu::QuatRotateVec(quatHalf, pts[1]);
    pts[6] = shu::QuatRotateVec(quatHalf, pts[2]);

    // 45 degree rotation around the z-axis.
    const shu::quat quat = shu::QuatAngleAxisRad(2.0f * SHU_PI * 0.125f, shu::Vec3f(0, 0, 1));

    int idx = 0;
    for (int i = 0; i < 7; i++)
    {
        g_diamond[idx] = pts[i];
        idx++;
    }

    shu::quat quatAccumulator;
    for (int i = 1; i < 8; i++)
    {
        quatAccumulator = quatAccumulator * quat;
        for (int pt = 0; pt < 7; pt++)
        {
            g_diamond[idx] = shu::QuatRotateVec(quatAccumulator, pts[pt]);
            idx++;
        }
    }
}

void
AddStandardSandBox()
{
    shoora_body body = {};
    shoora_shape_cube *BoxShape = nullptr;

    // Adding ground
    body.Position = shu::Vec3f(0, 0, 0);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(g_boxGround, ARRAY_SIZE(g_boxGround));
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Green);
    Scene->AddBody(std::move(body));

    // Adding wall at (50, 0, 0)
    body.Position = shu::Vec3f(50, 0, 0);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(g_boxWall0, ARRAY_SIZE(g_boxWall0));
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Blue);
    Scene->AddBody(std::move(body));

    // Adding wall at (-50, 0, 0)
    body.Position = shu::Vec3f(-50, 0, 0);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(g_boxWall0, ARRAY_SIZE(g_boxWall0));
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Orange);
    Scene->AddBody(std::move(body));

    // Adding wall at (0, 0, 25)
    body.Position = shu::Vec3f(0, 0, 25);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(g_boxWall1, ARRAY_SIZE(g_boxWall1));
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Red);
    Scene->AddBody(std::move(body));

    // Adding wall at (0, 0, -25)
    body.Position = shu::Vec3f(0, 0, -25);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(g_boxWall1, ARRAY_SIZE(g_boxWall1));
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Yellow);
    Scene->AddBody(std::move(body));
}

#if GJK_STEPTHROUGH
void
InitializeGJKDebugTest()
{
    diamondEulerAngles = shu::Vec3f(-79.5f, -141.5f, -155.5f);

    shoora_body body = {};
    shoora_shape_cube *BoxShape = nullptr;

    // Adding ground
    body.Position = shu::Vec3f(0, 5.15f, 33.87f);
    body.Rotation = shu::Quat(1, 0, 0, 0);
    body.LinearVelocity = shu::Vec3f(0.0f);
    body.AngularVelocity = shu::Vec3f(0.0f);
    body.InvMass = 0.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    BoxShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    body.Shape = new (BoxShape) shoora_shape_cube(width, 1.0f, depth);
    body.Scale = body.Shape->GetDim();
    body.Color = GetColor(colorU32::Proto_Green);
    Scene->AddBody(std::move(body));

    Scene->AddDiamondBody(shu::Vec3f(0, 5.4f, 24.75f), shu::Vec3f(1.0f), colorU32::Proto_Orange, 0.0f, 0.5f,
                          shu::Vec3f(0.0f));
}

#include <physics/gjk.h>

void DrawDebug()
{
    shoora_body *A = &Scene->Bodies[0];
    shoora_body *B = &Scene->Bodies[1];

    B->Rotation = shu::QuatFromEuler(diamondEulerAngles.x, diamondEulerAngles.y, diamondEulerAngles.z);

    shu::mat4f ModelA = shu::TRS(A->Position, A->Scale, A->Rotation);
    shu::mat4f ModelB = shu::TRS(B->Position, B->Scale, B->Rotation);

    i32 vertexCount_A = A->Shape->MeshFilter->VertexCount;
    i32 vertexCount_B = B->Shape->MeshFilter->VertexCount;

    shu::vec3f PtA, PtB;
#if 1
    b32 intersection = GJK_DoesIntersect(A, B, 0.0f, PtA, PtB);
#else
    GJK_ClosestPoints(A, B, PtA, PtB);
#endif

    for (i32 i = 0; i < vertexCount_A; ++i)
    {
        shu::vec3f ALocalPos = A->Shape->MeshFilter->Vertices[i].Pos;
        shu::vec3f AWorldPos = (ModelA * shu::Vec4f(ALocalPos, 1.0f)).xyz;

        for (i32 j = 0; j < vertexCount_B; ++j)
        {
            shu::vec3f BLocalPos = B->Shape->MeshFilter->Vertices[j].Pos;
            shu::vec3f BWorldPos = (ModelB * shu::Vec4f(BLocalPos, 1.0f)).xyz;

            shu::vec3f AB = AWorldPos - BWorldPos;
            shoora_graphics::DrawCube(AB, colorU32::White, .075f);
        }
    }
}
#endif

void
InitScene()
{
    // Bottom Wall (Static Rigidbody)
    shu::vec2f Window = shu::Vec2f((f32)GlobalWindowSize.x, (f32)GlobalWindowSize.y);

#if BUILD_CONVEX_THREADED
#if 1
    FillDiamond();

    shoora_shape_convex *ConvexShapeMemory = ShuAllocateStruct(shoora_shape_convex, MEMTYPE_GLOBAL);

    memory_arena ConvexArena{};
    size_t ConvexMemSize = shoora_shape_convex::GetRequiredSizeForConvexBuild(ARRAY_SIZE(g_diamond));
    SubArena(&ConvexArena, MEMTYPE_GLOBAL, ConvexMemSize);
    BuildConvexThreaded(GlobalJobQueue, ConvexShapeMemory, g_diamond, ARRAY_SIZE(g_diamond), OnConvexBodyReady,
                        &ConvexArena);
    int x = 0;
#else
    shoora_body body = {};
    body.Position = shu::Vec3f(0, 0, 10);
    body.Rotation = shu::Quat(0, 0, 0, 1);
    body.LinearVelocity = shu::Vec3f(0, 0, 0);
    body.InvMass = 1.0f;
    body.Mass = 1.0f;
    body.CoeffRestitution = 0.5f;
    body.FrictionCoeff = 0.5f;
    body.Shape = new (ConvexShapeMemory) shoora_shape_convex(g_diamond, ARRAY_SIZE(g_diamond));
    body.InertiaTensor = (body.Shape->InertiaTensor() * body.Mass);
    body.InverseInertiaTensor = body.InertiaTensor.IsZero() ? shu::Mat3f(0.0f) : body.InertiaTensor.Inverse();
    body.Scale = body.Shape->GetDim();
    body.SumForces = body.Acceleration = shu::vec3f::Zero();
    body.SumTorques = 0.0f;
    // body.Shape = std::make_unique<shoora_shape_convex>(GlobalJobQueue, g_diamond, ARRAY_SIZE(g_diamond));
    Scene->AddBody(std::move(body));
#endif
#endif

#if 1
    auto *bA = Scene->AddCubeBody(Pos, shu::Vec3f(5), colorU32::Proto_Red, 0.0f, .5f, shu::Vec3f(0, 0, 0));
    auto *bB = Scene->AddCubeBody(shu::Vec3f(0, -2.5f, 0), shu::Vec3f(5), colorU32::Proto_Blue, 1.0f, .5f);

    hinge_quat_constraint_3d *HingeJoint = ShuAllocateStruct(hinge_quat_constraint_3d, MEMTYPE_GLOBAL);
    new (HingeJoint) hinge_quat_constraint_3d();

    HingeJoint->A = bA;
    HingeJoint->B = bB;
    HingeJoint->AnchorPointLS_A = shu::Vec3f(-0.5f, -0.5f, -0.5f);
    HingeJoint->AnchorPointLS_B = shu::Vec3f(-0.5f,  0.5f, -0.5f);

    // HingeJoint->q0 = shu::QuatInverse(bA->Rotation) * bB->Rotation;

    shu::vec3f RotationAxis = shu::Vec3f(1, 0, 0);
    HingeJoint->AxisA = shu::QuatRotateVec(shu::QuatInverse(bA->Rotation), RotationAxis);
    HingeJoint->AxisB = shu::QuatRotateVec(shu::QuatInverse(bB->Rotation), RotationAxis);

    // bB->LinearVelocity = shu::Vec3f(100,  0,  0);
    // bB->LinearVelocity = shu::Vec3f( 0, 10,  0);
    // bB->LinearVelocity = shu::Vec3f( 0, 0, 10);
    Scene->Constraints3D.emplace_back(HingeJoint);

#endif

#if 0
    auto *Body = Scene->AddCubeBody(shu::Vec3f(25, 10, 0), shu::Vec3f(2.0f), colorU32::Proto_Blue, 1.0f, 1.0f, shu::Vec3f(0.0f));
    Body->LinearVelocity = shu::Vec3f(-300.0f, 0, 0);

    Scene->AddCubeBody(shu::Vec3f(-5.0f, 5.0f, 0.0f), shu::Vec3f(1.0f, 50, 50), colorU32::Red, 0.0f, 1.0f);

    AddStandardSandBox();
#endif

#if 0
#if GJK_STEPTHROUGH
    InitializeGJKDebugTest();
#else
    shoora_body *diamond = Scene->AddDiamondBody(shu::Vec3f(0, 8, 10), shu::Vec3f(1.0f), colorU32::Proto_Orange,
                                                 1.0f, 0.5f, shu::Vec3f(0.0f));
    AddStandardSandBox();

    shoora_body *SphereBody = Scene->AddSphereBody(shu::Vec3f(10, 8, 10), colorU32::White, .5f, 1.0f, .5f);
    SphereBody->LinearVelocity = shu::Vec3f(-10, 0, 0);
    SphereBody->FrictionCoeff = .5f;
#endif
#endif

#if 0
    i32 NumJoints = 10;

    shu::vec3f AnchorPos = shu::Vec3f(0, 15, 5);
    shoora_body *anchor = Scene->AddCubeBody(AnchorPos, shu::Vec3f(0.5f), colorU32::White, 0.0f, .5f);
    for(i32 i = 1; i < NumJoints; ++i)
    {
        shoora_body *JointAnchorBody = Scene->GetBody(i - 1);

        // shu::vec3f Delta = shu::Vec3f((f32)i * .5f, -(f32)i * 1.5f, 0.0f);
        shu::vec3f Delta = shu::Vec3f(0, -(f32)i, 0.0f);
        shoora_body *Body = Scene->AddCubeBody(AnchorPos + Delta, shu::Vec3f(0.5f), colorU32::White, 1.0f, .5f);
        Body->LinearVelocity = shu::Vec3f(5.0f, 0.0f, 0.0f);

        shu::vec3f JointAnchorWS = JointAnchorBody->Position;

        joint_constraint_3d *Joint = ShuAllocateStruct(joint_constraint_3d, MEMTYPE_GLOBAL);
        new (Joint) joint_constraint_3d();

        Joint->A = JointAnchorBody;
        Joint->AnchorPointLS_A = Joint->A->WorldToLocalSpace(JointAnchorWS);
        Joint->B = Body;
        Joint->AnchorPointLS_B = Joint->B->WorldToLocalSpace(JointAnchorWS);
        Scene->Constraints3D.emplace_back(Joint);
    }

    Scene->AddCubeBody(shu::Vec3f(1, 10, 5), shu::Vec3f(0.5f), colorU32::Proto_Orange, 0.0f, 1.0f);

    AddStandardSandBox();
#endif

#if 0
    f32 startY = 1;
    shu::rand Rand;
    shoora_body *Body;
    i32 StackSize = 20;
    for(i32 i = 0; i < StackSize; ++i)
    {
        // shu::vec3f RandRotation = shu::Vec3f(0.0f, Rand.RangeBetweenF32(-360.0f, 360.0f), 0.0f);
        shu::vec3f RandRotation = shu::Vec3f(0.0f, 0.0f, 0.0f);
        Body = Scene->AddCubeBody(shu::Vec3f(0, startY + (f32)i, 0), shu::Vec3f(1.0f),
                                  GetDebugColor(Rand.NextUInt32()), 1.0f, .5f, RandRotation);
        Body->FrictionCoeff = .5f;
    }

    AddStandardSandBox();
#endif

#if 0
    // Dynamic Bodies
    shoora_body Body;
    for(i32 x = 0; x < 6; x++)
    {
        for(i32 y = 0; y < 6; y++)
        {
            f32 Radius = 0.5f;
            f32 xx = (f32)(x - 1) * Radius * 1.5f;
            f32 zz = (f32)(y - 1) * Radius * 1.5f;

            f32 Mass = 1.0f;
            f32 Restitution = 0.5f;

            auto *b = Scene->AddSphereBody(shu::Vec3f(xx, 100.0f, zz), colorU32::Proto_Orange, Radius, Mass, Restitution);
            b->FrictionCoeff = 0.5f;
            b->LinearVelocity = shu::Vec3f(0.0f);
        }
    }

    // Static "floor"
    for (i32 x = 0; x < 3; x++)
    {
        for (i32 y = 0; y < 3; y++)
        {
            f32 Radius = 80.0f;
            f32 xx = (f32)(x - 1) * Radius * 0.25f;
            f32 zz = (f32)(y - 1) * Radius * 0.25f;

            f32 Mass = 0.0f;
            f32 Restitution = 0.99f;

            auto *b = Scene->AddSphereBody(shu::Vec3f(xx, 10.0f, zz), colorU32::Gray, Radius, Mass, Restitution);
            b->FrictionCoeff = 0.5f;
            b->LinearVelocity = shu::Vec3f(0.0f);
        }
    }
#endif
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
                                  Context->FragUnlitBuffers[Index].Handle, 2*sizeof(shu::mat4f), 0);
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
    CreateGraphicsPipeline(Context, "shaders/spirv/checkerboard.vert.spv", "shaders/spirv/checkerboard.frag.spv",
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
    GlobalFragUniformData.PointLightData[0].Pos = shu::Vec3f(1.62f, 3.2f, -1.65f);
    GlobalFragUniformData.PointLightData[0].Color = shu::Vec3f(1.0f, 0.0f, 0.0f);
    GlobalFragUniformData.PointLightData[0].Intensity = 2.575f;

    GlobalFragUniformData.PointLightData[1].Pos = shu::Vec3f(1.615f, 0.0f, -5.105f);
    GlobalFragUniformData.PointLightData[1].Color = shu::Vec3f(0.0f, 1.0f, 0.0f);
    GlobalFragUniformData.PointLightData[1].Intensity = 3.535f;

    GlobalFragUniformData.PointLightData[2].Pos = shu::Vec3f(2.47f, 3.27f, -13.645f);
    GlobalFragUniformData.PointLightData[2].Color = shu::Vec3f(0.0f, 0.0f, 1.0f);
    GlobalFragUniformData.PointLightData[2].Intensity = 2.575f;

    GlobalFragUniformData.PointLightData[3].Pos = shu::Vec3f(-2.575f, 0.785f, -11.24f);
    GlobalFragUniformData.PointLightData[3].Color = shu::Vec3f(1.0f, 0.0f, 1.0f);
    GlobalFragUniformData.PointLightData[3].Intensity = 2.575f;

    GlobalFragUniformData.SpotlightData.InnerCutoffAngles = 10.0f;
    GlobalFragUniformData.SpotlightData.OuterCutoffAngles = 14.367f;
    GlobalFragUniformData.SpotlightData.Color = shu::Vec3f(30.0f / 255.0f, 194.0 / 255.0f, 165.0 / 255.0f);
    GlobalFragUniformData.SpotlightData.Intensity = 5.0f;
}

void
InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo)
{
    shu::vec3f v0 = shu::Vec3f(31.12f, 43.12f, 57.213f);
    shu::quat q0 = shu::Quat(1, 2, 6, 7);
    shu::QuatNormalize(q0);

    shu::vec3f v0Proj = shu::QuatRotateVec(q0, v0);

    shu::mat3f R = shu::QuatRotationMatrix_Left(q0);
    shu::vec3f v0Proj_2 = R * v0;

    platform_memory GameMemory = AppInfo->GameMemory;
    InitializeMemory(GameMemory.PermSize, GameMemory.PermMemory, GameMemory.FrameMemorySize, GameMemory.FrameMemory);

#if _SHU_DEBUG
    isDebug = true;
#endif

    GlobalWindowSize = {AppInfo->WindowWidth, AppInfo->WindowHeight};
    GlobalJobQueue = AppInfo->JobQueue;
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

    shu::vec2u ScreenDim = shu::vec2u{AppInfo->WindowWidth, AppInfo->WindowHeight};
    CreateSwapchain(&VulkanContext->Device, &VulkanContext->Swapchain, ScreenDim, &SwapchainInfo);
    CreateRenderPass(RenderDevice, Swapchain, &VulkanContext->GraphicsRenderPass);
    CreateSwapchainFramebuffers(RenderDevice, Swapchain, VulkanContext->GraphicsRenderPass);

    shoora_camera *pCamera = &VulkanContext->Camera;
    SetupCamera(pCamera, shoora_projection::PROJECTION_PERSPECTIVE, 0.1f, 1000.0f, 16.0f / 9.0f,
                GlobalWindowSize.y, 45.0f, shu::Vec3f(0, 4, -10));
    pCamera->Pos = shu::Vec3f(1.42f, 30.42, -60.19f);
    pCamera->Front = shu::Vec3f(-0.0573437512f, -0.423883229, 0.903899729f);
    pCamera->Right = shu::Vec3f(0.997993648f, 0.0f, 0.0633131042f);
    pCamera->Up = shu::Vec3f(-0.0268373638f, 0.905716836f, 0.423032761f);
    pCamera->Yaw = 93.6300049f;
    pCamera->Pitch = -25.0799961;

#if 0
    pCamera->Pos = shu::Vec3f(14.1368380f, 106.438675f, -40.9848938f);
    pCamera->Front = shu::Vec3f(-0.332412988f, -0.475319535f, -0.814599872f);
    pCamera->Right = shu::Vec3f(-0.925878108f , 0.0f, 0.377822191f);
    pCamera->Up = shu::Vec3f(-0.179586262, 0.879813194, -0.440087944f);
    pCamera->Yaw = 247.801147f;
    pCamera->Pitch = -28.3801575f;
#endif

    // NOTE: Mesh Database setup
    shoora_mesh_database::MeshDbBegin(RenderDevice);
    shu::vec3f Poly1[] = {{0, 10, 1}, {10, 3, 1}, {10, -5, 1}, {-10, -5, 1}, {-10, 3, 1}};
    shoora_mesh_database::AddPolygonMeshToDb(Poly1, ARRAY_SIZE(Poly1));
    shu::vec3f Poly2[] = {{20, 60, 1}, {40, 20, 1}, {20, -60, 1}, {-20, -60, 1}, {-40, 20, 1}};
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

    Scene = ShuAllocateStruct(shoora_scene, MEMTYPE_GLOBAL);
    shoora_scene TempScene = shoora_scene{};
    SHU_MEMCOPY(&TempScene, Scene, sizeof(shoora_scene));

    Scene->Bodies.SetAllocator(MEMTYPE_FREELISTGLOBAL);
    Scene->Constraints2D.SetAllocator(MEMTYPE_FREELISTGLOBAL);
    Scene->PenetrationConstraints2D.SetAllocator(MEMTYPE_FREELISTGLOBAL);
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
    shu::mat4f Model = GlobalMat4Identity;
    shu::Scale(Model, shu::Vec3f(1.0f, 1.0f, 1.0f));
    shu::RotateGimbalLock(Model, shu::Vec3f(1.0f, 1.0f, 1.0f), Angle*AngleSpeed);
    shu::Translate(Model, shu::Vec3f(0.0f, 0.0f, 0.0f));
    GlobalPushConstBlock.Model = Model;

    shu::mat4f View = GlobalMat4Identity;
    View = Context->Camera.GetViewMatrix(View);
    GlobalVertUniformData.View = View;

    shu::mat4f Projection = Context->Camera.GetProjectionMatrix();
    GlobalVertUniformData.Projection = Projection;

    GlobalLightShaderData.View = View;
    GlobalLightShaderData.Projection = Projection;
    memcpy(Context->FragUnlitBuffers[ImageIndex].pMapped, &GlobalLightShaderData, sizeof(light_shader_vert_data));

    GlobalFragUniformData.ObjectColor = GlobalRenderState.MeshColorUniform;
    GlobalFragUniformData.CamPos = Context->Camera.Pos;

    GlobalFragUniformData.SpotlightData.Direction = shu::Normalize(Context->Camera.Front);
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
DrawCoordinateAxes()
{
    shoora_graphics::DrawLine3D(shu::Vec3f(-1000, 0, 0), shu::Vec3f(1000, 0, 0), colorU32::Proto_Red);
    shoora_graphics::DrawLine3D(shu::Vec3f(0, -1000, 0), shu::Vec3f(0, 1000, 0), colorU32::Proto_Green);
    shoora_graphics::DrawLine3D(shu::Vec3f(0, 0, -1000), shu::Vec3f(0, 0, 1000), colorU32::Proto_Blue);
}

static i32 NumPhysicsTicks = 60;
static f32 FixedDeltaTime = 1.0f / (f32)NumPhysicsTicks;
static f32 _dt = 0.0f;
void
DrawFrameInVulkan(shoora_platform_frame_packet *FramePacket)
{
#if 0
    if(Platform_GetKeyInputState(VirtualKeyCodes::SU_LEFTMOUSEBUTTON, KeyState::SHU_KEYSTATE_PRESS))
    {
        int x = 0;
    }
#endif

    Scene->Bodies[0].Position = Pos;

    // VK_CHECK(vkQueueWaitIdle(Context->Device.GraphicsQueue));
    shu::vec2f CurrentMousePos = shu::Vec2f(FramePacket->MouseXPos, FramePacket->MouseYPos);
    shu::vec2f CurrentMouseWorldPos = MouseToWorld(CurrentMousePos);

    Context->Camera.UpdateWindowSize(shu::Vec2f(GlobalWindowSize.x, GlobalWindowSize.y));

    ASSERT(Context != nullptr);
    // ASSERT(FramePacket->DeltaTime > 0.0f);
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
#endif
    if(Platform_GetKeyInputState(SU_SPACE, KeyState::SHU_KEYSTATE_PRESS))
    {
        GlobalDebugMode = !GlobalDebugMode;
    }
    if(Platform_GetKeyInputState('P', KeyState::SHU_KEYSTATE_PRESS))
    {
        GlobalPausePhysics = !GlobalPausePhysics;
    }

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
        DrawCoordinateAxes();

#if GJK_STEPTHROUGH
        DrawDebug();
#else
        // Scene->UpdateInput(CurrentMouseWorldPos);
        f32 pDt = GlobalPausePhysics ? 0.0f : GlobalDeltaTime;
        _dt += GlobalDeltaTime;
        if(_dt >= FixedDeltaTime)
        {
            // LogInfo("FDT: %f.\n", FixedDeltaTime);
            Scene->PhysicsUpdate(FixedDeltaTime, GlobalDebugMode);
            _dt = 0.0f;
        }
#endif
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
    // delete Scene;
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