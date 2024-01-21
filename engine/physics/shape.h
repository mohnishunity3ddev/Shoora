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

    // shoora_shape() = default;
    shoora_shape(shoora_primitive_type Type);

    virtual ~shoora_shape() = default;
    virtual f32 GetMomentOfInertia() const = 0;
    virtual Shu::vec3f GetDim() const = 0;
    virtual shoora_primitive_type GetType() const = 0;

    void Draw(const VkCommandBuffer &cmdBuffer);
};

struct shoora_shape_polygon : shoora_shape
{
    Shu::vec3f *WorldVertices;
    u32 VertexCount;

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

struct shoora_shape_triangle : shoora_shape
{
    shoora_shape_triangle() = delete;
    shoora_shape_triangle(u32 Base, u32 Height);
    virtual ~shoora_shape_triangle();

    u32 Base, Height;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
    virtual shoora_primitive_type GetType() const override;
};

#define SHAPE_H
#endif // SHAPE_H