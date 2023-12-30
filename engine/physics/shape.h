#if !defined(SHAPE_H)

#include <defines.h>
#include <mesh/primitive/geometry_primitive.h>

#define MOMENT_OF_INTERTIA_FUNC(name) f32 name()
typedef f32 getInertiaFunc();

struct shoora_shape
{
    shoora_primitive *Primitive;
    shoora_primitive_type Type;

    shoora_shape() = default;
    shoora_shape(shoora_primitive_type Type);

    virtual f32 GetMomentOfInertia() const = 0;
    virtual Shu::vec3f GetDim() const = 0;
    void Draw(const VkCommandBuffer &cmdBuffer);
};

struct shoora_shape_circle : shoora_shape
{
    shoora_shape_circle() = delete;
    shoora_shape_circle(u32 Radius);

    u32 Radius;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
};

struct shoora_shape_box : shoora_shape
{
    shoora_shape_box() = delete;
    shoora_shape_box(u32 Width, u32 Height);

    u32 Width, Height;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
};

struct shoora_shape_triangle : shoora_shape
{
    shoora_shape_triangle() = delete;
    shoora_shape_triangle(u32 Base, u32 Height);

    u32 Base, Height;
    virtual f32 GetMomentOfInertia() const override;
    virtual Shu::vec3f GetDim() const override;
};

#define SHAPE_H
#endif // SHAPE_H