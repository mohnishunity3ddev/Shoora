#if !defined(MESH_H)

#define SHU_CUSTOM_IMPLEMENTATION 0
// #undef SHU_CUSTOM_IMPLEMENTATION

#include <defines.h>
#include <loaders/image/image_loader.h>
#include <math/math.h>
#include <volk/volk.h>

enum shoora_model_alpha_mode
{
    AlphaMode_Opaque,
    AlphaMode_Mask,
    AlphaMode_Blend,
    AlphaMode_MaxCount,
};

struct shoora_model_material
{
    Shu::vec4f BaseColorFactor;
    i32 BaseColorTextureIndex = -1;
    i32 NormalTextureIndex = -1;
    i32 MetallicTextureIndex = -1;

    f32 AlphaCutoff;
    b32 DoubleSided;

    shoora_model_alpha_mode AlphaMode;

    VkDescriptorSet DescriptorSet;
    VkPipeline Pipeline;
};

struct shoora_model_texture
{
    const char *ImageFilename;
    shoora_image_data ImageData;
};

struct shoora_mesh_primitive
{
    u32 FirstIndex;
    u32 IndexCount;
    i32 MaterialIndex;
};

struct shoora_model_mesh
{
    shoora_mesh_primitive *Primitives;
    u32 PrimitiveCount;
};

struct shoora_model_node
{
    const char *Name;
    shoora_model_node *ParentNode;

    shoora_model_node **ChildNodes;
    u32 ChildrenCount;

    Shu::mat4f ModelMatrix;
    shoora_model_mesh Mesh;
};

struct shoora_model
{
    shoora_model_texture *Textures;
    u32 TextureCount;
    size_t TotalTextureSize;

    shoora_model_material *Materials;
    u32 MaterialCount;

    shoora_model_node **Nodes;
    u32 NodeCount;

    struct shoora_vertex_info *Vertices;
    u32 VertexCount;

    u32 *Indices;
    u32 IndicesCount;
};

void LoadModel(shoora_model *Model, const char *Path);
void CleanupModelResources(shoora_model *Model);

#define MESH_H
#endif // MESH_H