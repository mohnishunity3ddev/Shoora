#include "shape.h"

// NOTE: Sphere
shoora_shape_sphere::shoora_shape_sphere(f32 Radius) : shoora_shape(shoora_mesh_type::SPHERE)
{
    this->Radius = Radius;
    this->mCenterOfMass = shu::Vec3f(0.0f);
}

shoora_shape_sphere::~shoora_shape_sphere() { LogUnformatted("shoora_shape_sphere destructor called!\n"); }

shu::mat3f
shoora_shape_sphere::InertiaTensor() const
{
    shu::mat3f Result = shu::Mat3f(0.4f * this->Radius * this->Radius);
    return Result;
}

shu::vec3f
shoora_shape_sphere::GetDim() const
{
    shu::vec3f Result = shu::Vec3f(this->Radius, this->Radius, this->Radius);
    return Result;
}

shoora_mesh_type
shoora_shape_sphere::GetType() const
{
    return shoora_mesh_type::SPHERE;
}

shoora_bounds
shoora_shape_sphere::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    shoora_bounds Result;
    Result.Mins = Pos - shu::Vec3f(this->Radius);
    Result.Maxs = Pos + shu::Vec3f(this->Radius);
    return Result;
}

shoora_bounds
shoora_shape_sphere::GetBounds() const
{
    shoora_bounds Result;
    Result.Mins = shu::Vec3f(-this->Radius);
    Result.Maxs = shu::Vec3f(this->Radius);
    return Result;
}

shu::vec3f
shoora_shape_sphere::SupportPtWorldSpace(const shu::vec3f &Direction, const shu::vec3f &Position,
                             const shu::quat &Orientation, const f32 Bias) const
{
    // TODO: Remove this case.
#if 1
    shoora_mesh_filter *meshFilter = shoora_mesh_database::GetMeshFilter(this->Type);
    
    // NOTE: Find the point furthest in the direction.
    shu::vec3f Scale = this->GetDim();
    shu::mat4f Model = shu::TRS(Position, this->GetDim(), Orientation);
    shu::vec3f MaxPoint = (Model * shu::Vec4f(meshFilter->Vertices[0].Pos, 1.0f)).xyz;
    f32 MaxProj = MaxPoint.Dot(Direction);
    for(i32 i = 1; i < meshFilter->VertexCount; ++i)
    {
        auto Point = (Model * shu::Vec4f(meshFilter->Vertices[i].Pos, 1.0f)).xyz;
        auto PointDistance = Point.Dot(Direction);

        if(MaxProj < PointDistance)
        {
            MaxProj = PointDistance;
            MaxPoint = Point;
        }
    }

    shu::vec3f Normal = shu::Normalize(Direction);
    Normal *= Bias;

    shu::vec3f Result = MaxPoint + Normal;
    return Result;

#else
    shu::vec3f n = shu::Normalize(Direction);

    shu::vec3f SupportPoint = Position + n * (this->Radius + Bias);
    return SupportPoint;
#endif
}
