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
shoora_scene::AddCircleBody(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            f32 InitialRotation)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_circle>(Radius),
                     InitialRotation};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddBoxBody(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                         f32 InitialRotation)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_box>(Width, Height),
                     InitialRotation};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddPolygonBody(const u32 MeshId, const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                             f32 InitialRotation, f32 Scale)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution,
                     std::make_unique<shoora_shape_polygon>(MeshId, Scale), InitialRotation};
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
shoora_scene::CheckCollisions(b32 ShowContacts)
{
    // NOTE: Check for collision with the rest of the rigidbodies present in the Scene->
    for (i32 i = 0; i < (Bodies.size() - 1); ++i)
    {
        shoora_body *A = Bodies.data() + i;
        for (i32 j = (i + 1); j < Bodies.size(); ++j)
        {
            shoora_body *B = Bodies.data() + j;
            contact Contact;
            if (collision2d::IsColliding(A, B, Contact))
            {
                // NOTE: Visualizing the Collision Contact Info.
                if (ShowContacts)
                {
                    shoora_graphics::DrawCircle(Contact.Start.xy, 3, colorU32::Cyan);
                    shoora_graphics::DrawCircle(Contact.End.xy, 3, colorU32::Green);
                    Shu::vec2f ContactNormalLineEnd = Shu::Vec2f(Contact.Start.x + Contact.Normal.x * 30.0f,
                                                                 Contact.Start.y + Contact.Normal.y * 30.0f);
                    shoora_graphics::DrawLine(Contact.Start.xy, ContactNormalLineEnd, colorU32::Yellow, 2);

                    A->IsColliding = true;
                    B->IsColliding = true;
                }

                Contact.ResolveCollision();
            }
        }
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

        Shu::vec2f WeightForce = Shu::Vec2f(0.0f, -9.8f*SHU_PIXELS_PER_METER*Body->Mass);
        Body->AddForce(WeightForce);

#if 1
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
#endif

    }

    // integrate the acceleration due to the above forces and calculate the velocity.
    for(i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        auto *b = Bodies + BodyIndex;
        b->IntegrateForces(dt);
    }

    i32 cSize = Constraints2D.size();

    // NOTE: Here is where we do the warm starting. Apply impulses first using the cached Impulse Magnitude that we
    // have calculated the previous frame and applying it first before running the iterations. This reduces the
    // number of iterations we have to do to get a realistic result.
    for (i32 i = 0; i < cSize; ++i)
    {
        auto *C = Constraints2D[i];
        C->PreSolve();
    }

    // NOTE: Solve Constraints.
    // Solve the constraints based on the velocity calculated above and solve the constraints.
    // the solver is an impulse solver, so if in case, some corrective impulse has to be applied for the
    // constraint, then this Solve function already calls it before returning. So after this call, we have the
    // final velocity for the body and it can be integrated to get the position of the body.
    for (i32 iter = 0; iter < 5; ++iter)
    {
        for (i32 i = 0; i < cSize; ++i)
        {
            auto *C = Constraints2D[i];
            C->Solve();
        }
    }

    for (i32 i = 0; i < cSize; ++i)
    {
        auto *C = Constraints2D[i];
        C->PostSolve();
    }

    // Integrate the velocities to get the final position for the body.
    for(i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        auto *b = Bodies + BodyIndex;
        b->IntegrateVelocities(dt);
    }

    CheckCollisions(ShowContacts);
}

void
shoora_scene::Draw(b32 Wireframe)
{
    for(i32 jIndex = 0; jIndex < Constraints2D.size(); ++jIndex)
    {
        auto *c = Constraints2D[jIndex];
        Shu::vec2f pA = c->A->LocalToWorldSpace(c->AnchorPointLS_A);
        Shu::vec2f pB = c->B->LocalToWorldSpace(c->AnchorPointLS_A);
        shoora_graphics::DrawLine(pA, pB, colorU32::White, 1.0f);
    }

    for (u32 BodyIndex = 0; BodyIndex < Bodies.size(); ++BodyIndex)
    {
        shoora_body *Body = Bodies.data() + BodyIndex;
        auto *BodyShape = Body->Shape.get();

        u32 ColorU32 = Body->IsColliding ? colorU32::Red : colorU32::Green;
        // Shu::vec3f Color = GetColor(ColorU32);
        Shu::vec3f Color = Body->Color;

        Shu::mat4f Model = Shu::TRS(Body->Position, Body->Scale, Body->RotationRadians * RAD_TO_DEG,
                                    Shu::Vec3f(0, 0, 1));
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
