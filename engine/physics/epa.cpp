#include "epa.h"

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
    i32 Axes = 0;
    f32 MaxArea = 0.0f;
    for(i32 i = 0; i < 3; ++i)
    {
        i32 j = (i + 1) % 3;
        i32 k = (i + 2) % 3;

        shu::vec2f a = shu::Vec2f(s1[j], s1[k]);
        shu::vec2f b = shu::Vec2f(s2[j], s2[k]);
        shu::vec2f c = shu::Vec2f(s3[j], s3[k]);

        shu::vec2f ab = b - a;
        shu::vec2f ac = c - a;

        f32 Area = ab.x*ac.y - ab.y*ac.x;

        if((Area*Area) > (MaxArea*MaxArea))
        {
            MaxArea = Area;
            Axes = i;
        }
    }

    ASSERT(!NearlyEqual(MaxArea, 0.0f, 0.0001f));

    i32 x = (Axes + 1) % 3;
    i32 y = (Axes + 2) % 3;
    shu::vec2f pProj2D = shu::Vec2f(pProj[x], pProj[y]);
    shu::vec2f aProj2D = shu::Vec2f(s1[x], s1[y]);
    shu::vec2f bProj2D = shu::Vec2f(s2[x], s2[y]);
    shu::vec2f cProj2D = shu::Vec2f(s3[x], s3[y]);

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

#if _SHU_DEBUG
    ASSERT(NearlyEqual((bary.x + bary.y + bary.z), 1.0f, 0.0001f));
    auto test = ((bary.x * v1) + (bary.y * v2) + (bary.z * v3));
    ASSERT(test == Point);
#endif

    return bary;
}

shu::vec3f
NormalDirection(const tri_t &Tri, const shoora_dynamic_array<gjk_point> &Points)
{
    const shu::vec3f &A = Points[Tri.A].MinkowskiPoint;
    const shu::vec3f &B = Points[Tri.B].MinkowskiPoint;
    const shu::vec3f &C = Points[Tri.C].MinkowskiPoint;

    shu::vec3f ab = B - A;
    shu::vec3f ac = C - A;

    shu::vec3f Normal = ab.Cross(ac);
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
                DanglingEdges.emplace_back(Edges1[k]);
            }
        }
    }
}

f32
EPA_Expand(const shoora_body *A, const shoora_body *B, const f32 Bias, const gjk_point SimplexPoints[4],
           shu::vec3f &PointOnA, shu::vec3f &PointOnB)
{
    memory_arena *Arena = GetArena(shoora_memory_type::MEMTYPE_FRAME);
    ASSERT(Arena != nullptr);

    temporary_memory TempMemory = BeginTemporaryMemory(Arena);
    Arena->Log();

    freelist_allocator TempAllocator(ShuAllocate_(Arena, MEGABYTES(1)), MEGABYTES(1));

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
            break;
        }

        f32 Distance = SignedDistanceToTriangle(Triangles[ClosestTriangleIndex], NewPoint.MinkowskiPoint, Points);
        // NOTE: Cannot expand the polytope further.
        if (Distance < 0.0f)
        {
            break;
        }

        const i32 NewIdx = (i32)Points.size();
        Points.emplace_back(NewPoint);

        // NOTE: Remove Triangles that face this point.
        i32 NumRemoved = RemoveTrianglesFacingPoint(NewPoint.MinkowskiPoint, Triangles, Points);
        if(NumRemoved == 0)
        {
            break;
        }

        // NOTE: Find Dangling Edges.
        DanglingEdges.Clear();
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

            Triangles.emplace_back(Triangle);
        }
    }

    // NOTE: Get the Projection of the origin on the closest triangle.
    const i32 Idx = ClosestTriangle(Triangles, Points);
    const tri_t &Triangle = Triangles[Idx];
    shu::vec3f PtA_W = Points[Triangle.A].MinkowskiPoint;
    shu::vec3f PtB_W = Points[Triangle.B].MinkowskiPoint;
    shu::vec3f PtC_W = Points[Triangle.C].MinkowskiPoint;

    shu::vec3f OriginBaryCoords = BarycentricCoords(PtA_W, PtB_W, PtC_W, shu::Vec3f(0.0f));

    // NOTE: Get the Point on Shape A.
    shu::vec3f PtA_a = Points[Triangle.A].PointOnA;
    shu::vec3f PtB_a = Points[Triangle.B].PointOnA;
    shu::vec3f PtC_a = Points[Triangle.C].PointOnA;
    PointOnA = PtA_a*OriginBaryCoords.x + PtB_a*OriginBaryCoords.y + PtC_a*OriginBaryCoords.z;

    // NOTE: Get the Point on Shape B.
    shu::vec3f PtA_b = Points[Triangle.A].PointOnB;
    shu::vec3f PtB_b = Points[Triangle.B].PointOnB;
    shu::vec3f PtC_b = Points[Triangle.C].PointOnB;
    PointOnB = PtA_b*OriginBaryCoords.x + PtB_b*OriginBaryCoords.y + PtC_b*OriginBaryCoords.z;

    EndTemporaryMemory(TempMemory);
    Arena->Log();

    shu::vec3f Delta = PointOnB - PointOnA;
    f32 DeltaMagnitude = Delta.Magnitude();
    return DeltaMagnitude;
}