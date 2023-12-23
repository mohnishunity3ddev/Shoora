#if !defined(MESH_FILTER_H)

#include <defines.h>

struct shoora_mesh_filter
{
    struct shoora_vertex_info *Vertices;
    u32 VertexCount;
    u32 *Indices;
    u32 IndexCount;
};

#define MESH_FILTER_H
#endif