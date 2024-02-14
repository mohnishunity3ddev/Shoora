#include "vulkan_scene.h"

#include <utils/utils.h>
#include <physics/collision.h>
#include <physics/contact.h>
#include <renderer/vulkan/graphics/vulkan_graphics.h>

#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif

static b32 MouseTracking = false;
static Shu::vec2f MouseInitialDownPos = Shu::Vec2f(0);
static shoora_body *BodyToMove = nullptr;

struct scene_shader_data
{
    Shu::mat4f Mat;
    Shu::vec3f Col = {1, 1, 1};
};

shoora_scene::shoora_scene()
{
    Bodies.reserve(32);
    Constraints2D.reserve(64);
    PenetrationConstraints2D.reserve(64);
}

shoora_scene::~shoora_scene()
{
    LogWarnUnformatted("shoora scene destructor called!\n");

    size_t constraintsCount = Constraints2D.size();
    for(size_t i = 0; i < constraintsCount; ++i)
    {
        auto *C = Constraints2D[i];
        delete C;
    }
}

void
shoora_scene::AddMeshToScene(const Shu::vec3f *vPositions, u32 vCount)
{
}

shoora_body *
shoora_scene::AddCubeBody(const Shu::vec3f &Pos, const Shu::vec3f &Scale, u32 ColorU32, f32 Mass,
                          f32 Restitution, const Shu::vec3f &EulerAngles)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution,
                     std::make_unique<shoora_shape_cube>(Scale.x, Scale.y, Scale.z), EulerAngles};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddSphereBody(const Shu::vec3f &Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            const Shu::vec3f &EulerAngles)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_sphere>(Radius),
                     EulerAngles};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddCircleBody(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            const Shu::vec3f &EulerAngles)
{
    shoora_body body{GetColor(ColorU32), Shu::Vec3f(Pos, 1.0f), Mass, Restitution,
                     std::make_unique<shoora_shape_circle>(Radius), EulerAngles};
    Bodies.emplace_back(std::move(body));
    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddBoxBody(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                         const Shu::vec3f &EulerAngles)
{
    shoora_body body{GetColor(ColorU32), Shu::Vec3f(Pos, 1.0f), Mass, Restitution,
                     std::make_unique<shoora_shape_box>(Width, Height), EulerAngles};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddPolygonBody(const u32 MeshId, const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                             const Shu::vec3f &EulerAngles, f32 Scale)
{
    shoora_body body{GetColor(ColorU32), Shu::Vec3f(Pos, 1.0f), Mass, Restitution,
                     std::make_unique<shoora_shape_polygon>(MeshId, Scale), EulerAngles};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

void
shoora_scene::UpdateInput(const Shu::vec2f &CurrentMouseWorldPos)
{
    b32 LmbDown = Platform_GetKeyInputState(SU_LEFTMOUSEBUTTON, SHU_KEYSTATE_DOWN);

    for (u32 BodyIndex = 0; BodyIndex < Bodies.size(); ++BodyIndex)
    {
        shoora_body *Body = Bodies.data() + BodyIndex;

        if (LmbDown)
        {
            if (!MouseTracking)
            {
                if (Body->CheckIfClicked(CurrentMouseWorldPos))
                {
                    MouseTracking = true;
                    MouseInitialDownPos = Shu::ToVec2(Body->Position);
                    BodyToMove = Body;
                }
            }
            else if (BodyToMove != nullptr)
            {
                BodyToMove->Position = Shu::Vec3f(CurrentMouseWorldPos, BodyToMove->Position.z);
                BodyToMove->UpdateWorldVertices();
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
        MouseTracking = false;
        BodyToMove = nullptr;
    }
}

void
shoora_scene::PhysicsUpdate(f32 dt, b32 ShowContacts)
{
    // NOTE: If I am debugging, the frametime is going to be huge. So hence, clamping here.
#if _SHU_DEBUG
    if (dt > (1.0f/29.0f))
        dt = (1.0f/29.0f);
#endif

    i32 BodyCount = GetBodyCount();
    auto *Bodies = GetBodies();
    ASSERT(Bodies != nullptr);

    // Sum all the external forces to the body
    for(i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        ASSERT(BodyIndex < BodyCount);
        shoora_body *Body = Bodies + BodyIndex;

        Shu::vec3f WeightForce = Shu::Vec3f(0.0f, -9.8f*Body->Mass, 0.0f);
        Body->AddForce(WeightForce);
    }

    // integrate the acceleration due to the above forces and calculate the velocity.
    for (i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        auto *b = Bodies + BodyIndex;
        b->IntegrateForces(dt);
    }

    for(i32 i = 0; i < BodyCount; ++i)
    {
        for(i32 j = i+1; j < BodyCount; ++j)
        {
            shoora_body *BodyA = Bodies + i;
            shoora_body *BodyB = Bodies + j;

            contact Contacts[4];
            arr<contact> ContactsArr{Contacts, ARRAY_SIZE(Contacts)};
            if(collision::IsColliding(BodyA, BodyB, ContactsArr))
            {
                for(i32 cI = 0; cI < ContactsArr.size; ++cI)
                {
                    contact *C = ContactsArr.data + cI;

                    C->ResolveCollision();

                    if(ShowContacts)
                    {
                        shoora_graphics::DrawSphere(C->Start, .1f, colorU32::Cyan);
                        shoora_graphics::DrawSphere(C->End, .1f, colorU32::Green);
                    }
                }
            }
        }
    }

    // Integrate the velocities to get the final position for the body.
    for(i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        auto *b = Bodies + BodyIndex;
        b->IntegrateVelocities(dt);
    }
}

void
shoora_scene::Draw(b32 Wireframe)
{
#if 0
    // string connecting the anchor to the ragdoll head
    const shoora_body *Anchor = Bodies.data();
    Shu::vec2f pA = Anchor->Position.xy;
    Shu::vec2f pB = Bodies[1].Position.xy;
    shoora_graphics::DrawLine(pA, pB, colorU32::White, 1.0f);
    for (u32 BodyIndex = 1; BodyIndex < Bodies.size(); ++BodyIndex)
#endif

    for (u32 BodyIndex = 0; BodyIndex < Bodies.size(); ++BodyIndex)
    {
        shoora_body *Body = Bodies.data() + BodyIndex;
        auto *BodyShape = Body->Shape.get();

        u32 ColorU32 = Body->IsColliding ? colorU32::Red : colorU32::Green;
        // Shu::vec3f Color = GetColor(ColorU32);
        Shu::vec3f Color = Body->Color;

        Shu::mat4f Model = Shu::TRS(Body->Position, Body->Scale, Body->Rotation);
        scene_shader_data Value = {.Mat = Model, .Col = Color};

#if 1
        if(!Wireframe)
        {
            vkCmdPushConstants(shoora_graphics::GetCmdBuffer(), shoora_graphics::GetPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(scene_shader_data), &Value);
            Body->Draw();
        }
        else
        {
            // Body->DrawWireframe(Model, 2.5f, ColorU32);
            Body->DrawWireframe(Model, 2.5f, GetColorU32(Body->Color));
        }
#else
        vkCmdPushConstants(shoora_graphics::GetCmdBuffer(), shoora_graphics::GetPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(scene_shader_data), &Value);
        Body->Draw();
        Body->DrawWireframe(Model, 1.5f, 0xffffffff);
#endif
    }

    for (i32 cIndex = 0; cIndex < Constraints2D.size(); ++cIndex)
    {
        auto *c = Constraints2D[cIndex];
        auto pos = c->A->LocalToWorldSpace(Shu::Vec3f(c->AnchorPointLS_A));
        shoora_graphics::DrawCircle(pos.xy, 5, colorU32::Red);
    }
}

void
shoora_scene::DrawAxes(Shu::rect2d &Rect)
{
    auto left = Shu::Vec2f(Rect.x - (Rect.width / 2), Rect.y);
    auto right = Shu::Vec2f(Rect.x + (Rect.width / 2), Rect.y);
    shoora_graphics::DrawLine(left, right, 0xff313131, 1.0f);
    auto top = Shu::Vec2f(Rect.x, Rect.y + (Rect.height / 2));
    auto bottom = Shu::Vec2f(Rect.x, Rect.y - (Rect.height / 2));
    shoora_graphics::DrawLine(top, bottom, 0xff313131, 1.0f);
}

void
shoora_scene::AddConstraint2D(constraint_2d *Constraint)
{
    Constraints2D.push_back(Constraint);
}

i32
shoora_scene::GetConstraints2DCount()
{
    return Constraints2D.size();
}

// constraint_2d **
// shoora_scene::GetConstraints2D()
// {
//     return Constraints2D.data();
// }

i32
shoora_scene::GetBodyCount()
{
    return Bodies.size();
}

shoora_body *
shoora_scene::GetBodies()
{
    return Bodies.data();
}

shoora_body *
shoora_scene::GetBody(i32 Index)
{
    return Bodies.get(Index);
}
