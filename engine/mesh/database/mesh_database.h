#if !defined(MESH_DATABASE_H)

#include <defines.h>

#include <math/math.h>
#include <mesh/mesh_filter.h>
#include <renderer/vulkan/vulkan_renderer.h>
#include <renderer/vulkan/vulkan_vertex_definitions.h>

#define MAX_MESH_COUNT 256

enum shoora_mesh_type
{
    MESH_INVALID,
    LINE,
    POLYGON_2D,
    RECT_2D,
    CIRCLE,
    CUBE,
    SPHERE,
    CONVEX,
    CONVEX_DIAMOND,
    TRIANGLE,
    MAX_COUNT
};

struct shoora_mesh_info
{
    u32 IndexCount;
    i32 IndexOffset;
    i32 VertexOffset;
};

struct shoora_mesh
{
    shoora_mesh_filter MeshFilter;
    i32 VertexOffset, IndexOffset;
    shoora_mesh_type Type;

    shoora_mesh_info GetInfo()
    {
        shoora_mesh_info Result;
        Result.IndexCount = this->MeshFilter.IndexCount;
        Result.IndexOffset = this->IndexOffset;
        Result.VertexOffset = this->VertexOffset;
        return Result;
    }
};

struct shoora_mesh_database
{
    static void Destroy();

    static void MeshDbBegin(shoora_vulkan_device *RenderDevice);
    static void AddPolygonMeshToDb(const shu::vec3f *vPositions, i32 vCount);
    static void MeshDbEnd();

    static shoora_mesh *GetMesh(shoora_mesh_type Type);
    static shoora_mesh_filter *GetMeshFilter(shoora_mesh_type Type);
    static shoora_mesh_filter *GetCustomMeshFilter(i32 MeshId);

    static VkBuffer *GetVertexBufferHandlePtr();
    static VkBuffer GetIndexBufferHandle();
};

#define MESH_DATABASE_H
#endif // MESH_DATABASE_H