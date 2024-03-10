#include "vulkan_scene.h"

#include <utils/utils.h>
#include <physics/collision.h>
#include <physics/contact.h>
#include <renderer/vulkan/graphics/vulkan_graphics.h>

#include <memory/memory.h>

#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif

static b32 MouseTracking = false;
static shu::vec2f MouseInitialDownPos = shu::Vec2f(0);
static shoora_body *BodyToMove = nullptr;

struct scene_shader_data
{
    shu::mat4f Mat;
    shu::vec3f Col = {1, 1, 1};
};

shoora_scene::shoora_scene()
{
    Bodies.SetAllocator(MEMTYPE_FREELISTGLOBAL);
    Constraints2D.SetAllocator(MEMTYPE_FREELISTGLOBAL);
    PenetrationConstraints2D.SetAllocator(MEMTYPE_FREELISTGLOBAL);
}

shoora_scene::~shoora_scene()
{
    LogWarnUnformatted("shoora scene destructor called!\n");

#if 0
    size_t constraintsCount = Constraints2D.size();
    for(size_t i = 0; i < constraintsCount; ++i)
    {
        auto *C = Constraints2D[i];
        delete C;
    }
#endif
}

void
shoora_scene::AddMeshToScene(const shu::vec3f *vPositions, u32 vCount)
{
}

#if 0
shoora_body *
shoora_scene::AddBody(const shoora_body &Body)
{
    Bodies.emplace_back(std::move(Body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}
#endif

shoora_body *
shoora_scene::AddBody(shoora_body &&Body)
{
    // ASSERT(!"Not tested!");
    Bodies.emplace_back((shoora_body &&)Body);

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddCubeBody(const shu::vec3f &Pos, const shu::vec3f &Scale, u32 ColorU32, f32 Mass,
                          f32 Restitution, const shu::vec3f &EulerAngles)
{
    shoora_shape_cube *CubeShape = ShuAllocateStruct(shoora_shape_cube, MEMTYPE_GLOBAL);
    auto shape = shoora_shape_cube(Scale.x, Scale.y, Scale.z);
    SHU_MEMCOPY(&shape, CubeShape, sizeof(shoora_shape_cube));

    shoora_body Body{GetColor(ColorU32), Pos, Mass, Restitution, CubeShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddSphereBody(const shu::vec3f &Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            const shu::vec3f &EulerAngles)
{
    shoora_shape_sphere *SphereShape = ShuAllocateStruct(shoora_shape_sphere, MEMTYPE_GLOBAL);
    shoora_shape_sphere shape = shoora_shape_sphere(Radius);
    SHU_MEMCOPY(&shape, SphereShape, sizeof(shoora_shape_sphere));

    shoora_body Body{GetColor(ColorU32), Pos, Mass, Restitution, SphereShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddCircleBody(const shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            const shu::vec3f &EulerAngles)
{
    shoora_shape_circle *CircleShape = ShuAllocateStruct(shoora_shape_circle, MEMTYPE_GLOBAL);
    auto shape = shoora_shape_circle(Radius);
    SHU_MEMCOPY(&shape, CircleShape, sizeof(shoora_shape_circle));

    shoora_body Body{GetColor(ColorU32), shu::Vec3f(Pos, 1.0f), Mass, Restitution, CircleShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddBoxBody(const shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                         const shu::vec3f &EulerAngles)
{
    shoora_shape_box *BoxShape = ShuAllocateStruct(shoora_shape_box, MEMTYPE_GLOBAL);
    auto shape = shoora_shape_box(Width, Height);
    SHU_MEMCOPY(&shape, BoxShape, sizeof(shoora_shape_box));

    shoora_body Body{GetColor(ColorU32), shu::Vec3f(Pos, 1.0f), Mass, Restitution, BoxShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddPolygonBody(const u32 MeshId, const shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                             const shu::vec3f &EulerAngles, f32 Scale)
{
    shoora_shape_polygon *PolygonShape = ShuAllocateStruct(shoora_shape_polygon, MEMTYPE_GLOBAL);
    auto shape = shoora_shape_polygon(MeshId, Scale);
    SHU_MEMCOPY(&shape, PolygonShape, sizeof(shoora_shape_polygon));

    shoora_body body{GetColor(ColorU32), shu::Vec3f(Pos, 1.0f), Mass, Restitution, PolygonShape, EulerAngles};
    Bodies.emplace_back(std::move(body));

    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

void
shoora_scene::UpdateInput(const shu::vec2f &CurrentMouseWorldPos)
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
                    MouseInitialDownPos = shu::ToVec2(Body->Position);
                    BodyToMove = Body;
                }
            }
            else if (BodyToMove != nullptr)
            {
                BodyToMove->Position = shu::Vec3f(CurrentMouseWorldPos, BodyToMove->Position.z);
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
shoora_scene::PhysicsUpdate(f32 dt, b32 DebugMode)
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

        shu::vec3f WeightForce = shu::Vec3f(0.0f, -9.8f*Body->Mass, 0.0f);
        Body->AddForce(WeightForce);
    }

    // integrate the acceleration due to the above forces and calculate the velocity.
    for (i32 BodyIndex = 0; BodyIndex < BodyCount; ++BodyIndex)
    {
        auto *b = Bodies + BodyIndex;
        b->IntegrateForces(dt);
    }

    // Broadphase
    memory_arena *FrameArena = GetArena(MEMTYPE_FRAME);
    temporary_memory MemoryFlush = BeginTemporaryMemory(FrameArena);
    collision_pair *CollisionPairs = ShuAllocateArray(collision_pair, BodyCount * BodyCount, MEMTYPE_FRAME);
    SHU_MEMZERO(CollisionPairs, BodyCount * BodyCount * sizeof(collision_pair));
    i32 FinalPairsCount = 0;

    // shoora_dynamic_array<collision_pair> CollisionPairs{MEMTYPE_FREELISTGLOBAL};
    // CollisionPairs.reserve(BodyCount*BodyCount);
    broad_phase::BroadPhase(Bodies, BodyCount, CollisionPairs, FinalPairsCount, dt);

    i32 NumContacts = 0;
    const int MaxContacts = BodyCount * BodyCount;

    size_t contactsMemSize = sizeof(contact) * FinalPairsCount;
    contact *Contacts = (contact *)_alloca(contactsMemSize);
    memset(Contacts, 0, contactsMemSize);

    for(i32 i = 0; i < FinalPairsCount; ++i)
    {
        const collision_pair &Pair = CollisionPairs[i];
        shoora_body *BodyA = &Bodies[Pair.A];
        shoora_body *BodyB = &Bodies[Pair.B];

        if(BodyA->IsStatic() && BodyB->IsStatic()) { continue; }

        contact _Contacts[MaxContactCountPerPair];
        i32 ContactCount = 0;
        if(collision::IsColliding(BodyA, BodyB, dt, _Contacts, ContactCount))
        {
            ASSERT(ContactCount > 0 && ContactCount <= MaxContactCountPerPair);
            for(i32 i = 0; i < ContactCount; ++i)
            {
                // TODO)): If this assert gets hit, maybe change MaxContacts to include MaxContactCountPerPair // there.
                ASSERT(NumContacts != (MaxContacts-1));
                Contacts[NumContacts++] = _Contacts[i];
                if (DebugMode)
                {
                    shoora_graphics::DrawSphere(Contacts[i].ReferenceHitPointA, .1f, colorU32::Cyan);
                    shoora_graphics::DrawSphere(Contacts[i].IncidentHitPointB, .1f, colorU32::Green);
                }
            }
        }
    }

    // NOTE: Sort the timeofImpacts from earliest to latest.
    if(NumContacts > 1)
    {
        QuicksortRecursive(Contacts, 0, NumContacts, CompareContacts);
    }

    // NOTE: This is where we breakup the update routine for resolving the contacts.
    // The toi's have been sorted above from earliest to latest. So the first contact in "Contacts" will have the
    // shortest toi out of all of them. Whatever it is, we advance the bodies by that time(which is really the
    // fraction of the current frame's deltaTime). so we advance all bodies by the first contact's toi. add that to
    // the accumulatedTime. The next iteration, we have the next bigger toi. Since we already moved by the first
    // toi, we subtract the first toi from the second toi and advance all the bodies in the contacts list by
    // this difference amount. In the end, if there was "TimeRemaining" we advance all the bodies by that amount.
    // IMPORTANT: NOTE: Why are we doing this?
    // let's say there are three bodies that are colliding - A, B, C. A-B contact has the least toi, so it advances
    // by that time. Since A and B collided, their post-collision velocities were such that B now should collide
    // with C, which originally was colliding with A. We can potentially call IsColliding again to generate new
    // contacts, but we are not doing that since that will be very expensive. But we atleast do the first collision
    // handling correctly using Continuous Collision Detection data that we did earlier by only advancing the frame
    // by toi instead of advancing by its Full DeltaTime which is in the dt variable passed to this function.
    f32 AccumulatedTime = 0.0f;
    for (i32 i = 0; i < NumContacts; ++i)
    {
        contact &Contact = Contacts[i];
        const f32 local_dt = Contact.TimeOfImpact - AccumulatedTime;

        shoora_body *BodyA = Contact.ReferenceBodyA;
        shoora_body *BodyB = Contact.IncidentBodyB;

        if(BodyA->IsStatic() && BodyB->IsStatic()) { continue; }

        // Velocity + Position Update.
        for (i32 j = 0; j < BodyCount; ++j)
        {
            auto *b = Bodies + j;
            b->Update(local_dt);
        }

        Contact.ResolveCollision();
        AccumulatedTime += local_dt;
    }

    // NOTE: Making sure that we advance all the bodies by the full dt time passed in here. They have already
    // advanced by the "AccumulatedTime" amount, doing the remaining time here.
    const f32 TimeRemaining = dt - AccumulatedTime;
    if(TimeRemaining > 0.0f)
    {
        for (i32 j = 0; j < BodyCount; ++j)
        {
            auto *b = Bodies + j;
            b->Update(TimeRemaining);
        }
    }

    if(DebugMode)
    {
        for (i32 i = 0; i < BodyCount; ++i)
        {
            auto *b = Bodies + i;
            auto bounds = b->Shape->GetBounds(b->Position, b->Rotation);
            bounds.Draw();
        }
    }

    EndTemporaryMemory(MemoryFlush);
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

        shoora_shape *BodyShape = Body->Shape;

        u32 ColorU32 = Body->IsColliding ? colorU32::Red : colorU32::Green;
        // Shu::vec3f Color = GetColor(ColorU32);
        shu::vec3f Color = Body->Color;

        shu::mat4f Model = shu::TRS(Body->Position, Body->Scale, Body->Rotation);
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
        auto pos = c->A->LocalToWorldSpace(shu::Vec3f(c->AnchorPointLS_A));
        shoora_graphics::DrawCircle(pos.xy, 5, colorU32::Red);
    }
}

void
shoora_scene::DrawAxes(shu::rect2d &Rect)
{
    auto left = shu::Vec2f(Rect.x - (Rect.width / 2), Rect.y);
    auto right = shu::Vec2f(Rect.x + (Rect.width / 2), Rect.y);
    shoora_graphics::DrawLine2D(left, right, 0xff313131, 1.0f);
    auto top = shu::Vec2f(Rect.x, Rect.y + (Rect.height / 2));
    auto bottom = shu::Vec2f(Rect.x, Rect.y - (Rect.height / 2));
    shoora_graphics::DrawLine2D(top, bottom, 0xff313131, 1.0f);
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
