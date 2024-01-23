#if !defined(SHAPE_H)

#include <defines.h>
#include <mesh/primitive/geometry_primitive.h>
#include <math/math.h>

#define MOMENT_OF_INTERTIA_FUNC(name) f32 name()
typedef f32 getInertiaFunc();

struct shoora_shape
{
    shoora_primitive *Primitive;

    shoora_primitive_type Type;
    b32 isPrimitive;

    // shoora_shape() = default;
    shoora_shape(shoora_primitive_type Type, b32 isPrimitive = true);

    virtual ~shoora_shape() = default;
    virtual f32 GetMomentOfInertia() const = 0;
    virtual Shu::vec3f GetDim() const = 0;
    virtual shoora_primitive_type GetType() const = 0;

    shoora_mesh_filter *GetMeshFilter();
};

struct shoora_shape_polygon : shoora_shape
{
    Shu::vec3f *WorldVertices;
    u32 VertexCount;

    // NOTE: This constructor is for use for any random polygon shape.
    shoora_shape_polygon(Shu::vec3f *LocalVertices, i32 VertexCount);

    // NOTE: this constructor is for use by rects
    shoora_shape_polygon(shoora_primitive_type Type);
    virtual ~shoora_shape_polygon();

    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_primitive_type GetType() const override;

    f32 FindMinSeparation(shoora_shape_polygon *Other, Shu::vec2f &SeparationAxis, Shu::vec2f &Point);

    Shu::vec3f GetEdgeAt(i32 index);
    void UpdateWorldVertices(Shu::mat4f &ModelMatrix);
};

struct shoora_shape_circle : shoora_shape
{
    // shoora_shape_circle() = delete;
    shoora_shape_circle(u32 Radius);
    virtual ~shoora_shape_circle();

    f32 Radius;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_primitive_type GetType() const override;
};

struct shoora_shape_box : shoora_shape_polygon
{
    // shoora_shape_box() = delete;
    shoora_shape_box(u32 Width, u32 Height);
    virtual ~shoora_shape_box();

    u32 Width, Height;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_primitive_type GetType() const override;
};

#define SHAPE_H
#endif // SHAPE_H