#if !defined(SHAPE_H)

#include <defines.h>
#include <mesh/database/mesh_database.h>
#include <math/math.h>
#include <containers/dynamic_array.h>
#include "bounds.h"
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

    virtual void Build(const Shu::vec3f *Points, const i32 Num) {}

    virtual ~shoora_shape() = default;
    virtual Shu::mat3f InertiaTensor() const = 0;
    virtual Shu::vec3f GetDim() const = 0;
    virtual shoora_mesh_type GetType() const = 0;
    virtual Shu::vec3f GetCenterOfMass() { return mCenterOfMass; }

    // NOTE: The support point of the shape is a point on the shape that is the furthest away in a particular
    // direction. Used in the GJK algorithm used in collision detection for convex shapes.
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const = 0;

    // NOTE: FastestLinearSpeed is used in continuous collision detection. It's necessary for objects that are
    // "long". Since "long" objects can rotate and still hit objects even though they have ZERO Linear Velocity.
    // Sphere's on the other hand are not "long" since they are equally "long" in all directions. So if they have
    // zero linear velocity, we dont have to calculate their linear velocity bound since just them rotating will
    // not collide with other objects just because of its angular velocity.
    virtual f32
    FastestLinearSpeed(const Shu::vec3f &AngularVelocity, const Shu::vec3f &Direction) const { return 0.0f; }

    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const = 0;
    virtual shoora_bounds GetBounds() const = 0;

    shoora_mesh_filter *GetMeshFilter();

  protected:
    Shu::vec3f mCenterOfMass;
};

struct shoora_shape_polygon : shoora_shape
{
    Shu::vec3f *WorldVertices;

    u32 VertexCount;
    i32 MeshId = -1;
    f32 Scale;

    // NOTE: This constructor is for use for any random polygon shape.
    shoora_shape_polygon(i32 MeshId, f32 Scale = 1.0f);

    // NOTE: this constructor is for use by rects
    shoora_shape_polygon(shoora_mesh_type Type);
    virtual ~shoora_shape_polygon();

    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;

    f32 FindMinSeparation(shoora_shape_polygon *Other, i32 &ReferenceEdgeIndex, Shu::vec2f &SupportPoint);

    Shu::vec3f GetEdgeAt(i32 index);

    // NOTE: The incident edge on this shape is the one whose normal is the one which most opposes the passed in
    // reference edge's normal. In other words the incident edge has the WORST alignment with the reference edge.
    i32 GetIncidentEdgeIndex(const Shu::vec2f &ReferenceEdgeNormal);
    i32 GetNextVertexIndex(i32 EdgeIndex);
    i32 ClipSegmentToLine(Shu::vec2f Contacts[2], Shu::vec2f Clipped[2], Shu::vec2f r0, Shu::vec2f r1);
    void UpdateWorldVertices(Shu::mat4f &ModelMatrix);
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_circle : shoora_shape
{
    // shoora_shape_circle() = delete;
    shoora_shape_circle(f32 Radius);
    virtual ~shoora_shape_circle();

    f32 Radius;
    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_sphere : shoora_shape
{
    // shoora_shape_circle() = delete;
    shoora_shape_sphere(f32 Radius);
    virtual ~shoora_shape_sphere();

    f32 Radius;
    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual Shu::vec3f Support(const Shu::vec3f &DirectionNormalized, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_box : shoora_shape_polygon
{
    // shoora_shape_box() = delete;
    shoora_shape_box(u32 Width, u32 Height);
    virtual ~shoora_shape_box();

    u32 Width, Height;
    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;
};

struct shoora_shape_cube : shoora_shape
{
  public:
    shoora_shape_cube() = delete;
    explicit shoora_shape_cube(u32 Width, u32 Height, u32 Depth);
    explicit shoora_shape_cube(const Shu::vec3f *Points, const i32 Num);
    virtual void Build(const Shu::vec3f *Points, const i32 Num) override;

    virtual ~shoora_shape_cube();

    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;

    // NOTE: Takes in a direction, and returns the position of the vertex in the shape, which is the furthest in
    // this direction.
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;

    // NOTE: To be used in CCD. Takes in Angular Velocity of the object and the Direction and returns the max
    // velocity of the vertex travelling the fastest in this Direction.
    virtual f32 FastestLinearSpeed(const Shu::vec3f &AngularVelocity, const Shu::vec3f &Direction) const override;

  public:
    u32 Width, Height, Depth;
    Shu::vec3f mPoints[8];
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
    explicit shoora_shape_convex(const Shu::vec3f *Points, const i32 Num);
    explicit shoora_shape_convex(platform_work_queue *Queue, const Shu::vec3f *Points, const i32 Num);
    virtual void Build(const Shu::vec3f *Points, const i32 Num) override;

    virtual ~shoora_shape_convex();

    virtual Shu::mat3f InertiaTensor() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
    virtual Shu::vec3f GetDim() const override { return Shu::Vec3f(); }
    // NOTE: Takes in a direction, and returns the position of the vertex in the shape, which is the furthest in
    // this direction.
    virtual Shu::vec3f Support(const Shu::vec3f &Direction, const Shu::vec3f &Position,
                               const Shu::quat &Orientation, const f32 Bias) const override;
    // NOTE: To be used in CCD. Takes in Angular Velocity of the object and the Direction and returns the max
    // velocity of the vertex travelling the fastest in this Direction.
    virtual f32 FastestLinearSpeed(const Shu::vec3f &AngularVelocity, const Shu::vec3f &Direction) const override;

  public:
    shoora_dynamic_array<Shu::vec3f> mPoints;
    shoora_bounds mBounds;
    Shu::mat3f mInertiaTensor;

  private:
    i32 FindPointFurthestInDirection(const Shu::vec3f *Points, const i32 Num, const Shu::vec3f &Direction);
    // NOTE: A and B describe the end points of the line.
    f32 DistanceFromLine(const Shu::vec3f &A, const Shu::vec3f &B, const Shu::vec3f &Point);
    Shu::vec3f FindPointFurthestFromLine(const Shu::vec3f *Points, const i32 Num, const Shu::vec3f &PointA,
                                         const Shu::vec3f &PointB);
    f32 DistanceFromTriangle(const Shu::vec3f &A, const Shu::vec3f &B, const Shu::vec3f &C,
                             const Shu::vec3f &Point);
    Shu::vec3f FindPointFurthestFromTriangle(const Shu::vec3f *Points, const i32 Num, const Shu::vec3f &PointA,
                                             const Shu::vec3f &PointB, const Shu::vec3f &PointC);
    void BuildTetrahedron(const Shu::vec3f *Vertices, const i32 VertexCount,
                          arr<Shu::vec3f> &HullPoints, arr<tri_t> &HullTris);
    void ExpandConvexHull(arr<Shu::vec3f> &HullPoints, arr<tri_t> &HullTris,
                          const shoora_dynamic_array<Shu::vec3f> &Vertices);
    // NOTE: Remove any points inside the tetrahedron from the lists.
    void RemoveInternalVertices(const arr<Shu::vec3f> &HullPoints,
                                const arr<tri_t> &HullTris, arr<Shu::vec3f> &Vertices);
    // NOTE: This will compare the incoming edge with all the edges in the facing tris and then return true if it's
    // unique.
    b32 IsEdgeUnique(const tri_t *Tris, const arr<i32> &FacingTris, const i32 IgnoreTri, const edge_t &Edge);
    void AddPoint(arr<Shu::vec3f> &HullPoints,
                  arr<tri_t> &HullTris, const Shu::vec3f &Point);
    void RemoveUnreferencedVertices(arr<Shu::vec3f> &HullPoints,
                                    arr<tri_t> &HullTris);
    void BuildConvexHull(const shoora_dynamic_array<Shu::vec3f> &Vertices, arr<Shu::vec3f> &HullPoints,
                         arr<tri_t> &HullTris);
    b32 IsExternal(const arr<Shu::vec3f> &Points, const arr<tri_t> &Tris, const Shu::vec3f &Point);
    Shu::vec3f CalculateCenterOfMass(const arr<Shu::vec3f> &Points, const arr<tri_t> &Tris);
    Shu::mat3f CalculateInertiaTensor(const arr<Shu::vec3f> &Points, const arr<tri_t> &Tris);
};

#define SHAPE_H
#endif // SHAPE_H