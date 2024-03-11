#if !defined(SHAPE_H)

#include <defines.h>
#include <mesh/database/mesh_database.h>
#include <math/math.h>
#include <containers/dynamic_array.h>
#include "../bounds.h"
#include <platform/platform.h>

#define MOMENT_OF_INTERTIA_FUNC(name) f32 name()
typedef f32 getInertiaFunc();

struct shoora_shape
{
    shoora_mesh_filter *MeshFilter;

    shoora_mesh_type Type;
    b32 isPrimitive;

    // shoora_shape() = default;
    shoora_shape(shoora_mesh_type Type);
    shoora_shape(shoora_mesh_type Type, shoora_mesh_filter *MeshFilter);

    virtual void Build(const shu::vec3f *Points, const i32 Num, memory_arena *Arena  = nullptr) {}

    virtual ~shoora_shape() = default;
    virtual shu::mat3f InertiaTensor() const = 0;
    virtual shu::vec3f GetDim() const = 0;
    virtual shoora_mesh_type GetType() const = 0;
    virtual shu::vec3f GetCenterOfMass() { return mCenterOfMass; }

    // NOTE: The support point of the shape is a point on the shape that is the furthest away in a particular
    // direction. Used in the GJK algorithm used in collision detection for convex shapes.
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const = 0;

    // NOTE: FastestLinearSpeed is used in continuous collision detection. It's necessary for objects that are
    // "long". Since "long" objects can rotate and still hit objects even though they have ZERO Linear Velocity.
    // Sphere's on the other hand are not "long" since they are equally "long" in all directions. So if they have
    // zero linear velocity, we dont have to calculate their linear velocity bound since just them rotating will
    // not collide with other objects just because of its angular velocity.
    virtual f32
    FastestLinearSpeed(const shu::vec3f &AngularVelocity, const shu::vec3f &Direction) const { return 0.0f; }

    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const = 0;
    virtual shoora_bounds GetBounds() const = 0;

    shoora_mesh_filter *GetMeshFilter();

  protected:
    shu::vec3f mCenterOfMass;
};

struct shoora_shape_polygon : shoora_shape
{
    shu::vec3f *WorldVertices;

    u32 VertexCount;
    i32 MeshId = -1;
    f32 Scale;

    // NOTE: This constructor is for use for any random polygon shape.
    shoora_shape_polygon(i32 MeshId, f32 Scale = 1.0f);

    // NOTE: this constructor is for use by rects
    shoora_shape_polygon(shoora_mesh_type Type);
    virtual ~shoora_shape_polygon();

    virtual shu::mat3f InertiaTensor() const override;
    virtual shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;

    f32 FindMinSeparation(shoora_shape_polygon *Other, i32 &ReferenceEdgeIndex, shu::vec2f &SupportPoint);

    shu::vec3f GetEdgeAt(i32 index);

    // NOTE: The incident edge on this shape is the one whose normal is the one which most opposes the passed in
    // reference edge's normal. In other words the incident edge has the WORST alignment with the reference edge.
    i32 GetIncidentEdgeIndex(const shu::vec2f &ReferenceEdgeNormal);
    i32 GetNextVertexIndex(i32 EdgeIndex);
    i32 ClipSegmentToLine(shu::vec2f Contacts[2], shu::vec2f Clipped[2], shu::vec2f r0, shu::vec2f r1);
    void UpdateWorldVertices(shu::mat4f &ModelMatrix);
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_circle : shoora_shape
{
    // shoora_shape_circle() = delete;
    shoora_shape_circle(f32 Radius);
    virtual ~shoora_shape_circle();

    f32 Radius;
    virtual shu::mat3f InertiaTensor() const override;
    virtual shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_sphere : shoora_shape
{
    // shoora_shape_circle() = delete;
    shoora_shape_sphere(f32 Radius);
    virtual ~shoora_shape_sphere();

    f32 Radius;
    virtual shu::mat3f InertiaTensor() const override;
    virtual shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual shu::vec3f Support(const shu::vec3f &DirectionNormalized, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_box : shoora_shape_polygon
{
    // shoora_shape_box() = delete;
    shoora_shape_box(u32 Width, u32 Height);
    virtual ~shoora_shape_box();

    u32 Width, Height;
    virtual shu::mat3f InertiaTensor() const override;
    virtual shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_cube : shoora_shape
{
  public:
    shoora_shape_cube() = delete;
    explicit shoora_shape_cube(u32 Width, u32 Height, u32 Depth);
    explicit shoora_shape_cube(const shu::vec3f *Points, const i32 Num);
    virtual void Build(const shu::vec3f *Points, const i32 Num, memory_arena *Arena = nullptr) override;

    virtual ~shoora_shape_cube();

    virtual shu::mat3f InertiaTensor() const override;
    virtual shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;

    // NOTE: Takes in a direction, and returns the position of the vertex in the shape, which is the furthest in
    // this direction.
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;

    // NOTE: To be used in CCD. Takes in Angular Velocity of the object and the Direction and returns the max
    // velocity of the vertex travelling the fastest in this Direction.
    virtual f32 FastestLinearSpeed(const shu::vec3f &AngularVelocity, const shu::vec3f &Direction) const override;

  public:
    u32 Width, Height, Depth;
    shu::vec3f mPoints[8];
    shoora_bounds mBounds;
};

struct tri_t
{
    i32 A;
    i32 B;
    i32 C;
};

struct edge_t
{
    i32 A;
    i32 B;

    b32 operator == (const edge_t &Other) const
    {
        b32 Result = ((A == Other.A && B == Other.B) || (A == Other.B && B == Other.A));
        return Result;
    }
};

struct shoora_shape_convex : shoora_shape
{
  public:
    shoora_shape_convex() = delete;
    explicit shoora_shape_convex(const shu::vec3f *Points, const i32 Num, memory_arena *Arena = nullptr);
    virtual void Build(const shu::vec3f *Points, const i32 Num, memory_arena *Arena = nullptr) override;

    virtual ~shoora_shape_convex();

    virtual shu::mat3f InertiaTensor() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual shu::vec3f GetDim() const override { return shu::Vec3f(); }
    // NOTE: pMareturns the position of the vertex in the shape, which is the furthest in
    // this direction.
    virtual shu::vec3f Support(const shu::vec3f &Direction, const shu::vec3f &Position,
                               const shu::quat &Orientation, const f32 Bias) const override;
    // NOTE: To be used in CCD. Takes in Angular Velocity of the object and the Direction and returns the max
    // velocity of the vertex travelling the fastest in this Direction.
    virtual f32 FastestLinearSpeed(const shu::vec3f &AngularVelocity, const shu::vec3f &Direction) const override;

    static size_t GetRequiredSizeForConvexBuild(u32 TotalNumPoints);

  public:
    shu::vec3f *Points = nullptr, *HullPoints = nullptr;
    u32 *HullIndices = nullptr;
    i32 NumPoints = 0, NumHullPoints = 0, NumHullIndices = 0;
    shoora_bounds mBounds;
    shu::mat3f mInertiaTensor;
    shoora_vulkan_buffer VertexBuffer;
    shoora_vulkan_buffer IndexBuffer;

  private:
    i32 FindPointFurthestInDirection(const shu::vec3f *Points, const i32 Num, const shu::vec3f &Direction);
    // NOTE: A and B describe the end points of the line.
    f32 DistanceFromLine(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &Point);
    shu::vec3f FindPointFurthestFromLine(const shu::vec3f *Points, const i32 Num, const shu::vec3f &PointA,
                                         const shu::vec3f &PointB);
    f32 DistanceFromTriangle(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C,
                             const shu::vec3f &Point);
    shu::vec3f FindPointFurthestFromTriangle(const shu::vec3f *Points, const i32 Num, const shu::vec3f &PointA,
                                             const shu::vec3f &PointB, const shu::vec3f &PointC);
    void BuildTetrahedron(const shu::vec3f *Vertices, const i32 VertexCount,
                          stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris);
    void ExpandConvexHull(stack_array<shu::vec3f> &HullPoints, stack_array<tri_t> &HullTris,
                          const shu::vec3f *Vertices, const i32 VertexCount);
    // NOTE: Remove any points inside the tetrahedron from the lists.
    void RemoveInternalVertices(const stack_array<shu::vec3f> &HullPoints,
                                const stack_array<tri_t> &HullTris, stack_array<shu::vec3f> &Vertices);
    // NOTE: This will compare the incoming edge with all the edges in the facing tris and then return true if it's
    // unique.
    b32 IsEdgeUnique(const tri_t *Tris, const stack_array<i32> &FacingTris, const i32 IgnoreTri, const edge_t &Edge);
    void AddPoint(stack_array<shu::vec3f> &HullPoints,
                  stack_array<tri_t> &HullTris, const shu::vec3f &Point);
    void RemoveUnreferencedVertices(stack_array<shu::vec3f> &HullPoints,
                                    stack_array<tri_t> &HullTris);
    void BuildConvexHull(const shu::vec3f *Vertices, const i32 VertexCount, stack_array<shu::vec3f> &HullPoints,
                         stack_array<tri_t> &HullTris);
    b32 IsExternal(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris, const shu::vec3f &Point);
    shu::vec3f CalculateCenterOfMass(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris);
    shu::mat3f CalculateInertiaTensor(const stack_array<shu::vec3f> &Points, const stack_array<tri_t> &Tris);
};

#define SHAPE_H
#endif // SHAPE_H