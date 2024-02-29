#if !defined(MESH_LOADER_H)

#define SHU_CUSTOM_IMPLEMENTATION 0
// #undef SHU_CUSTOM_IMPLEMENTATION

#include <defines.h>
#include <loaders/image/image_loader.h>
#include <math/math.h>
#include <volk/volk.h>
#include <mesh/mesh_filter.h>

enum shoora_mesh_alpha_mode
{
    AlphaMode_Opaque,
    AlphaMode_Mask,
    AlphaMode_Blend,
    AlphaMode_MaxCount,
};

struct shoora_mesh_material
{
    shu::vec4f BaseColorFactor;
    i32 BaseColorTextureIndex = -1;
    i32 NormalTextureIndex = -1;
    i32 MetallicTextureIndex = -1;

    f32 AlphaCutoff;
    b32 DoubleSided;

    shoora_mesh_alpha_mode AlphaMode;

    VkDescriptorSet DescriptorSet;
    VkPipeline Pipeline;
};

struct shoora_mesh_texture
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

struct shoora_mesh_cgltf_node
{
    const char *Name;
    shoora_mesh_cgltf_node *ParentNode;

    shoora_mesh_cgltf_node **ChildNodes;
    u32 ChildrenCount;

    shu::mat4f ModelMatrix;
    shoora_mesh_primitive *Primitives;
    u32 PrimitiveCount;
};

struct shoora_model
{
    shoora_mesh_texture *Textures;
    u32 TextureCount;
    size_t TotalTextureSize;

    shoora_mesh_material *Materials;
    u32 MaterialCount;

    shoora_mesh_cgltf_node **Nodes;
    u32 NodeCount;

    shoora_mesh_filter MeshFilter;
};

void LoadModel(shoora_model *Model, const char *Path);
void CleanupModelResources(shoora_model *Model);

#define MESH_LOADER_H
#endif // MESH_LOADER_H