#include "epa.h"

#if EPA_DEBUG
#include <platform/platform.h>
#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif
#include <renderer/vulkan/graphics/vulkan_graphics.h>

static u32 DebugColors[] = {
     0xffff0000, // Red
     0xff00ff00, // Green
     0xff0000ff, // Blue
     0xff00ffff, // Cyan
     0xffff00ff, // Magenta
     0xffffff00, // Yellow
     0xffffffff, // White
     0xff313131, // Gray
     0xffFB8621, // Proto_Orange
     0xffFFB900, // Proto_Yellow
     0xff6DA174, // Proto_Green
     0xffBD4334, // Proto_Red
     0xff697FC4 // Proto_Blue
};

static b32 EPADebug_Initialized = false;
static epa_debug_result EPADebug_Results[256];
static i32 EPADebug_ResultCount = 0;
static i32 EPADebug_CurrentStep = 0;

void
InitializeEPADebug()
{
    SHU_MEMZERO(EPADebug_Results, sizeof(epa_debug_result) * ARRAY_SIZE(EPADebug_Results));
    EPADebug_ResultCount = 0;

    EPADebug_Initialized = true;
}

void
EPADebug_AddEntry(const shoora_dynamic_array<tri_t> &Triangles, gjk_point *Points, i32 NewPointIndex)
{
    ASSERT(EPADebug_ResultCount + 1 < ARRAY_SIZE(EPADebug_Results));

    epa_debug_result *Result = EPADebug_Results + EPADebug_ResultCount++;
    i32 TriCount = Triangles.size();
    Result->TriangleCount = TriCount;
    Result->GJKPoints = Points;
    Result->NewPointIndex = NewPointIndex;

    for(i32 i = 0; i < TriCount; ++i)
    {
        Result->Triangles[i] = Triangles[i];
    }
}

void
EPADebug_AddEntry(const tri_t &Triangle, gjk_point *Points, const shu::vec3f &Normal, i32 NewPointIndex)
{
    ASSERT(EPADebug_ResultCount + 1 < ARRAY_SIZE(EPADebug_Results));

    epa_debug_result *Result = EPADebug_Results + EPADebug_ResultCount++;

    Result->TriangleCount = 1;
    Result->GJKPoints = Points;
    Result->Triangles[0] = Triangle;
    Result->NormalDir = shu::Normalize(Normal);
    Result->NewPointIndex = NewPointIndex;
    Result->DoubleSided = true;
}

void
EPADebug_Visualize()
{
    if (Platform_GetKeyInputState(SU_ALPHABETKEYM, KeyState::SHU_KEYSTATE_PRESS))
    {
        EPADebug_CurrentStep = (EPADebug_CurrentStep + 1) % EPADebug_ResultCount;
    }
    if (Platform_GetKeyInputState(SU_ALPHABETKEYN, KeyState::SHU_KEYSTATE_PRESS))
    {
        EPADebug_CurrentStep = (EPADebug_CurrentStep > 0) ? (EPADebug_CurrentStep - 1)
                                                          : (EPADebug_ResultCount - 1);
    }

    const epa_debug_result *Curr = EPADebug_Results + EPADebug_CurrentStep;
    if(Curr->NewPointIndex != -1)
    {
        shu::vec3f NewPoint = Curr->GJKPoints[Curr->NewPointIndex].MinkowskiPoint;
        shoora_graphics::DrawCube(NewPoint, colorU32::Cyan, .2f);
    }

    if(!Curr->NormalDir.IsZero())
    {
        shu::vec3f O = shu::Vec3f(0.0f);
        shoora_graphics::DrawLine3D(O, O + Curr->NormalDir*10.0f, colorU32::Cyan);
    }

    for(i32 i = 0; i < Curr->TriangleCount; ++i)
    {
        tri_t Tri = Curr->Triangles[i];
        shu::vec3f A = Curr->GJKPoints[Tri.A].MinkowskiPoint;
        shu::vec3f B = Curr->GJKPoints[Tri.B].MinkowskiPoint;
        shu::vec3f C = Curr->GJKPoints[Tri.C].MinkowskiPoint;

        u32 Color = DebugColors[i % ARRAY_SIZE(DebugColors)];
        shoora_graphics::DrawTriangle(A, B, C, Color);

        if(Curr->DoubleSided)
        {
            shoora_graphics::DrawTriangle(A, C, B, Color);
        }
    }
}

#endif

#if 0
shu::vec3f
BarycentricCoords(const shu::vec3f &v1, const shu::vec3f &v2, const shu::vec3f &v3, const shu::vec3f &Point)
{
    shu::vec3f s1 = v1 - Point;
    shu::vec3f s2 = v2 - Point;
    shu::vec3f s3 = v3 - Point;

    shu::vec3f Normal = (s2 - s1).Cross(s3 - s1);
    // NOTE: "Point" projected on the Triangle ABC Plane.
    shu::vec3f pProj = Normal * (s1.Dot(Normal) / Normal.SqMagnitude());

    // NOTE: Find the axes with the greatest projected area.
    i32 Axes = -1;
    f32 MaxArea = 0.0f;

    f32 nx = SHU_ABSOLUTE(Normal.x);
    f32 ny = SHU_ABSOLUTE(Normal.y);
    f32 nz = SHU_ABSOLUTE(Normal.z);
    i32 a0, a1;

    // NOTE: The Highest Normal component is the axis of maximum projected triangle area.
    if((nx > ny) && (nx > nz))
    {
        Axes = 0;
        a0 = 1;
        a1 = 2;
    }
    else if((ny > nx) && (ny > nz))
    {
        Axes = 1;
        a0 = 0;
        a1 = 2;
    }
    else if((nz > nx) && (nz > ny))
    {
        Axes = 2;
        a0 = 0;
        a1 = 1;
    }

    if(Axes == -1)
    {
        return shu::Vec3f(1, 0, 0);
    }
    // ASSERT(Axes != -1);
    // ASSERT((a0 != a1) && (a0 >= 0) && (a0 <= 2) && (a1 >= 0) && (a1 <= 2));

    shu::vec2f pProj2D = shu::Vec2f(pProj[a0], pProj[a1]);
    shu::vec2f aProj2D = shu::Vec2f(s1[a0], s1[a1]);
    shu::vec2f bProj2D = shu::Vec2f(s2[a0], s2[a1]);
    shu::vec2f cProj2D = shu::Vec2f(s3[a0], s3[a1]);
    shu::vec2f abProj = bProj2D - aProj2D;
    shu::vec2f acProj = cProj2D - aProj2D;

    MaxArea = abProj.x*acProj.y - abProj.y*acProj.x; // ab X ac
    if(NearlyEqual(MaxArea, 0.0f, 0.0001f))
    {
        return shu::Vec3f(1, 0, 0);
    }

    // NOTE: Areas of projected triangles PAB, PBC, PCA.
    shu::vec2f pa = aProj2D - pProj2D;
    shu::vec2f pb = bProj2D - pProj2D;
    shu::vec2f pc = cProj2D - pProj2D;
    f32 areaPAB = pa.x*pb.y - pa.y*pb.x; // pa X pb
    f32 areaPBC = pb.x*pc.y - pb.y*pc.x; // pb X pc
    f32 areaPCA = pc.x*pa.y - pc.y*pa.x; // pc X pa
    f32 OneOverAreaABC = 1.0f / MaxArea;

    shu::vec3f bary = shu::Vec3f(areaPBC, areaPCA, areaPAB);
    bary *= OneOverAreaABC;

    // TODO: Check why this is failing in the EPA Routine!
#if 0
    ASSERT(NearlyEqual((bary.x + bary.y + bary.z), 1.0f, 0.0001f));
    auto test = ((bary.x * v1) + (bary.y * v2) + (bary.z * v3));
    ASSERT(test == Point);
#endif

    return bary;
}
#endif

shu::vec3f
NormalDirection(const tri_t &Tri, const shoora_dynamic_array<gjk_point> &Points)
{
    const shu::vec3f &A = Points[Tri.A].MinkowskiPoint;
    const shu::vec3f &B = Points[Tri.B].MinkowskiPoint;
    const shu::vec3f &C = Points[Tri.C].MinkowskiPoint;

    shu::vec3f ab = B - A;
    shu::vec3f ac = C - A;

    shu::vec3f Normal = ab.Cross(ac);
    // ASSERT(!Normal.IsZero());
    Normal = shu::Normalize(Normal);
    return Normal;
}

f32
SignedDistanceToTriangle(const tri_t &Tri, const shu::vec3f &Point, const shoora_dynamic_array<gjk_point> &Points)
{
    const shu::vec3f Normal = NormalDirection(Tri, Points);
    const shu::vec3f &A = Points[Tri.A].MinkowskiPoint;
    const shu::vec3f AToPoint = Point - A;
    const f32 Distance = AToPoint.Dot(Normal);

    return Distance;
}

i32
ClosestTriangle(const shoora_dynamic_array<tri_t> &Triangles, const shoora_dynamic_array<gjk_point> &Points)
{
    f32 MinDistanceSq = 1e10f;

    i32 idx = -1;
    for(i32 i = 0; i < Triangles.size(); ++i)
    {
        const tri_t &Triangle = Triangles[i];

        f32 Distance = SignedDistanceToTriangle(Triangle, shu::Vec3f(0.0f), Points);
        f32 DistanceSq = Distance * Distance;
        if (DistanceSq < MinDistanceSq)
        {
            idx = i;
            MinDistanceSq = DistanceSq;
        }
    }

    return idx;
}

b32
HasPoint(const shu::vec3f &W, const shoora_dynamic_array<tri_t> &Triangles,
         const shoora_dynamic_array<gjk_point> &Points)
{
    const f32 Epsilon = .001f * .001f;
    shu::vec3f Delta;

    for (i32 i = 0; i < Triangles.size(); ++i)
    {
        const tri_t &Triangle = Triangles[i];

        Delta = W - Points[Triangle.A].MinkowskiPoint;
        if(Delta.SqMagnitude() < Epsilon) { return true; }

        Delta = W - Points[Triangle.B].MinkowskiPoint;
        if(Delta.SqMagnitude() < Epsilon) { return true; }

        Delta = W - Points[Triangle.C].MinkowskiPoint;
        if(Delta.SqMagnitude() < Epsilon) { return true; }
    }

    return false;
}

i32
RemoveTrianglesFacingPoint(const shu::vec3f &Point, shoora_dynamic_array<tri_t> &Triangles,
                           const shoora_dynamic_array<gjk_point> &Points)
{
    i32 NumRemoved = 0;
    for(i32 i = 0; i < Triangles.size(); ++i)
    {
        const tri_t &Triangle = Triangles[i];
        f32 Distance = SignedDistanceToTriangle(Triangle, Point, Points);

        if(Distance > 0.0f)
        {
            // NOTE: This triangle faces the point.
            Triangles.erase(i);
            i--;
            NumRemoved++;
        }
    }

    return NumRemoved;
}

void
FindDanglingEdges(shoora_dynamic_array<edge_t> &DanglingEdges, const shoora_dynamic_array<tri_t> &Triangles)
{
    DanglingEdges.Clear();

    for(i32 i = 0; i < Triangles.size(); ++i)
    {
        const tri_t &Triangle1 = Triangles[i];

        edge_t Edges1[3];
        Edges1[0] = {.A = Triangle1.A, .B = Triangle1.B};
        Edges1[1] = {.A = Triangle1.B, .B = Triangle1.C};
        Edges1[2] = {.A = Triangle1.C, .B = Triangle1.A};
        i32 SharedTrianglesCount[3] = { 0, 0, 0 };

        for(i32 j = 0; j < Triangles.size(); ++j)
        {
            if(i == j)
            {
                continue;
            }

            const tri_t &Triangle2 = Triangles[j];

            edge_t Edges2[3];
            Edges2[0] = {.A = Triangle2.A, .B = Triangle2.B};
            Edges2[1] = {.A = Triangle2.B, .B = Triangle2.C};
            Edges2[2] = {.A = Triangle2.C, .B = Triangle2.A};

            for(i32 k = 0; k < 3; ++k)
            {
                if(Edges1[k] == Edges2[0]) { SharedTrianglesCount[k]++; }
                if(Edges1[k] == Edges2[1]) { SharedTrianglesCount[k]++; }
                if(Edges1[k] == Edges2[2]) { SharedTrianglesCount[k]++; }
            }
        }

        // NOTE: An Edge that isn't shared among atleast two triangles is said to be dangling.
        for(i32 k = 0; k < 3; ++k)
        {
            if(SharedTrianglesCount[k] == 0)
            {
                edge_t EdgeToAdd = Edges1[k];
#if _SHU_DEBUG
                EdgeToAdd.Validate();
#endif
                DanglingEdges.emplace_back(EdgeToAdd);
            }
        }
    }
}

f32
EPA_Expand(const shoora_body *A, const shoora_body *B, const f32 Bias, const gjk_point SimplexPoints[4],
           shu::vec3f &PointOnA, shu::vec3f &PointOnB)
{
#if EPA_DEBUG
    InitializeEPADebug();
#endif

    memory_arena *Arena = GetArena(shoora_memory_type::MEMTYPE_FRAME);
    ASSERT(Arena != nullptr);

    temporary_memory TempMemory = BeginTemporaryMemory(Arena);
    // TempMemory.Arena->Log();
    size_t TempMemorySize = MEGABYTES(1);
    freelist_allocator TempAllocator(ShuAllocate_(TempMemory.Arena, TempMemorySize), TempMemorySize);

    shoora_dynamic_array<gjk_point> Points(&TempAllocator, 1024);
    shoora_dynamic_array<tri_t> Triangles(&TempAllocator, 1024);
    shoora_dynamic_array<edge_t> DanglingEdges(&TempAllocator, 1024*3);

    // LogInfo("Arena Used: %zu.\n", Arena->Used);
    shu::vec3f Center{0, 0, 0};
    for (i32 i = 0; i < 4; ++i)
    {
        Points.emplace_back(SimplexPoints[i]);
        Center += SimplexPoints[i].MinkowskiPoint;
    }
    Center *= 0.25f;

    ASSERT(Points.size() == 4);

    // NOTE: Build Triangles.
    for (i32 i = 0; i < 4; ++i)
    {
        i32 j = (i + 1) % 4;
        i32 k = (i + 2) % 4;

        tri_t Triangle = {i, j, k};
        i32 UnusedPoint = (i + 3) % 4;
        f32 Distance = SignedDistanceToTriangle(Triangle, Points[UnusedPoint].MinkowskiPoint, Points);

        // NOTE: The unused point is always on the negative/inside of the triangle, make sure that the normal
        // points away.
        if(Distance > 0.0f)
        {
            SWAP(Triangle.A, Triangle.B);
        }
        Triangles.emplace_back(Triangle);
    }

#if EPA_DEBUG
    EPADebug_AddEntry(Triangles, Points.data(), -1);
#endif

    // NOTE: Expand the Simplex to find the closest face of the Minkowski Convex Hull to the origin.
    while(1)
    {
        const i32 ClosestTriangleIndex = ClosestTriangle(Triangles, Points);
        shu::vec3f Normal = NormalDirection(Triangles[ClosestTriangleIndex], Points);

        const gjk_point NewPoint = GJK_Support(A, B, Normal, Bias);

        // NOTE: If Point already exists in the polytope, then just stop because we cannot expand the polytope any
        // further.
        if(HasPoint(NewPoint.MinkowskiPoint, Triangles, Points))
        {
#if EPA_DEBUG
            LogTraceUnformatted("BREAK: HasPoint!\n");
#endif
            break;
        }

        f32 Distance = SignedDistanceToTriangle(Triangles[ClosestTriangleIndex], NewPoint.MinkowskiPoint, Points);
        // NOTE: Cannot expand the polytope further.
        if(Distance < 0.0f)
        {
#if EPA_DEBUG
            LogTraceUnformatted("BREAK: Distance is negative!\n");
#endif
            break;
        }

        const i32 NewIdx = (i32)Points.size();
        Points.emplace_back(NewPoint);

#if EPA_DEBUG
        tri_t closestTriangle = Triangles[ClosestTriangleIndex];
        EPADebug_AddEntry(closestTriangle, Points.data(), Normal, Points.size() - 1);
#endif
        // NOTE: Remove Triangles that face this point.
        i32 NumRemoved = RemoveTrianglesFacingPoint(NewPoint.MinkowskiPoint, Triangles, Points);
        if(NumRemoved == 0)
        {
#if EPA_DEBUG
            LogTraceUnformatted("BREAK: NumRemoved == 0!\n");
#endif
            break;
        }

        // NOTE: Find Dangling Edges.
        FindDanglingEdges(DanglingEdges, Triangles);

        // NOTE: In theory the edges should be in proper CCW Order. So we only need to add the point "a" in order
        // to create new triangles that face away from the origin.
        for (i32 i = 0; i < DanglingEdges.size(); ++i)
        {
            const edge_t &Edge = DanglingEdges[i];
            tri_t Triangle = {NewIdx, Edge.B, Edge.A};

            // NOTE: Make sure its oriented properly.
            f32 Distance = SignedDistanceToTriangle(Triangle, Center, Points);
            if (Distance > 0.0f)
            {
                SWAP(Triangle.B, Triangle.C);
            }

#if _SHU_DEBUG
            Triangle.Validate();
#endif
            Triangles.emplace_back(Triangle);
        }

#if EPA_DEBUG
        EPADebug_AddEntry(Triangles, Points.data(), Points.size() - 1);
#endif
    }

    // NOTE: Get the Projection of the origin on the closest triangle.
    const i32 Idx = ClosestTriangle(Triangles, Points);
    const tri_t &Triangle = Triangles[Idx];
#if EPA_DEBUG
    EPADebug_AddEntry(Triangle, Points.data(), shu::Vec3f(0.0f), -1);
#endif

    gjk_point PtA = Points[Triangle.A];
    gjk_point PtB = Points[Triangle.B];
    gjk_point PtC = Points[Triangle.C];

#if EPA_DEBUG
    EPADebug_Visualize();
#endif

    shu::vec3f PtA_W = PtA.MinkowskiPoint;
    shu::vec3f PtB_W = PtB.MinkowskiPoint;
    shu::vec3f PtC_W = PtC.MinkowskiPoint;
    shu::vec3f OriginBaryCoords = shu::BarycentricProjectionMethod(PtA_W, PtB_W, PtC_W, shu::Vec3f(0.0f));

#if EPA_DEBUG
    shu::vec3f ProjectedPoint = OriginBaryCoords.x * PtA_W + OriginBaryCoords.y * PtB_W +
                                OriginBaryCoords.z * PtC_W;
    shoora_graphics::DrawCube(ProjectedPoint, colorU32::Yellow, .2f);
#endif

    // NOTE: Get the Point on Shape A.
    shu::vec3f PtA_a = PtA.PointOnA;
    shu::vec3f PtB_a = PtB.PointOnA;
    shu::vec3f PtC_a = PtC.PointOnA;
    PointOnA = PtA_a*OriginBaryCoords.x + PtB_a*OriginBaryCoords.y + PtC_a*OriginBaryCoords.z;

    // NOTE: Get the Point on Shape B.
    shu::vec3f PtA_b = PtA.PointOnB;
    shu::vec3f PtB_b = PtB.PointOnB;
    shu::vec3f PtC_b = PtC.PointOnB;
    PointOnB = PtA_b*OriginBaryCoords.x + PtB_b*OriginBaryCoords.y + PtC_b*OriginBaryCoords.z;

    EndTemporaryMemory(TempMemory);
    // Arena->Log();

    shu::vec3f Delta = PointOnB - PointOnA;
    f32 DeltaMagnitude = Delta.Magnitude();
    return DeltaMagnitude;
}