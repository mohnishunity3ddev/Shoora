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

    Constraints3D.SetAllocator(MEMTYPE_FREELISTGLOBAL);

    Bodies.reserve(32);
    Constraints3D.reserve(32);

    Manifolds.Manifolds.SetAllocator(MEMTYPE_FREELISTGLOBAL);
    Manifolds.Manifolds.reserve(128);
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

// TODO: Make AddBody Thread Safe.
shoora_body *
shoora_scene::AddBody(shoora_body &&Body)
{
    // ASSERT(!"Not tested!");
    Bodies.emplace_back((shoora_body &&)Body);

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
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

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddSphereBody(const shu::vec3f &Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                            const shu::vec3f &EulerAngles)
{
    shoora_shape_sphere *SphereShape = ShuAllocateStruct(shoora_shape_sphere, MEMTYPE_GLOBAL);
    new (SphereShape) shoora_shape_sphere(Radius);

    shoora_body Body{GetColor(ColorU32), Pos, Mass, Restitution, SphereShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
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

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

shoora_body *
shoora_scene::AddDiamondBody(const shu::vec3f &Pos, const shu::vec3f &Scale, u32 ColorU32, f32 Mass,
                             f32 Restitution, const shu::vec3f &EulerAngles)
{
    shoora_shape_convex *DiamondShape = ShuAllocateStruct(shoora_shape_convex, MEMTYPE_GLOBAL);
    new (DiamondShape) shoora_shape_convex();
    DiamondShape->SetCenterOfMass(shu::Vec3f(0.0f, 0.0f, -0.082f));
    DiamondShape->mInertiaTensor = shu::Mat3f(0.2484f, 0.0f, 0.0f, 0.0f, 0.248386f, 0.0f, 0.0f, 0.0f, 0.341404f);
    shu::vec3f DiamondBounds_Min = shu::Vec3f(-1.0f, -1.0f, -1.0f);
    shu::vec3f DiamondBounds_Max = shu::Vec3f(1.0f, 1.0f, 0.4f);
    shoora_bounds DiamondBounds = shoora_bounds(DiamondBounds_Min, DiamondBounds_Max);
    DiamondShape->Scale = Scale;
    DiamondShape->Type = shoora_mesh_type::CONVEX_DIAMOND;
    DiamondShape->MeshFilter = shoora_mesh_database::GetMeshFilter(CONVEX_DIAMOND);

    DiamondShape->Points = ShuAllocateArray(shu::vec3f, DiamondShape->MeshFilter->VertexCount, MEMTYPE_GLOBAL);
    for (i32 i = 0; i < DiamondShape->MeshFilter->VertexCount; ++i)
    {
        DiamondShape->Points[i] = DiamondShape->MeshFilter->Vertices[i].Pos;
    }
    DiamondShape->NumPoints = DiamondShape->MeshFilter->VertexCount;
    DiamondShape->HullIndices = DiamondShape->MeshFilter->Indices;
    DiamondShape->NumHullIndices = DiamondShape->MeshFilter->IndexCount;

    shoora_body Body{GetColor(ColorU32), Pos, Mass, Restitution, DiamondShape, EulerAngles};
    Bodies.emplace_back(std::move(Body));

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
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

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
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

    // TODO: if the body array gets moved to a new area of memory, whoever received this pointer will have old
    // data.
    shoora_body *b = Bodies.get(Bodies.size() - 1);
    return b;
}

#if 0
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
#endif

void
shoora_scene::PhysicsUpdate(f32 dt, b32 DebugMode)
{
    // NOTE: If I am debugging, the frametime is going to be huge. So hence, clamping here.
#if _SHU_DEBUG
    if(dt > (1.0f/29.0f))
    {
        dt = (1.0f / 29.0f);
    }
#endif
    Manifolds.RemoveExpired();

    i32 BodyCount = GetBodyCount();
    auto *Bodies = GetBodies();
    ASSERT(Bodies != nullptr);

#if 0
    auto *diamond = Bodies;
    auto startPos = diamond->Position;
    shu::vec3f endPos, deltaPos;
#endif
    memory_arena *FrameArena = GetArena(MEMTYPE_FRAME);
    temporary_memory MemoryFlush = BeginTemporaryMemory(FrameArena);

    // TODO: Make this array Dynamic.
    penetration_constraint_3d PenetrationConstraints3D[32];
    i32 PenetrationConstraintCount = 0;

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

        // TODO: No need for MaxContactCounts here since Contact Manifolds have been added.
        contact _Contacts[MaxContactCountPerPair];
        i32 ContactCount = 0;
        if(collision::IsColliding(BodyA, BodyB, dt, _Contacts, ContactCount))
        {
            // ASSERT(ContactCount > 0 && ContactCount <= MaxContactCountPerPair);
            ASSERT(ContactCount == 1);
            // TODO)): If this assert gets hit, maybe change MaxContacts to include MaxContactCountPerPair there.
            // ASSERT(NumContacts != (MaxContacts - 1));
            const contact &Contact = _Contacts[0];
            if (Contact.TimeOfImpact == 0.0f)
            {
                // NOTE: Static contact
#if 0
                penetration_constraint_3d PenConstraint;
                PenConstraint.A = BodyA;
                PenConstraint.B = BodyB;

                PenConstraint.AnchorPointLS_A = Contact.ReferenceHitPointA_LocalSpace;
                PenConstraint.AnchorPointLS_B = Contact.IncidentHitPointB_LocalSpace;

                // NOTE: Normal in body A's local space.
                shu::vec3f Normal = shu::QuatRotateVec(shu::QuatConjugate(PenConstraint.A->Rotation),
                                                       -Contact.Normal);
                PenConstraint.Normal_LocalSpaceA = shu::Normalize(Normal);

                ASSERT(PenetrationConstraintCount <= 30);
                PenetrationConstraints3D[PenetrationConstraintCount++] = PenConstraint;
#endif
                Manifolds.AddContact(Contact);
            }
            else
            {
                // NOTE: Intra-Frame Contact.
                Contacts[NumContacts++] = Contact;
            }
            if (DebugMode)
            {
                shoora_graphics::DrawSphere(Contacts[i].ReferenceHitPointA, .1f, colorU32::Cyan);
                shoora_graphics::DrawSphere(Contacts[i].IncidentHitPointB, .1f, colorU32::Green);
            }
        }
    }

    // NOTE: Sort the timeofImpacts from earliest to latest.
    if(NumContacts > 1)
    {
        QuicksortRecursive(Contacts, 0, NumContacts, CompareContacts);
    }

    // NOTE: Solve Constraints
    i32 NumConstraints = this->Constraints3D.size();
    for (i32 i = 0; i < NumConstraints; ++i)
    {
        this->Constraints3D[i]->PreSolve(dt);
    }

#if 0
    for (i32 i = 0; i < PenetrationConstraintCount; ++i)
    {
        PenetrationConstraints3D[i].PreSolve(dt);
    }
#endif
    this->Manifolds.PreSolve(dt);

    const i32 NumIterations = 5;
    for(i32 i = 0; i < NumIterations; ++i)
    {
        for(i32 j = 0; j < NumConstraints; ++j)
        {
            this->Constraints3D[j]->Solve();
        }
#if 0
        for (i32 i = 0; i < PenetrationConstraintCount; ++i)
        {
            PenetrationConstraints3D[i].Solve();
        }
#endif
        this->Manifolds.Solve();
    }

    for (i32 i = 0; i < NumConstraints; ++i)
    {
        this->Constraints3D[i]->PostSolve();
    }
#if 0
    for (i32 i = 0; i < PenetrationConstraintCount; ++i)
    {
        PenetrationConstraints3D[i].PostSolve();
    }
#endif
    this->Manifolds.PostSolve();

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
#if 0
            if(b == diamond)
            {
                endPos = diamond->Position;
                deltaPos = endPos - startPos;
                LogInfo("1. delta Magnitude: %f.\n", deltaPos.SqMagnitude());
            }
#endif
        }

        Contact.ResolveCollision();
#if 0
        if (Contact.ReferenceBodyA == diamond || Contact.IncidentBodyB == diamond)
        {
            endPos = diamond->Position;
            deltaPos = endPos - startPos;
            LogFatal("2. delta Magnitude: %f.\n", deltaPos.SqMagnitude());
        }
#endif
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
#if 0
            if (b == diamond)
            {
                endPos = diamond->Position;
                deltaPos = endPos - startPos;
                LogError("3. delta Magnitude: %f.\n", deltaPos.SqMagnitude());
            }
#endif
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

#if 0
    endPos = diamond->Position;
    deltaPos = endPos - startPos;
    LogWarn("startPos: {%f, %f, %f} || endPos: {%f, %f, %f} || delta: %f || Linear Speed: %f || Angular Speed: %f "
            "|| frameCount: %d.\n",
            startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z, deltaPos.SqMagnitude(),
            diamond->LinearVelocity.SqMagnitude(), diamond->AngularVelocity.SqMagnitude(), frameCount);
#endif

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

#if 0
    for (i32 cIndex = 0; cIndex < Constraints2D.size(); ++cIndex)
    {
        auto *c = Constraints2D[cIndex];
        auto pos = c->A->LocalToWorldSpace(shu::Vec3f(c->AnchorPointLS_A));
        shoora_graphics::DrawCircle(pos.xy, 5, colorU32::Red);
    }
#endif
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
