#if !defined(SHAPE_H)

#include <defines.h>
#include <mesh/database/mesh_database.h>
#include <math/math.h>
#include "bounds.h"

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

    virtual ~shoora_shape() = default;
    virtual Shu::mat3f InertiaTensor() const = 0;
    virtual Shu::vec3f GetDim() const = 0;
    virtual shoora_mesh_type GetType() const = 0;
    virtual Shu::vec3f GetCenterOfMass() { return CenterOfMass; }

    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const = 0;
    virtual shoora_bounds GetBounds() const = 0;

    shoora_mesh_filter *GetMeshFilter();

  protected:
    Shu::vec3f CenterOfMass;
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
};

struct shoora_shape_cube : shoora_shape
{
    shoora_shape_cube(u32 Width, u32 Height, u32 Depth);
    virtual ~shoora_shape_cube();

    u32 Width, Height, Depth;
    virtual Shu::mat3f InertiaTensor() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_mesh_type GetType() const override;
    virtual shoora_bounds GetBounds(const Shu::vec3f &Pos, const Shu::quat &Orientation) const override;
    virtual shoora_bounds GetBounds() const override;
};

#define SHAPE_H
#endif // SHAPE_H