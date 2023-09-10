#include "mesh.h"

#ifndef CGLTF_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#endif
#include "meshloader/cgltf.h"

#ifndef FAST_OBJ_IMPLEMENTATION
#define FAST_OBJ_IMPLEMENTATION
#endif
#include "meshloader/fast_obj.h"

void
LoadMesh(const char *Path)
{
    cgltf_options Options = {};
    cgltf_data *MeshData = nullptr;
    cgltf_result Result = cgltf_parse_file(&Options, Path, &MeshData);

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
    if (Result != cgltf_result_success)
    {
        ASSERT(!"Validation Failed!");
    }

    for(cgltf_size MeshIndex = 0;
        MeshIndex < MeshData->meshes_count;
        ++MeshIndex)
    {
        const cgltf_mesh *Mesh = &MeshData->meshes[MeshIndex];

        for(cgltf_size PrimitiveIndex = 0;
            PrimitiveIndex < Mesh->primitives_count;
            ++PrimitiveIndex)
        {
            const cgltf_primitive *Primitive = &Mesh->primitives[PrimitiveIndex];

            // Access vertex data (positions, normals, etc.)
            const cgltf_accessor *PosAccessor = Primitive->attributes[0].data;

            // Access indices (if present)
            const cgltf_accessor *IndexAccessor = Primitive->indices;

            // Access material data
            const cgltf_material *Material = Primitive->material;
        }
    }

    for(u32 TextureIndex = 0; TextureIndex < MeshData->textures_count; ++TextureIndex)
    {
        cgltf_texture *Texture = &MeshData->textures[TextureIndex];

        cgltf_image *Image = Texture->image;

        if(Image != nullptr)
        {
            if(strcmp(Image->mime_type, "image/jpeg") == 0)
            {
                char *Uri = Image->uri;
                int Width, Height, BytesPerPixel;
            }
            else if(strcmp(Image->mime_type, "image/png") == 0)
            {
                char *Uri = Image->uri;
                int Width, Height, BytesPerPixel;
            }
        }
    }

    Free((void **)&MeshData);
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
