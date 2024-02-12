#if !defined(GEOMETRY_PRIMITIVE_H)

#include <defines.h>
#include <mesh/mesh_filter.h>
#include <renderer/vulkan/vulkan_buffer.h>

#define CIRCLE_PRIMITIVE_RESOLUTION 40

struct shoora_primitive_collection
{
    shoora_primitive_collection() {}

    static void GenerateCircleMesh(struct shoora_mesh *Mesh, shoora_vertex_info *VerticesMemory,
                                   u32 *IndicesMemory, u32 &RunningVertexCount, u32 &RunningIndexCount);

    static void GenerateUVSphereMesh();
    static shoora_mesh_filter GetPrimitiveInfo(u32 Type);
    static i32 GetTotalVertexCount();
    static i32 GetTotalIndexCount();

    static void Cleanup();
};

#define GEOMETRY_PRIMITIVE_H
#endif // GEOMETRY_PRIMITIVE_H