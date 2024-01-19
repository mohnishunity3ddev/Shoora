#include "shape.h"

// NOTE: Shape stuff
shoora_shape::shoora_shape(shoora_primitive_type Type)
{
    this->Type = Type;
    this->Primitive = shoora_primitive_collection::GetPrimitive(Type);
}

void
shoora_shape::Draw(const VkCommandBuffer &cmdBuffer)
{
    this->Primitive->Draw(cmdBuffer);
}

// NOTE: Polygon stuff
shoora_shape_polygon::shoora_shape_polygon(shoora_primitive_type Type) : shoora_shape(Type)
{
    this->VertexCount = this->Primitive->MeshFilter.VertexCount;
    this->WorldVertices = new Shu::vec3f[this->VertexCount];
}

shoora_shape_polygon::~shoora_shape_polygon()
{
    if (this->WorldVertices != nullptr)
    {
        delete[] this->WorldVertices;
        this->WorldVertices = nullptr;
    }
}

f32
shoora_shape_polygon::GetMomentOfInertia() const
{
    return 0.0f;
}

Shu::vec3f
shoora_shape_polygon::GetDim() const
{
    auto Result = Shu::Vec3f(0.0f);
    return Result;
}

void
shoora_shape_polygon::UpdateWorldVertices(Shu::mat4f &ModelMatrix)
{
    const auto *LocalVertices = this->Primitive->MeshFilter.Vertices;

    for (u32 i = 0; i < this->VertexCount; ++i)
    {
        this->WorldVertices[i] = (ModelMatrix * LocalVertices[i].Pos).xyz;
    }
}

Shu::vec3f
shoora_shape_polygon::GetEdgeAt(i32 Index)
{
    ASSERT(Index < this->VertexCount);
    auto currentVertex = this->WorldVertices[Index];
    auto nextVertex = this->WorldVertices[(Index + 1) % this->VertexCount];

    Shu::vec3f Result = nextVertex - currentVertex;
    return Result;
}

shoora_primitive_type
shoora_shape_polygon::GetType() const
{
    return shoora_primitive_type::POLYGON;
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
shoora_shape_polygon::FindMinSeparation(shoora_shape_polygon *other)
{
    auto VertexCountA = this->Primitive->MeshFilter.VertexCount;
    auto VertexCountB = other->Primitive->MeshFilter.VertexCount;

    f32 BestSeparation = -__FLT_MAX__;
    for (i32 i = 0; i < VertexCountA; ++i)
    {
        auto vA = this->WorldVertices[i].xy;
        auto Normal = this->GetEdgeAt(i).xy.Normal();

        f32 MinSeparation = __FLT_MAX__;
        for (i32 j = 0; j < VertexCountB; ++j)
        {
            auto vB = other->WorldVertices[j].xy;
            f32 CurrentBVertexSeparation = (vB - vA).Dot(Normal);

            MinSeparation = MIN(CurrentBVertexSeparation, MinSeparation);
        }

        BestSeparation = MAX(BestSeparation, MinSeparation);
    }

    return BestSeparation;
}

// NOTE: Circle
shoora_shape_circle::shoora_shape_circle(u32 Radius) : shoora_shape(shoora_primitive_type::CIRCLE)
{
    this->Radius = Radius;
}

shoora_shape_circle::~shoora_shape_circle()
{
    LogUnformatted("shoora_shape_circle destructor called!\n");
}

f32
shoora_shape_circle::GetMomentOfInertia() const
{
    f32 Result = 0.5f * this->Radius * this->Radius;
    return Result;
}

Shu::vec3f
shoora_shape_circle::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Radius, this->Radius, 1.0f);
    return Result;
}

shoora_primitive_type
shoora_shape_circle::GetType() const
{
    return shoora_primitive_type::CIRCLE;
}


// NOTE: Box Stuff
shoora_shape_box::shoora_shape_box(u32 Width, u32 Height) : shoora_shape_polygon(shoora_primitive_type::RECT_2D)
{
    this->Width = Width;
    this->Height = Height;
}

shoora_shape_box::~shoora_shape_box()
{
    LogUnformatted("shoora_shape_box destructor called!\n");
}

f32
shoora_shape_box::GetMomentOfInertia() const
{
    f32 Result = (this->Width * this->Width + this->Height * this->Height) / 12.0f;
    return Result;
}

Shu::vec3f
shoora_shape_box::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Width, this->Height, 1.0f);
    return Result;
}

shoora_primitive_type
shoora_shape_box::GetType() const
{
    return shoora_primitive_type::RECT_2D;
}

// NOTE: Triangle
shoora_shape_triangle::shoora_shape_triangle(u32 Base, u32 Height) : shoora_shape(shoora_primitive_type::TRIANGLE)
{
    this->Base = Base;
    this->Height = Height;
}

shoora_shape_triangle::~shoora_shape_triangle()
{
    LogUnformatted("shoora_shape_triangle destructor called!\n");
}

f32
shoora_shape_triangle::GetMomentOfInertia() const
{
    f32 b = this->Base;
    f32 h = this->Height;
    f32 Result = (b * (h * h * h)) / 36.0f;
    return Result;
}

Shu::vec3f
shoora_shape_triangle::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Base, this->Height, 1.0f);
    return Result;
}

shoora_primitive_type
shoora_shape_triangle::GetType() const
{
    return shoora_primitive_type::TRIANGLE;
}
