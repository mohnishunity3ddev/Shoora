#include "geometry_primitive.h"

#include <memory.h>
#include <renderer/vulkan/vulkan_vertex_definitions.h>

static shoora_primitive_collection PrimitiveCollection = {};

static shoora_vertex_info TriangleVertices[] =
{
    {.Pos = Shu::vec3f{ 0.0f,  0.5f, 0.0f}, .Color = Shu::vec3f{1, 0, 0}},
    {.Pos = Shu::vec3f{ 0.5f, -0.5f, 0.0f}, .Color = Shu::vec3f{0, 1, 0}},
    {.Pos = Shu::vec3f{-0.5f, -0.5f, 0.0f}, .Color = Shu::vec3f{0, 0, 1}}
};
static u32 TriangleIndices[] = {0, 1, 2};

static shoora_vertex_info RectVertices[] =
{
    {.Pos = Shu::vec3f{ 0.5f,  0.5f, 0.0f}, .UV = Shu::vec2f{1, 1}, .Color = Shu::vec3f{1, 0, 0}},
    {.Pos = Shu::vec3f{ 0.5f, -0.5f, 0.0f}, .UV = Shu::vec2f{1, 0}, .Color = Shu::vec3f{0, 1, 0}},
    {.Pos = Shu::vec3f{-0.5f, -0.5f, 0.0f}, .UV = Shu::vec2f{0, 0}, .Color = Shu::vec3f{0, 0, 1}},
    {.Pos = Shu::vec3f{-0.5f,  0.5f, 0.0f}, .UV = Shu::vec2f{0, 1}, .Color = Shu::vec3f{0, 0, 0}},
};
static u32 RectIndices[] = {0, 1, 2, 0, 2, 3};

static Shu::vec3f CubeVertexPositions[] =
{
    Shu::vec3f{ 1.0f,  1.0f,  -1.0f},   // Top-Right
    Shu::vec3f{ 1.0f, -1.0f,  -1.0f},   // Bottom-Right
    Shu::vec3f{-1.0f, -1.0f,  -1.0f},   // Bottom-Left
    Shu::vec3f{-1.0f,  1.0f,  -1.0f},   // Top-Left
    Shu::vec3f{ 1.0f,  1.0f,   1.0f},   // Top-Right
    Shu::vec3f{ 1.0f, -1.0f,   1.0f},   // Bottom-Right
    Shu::vec3f{-1.0f, -1.0f,   1.0f},   // Bottom-Left
    Shu::vec3f{-1.0f,  1.0f,   1.0f}    // Top-Left
};

// NOTE: Cube
static shoora_vertex_info CubeVertices[] =
{
    // Front Face
    {.Pos = CubeVertexPositions[0], .UV = Shu::vec2f{1, 1}}, // 0
    {.Pos = CubeVertexPositions[1], .UV = Shu::vec2f{1, 0}}, // 1
    {.Pos = CubeVertexPositions[2], .UV = Shu::vec2f{0, 0}}, // 2
    {.Pos = CubeVertexPositions[3], .UV = Shu::vec2f{0, 1}}, // 3
    // Right Face
    {.Pos = CubeVertexPositions[0], .UV = Shu::vec2f{0, 1}}, // 4
    {.Pos = CubeVertexPositions[1], .UV = Shu::vec2f{0, 0}}, // 5
    {.Pos = CubeVertexPositions[5], .UV = Shu::vec2f{1, 0}}, // 6
    {.Pos = CubeVertexPositions[4], .UV = Shu::vec2f{1, 1}}, // 7
    // Back Face
    {.Pos = CubeVertexPositions[7], .UV = Shu::vec2f{1, 1}}, // 8
    {.Pos = CubeVertexPositions[6], .UV = Shu::vec2f{1, 0}}, // 9
    {.Pos = CubeVertexPositions[5], .UV = Shu::vec2f{0, 0}}, // 10
    {.Pos = CubeVertexPositions[4], .UV = Shu::vec2f{0, 1}}, // 11
    // Left Face
    {.Pos = CubeVertexPositions[6], .UV = Shu::vec2f{0, 0}}, // 12
    {.Pos = CubeVertexPositions[2], .UV = Shu::vec2f{1, 0}}, // 13
    {.Pos = CubeVertexPositions[3], .UV = Shu::vec2f{1, 1}}, // 14
    {.Pos = CubeVertexPositions[7], .UV = Shu::vec2f{0, 1}}, // 15
    // Top Face
    {.Pos = CubeVertexPositions[3], .UV = Shu::vec2f{0, 0}}, // 16
    {.Pos = CubeVertexPositions[0], .UV = Shu::vec2f{1, 0}}, // 17
    {.Pos = CubeVertexPositions[4], .UV = Shu::vec2f{1, 1}}, // 18
    {.Pos = CubeVertexPositions[7], .UV = Shu::vec2f{0, 1}}, // 19
    // Bottom Face
    {.Pos = CubeVertexPositions[2], .UV = Shu::vec2f{0, 0}}, // 20
    {.Pos = CubeVertexPositions[1], .UV = Shu::vec2f{1, 0}}, // 21
    {.Pos = CubeVertexPositions[5], .UV = Shu::vec2f{1, 1}}, // 22
    {.Pos = CubeVertexPositions[6], .UV = Shu::vec2f{0, 1}}, // 23
};
static u32 CubeIndices[] = { 0,  1,  2,  0,  2,  3,                      // Front Face
                             4,  7,  6,  4,  6,  5,                      // Right Face
                             9, 10,  8,  8, 10, 11,                      // Back Face
                            14, 13, 12, 15, 14, 12,                      // Left Face
                            17, 16, 19, 17, 19, 18,                      // Top Face
                            20, 21, 23, 21, 22, 23};                     // Bottom Face

void
GenerateCubeNormals(shoora_vertex_info *VertexInfo, u32 *Indices, u32 IndexCount)
{
    for(u32 Index = 0;
        Index < IndexCount;
        Index += 3)
    {
        u32 I0 = Indices[Index + 0];
        u32 I1 = Indices[Index + 1];
        u32 I2 = Indices[Index + 2];

        Shu::vec3f Edge1 = VertexInfo[I1].Pos - VertexInfo[I0].Pos;
        Shu::vec3f Edge2 = VertexInfo[I2].Pos - VertexInfo[I0].Pos;

        Shu::vec3f Normal = Shu::Normalize(Shu::Cross(Edge1, Edge2));

        VertexInfo[I0].Normal = Normal;
        VertexInfo[I1].Normal = Normal;
        VertexInfo[I2].Normal = Normal;
    }
}

void
GenerateCubeTangentInformation(shoora_vertex_info *VertexInfo, u32 *Indices, u32 IndexCount)
{
    for(u32 Index = 0;
        Index < IndexCount;
        Index += 3)
    {
        u32 I0 = Indices[Index + 0];
        u32 I1 = Indices[Index + 1];
        u32 I2 = Indices[Index + 2];

        shoora_vertex_info *V0 = VertexInfo + I0;
        shoora_vertex_info *V1 = VertexInfo + I1;
        shoora_vertex_info *V2 = VertexInfo + I2;

        Shu::vec3f Edge1 = V1->Pos - V0->Pos;
        Shu::vec3f Edge2 = V2->Pos - V0->Pos;

        f32 DeltaU1 = V1->UV.x - V0->UV.x;
        f32 DeltaV1 = V1->UV.y - V0->UV.y;
        f32 DeltaU2 = V2->UV.x - V0->UV.x;
        f32 DeltaV2 = V2->UV.y - V0->UV.y;
        f32 Denominator = DeltaU1*DeltaV2 - DeltaV1*DeltaU2;
        f32 OneByDenominator = 1.0f / Denominator;
        ASSERT(Denominator != 0.0f);

        Shu::vec4f Tangent;
        Tangent.x = OneByDenominator * (DeltaV2*Edge1.x - DeltaU2*Edge2.x);
        Tangent.y = OneByDenominator * (DeltaV2*Edge1.y - DeltaU2*Edge2.y);
        Tangent.z = OneByDenominator * (DeltaV2*Edge1.z - DeltaU2*Edge2.z);
        Tangent.w = 1.0f;
        Tangent.xyz = Shu::Normalize(Tangent.xyz);

        V0->Tangent = Tangent;
        V1->Tangent = Tangent;
        V2->Tangent = Tangent;

#if CALCULATE_BITANGENT
        Shu::vec3f BiTangent;
        Tangent.x = OneByDenominator * (DeltaU1*Edge2.x - DeltaV1*Edge1.x);
        Tangent.y = OneByDenominator * (DeltaU1*Edge2.y - DeltaV1*Edge1.y);
        Tangent.z = OneByDenominator * (DeltaU1*Edge2.z - DeltaV1*Edge1.z);
        BiTangent = Shu::Normalize(BiTangent);
        // Orthogonalize the tangent and the bitangent.
        Shu::vec3f TangentProj = BiTangent*Dot(Tangent, BiTangent);
        Tangent -= TangentProj;
        f32 D = Dot(Tangent, BiTangent);
        ASSERT(ABSOLUTE(D) <= FLT_EPSILON);

        V0->BiTangent = BiTangent;
        V1->BiTangent = BiTangent;
        V2->BiTangent = BiTangent;
#endif
    }
}

Shu::vec3f RandomCubePositions[] = {Shu::Vec3f(0.0f, 0.0f, 0.0f),    Shu::Vec3f(2.0f, 5.0f, -15.0f),
                                    Shu::Vec3f(-1.5f, -2.2f, -2.5f), Shu::Vec3f(-3.8f, -2.0f, -12.3f),
                                    Shu::Vec3f(2.4f, -0.4f, -3.5f),  Shu::Vec3f(-1.7f, 3.0f, -7.5f),
                                    Shu::Vec3f(1.3f, -2.0f, -2.5f),  Shu::Vec3f(1.5f, 2.0f, -2.5f),
                                    Shu::Vec3f(1.5f, 0.2f, -1.5f),   Shu::Vec3f(-1.3f, 1.0f, -1.5f)};

void
GenerateCircleMesh(shoora_mesh_filter *MeshFilter, u32 Resolution)
{
    MeshFilter->VertexCount = Resolution + 1;
    MeshFilter->Vertices = (shoora_vertex_info *)malloc(sizeof(shoora_vertex_info) * MeshFilter->VertexCount);
    MeshFilter->Vertices[0].Pos = {0.0f, 0.0f, 1.0f};

    f32 AngleStep = (2.0f * SHU_PI) / Resolution;
    f32 Radius = 1.0f;
    for(u32 Index = 0;
        Index < Resolution;
        ++Index)
    {
        f32 xPos = Shu::CosRad(AngleStep * Index);
        f32 yPos = Shu::SinRad(AngleStep * Index);

        MeshFilter->Vertices[Index+1] = {xPos, yPos, 1.0f};
    }

    MeshFilter->IndexCount = Resolution*3;
    MeshFilter->Indices = (u32 *)malloc(sizeof(u32) * MeshFilter->IndexCount);

    for(u32 Index = 1;
        Index < Resolution;
        ++Index)
    {
        u32 t = 3*(Index - 1);
        MeshFilter->Indices[t + 0] = Index;
        MeshFilter->Indices[t + 1] = 0;
        MeshFilter->Indices[t + 2] = Index + 1;
    }

    u32 NextPos = (3*(Resolution - 1));
    MeshFilter->Indices[NextPos] = Resolution;
    MeshFilter->Indices[NextPos + 1] = 0;
    MeshFilter->Indices[NextPos + 2] = 1;
}

void
InitializePrimitives()
{
    PrimitiveCollection.Circle.PrimitiveType = shoora_primitive_type::PRIMITIVE_TYPE_CIRCLE;
    GenerateCircleMesh(&PrimitiveCollection.Circle.MeshFilter, 40);
}

shoora_mesh_filter *
GetPrimitive2d(shoora_primitive_type Type)
{
    shoora_mesh_filter *Result = nullptr;
    switch (Type)
    {
        case shoora_primitive_type::PRIMITIVE_TYPE_CIRCLE:
        {
            Result = &PrimitiveCollection.Circle.MeshFilter;
        } break;

        SHU_INVALID_DEFAULT;
    }

    ASSERT(Result != nullptr);
    return Result;
}