#include "shape.h"

shoora_shape_convex::shoora_shape_convex(const shu::vec3f *Points, const i32 Num)
    : shoora_shape(shoora_mesh_type::CONVEX)
{
    this->Build(Points, Num);
}

struct build_work_data
{
    shoora_shape_convex *ConvexShape;
    shu::vec3f *Points;
    i32 Num;
};

PLATFORM_WORK_QUEUE_CALLBACK(BuildWork)
{
    build_work_data *Work = (build_work_data *)Args;
    Work->ConvexShape->Build(Work->Points, Work->Num);
    delete Work;
    LogInfoUnformatted("Convex Shape Built!!\n");
}

shoora_shape_convex::shoora_shape_convex(platform_work_queue *Queue, const shu::vec3f *Points, const i32 Num)
    : shoora_shape(shoora_mesh_type::CONVEX)
{
    build_work_data *Args = new build_work_data();
    Args->ConvexShape = this;
    Args->Points = (shu::vec3f *)Points;
    Args->Num = Num;

    Platform_AddWorkEntry(Queue, BuildWork, Args);
}

shoora_shape_convex::~shoora_shape_convex()
{
    LogInfoUnformatted("Destructor for convex hull called!\n");
}

void
shoora_shape_convex::Build(const shu::vec3f *Points, const i32 Num)
{
    this->mPoints.Clear();
    ASSERT(this->mPoints.capacity() == 0);
    this->mPoints.reserve(Num);

    for (i32 i = 0; i < Num; ++i)
    {
        this->mPoints.emplace_back(Points[i]);
    }

    shu::vec3f *HullPointsArr = (shu::vec3f *) _alloca(sizeof(shu::vec3f) * Num);
    stack_array<shu::vec3f> HullPoints(HullPointsArr, Num);
    tri_t *HullTrianglesArr = (tri_t *) _alloca(sizeof(tri_t) * Num * 3);
    stack_array<tri_t> HullTriangles(HullTrianglesArr, Num*3);
    BuildConvexHull(this->mPoints, HullPoints, HullTriangles);

    for (i32 i = 0; i < HullPoints.size; ++i)
    {
        this->mPoints[i] = HullPoints.data[i];
    }

    // Expand the bounds
    mBounds.Clear();
    mBounds.Expand(mPoints.data(), mPoints.size());

    mCenterOfMass = CalculateCenterOfMass(HullPoints, HullTriangles);
    mInertiaTensor = CalculateInertiaTensor(HullPoints, HullTriangles);
    int x = 0;
}

shu::mat3f
shoora_shape_convex::InertiaTensor() const
{
    shu::mat3f Result = this->mInertiaTensor;
    return Result;
}

shoora_mesh_type
shoora_shape_convex::GetType() const
{
    return shoora_mesh_type::CONVEX;
}

shoora_bounds
shoora_shape_convex::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    shu::vec3f Corners[8];
    Corners[0] = shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Mins.y, this->mBounds.Mins.z);
    Corners[1] = shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Mins.y, this->mBounds.Maxs.z);
    Corners[2] = shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Maxs.y, this->mBounds.Mins.z);
    Corners[3] = shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Mins.y, this->mBounds.Mins.z);

    Corners[4] = shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Maxs.y, this->mBounds.Maxs.z);
    Corners[5] = shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Maxs.y, this->mBounds.Mins.z);
    Corners[6] = shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Mins.y, this->mBounds.Maxs.z);
    Corners[7] = shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Maxs.y, this->mBounds.Maxs.z);

    shoora_bounds Bounds;
    for (i32 i = 0; i < 8; ++i)
    {
        Corners[i] = shu::QuatRotateVec(Orientation, Corners[i]) + Pos;
        Bounds.Expand(Corners[i]);
    }

    return Bounds;
}

shoora_bounds
shoora_shape_convex::GetBounds() const
{
    shoora_bounds Result = this->mBounds;
    return Result;
}

shu::vec3f
shoora_shape_convex::Support(const shu::vec3f &Direction, const shu::vec3f &Position, const shu::quat &Orientation,
                             const f32 Bias) const
{
    // TODO: To Be Implemented
    shu::vec3f Support;
    return Support;
}

f32
shoora_shape_convex::FastestLinearSpeed(const shu::vec3f &AngularVelocity, const shu::vec3f &Direction) const
{
    f32 MaxSpeed = -SHU_FLOAT_MIN;
    for(i32 i = 0; i < mPoints.size(); ++i)
    {
        const shu::vec3f r = this->mPoints[i] - this->mCenterOfMass;
        shu::vec3f LinearVelocity = AngularVelocity.Cross(r);

        f32 speed = LinearVelocity.Dot(Direction);
        if(speed > MaxSpeed)
        {
            MaxSpeed = speed;
        }
    }

    return MaxSpeed;
}

// Private stuff
i32
shoora_shape_convex::FindPointFurthestInDirection(const shu::vec3f *Points, const i32 Num,
                                                  const shu::vec3f &Direction)
{
    i32 MaxId = 0;
    f32 MaxDist = Direction.Dot(Points[0]);
    for(i32 i = 1; i < Num; ++i)
    {
        f32 Distance = Direction.Dot(Points[i]);
        if(Distance > MaxDist)
        {
            MaxDist = Distance;
            MaxId = i;
        }
    }

    return MaxId;
}

f32
shoora_shape_convex::DistanceFromLine(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &Point)
{
    shu::vec3f LineDirection = shu::Normalize(B - A);

    shu::vec3f RayToPoint = Point - A;
    shu::vec3f ParallelComp = RayToPoint.Dot(LineDirection) * LineDirection;
    shu::vec3f PerpendicularComp = RayToPoint - ParallelComp;

    f32 DistanceFromLine = PerpendicularComp.Magnitude();
    return DistanceFromLine;
}

shu::vec3f
shoora_shape_convex::FindPointFurthestFromLine(const shu::vec3f *Points, const i32 Num, const shu::vec3f &PointA,
                                               const shu::vec3f &PointB)
{
    f32 MaxDistance = this->DistanceFromLine(PointA, PointB, Points[0]);
    i32 MaxId = 0;

    for(i32 i = 0; i < Num; ++i)
    {
        f32 Distance = this->DistanceFromLine(PointA, PointB, Points[i]);
        if(Distance > MaxDistance)
        {
            MaxDistance = Distance;
            MaxId = i;
        }
    }

    return Points[MaxId];
}

f32
shoora_shape_convex::DistanceFromTriangle(const shu::vec3f &TriPointA, const shu::vec3f &TriPointB,
                                          const shu::vec3f &TriPointC, const shu::vec3f &Point)
{
    shu::vec3f AB = TriPointB - TriPointA;
    shu::vec3f AC = TriPointC - TriPointA;

    shu::vec3f Normal = AB.Cross(AC);
    Normal = shu::Normalize(Normal);

    shu::vec3f Ray = Point - TriPointA;

    f32 Distance = Ray.Dot(Normal);
    return Distance;
}

shu::vec3f
shoora_shape_convex::FindPointFurthestFromTriangle(const shu::vec3f *Points, const i32 Num,
                                                   const shu::vec3f &TriPointA, const shu::vec3f &TriPointB,
                                                   const shu::vec3f &TriPointC)
{
    i32 MaxId = 0;
    f32 MaxDistance = this->DistanceFromTriangle(TriPointA, TriPointB, TriPointC, Points[0]);

    for (i32 i = 1; i < Num; ++i)
    {
        f32 Dist = DistanceFromTriangle(TriPointA, TriPointB, TriPointC, Points[i]);
        if(Dist > MaxDistance)
        {
            MaxDistance = Dist;
            MaxId = i;
        }
    }

    return Points[MaxId];
}

void
shoora_shape_convex::BuildTetrahedron(const shu::vec3f *Vertices, const i32 VertexCount,
                                      stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris)
{
    HullPoints.clear();
    HullTris.clear();

    shu::vec3f Points[4];

    i32 IdX = FindPointFurthestInDirection(Vertices, VertexCount, shu::Vec3f(1, 0, 0));
    Points[0] = Vertices[IdX];

    IdX = FindPointFurthestInDirection(Vertices, VertexCount, Points[0]*-1.0f);
    Points[1] = Vertices[IdX];
    Points[2] = FindPointFurthestFromLine(Vertices, VertexCount, Points[0], Points[1]);
    Points[3] = FindPointFurthestFromTriangle(Vertices, VertexCount, Points[0], Points[1], Points[2]);

    // NOTE: This is important for making sure that the ordering is CCW for all faces.
    // Since 0, 1, 2 appear counter-clockwise, in that case the Point D will be on the side of the direction
    // AB.Cross(AC) in DistanceFromTriangle function.
    f32 Distance = DistanceFromTriangle(Points[0], Points[1], Points[2], Points[3]);
    if(Distance > 0.0f)
    {
        SWAP(Points[0], Points[1]);
    }

    // Build the terahedron
    HullPoints.add(Points[0]);
    HullPoints.add(Points[1]);
    HullPoints.add(Points[2]);
    HullPoints.add(Points[3]);

    tri_t Tri;
    Tri.A = 0;
    Tri.B = 1;
    Tri.C = 2;
    HullTris.add(Tri);

    Tri.A = 0;
    Tri.B = 2;
    Tri.C = 3;
    HullTris.add(Tri);

    Tri.A = 2;
    Tri.B = 1;
    Tri.C = 3;
    HullTris.add(Tri);

    Tri.A = 1;
    Tri.B = 0;
    Tri.C = 3;
    HullTris.add(Tri);
}

void
shoora_shape_convex::ExpandConvexHull(stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris,
                                      const shoora_dynamic_array<shu::vec3f> &Vertices)
{
    shu::vec3f *ExternalVertsArr = (shu::vec3f *)_alloca(sizeof(shu::vec3f) * Vertices.size());
    stack_array<shu::vec3f> ExternalVerts(ExternalVertsArr, Vertices.size());

    for(i32 i = 0; i < Vertices.size(); ++i)
    {
        ExternalVerts.add(Vertices[i]);
    }

    RemoveInternalVertices(HullPoints, HullTris, ExternalVerts);

    while(ExternalVerts.size > 0)
    {
        i32 PointIdX = this->FindPointFurthestInDirection(ExternalVerts.data, (i32)ExternalVerts.size,
                                                          ExternalVerts.data[0]);

        shu::vec3f Point = ExternalVerts.data[PointIdX];

        ExternalVerts.erase(PointIdX);

        AddPoint(HullPoints, HullTris, Point);
        RemoveInternalVertices(HullPoints, HullTris, ExternalVerts);
    }

    RemoveUnreferencedVertices(HullPoints, HullTris);
}

void
shoora_shape_convex::RemoveInternalVertices(const stack_array<shu::vec3f> &HullPoints, const stack_array<tri_t> &HullTris,
                                            stack_array<shu::vec3f> &CheckVertices)
{
    for (i32 i = 0; i < CheckVertices.size; ++i)
    {
        const shu::vec3f &Point = CheckVertices.data[i];

        b32 IsExternal = false;
        for (i32 t = 0; t < HullTris.size; ++t)
        {
            const tri_t &Tri = HullTris.data[t];
            const shu::vec3f &A = HullPoints.data[Tri.A];
            const shu::vec3f &B = HullPoints.data[Tri.B];
            const shu::vec3f &C = HullPoints.data[Tri.C];

            // NOTE: If the Point is in front of any triangle, it is external
            f32 Distance = DistanceFromTriangle(A, B, C, Point);
            if(Distance > 0.0f)
            {
                IsExternal = true;
                break;
            }
        }

        // If Its not external, then it should be internal to the tetrahedron, hence should be removed.
        if (!IsExternal)
        {
            CheckVertices.erase(i);
            --i;
        }
    }

    // Remove any points that are a little too close to the Hull Points.
    for (i32 i = 0; i < CheckVertices.size; ++i)
    {
        const shu::vec3f &Point = CheckVertices.data[i];

        b32 IsTooClose = false;
        for (i32 j = 0; j < HullPoints.size; ++j)
        {
            shu::vec3f HullPoint = HullPoints.data[j];
            shu::vec3f Ray = HullPoint - Point;
            if(Ray.SqMagnitude() < 0.001f * .001f)
            {
                IsTooClose = true;
                break;
            }
        }

        if(IsTooClose)
        {
            CheckVertices.erase(i);
            --i;
        }
    }
}

b32
shoora_shape_convex::IsEdgeUnique(const tri_t *Tris, const stack_array<i32> &FacingTris, const i32 IgnoreTri,
                                  const edge_t &Edge)
{
    for (i32 i = 0; i < FacingTris.size; ++i)
    {
        const i32 TriIdX = FacingTris.data[i];
        if(IgnoreTri == TriIdX)
        {
            continue;
        }

        const tri_t &Tri = Tris[TriIdX];

        edge_t Edges[3];
        Edges[0].A = Tri.A;
        Edges[0].B = Tri.B;

        Edges[1].A = Tri.B;
        Edges[1].B = Tri.C;

        Edges[2].A = Tri.C;
        Edges[2].B = Tri.A;

        for(i32 e = 0; e < 3; ++e)
        {
            if (Edge == Edges[e])
            {
                return false;
            }
        }
    }

    return true;
}

void
shoora_shape_convex::AddPoint(stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris, const shu::vec3f &Point)
{
    // This Point is outsid.
    // Now we need to remove old triangles and build new ones.

    // Find all the triangles that face this point.
    i32 *FacingTrisArr = (i32 *) _alloca(HullTris.size * sizeof(i32));
    stack_array<i32> FacingTris(FacingTrisArr, HullTris.size);
    for(i32 i = (i32)(HullTris.size - 1); i >= 0; --i)
    {
        const tri_t &Tri = HullTris.data[i];

        const shu::vec3f &A = HullPoints.data[Tri.A];
        const shu::vec3f &B = HullPoints.data[Tri.B];
        const shu::vec3f &C = HullPoints.data[Tri.C];

        const f32 Distance = DistanceFromTriangle(A, B, C, Point);
        if(Distance > 0.0f)
        {
            FacingTris.add(i);
        }
    }

    // NOTE: Now find all the edges that areunique to the tris, these will be the new edges that will form triangles
    edge_t *UniqueEdgesArr = (edge_t *) _alloca(sizeof(edge_t) * 3 * FacingTris.size);
    stack_array<edge_t> UniqueEdges(UniqueEdgesArr, 3 * FacingTris.size);
    for (i32 i = 0; i < FacingTris.size; i++)
    {
        const i32 TriIdX = FacingTris.data[i];
        const tri_t &Tri = HullTris.data[TriIdX];

        edge_t Edges[3];
        Edges[0].A = Tri.A;
        Edges[0].B = Tri.B;

        Edges[1].A = Tri.B;
        Edges[1].B = Tri.C;

        Edges[2].A = Tri.C;
        Edges[2].B = Tri.A;

        for(i32 e = 0; e < 3; ++e)
        {
            if(IsEdgeUnique(HullTris.data, FacingTris, TriIdX, Edges[e]))
            {
                UniqueEdges.add(Edges[e]);
            }
        }
    }

    // Now remove the old facing tris.
    for(i32 i = 0; i < FacingTris.size; ++i)
    {
        HullTris.erase(FacingTris.data[i]);
    }

    // Now Add the new Point.
    HullPoints.add(Point);
    const i32 NewPointIdx = (i32)HullPoints.size - 1;

    // Now add triangles for each unique edge.
    for (i32 i = 0; i < UniqueEdges.size; ++i)
    {
        const edge_t &Edge = UniqueEdges.data[i];
        tri_t Tri;
        Tri.A = Edge.A;
        Tri.B = Edge.B;
        Tri.C = NewPointIdx;
        HullTris.add(Tri);
    }
}

void
shoora_shape_convex::RemoveUnreferencedVertices(stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris)
{
    for (i32 i = 0; i < HullPoints.size; ++i)
    {
        b32 IsUsed = false;
        for (i32 j = 0; j < HullTris.size; ++j)
        {
            const tri_t &Tri = HullTris.data[j];

            if (Tri.A == i || Tri.B == i || Tri.C == i)
            {
                IsUsed = true;
                break;
            }
        }

        if(IsUsed)
        {
            continue;
        }

        for (i32 j = 0; j < HullTris.size; ++j)
        {
            tri_t &Tri = HullTris.data[j];
            if (Tri.A > i) { Tri.A--; }
            if (Tri.B > i) { Tri.B--; }
            if (Tri.C > i) { Tri.C--; }
        }

        HullPoints.erase(i);
        i--;
    }
}

void
shoora_shape_convex::BuildConvexHull(const shoora_dynamic_array<shu::vec3f> &Vertices, stack_array<shu::vec3f> &HullPoints,
                                     stack_array<tri_t> &HullTris)
{
    if(Vertices.size() < 4)
    {
        return;
    }

    // Build a Tetrahedron
    BuildTetrahedron(Vertices.data(), (i32)Vertices.size(), HullPoints, HullTris);
    ExpandConvexHull(HullPoints, HullTris, Vertices);
}

b32
shoora_shape_convex::IsExternal(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris, const shu::vec3f &Point)
{
    b32 IsExternal = false;
    for (i32 t = 0; t < Tris.size; ++t)
    {
        const tri_t &tri = Tris.data[t];
        const shu::vec3f &A = Points.data[tri.A];
        const shu::vec3f &B = Points.data[tri.B];
        const shu::vec3f &C = Points.data[tri.C];

        // If the point is in front of any triangle, then its external.
        f32 Distance = DistanceFromTriangle(A, B, C, Point);
        if (Distance > 0.0f)
        {
            IsExternal = true;
            break;
        }
    }

    return IsExternal;
}

shu::vec3f
shoora_shape_convex::CalculateCenterOfMass(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris)
{
    const i32 NumSamples = 100;
    shoora_bounds Bounds;
    Bounds.Expand(Points.data, Points.size);

    shu::vec3f CenterOfMass{};
    const f32 dx = Bounds.WidthX() / (f32)NumSamples;
    const f32 dy = Bounds.WidthY() / (f32)NumSamples;
    const f32 dz = Bounds.WidthZ() / (f32)NumSamples;

    i32 SampleCount = 0;
    for(f32 x = Bounds.Mins.x; x < Bounds.Maxs.x; x += dx)
    {
        for(f32 y = Bounds.Mins.y; y < Bounds.Maxs.y; y += dy)
        {
            for(f32 z = Bounds.Mins.z; z < Bounds.Maxs.z; z += dz)
            {
                shu::vec3f Point = shu::Vec3f(x, y, z);
                if(IsExternal(Points, Tris, Point))
                {
                    continue;
                }

                CenterOfMass += Point;
                SampleCount++;
            }
        }
    }

    CenterOfMass /= (f32)SampleCount;
    return CenterOfMass;
}

shu::mat3f
shoora_shape_convex::CalculateInertiaTensor(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris)
{
    const i32 NumSamples = 100;

    shoora_bounds Bounds;
    Bounds.Expand(Points.data, (i32)Points.size);

    shu::mat3f Tensor{};
    const f32 dx = Bounds.WidthX() / (f32)NumSamples;
    const f32 dy = Bounds.WidthY() / (f32)NumSamples;
    const f32 dz = Bounds.WidthZ() / (f32)NumSamples;

    i32 SampleCount = 0;
    for (f32 x = Bounds.Mins.x; x < Bounds.Maxs.x; x += dx)
    {
        for (f32 y = Bounds.Mins.y; y < Bounds.Maxs.y; y += dy)
        {
            for (f32 z = Bounds.Mins.z; z < Bounds.Maxs.z; z += dz)
            {
                shu::vec3f Point = shu::Vec3f(x, y, z);
                if(IsExternal(Points, Tris, Point))
                {
                    continue;
                }

                // Get the point relative to the center of mass.
                Point -= mCenterOfMass;

                Tensor.m00 += Point.y*Point.y + Point.z*Point.z;
                Tensor.m01 += -1.0f * Point.x * Point.y;
                Tensor.m02 += -1.0f * Point.x * Point.z;

                Tensor.m10 += -1.0f * Point.x * Point.y;
                Tensor.m11 += Point.x*Point.x + Point.z*Point.z;
                Tensor.m12 += -1.0f * Point.y * Point.z;

                Tensor.m20 += -1.0f * Point.z * Point.x;
                Tensor.m21 += -1.0f * Point.z * Point.y;
                Tensor.m22 += Point.x*Point.x + Point.y*Point.y;

                SampleCount++;
            }
        }
    }

    Tensor *= 1.0f / (f32)SampleCount;
    return Tensor;
}