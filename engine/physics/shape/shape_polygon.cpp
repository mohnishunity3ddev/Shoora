#include "shape.h"

shoora_shape_polygon::shoora_shape_polygon(shoora_mesh_type Type)
    : shoora_shape(Type)
{
    this->VertexCount = MeshFilter->VertexCount;
    this->WorldVertices = new shu::vec3f[this->VertexCount];
}

shoora_shape_polygon::shoora_shape_polygon(i32 MeshId, f32 Scale)
    : shoora_shape(shoora_mesh_type::POLYGON_2D, shoora_mesh_database::GetCustomMeshFilter(MeshId))
{
    this->Scale = Scale;
    this->VertexCount = MeshFilter->VertexCount;
    ASSERT(this->VertexCount != 0);
    this->WorldVertices = new shu::vec3f[this->VertexCount];

    // TODO: Change this. Arbitrary polygons may not have their center of mass in their center.
    this->mCenterOfMass = shu::Vec3f(0.0f);
}

shoora_shape_polygon::~shoora_shape_polygon()
{
    ASSERT(this->WorldVertices != nullptr);
    delete[] this->WorldVertices;
    this->WorldVertices = nullptr;
}

shu::mat3f
shoora_shape_polygon::InertiaTensor() const
{
    // TODO: Calculate real inertia tensor.
    shu::mat3f Result = shu::Mat3f(5000.0f);
    return Result;
}

shu::vec3f
shoora_shape_polygon::GetDim() const
{
    auto Result = shu::Vec3f(Scale, Scale, 1.0f);
    return Result;
}

void
shoora_shape_polygon::UpdateWorldVertices(shu::mat4f &ModelMatrix)
{
    const auto *LocalVertices = MeshFilter->Vertices;
    for (u32 i = 0; i < this->VertexCount; ++i)
    {
        this->WorldVertices[i] = (ModelMatrix * LocalVertices[i].Pos).xyz;
    }
}

shu::vec3f
shoora_shape_polygon::GetEdgeAt(i32 Index)
{
    ASSERT(Index < this->VertexCount);
    auto currentVertex = this->WorldVertices[Index];
    auto nextVertex = this->WorldVertices[(Index + 1) % this->VertexCount];

    shu::vec3f Result = nextVertex - currentVertex;
    return Result;
}

i32
shoora_shape_polygon::GetIncidentEdgeIndex(const shu::vec2f &ReferenceEdgeNormal)
{
    i32 IncidentEdgeIndex = -1;

    f32 minDot = SHU_FLOAT_MAX;
    for (i32 i = 0; i < this->VertexCount; ++i)
    {
        auto edge = GetEdgeAt(i);
        auto normal = edge.xy.Normal();

        auto d = normal.Dot(ReferenceEdgeNormal);
        if(d < minDot)
        {
            minDot = d;
            IncidentEdgeIndex = i;
        }
    }

    return IncidentEdgeIndex;
}

i32
shoora_shape_polygon::GetNextVertexIndex(i32 EdgeIndex)
{
    i32 Result = -1;
    Result = (EdgeIndex + 1) % this->VertexCount;
    ASSERT(Result != -1);
    return Result;
}

i32
shoora_shape_polygon::ClipSegmentToLine(shu::vec2f Contacts[2], shu::vec2f Clipped[2], shu::vec2f r0,
                                        shu::vec2f r1)
{
    i32 NumVertices = 0;

    shu::vec2f Normal = (r1 - r0).Normal();
    f32 Distance0 = (Contacts[0] - r0).Dot(Normal);
    f32 Distance1 = (Contacts[1] - r0).Dot(Normal);

    if(Distance0 <= 0.0f)
    {
        Clipped[NumVertices++] = Contacts[0];
    }
    if(Distance1 <= 0.0f)
    {
        Clipped[NumVertices++] = Contacts[1];
    }

    if(Distance0 * Distance1 < 0)
    {
        f32 TotalDistance = Distance0 - Distance1;
        f32 t = Distance0 / TotalDistance;

        shu::vec2f Clip = Contacts[0] + (Contacts[1] - Contacts[0]) * t;
        Clipped[NumVertices++] = Clip;
    }

    return NumVertices;
}

shoora_mesh_type
shoora_shape_polygon::GetType() const
{
    return shoora_mesh_type::POLYGON_2D;
}

// IMPORTANT: NOTE: We are using the Separate Axis Theorem to find any one axis which acts as a plane that divides
// the two rects(or obliqued bounding boxes). Here, if the bodies are colliding, we find the Axis(which tells us
// about the collision) which gives the minimum separation between the two bodies which in turn gives the maximum
// penetration amongst the two. This will help us in getting contact information which we'll use to resolve the
// collision later. Here, we take all edges of the polygon/rect_2d and get their normals, and we test these normals
// to see whether they divide the two bodies, if we find atleast one such axis, it is guaranteed that the bodies
// are not colliding. if we find multiple axis where the bodies are colliding, we try to find the one which gives
// us the minimum separation between the two bodies. How do we calculate separation?
// 1-> We take an edge normal on body a
// 2-> we test it against all vertices in body b, by taking the vector Vb - Va joining the two vertices of a and b,
//     and project it onto this normal, this projection will be the separation between the two bodies along the
//     normal.
// 3-> We keep track of the most minimum separation for one normal axis (when tested against all the vertices of b)
// 4-> And we take the maximum of all these "minimums" when other normals are also tested for all vertices of b.
// IMPORTANT: NOTE: if the separation is negative, then there was collision otherwise no collision.
f32
shoora_shape_polygon::FindMinSeparation(shoora_shape_polygon *Other, i32 &ReferenceEdgeIndex, shu::vec2f &SupportPoint)
{
    auto *MeshFilterA = this->GetMeshFilter();
    ASSERT(MeshFilterA != nullptr);

    auto *MeshFilterB = Other->GetMeshFilter();
    ASSERT(MeshFilterB != nullptr);

    auto VertexCountA = MeshFilterA->VertexCount;
    auto VertexCountB = MeshFilterB->VertexCount;

    f32 BestSeparation = SHU_FLOAT_MIN;
    for (i32 i = 0; i < VertexCountA; ++i)
    {
        auto vA = this->WorldVertices[i].xy;
        auto EdgeA = this->GetEdgeAt(i).xy;
        auto Normal = EdgeA.Normal();

        f32 MinSeparation = SHU_FLOAT_MAX;
        shu::vec2f MinVertex;
        for (i32 j = 0; j < VertexCountB; ++j)
        {
            auto vB = Other->WorldVertices[j].xy;
            f32 Projection = (vB - vA).Dot(Normal);

            // NOTE: Tracking the highest distance a vertex on body B is from a particular edge in body A for this
            // iteration of the loop.
            if (MinSeparation > Projection)
            {
                MinSeparation = Projection;
                MinVertex = vB;
            }
        }

        // NOTE: Tracking the max(lowest distance a point on body B is "Behind" an edge in body A)
        if(BestSeparation < MinSeparation) {
            BestSeparation = MinSeparation;
            ReferenceEdgeIndex = i;
            SupportPoint = MinVertex;
        }
    }

    return BestSeparation;
}

shoora_bounds
shoora_shape_polygon::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shoora_bounds
shoora_shape_polygon::GetBounds() const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shu::vec3f
shoora_shape_polygon::SupportPtWorldSpace(const shu::vec3f &Direction, const shu::vec3f &Position,
                              const shu::quat &Orientation, const f32 Bias) const
{
    // TODO): TO BE IMPLEMENTED
    shu::vec3f Result;
    return Result;
}
