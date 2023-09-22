#include "mesh.h"

#include <platform/platform.h>
#include <utils/utils.h>

#ifndef CGLTF_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#endif
#include "meshloader/cgltf.h"
#include <renderer/vulkan/vulkan_vertex_definitions.h>

#if 0
#ifndef FAST_OBJ_IMPLEMENTATION
#define FAST_OBJ_IMPLEMENTATION
#endif
#include "meshloader/fast_obj.h"
#endif

void
GetBasePath(const char *Path, char *BasePath)
{
    u32 Pos = StringFindLastOf(Path, '/');
    StringSubString(Path, 0, Pos, BasePath);
}

i32
GetImageIndex(const char *ImageFilename, shoora_model *Model)
{
    i32 Result = -1;

    for(u32 Index = 0;
        Index < Model->TextureCount;
        ++Index)
    {
        const char *LoadedImageFilename = Model->Textures[Index].ImageFilename;

        if(StringsEqual(ImageFilename, LoadedImageFilename))
        {
            Result = Index;
            break;
        }
    }

    ASSERT(Result != -1);
    return Result;
}

void
Free(void **Data)
{
    if (*Data != nullptr)
    {
        cgltf_free((cgltf_data *)*Data);
        *Data = nullptr;
    }
}

void
LoadTextureImages(cgltf_image *Images, u32 ImageCount, const char *BasePath, shoora_model *Model)
{
    Model->TextureCount = ImageCount;
    Model->Textures = (shoora_model_texture *)malloc(sizeof(shoora_model_texture)*ImageCount);
    memset(Model->Textures, 0, sizeof(shoora_model_texture) * ImageCount);
    LogInfoUnformatted("Images are: \n");

    for(u32 Index = 0;
        Index < ImageCount;
        ++Index)
    {
        shoora_model_texture *Tex = Model->Textures + Index;
        Tex->ImageFilename = Images[Index].uri;

        char ImagePath[512];
        StringConcat(BasePath, Images[Index].uri, ImagePath);

        // TODO)): Load the Image Data.
        LogInfo("Image[%d]: %s \n", Index, ImagePath);

        Tex->ImageData = LoadImageFile(ImagePath);
        Model->TotalTextureSize += Tex->ImageData.TotalSize;
    }
}

u32
GetMaterialIndex(cgltf_material *glTFMaterials, cgltf_material *PrimitiveMaterial)
{
    u32 Result = (u32)(PrimitiveMaterial - glTFMaterials);
    return Result;
}

void
LoadMaterials(cgltf_material *Materials, u32 MaterialCount, shoora_model *Model)
{
    Model->Materials = (shoora_model_material *)malloc(sizeof(shoora_model_material)*MaterialCount);
    Model->MaterialCount = MaterialCount;

    for(u32 Index = 0;
        Index < MaterialCount;
        ++Index)
    {
        cgltf_material *GLTFMat = Materials + Index;
        shoora_model_material *Mat = Model->Materials + Index;

        if(GLTFMat->has_pbr_metallic_roughness &&
           GLTFMat->pbr_metallic_roughness.base_color_texture.texture != nullptr)
        {
            const char* BaseColorTexUri = GLTFMat->pbr_metallic_roughness.base_color_texture.texture->image->uri;
            ASSERT(BaseColorTexUri);
            Mat->BaseColorTextureIndex = GetImageIndex(BaseColorTexUri, Model);
        }
        else
        {
            Mat->BaseColorTextureIndex = -1;
        }

        if(GLTFMat->normal_texture.texture != nullptr)
        {
            const char *NormalTexUri = GLTFMat->normal_texture.texture->image->uri;
            ASSERT(NormalTexUri);
            Mat->NormalTextureIndex = GetImageIndex(NormalTexUri, Model);
        }
        else
        {
            Mat->NormalTextureIndex = -1;
        }

        Mat->AlphaCutoff = GLTFMat->alpha_cutoff;
        Mat->AlphaMode = (shoora_model_alpha_mode)GLTFMat->alpha_mode;
        Mat->DoubleSided = GLTFMat->double_sided;
        // Mat->BaseColorFactor = GLTFMat->BaseColorFactor;
    }
}

void
GetTotalVertexIndicesCount(const cgltf_scene *pScenes, const u32 SceneCount,
                           u32 *pVertexCount, u32 *pIndexCount, u32 *pNodeCount)
{
    for(u32 SceneIndex = 0;
        SceneIndex < SceneCount;
        ++SceneIndex)
    {
        const cgltf_scene *Scene = pScenes + SceneIndex;

        *pNodeCount += Scene->nodes_count;
        for(u32 NodeIndex = 0;
            NodeIndex < Scene->nodes_count;
            ++NodeIndex)
        {
            cgltf_node *Node = Scene->nodes[NodeIndex];
            cgltf_mesh *NodeMesh = Node->mesh;
            *pNodeCount += Node->children_count;

            for(u32 PrimitiveIndex = 0;
                PrimitiveIndex < NodeMesh->primitives_count;
                ++PrimitiveIndex)
            {
                cgltf_primitive *Primitive = NodeMesh->primitives + PrimitiveIndex;

                u32 AttributeCount = Primitive->attributes_count;
                for(u32 AttrIndex = 0;
                    AttrIndex < AttributeCount;
                    ++AttrIndex)
                {
                    const cgltf_attribute *Attribute = &Primitive->attributes[AttrIndex];
                    if(Attribute->type == cgltf_attribute_type_position)
                    {
                        u32 VertexCount = Attribute->data->count;
                        *pVertexCount += VertexCount;
                        break;
                    }
                }

                *pIndexCount += Primitive->indices->count;
            }
        }
    }
}

void
LoadGLTFNode(cgltf_node *InputNode, cgltf_material *glTFMaterials, shoora_model *Model, shoora_model_node *Parent)
{
    cgltf_mesh *NodeMesh = InputNode->mesh;

    shoora_model_node *CurrentNode = (shoora_model_node *)malloc(sizeof(shoora_model_node));
    memset(CurrentNode, 0, sizeof(shoora_model_node));

    CurrentNode->Name = InputNode->name;
    CurrentNode->ParentNode = Parent;

    CurrentNode->ModelMatrix = Shu::Mat4(1.0f);
    if(InputNode->has_scale)
    {
        CurrentNode->ModelMatrix = Shu::Scale(CurrentNode->ModelMatrix, Shu::MakeVec3(InputNode->scale));
    }
    if(InputNode->has_rotation)
    {
        // TODO)): Make quaternion support since gltf contains quaternions for rotation data.
        i32 x = 0;
    }
    if(InputNode->has_translation)
    {
        CurrentNode->ModelMatrix = Shu::Translate(CurrentNode->ModelMatrix, Shu::MakeVec3(InputNode->translation));
    }
    if(InputNode->has_matrix)
    {
        // TODO)): Read more about what does this? Do I need to calculate the model matrix if the gltf input contains a matrix?
        i32 x = 0;
    }

    if(InputNode->children_count > 0)
    {
        for(u32 NodeChildIndex = 0;
            NodeChildIndex < InputNode->children_count;
            ++NodeChildIndex)
        {
            cgltf_node *ChildNode = InputNode->children[NodeChildIndex];
            LoadGLTFNode(ChildNode, glTFMaterials, Model, CurrentNode);
        }
    }

    CurrentNode->Mesh.PrimitiveCount = NodeMesh->primitives_count;
    CurrentNode->Mesh.Primitives = (shoora_mesh_primitive *)malloc(CurrentNode->Mesh.PrimitiveCount *
                                                                   sizeof(shoora_mesh_primitive));
    memset(CurrentNode->Mesh.Primitives, 0, sizeof(shoora_mesh_primitive));

    for(u32 PrimitiveIndex = 0;
        PrimitiveIndex < NodeMesh->primitives_count;
        ++PrimitiveIndex)
    {
        cgltf_primitive *Primitive = NodeMesh->primitives + PrimitiveIndex;
        ASSERT(Primitive->type == cgltf_primitive_type_triangles);

        u32 VertexStart = Model->VertexCount;

        // NOTE: Vertices
        {
            const f32 *PositionBuffer = nullptr;
            const f32 *NormalsBuffer = nullptr;
            const f32 *ColorsBuffer = nullptr;
            const f32 *TexCoordsBuffer = nullptr;
            const f32 *TangentsBuffer = nullptr;

            u64 VertexCount = 0;

            u32 AttributeCount = Primitive->attributes_count;
            for(u32 AttrIndex = 0;
                AttrIndex < AttributeCount;
                ++AttrIndex)
            {
                const cgltf_attribute *Attribute = &Primitive->attributes[AttrIndex];
                switch (Attribute->type)
                {
                    case cgltf_attribute_type_position:
                    {
                        cgltf_accessor *Accessor = Attribute->data;
                        cgltf_buffer_view *BufferView = Accessor->buffer_view;

                        u8 *BuffData = (u8 *)BufferView->buffer->data;
                        PositionBuffer = (const f32 *)(BuffData + (Accessor->offset + BufferView->offset));
                        VertexCount = Accessor->count;
                    }
                    break;

                    case cgltf_attribute_type_normal:
                    {
                        cgltf_accessor *Accessor = Attribute->data;
                        cgltf_buffer_view *BufferView = Accessor->buffer_view;

                        u8 *BuffData = (u8 *)BufferView->buffer->data;
                        NormalsBuffer = (const f32 *)(BuffData + (Accessor->offset + BufferView->offset));
                    } break;

                    case cgltf_attribute_type_color:
                    {
                        cgltf_accessor *Accessor = Attribute->data;
                        cgltf_buffer_view *BufferView = Accessor->buffer_view;

                        u8 *BuffData = (u8 *)BufferView->buffer->data;
                        ColorsBuffer = (const f32 *)(BuffData + (Accessor->offset + BufferView->offset));
                    } break;

                    case cgltf_attribute_type_tangent:
                    {
                        cgltf_accessor *Accessor = Attribute->data;
                        cgltf_buffer_view *BufferView = Accessor->buffer_view;

                        u8 *BuffData = (u8 *)BufferView->buffer->data;
                        TangentsBuffer = (const f32 *)(BuffData + (Accessor->offset + BufferView->offset));
                    } break;

                    case cgltf_attribute_type_texcoord:
                    {
                        cgltf_accessor *Accessor = Attribute->data;
                        cgltf_buffer_view *BufferView = Accessor->buffer_view;

                        u8 *BuffData = (u8 *)BufferView->buffer->data;
                        TexCoordsBuffer = (const f32 *)(BuffData + (Accessor->offset + BufferView->offset));
                    } break;

                    SHU_INVALID_DEFAULT
                }
            }

            for(u32 VertexIndex = 0;
                VertexIndex < VertexCount;
                ++VertexIndex)
            {
                shoora_vertex_info Vert = {};
                Vert.Pos        = Shu::MakeVec3(&PositionBuffer[VertexIndex*3]);
                Vert.Normal     = Shu::MakeVec3(&NormalsBuffer[VertexIndex*3]);
                Vert.Color      = ColorsBuffer ? Shu::MakeVec3(&ColorsBuffer[VertexIndex*3]) : Shu::Vec3(1.0f);
                Vert.UV         = TexCoordsBuffer ? Shu::MakeVec2(&TexCoordsBuffer[VertexIndex*2]) : Shu::Vec2(0.0f);
                Vert.Tangent    = TangentsBuffer ? Shu::MakeVec4(&TangentsBuffer[VertexIndex*4]) : Shu::Vec4(0.0f);

                Model->Vertices[Model->VertexCount++] = Vert;
            }
        }

        // NOTE: Indices
        {
            u32 FirstIndex = Model->IndicesCount;

            cgltf_accessor *IndexAccessor = Primitive->indices;
            cgltf_buffer_view *IndexBufferView = IndexAccessor->buffer_view;
            cgltf_buffer *IndexBuffer = IndexBufferView->buffer;

            switch(IndexAccessor->component_type)
            {
                case cgltf_component_type_r_32u:
                {
                    const u32 *Buf = (u32 *)((u8 *)IndexBuffer->data +
                                            IndexAccessor->offset + IndexBufferView->offset);
                    for(u64 Index = 0;
                        Index < IndexAccessor->count;
                        ++Index)
                    {
                        Model->Indices[Model->IndicesCount++] = Buf[Index] + VertexStart;
                    }
                } break;

                case cgltf_component_type_r_16u:
                {
                    const u16 *Buf = (u16 *)((u8 *)IndexBuffer->data + IndexAccessor->offset +
                                            IndexBufferView->offset);
                    for(u64 Index = 0;
                        Index < IndexAccessor->count;
                        ++Index)
                    {
                        Model->Indices[Model->IndicesCount++] = (u32)(Buf[Index] + VertexStart);
                    }
                } break;

                case cgltf_component_type_r_8u:
                {
                    const u8 *Buf = (u8 *)((u8 *)IndexBuffer->data + IndexAccessor->offset +
                                        IndexBufferView->offset);
                    for(u64 Index = 0;
                        Index < IndexAccessor->count;
                        ++Index)
                    {
                        Model->Indices[Model->IndicesCount++] = (u32)(Buf[Index] + VertexStart);
                    }
                } break;

                SHU_INVALID_DEFAULT
            }

            shoora_mesh_primitive MeshPrimitive;
            MeshPrimitive.FirstIndex = FirstIndex;
            MeshPrimitive.IndexCount = IndexAccessor->count;
            // TODO)): This is remaining!
            MeshPrimitive.MaterialIndex = GetMaterialIndex(glTFMaterials, Primitive->material);
            CurrentNode->Mesh.Primitives[PrimitiveIndex] = MeshPrimitive;
        }
    }
    
    if(Parent)
    {
        Parent->ChildNodes[Parent->ChildrenCount++] = CurrentNode;
    }
    else
    {
        Model->Nodes[Model->NodeCount++] = CurrentNode;
    }

#if 0
    for(u32 Index = 0;
        Index < Model->IndicesCount;
        ++Index)
    {
        u32 VertIndex = Model->Indices[Index];
        ASSERT(VertIndex < Model->VertexCount);
    }
#endif
}

void
LoadModel(shoora_model *Model, const char *Path)
{
    cgltf_options Options = {};
    cgltf_data *MeshData = nullptr;
    cgltf_result Result = cgltf_parse_file(&Options, Path, &MeshData);
    *Model = {};

    if(Result != cgltf_result_success)
    {
        Free((void **)&MeshData);
        ASSERT(!"Could not load the mesh file!");
    }

    Result = cgltf_load_buffers(&Options, MeshData, Path);
    if(Result != cgltf_result_success)
    {
        ASSERT(!"Loading Fault!");
    }

    Result = cgltf_validate(MeshData);
    if(Result != cgltf_result_success)
    {
        ASSERT(!"Validation Failed!");
    }

    char BasePath[512];
    GetBasePath(Path, BasePath);
    LoadTextureImages(MeshData->images, MeshData->images_count, BasePath, Model);
    LoadMaterials(MeshData->materials, MeshData->materials_count, Model);

    u32 TotalVerticesCount = 0;
    u32 TotalIndicesCount = 0;
    u32 TotalNodesCount = 0;
    GetTotalVertexIndicesCount(MeshData->scenes, MeshData->scenes_count,
                               &TotalVerticesCount, &TotalIndicesCount, &TotalNodesCount);
    Model->Vertices = (shoora_vertex_info *)malloc(TotalVerticesCount * sizeof(shoora_vertex_info));
    memset(Model->Vertices, 0, sizeof(shoora_vertex_info)*TotalVerticesCount);
    Model->Indices = (u32 *)malloc(TotalIndicesCount * sizeof(u32));
    memset(Model->Indices, 0, sizeof(u32)*TotalIndicesCount);
    Model->Nodes = (shoora_model_node **)malloc(TotalNodesCount*sizeof(shoora_model_node *));
    memset(Model->Nodes, 0, sizeof(shoora_model_node *)*TotalNodesCount);

    for(u32 SceneIndex = 0;
        SceneIndex < MeshData->scenes_count;
        ++SceneIndex)
    {
        cgltf_scene *Scene = MeshData->scene + SceneIndex;
        for(u32 NodeIndex = 0;
            NodeIndex < Scene->nodes_count;
            ++NodeIndex)
        {
            cgltf_node *Node = Scene->nodes[NodeIndex];
            LoadGLTFNode(Node, MeshData->materials, Model, nullptr);
        }
    }

    ASSERT(Model->VertexCount == TotalVerticesCount);
    ASSERT(Model->IndicesCount == TotalIndicesCount);
    ASSERT(Model->NodeCount == TotalNodesCount);
}

void
CleanupModelResources(shoora_model *Model)
{
    // TODO)): To be Implemented!
}
