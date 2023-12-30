#if !defined(GEOMETRY_PRIMITIVE_H)

#include <defines.h>
#include <mesh/mesh_filter.h>
#include <renderer/vulkan/vulkan_buffer.h>
#include <renderer/vulkan/vulkan_renderer.h>

enum shoora_primitive_type
{
    NONE,
    LINE,
    TRIANGLE,
    RECT_2D,
    CIRCLE,
    CUBE,
    SPHERE,
    MAX_COUNT
};

struct shoora_primitive
{
    shoora_mesh_filter MeshFilter;
    u32 VertexOffset, IndexOffset;
    shoora_primitive_type PrimitiveType;

    void Draw(const VkCommandBuffer &cmdBuffer);
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
    shoora_primitive Triangle;
    shoora_primitive Rect2D;
    shoora_primitive Circle;
    shoora_primitive Cube;
    shoora_primitive Sphere;

    shoora_vulkan_buffer vBuffer;
    shoora_vulkan_buffer iBuffer;

    shoora_vulkan_device *RenderDevice;
};

void DrawLine(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout, const Shu::vec2f P0,
              const Shu::vec2f P1, u32 ColorU32, f32 Thickness);
void DrawRect(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout, i32 X, i32 Y, u32 Width,
              u32 Height, u32 ColorU32);
void DrawSpring(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout, const Shu::vec2f &startPos,
                const Shu::vec2f &endPos, f32 restLength, f32 thickness, i32 nDivisions, u32 Color);

#define GEOMETRY_PRIMITIVE_H
#endif // GEOMETRY_PRIMITIVE_H