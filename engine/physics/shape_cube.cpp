#include "shape.h"

// *NOTE: Cube Stuff
shoora_shape_cube::shoora_shape_cube(u32 Width, u32 Height, u32 Depth) : shoora_shape(shoora_mesh_type::CUBE)
{
    this->Width = Width;
    this->Height = Height;
    this->Depth = Depth;
    this->mCenterOfMass = Shu::Vec3f(0.0f);
}

shoora_shape_cube::shoora_shape_cube(const Shu::vec3f *Points, const i32 Num)
    : shoora_shape(shoora_mesh_type::CUBE)
{
    this->Build(Points, Num);
}

void
shoora_shape_cube::Build(const Shu::vec3f *Points, const i32 Num)
{
    for (i32 i = 0; i < Num; ++i)
    {
        mBounds.Expand(Points[i]);
    }

    memset(mPoints, 0, (ARRAY_SIZE(mPoints) * sizeof(Shu::vec3f)));
    mPoints[0] = Shu::Vec3f(mBounds.Mins.x, mBounds.Mins.y, mBounds.Mins.z);
    mPoints[1] = Shu::Vec3f(mBounds.Maxs.x, mBounds.Mins.y, mBounds.Mins.z);
    mPoints[2] = Shu::Vec3f(mBounds.Mins.x, mBounds.Maxs.y, mBounds.Mins.z);
    mPoints[3] = Shu::Vec3f(mBounds.Mins.x, mBounds.Mins.y, mBounds.Maxs.z);

    mPoints[4] = Shu::Vec3f(mBounds.Maxs.x, mBounds.Maxs.y, mBounds.Maxs.z);
    mPoints[5] = Shu::Vec3f(mBounds.Mins.x, mBounds.Maxs.y, mBounds.Maxs.z);
    mPoints[6] = Shu::Vec3f(mBounds.Maxs.x, mBounds.Mins.y, mBounds.Maxs.z);
    mPoints[7] = Shu::Vec3f(mBounds.Maxs.x, mBounds.Maxs.y, mBounds.Mins.z);

    mCenterOfMass = (this->mBounds.Mins + this->mBounds.Maxs) * 0.5f;

    this->Width = this->mBounds.WidthX();
    this->Height = this->mBounds.WidthY();
    this->Depth = this->mBounds.WidthZ();
}

shoora_shape_cube::~shoora_shape_cube() { LogUnformatted("shoora_shape_cube destructor called!\n"); }

Shu::mat3f
shoora_shape_cube::InertiaTensor() const
{
    // Inertia tensor for the box centered around zero.
    const f32 dx = this->mBounds.Maxs.x - this->mBounds.Mins.x;
    const f32 dy = this->mBounds.Maxs.y - this->mBounds.Mins.y;
    const f32 dz = this->mBounds.Maxs.z - this->mBounds.Mins.z;

    Shu::mat3f Tensor = Shu::Mat3f(0.0f);
    Tensor.m00 = (dy*dy + dz*dz) / 12.0f;
    Tensor.m01 = (dx*dx + dz*dz) / 12.0f;
    Tensor.m02 = (dx*dx + dy*dy) / 12.0f;

    // Parallel Axis theorem to get moi tensor around an axis not passing through center of mass. The origin is the
    // center of mass for this shape. Since this is in model space.
    Shu::vec3f COM;
    COM.x = (this->mBounds.Maxs.x + this->mBounds.Mins.x) * 0.5f;
    COM.y = (this->mBounds.Maxs.y + this->mBounds.Mins.y) * 0.5f;
    COM.z = (this->mBounds.Maxs.z + this->mBounds.Mins.z) * 0.5f;

    const Shu::vec3f R = Shu::Vec3f(0.0f) - COM;
    const f32 RSquared = R.SqMagnitude();
    Shu::mat3f ParallelAxisTensor;
    ParallelAxisTensor.Rows[0] = Shu::Vec3f(RSquared - R.x*R.x, R.x*R.y, R.x*R.z);
    ParallelAxisTensor.Rows[1] = Shu::Vec3f(R.y*R.x, RSquared - R.y*R.y, R.y*R.z);
    ParallelAxisTensor.Rows[2] = Shu::Vec3f(R.z*R.x, R.z*R.y, RSquared - R.z*R.z);

    Tensor += ParallelAxisTensor;
    return Tensor;
}

Shu::vec3f
shoora_shape_cube::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Width, this->Height, this->Depth);
    return Result;
}

shoora_mesh_type
shoora_shape_cube::GetType() const
{
    return shoora_mesh_type::CUBE;
}

shoora_bounds
shoora_shape_cube::GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const
{
    Shu::vec3f Corners[8];
    Corners[0] = Shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Mins.y, this->mBounds.Mins.z);
    Corners[1] = Shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Mins.y, this->mBounds.Maxs.z);
    Corners[2] = Shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Maxs.y, this->mBounds.Mins.z);
    Corners[3] = Shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Mins.y, this->mBounds.Mins.z);

    Corners[4] = Shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Maxs.y, this->mBounds.Maxs.z);
    Corners[5] = Shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Maxs.y, this->mBounds.Mins.z);
    Corners[6] = Shu::Vec3f(this->mBounds.Maxs.x, this->mBounds.Mins.y, this->mBounds.Maxs.z);
    Corners[7] = Shu::Vec3f(this->mBounds.Mins.x, this->mBounds.Maxs.y, this->mBounds.Maxs.z);

    shoora_bounds Bounds;
    for (i32 i = 0; i < 8; ++i)
    {
        Corners[i] = Shu::QuatRotateVec(Orientation, Corners[i]) + Pos;
        Bounds.Expand(Corners[i]);
    }

    return Bounds;
}

shoora_bounds
shoora_shape_cube::GetBounds() const
{
    shoora_bounds Result = this->mBounds;
    return Result;
}

Shu::vec3f
shoora_shape_cube::Support(const Shu::vec3f &Direction, const Shu::vec3f &Position, const Shu::quat &Orientation,
                           const f32 Bias) const
{
    // NOTE: Find the furthest point/vertex in the given direction.
    Shu::vec3f MaxPoint = Shu::QuatRotateVec(Orientation, this->mPoints[0]) + Position;
    f32 MaxDistance = Direction.Dot(MaxPoint);

    for(i32 i = 1; i < ARRAY_SIZE(mPoints); ++i)
    {
        const Shu::vec3f Point = Shu::QuatRotateVec(Orientation, this->mPoints[i]) + Position;
        const f32 PointDistance = Direction.Dot(Point);

        if(PointDistance > MaxDistance)
        {
            MaxDistance = PointDistance;
            MaxPoint = Point;
        }
    }

    Shu::vec3f Norm = Shu::Normalize(Direction);
    Norm *= Bias;

    Shu::vec3f Result = MaxPoint + Norm;
    return Result;
}

f32
shoora_shape_cube::FastestLinearSpeed(const Shu::vec3f &AngularVelocity, const Shu::vec3f &Direction) const
{
    f32 MaxSpeed = 0.0f;
    for(i32 i = 0; i < ARRAY_SIZE(this->mPoints); ++i)
    {
        Shu::vec3f r = this->mPoints[i] - this->mCenterOfMass;
        Shu::vec3f LinearVelocity = AngularVelocity.Cross(r);
        // LinearVelocity in the direction of the "Direction" passed in here.
        f32 Speed = Direction.Dot(LinearVelocity);

        if(Speed > MaxSpeed)
        {
            MaxSpeed = Speed;
        }
    }

    return MaxSpeed;
}