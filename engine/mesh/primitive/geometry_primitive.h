#if !defined(GEOMETRY_PRIMITIVE_H)

#include <defines.h>
#include <mesh/mesh_filter.h>
#include <renderer/vulkan/vulkan_buffer.h>
#include <renderer/vulkan/vulkan_renderer.h>

enum shoora_primitive_type
{
    NONE,
    LINE,
    POLYGON_2D,
    RECT_2D,
    CIRCLE,
    CUBE,
    SPHERE,
    MAX_COUNT
};

struct shoora_primitive_info
{
    u32 IndexCount;
    i32 IndexOffset;
    i32 VertexOffset;
};

struct shoora_primitive
{
    shoora_mesh_filter MeshFilter;
    u32 VertexOffset, IndexOffset;
    shoora_primitive_type PrimitiveType;

    shoora_primitive_info GetInfo();
};

struct shoora_primitive_collection
{
    shoora_primitive_collection() {}
    static void Initialize(shoora_vulkan_device *Device, u32 CircleResolution);

    static VkBuffer *GetVertexBufferHandlePtr();
    static VkBuffer GetIndexBufferHandle();
    static void Destroy();
    static shoora_primitive *GetPrimitive(shoora_primitive_type Type);

  private:
    shoora_primitive Line;
    shoora_primitive Rect2D;
    shoora_primitive Circle;
    shoora_primitive Cube;
    shoora_primitive Sphere;

    shoora_vulkan_buffer vBuffer;
    shoora_vulkan_buffer iBuffer;

    shoora_vulkan_device *RenderDevice;
};

#define GEOMETRY_PRIMITIVE_H
#endif // GEOMETRY_PRIMITIVE_H