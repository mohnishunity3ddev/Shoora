#include "geometry_primitive.h"

#include <memory.h>
#include <renderer/vulkan/vulkan_vertex_definitions.h>
#include <utils/utils.h>
#include <mesh/database/mesh_database.h>

static shoora_vertex_info LineVertices[2] =
{
    {.Pos = shu::Vec3f(-0.5f, -0.5f, 1.0f)},
    {.Pos = shu::Vec3f( 0.5f,  0.5f, 1.0f)},
};

static shoora_vertex_info TriangleVertices[] =
{
    // {.Pos = shu::vec3f{ 0.0f,  0.5f, 0.0f}, .Color = shu::vec3f{1, 0, 0}},
    // {.Pos = shu::vec3f{ 0.5f, -0.5f, 0.0f}, .Color = shu::vec3f{0, 1, 0}},
    // {.Pos = shu::vec3f{-0.5f, -0.5f, 0.0f}, .Color = shu::vec3f{0, 0, 1}}
    {.Pos = shu::vec3f{ 0.0f, 0.0f, 0.0f}, .Color = shu::vec3f{1, 0, 0}},
    {.Pos = shu::vec3f{ 0.0f, 1.0f, 0.0f}, .Color = shu::vec3f{0, 1, 0}},
    {.Pos = shu::vec3f{ 1.0f, 0.0f, 0.0f}, .Color = shu::vec3f{0, 0, 1}}
};
static u32 TriangleIndices[] = {0, 1, 2};

static shoora_vertex_info RectVertices[] =
{
    {.Pos = shu::vec3f{ 0.5f,  0.5f, 1.0f}, .UV = shu::vec2f{1, 1}, .Color = shu::vec3f{1, 0, 0}},
    {.Pos = shu::vec3f{ 0.5f, -0.5f, 1.0f}, .UV = shu::vec2f{1, 0}, .Color = shu::vec3f{0, 1, 0}},
    {.Pos = shu::vec3f{-0.5f, -0.5f, 1.0f}, .UV = shu::vec2f{0, 0}, .Color = shu::vec3f{0, 0, 1}},
    {.Pos = shu::vec3f{-0.5f,  0.5f, 1.0f}, .UV = shu::vec2f{0, 1}, .Color = shu::vec3f{0, 0, 0}},
};
static u32 RectIndices[] = {0, 1, 2, 0, 2, 3};

static shu::vec3f CubeVertexPositions[] =
{
    shu::vec3f{ 0.5f,  0.5f,  -0.5f},   // Top-Right
    shu::vec3f{ 0.5f, -0.5f,  -0.5f},   // Bottom-Right
    shu::vec3f{-0.5f, -0.5f,  -0.5f},   // Bottom-Left
    shu::vec3f{-0.5f,  0.5f,  -0.5f},   // Top-Left
    shu::vec3f{ 0.5f,  0.5f,   0.5f},   // Top-Right
    shu::vec3f{ 0.5f, -0.5f,   0.5f},   // Bottom-Right
    shu::vec3f{-0.5f, -0.5f,   0.5f},   // Bottom-Left
    shu::vec3f{-0.5f,  0.5f,   0.5f}    // Top-Left
};

static shoora_model UVSphereModel{};
static b32 MeshesGenerated = false;

// NOTE: Cube
static shoora_vertex_info CubeVertices[] =
{
    // Front Face
    {.Pos = CubeVertexPositions[0], .Normal = shu::Vec3f(0, 0, -1), .UV = shu::vec2f{1, 1}}, // 0
    {.Pos = CubeVertexPositions[1], .Normal = shu::Vec3f(0, 0, -1), .UV = shu::vec2f{1, 0}}, // 1
    {.Pos = CubeVertexPositions[2], .Normal = shu::Vec3f(0, 0, -1), .UV = shu::vec2f{0, 0}}, // 2
    {.Pos = CubeVertexPositions[3], .Normal = shu::Vec3f(0, 0, -1), .UV = shu::vec2f{0, 1}}, // 3
    // Right Face
    {.Pos = CubeVertexPositions[0], .Normal = shu::Vec3f(1, 0, 0), .UV = shu::vec2f{0, 1}}, // 4
    {.Pos = CubeVertexPositions[1], .Normal = shu::Vec3f(1, 0, 0), .UV = shu::vec2f{0, 0}}, // 5
    {.Pos = CubeVertexPositions[5], .Normal = shu::Vec3f(1, 0, 0), .UV = shu::vec2f{1, 0}}, // 6
    {.Pos = CubeVertexPositions[4], .Normal = shu::Vec3f(1, 0, 0), .UV = shu::vec2f{1, 1}}, // 7
    // Back Face
    {.Pos = CubeVertexPositions[7], .Normal = shu::Vec3f(0, 0, 1), .UV = shu::vec2f{1, 1}}, // 8
    {.Pos = CubeVertexPositions[6], .Normal = shu::Vec3f(0, 0, 1), .UV = shu::vec2f{1, 0}}, // 9
    {.Pos = CubeVertexPositions[5], .Normal = shu::Vec3f(0, 0, 1), .UV = shu::vec2f{0, 0}}, // 10
    {.Pos = CubeVertexPositions[4], .Normal = shu::Vec3f(0, 0, 1), .UV = shu::vec2f{0, 1}}, // 11
    // Left Face
    {.Pos = CubeVertexPositions[6], .Normal = shu::Vec3f(-1, 0, 0), .UV = shu::vec2f{0, 0}}, // 12
    {.Pos = CubeVertexPositions[2], .Normal = shu::Vec3f(-1, 0, 0), .UV = shu::vec2f{1, 0}}, // 13
    {.Pos = CubeVertexPositions[3], .Normal = shu::Vec3f(-1, 0, 0), .UV = shu::vec2f{1, 1}}, // 14
    {.Pos = CubeVertexPositions[7], .Normal = shu::Vec3f(-1, 0, 0), .UV = shu::vec2f{0, 1}}, // 15
    // Top Face
    {.Pos = CubeVertexPositions[3], .Normal = shu::Vec3f(0, 1, 0), .UV = shu::vec2f{0, 0}}, // 16
    {.Pos = CubeVertexPositions[0], .Normal = shu::Vec3f(0, 1, 0), .UV = shu::vec2f{1, 0}}, // 17
    {.Pos = CubeVertexPositions[4], .Normal = shu::Vec3f(0, 1, 0), .UV = shu::vec2f{1, 1}}, // 18
    {.Pos = CubeVertexPositions[7], .Normal = shu::Vec3f(0, 1, 0), .UV = shu::vec2f{0, 1}}, // 19
    // Bottom Face
    {.Pos = CubeVertexPositions[2], .Normal = shu::Vec3f(0, -1, 0), .UV = shu::vec2f{0, 0}}, // 20
    {.Pos = CubeVertexPositions[1], .Normal = shu::Vec3f(0, -1, 0), .UV = shu::vec2f{1, 0}}, // 21
    {.Pos = CubeVertexPositions[5], .Normal = shu::Vec3f(0, -1, 0), .UV = shu::vec2f{1, 1}}, // 22
    {.Pos = CubeVertexPositions[6], .Normal = shu::Vec3f(0, -1, 0), .UV = shu::vec2f{0, 1}}, // 23
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

        shu::vec3f Edge1 = VertexInfo[I1].Pos - VertexInfo[I0].Pos;
        shu::vec3f Edge2 = VertexInfo[I2].Pos - VertexInfo[I0].Pos;

        shu::vec3f Normal = shu::Normalize(shu::Cross(Edge1, Edge2));

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

        shu::vec3f Edge1 = V1->Pos - V0->Pos;
        shu::vec3f Edge2 = V2->Pos - V0->Pos;

        f32 DeltaU1 = V1->UV.x - V0->UV.x;
        f32 DeltaV1 = V1->UV.y - V0->UV.y;
        f32 DeltaU2 = V2->UV.x - V0->UV.x;
        f32 DeltaV2 = V2->UV.y - V0->UV.y;
        f32 Denominator = DeltaU1*DeltaV2 - DeltaV1*DeltaU2;
        f32 OneByDenominator = 1.0f / Denominator;
        ASSERT(Denominator != 0.0f);

        shu::vec4f Tangent;
        Tangent.x = OneByDenominator * (DeltaV2*Edge1.x - DeltaU2*Edge2.x);
        Tangent.y = OneByDenominator * (DeltaV2*Edge1.y - DeltaU2*Edge2.y);
        Tangent.z = OneByDenominator * (DeltaV2*Edge1.z - DeltaU2*Edge2.z);
        Tangent.w = 1.0f;
        Tangent.xyz = shu::Normalize(Tangent.xyz);

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

u32 DiamondIndices[] =
{
    6,  5,  7,  1,  5,  8,  5,  6,  8,  5,  1,  9,  7,  5,  9,  4,  1,  10, 1,  8,  10, 1,
    4,  11, 9,  1,  11, 10, 8,  12, 8,  6,  12, 7,  9,  13, 6,  7,  14, 4,  3,  15, 11, 4,
    15, 11, 15, 16, 9,  11, 16, 13, 9,  16, 7,  13, 17, 2,  17, 18, 17, 13, 18, 13, 16, 18,
    18, 16, 19, 2,  18, 19, 15, 3,  19, 16, 15, 19, 14, 7,  20, 7,  17, 20, 2,  19, 21, 19,
    3,  21, 17, 2,  22, 2,  21, 23, 22, 2,  23, 21, 3,  24, 23, 21, 24, 23, 24, 25, 22, 23,
    25, 14, 20, 26, 20, 17, 26, 17, 22, 26, 24, 3,  27, 3,  4,  27, 25, 24, 28, 24, 27, 28,
    22, 25, 29, 29, 25, 30, 0,  30, 31, 25, 28, 31, 30, 25, 31, 28, 27, 31, 29, 30, 32, 30,
    0,  32, 14, 26, 33, 26, 22, 33, 22, 29, 33, 0,  31, 34, 31, 27, 34, 27, 4,  34, 29, 32,
    35, 35, 32, 36, 0,  34, 37, 32, 0,  37, 36, 32, 37, 36, 37, 38, 35, 36, 38, 33, 29, 39,
    29, 35, 39, 14, 33, 39, 34, 4,  40, 37, 34, 40, 38, 37, 41, 37, 40, 41, 35, 38, 42, 38,
    41, 43, 42, 38, 43, 41, 40, 44, 43, 41, 44, 43, 44, 45, 42, 43, 45, 39, 35, 46, 35, 42,
    46, 14, 39, 46, 40, 4,  47, 44, 40, 47, 45, 44, 48, 44, 47, 48, 42, 45, 49, 12, 6,  49,
    12, 49, 50, 49, 45, 50, 45, 48, 50, 48, 47, 51, 50, 48, 51, 12, 50, 51, 46, 42, 52, 42,
    49, 52, 14, 46, 52, 47, 4,  53, 51, 47, 53, 4,  10, 53, 51, 53, 54, 53, 10, 54, 10, 12,
    54, 12, 51, 54, 14, 52, 55, 52, 49, 55, 6,  14, 55, 49, 6,  55
};

shu::vec3f DiamondCenterOfMass = shu::Vec3f(0.0f, 0.0f, -0.082f);
shu::mat3f DiamondInertiaTensor = shu::Mat3f(0.2484f, 0.0f,      0.0f,
                                             0.0f,    0.248386f, 0.0f,
                                             0.0f,    0.0f,      0.341404f);
shu::vec3f DiamondBounds_Min = shu::Vec3f(-1.0f, -1.0f, -1.0f);
shu::vec3f DiamondBounds_Max = shu::Vec3f( 1.0f,  1.0f,  0.4f);

static shoora_vertex_info DiamondVertices[56] = {
    {.Pos = shu::Vec3f(-1.0f, 0.0f, 0.0f),            .Normal = shu::Vec3f(-0.956989f,  0.042307f, -0.287022f)},
    {.Pos = shu::Vec3f( 1.0f, 0.0f, 0.0f),            .Normal = shu::Vec3f( 0.971188f,  0.000000f, -0.238315f)},
    {.Pos = shu::Vec3f(0.0f, 1.0f, 0.1f),             .Normal = shu::Vec3f( 0.036001f,  0.913661f,  0.404880f)},
    {.Pos = shu::Vec3f(0.0f, 0.1f, -1.0f),            .Normal = shu::Vec3f( 0.000000f,  0.519632f, -0.854391f)},
    {.Pos = shu::Vec3f(0.1f, 0.0f, -1.0f),            .Normal = shu::Vec3f( 0.311251f, -0.000000f, -0.950328f)},
    {.Pos = shu::Vec3f(1.0f, 0.0f, 0.1f),             .Normal = shu::Vec3f( 0.875251f,  0.000000f,  0.483669f)},
    {.Pos = shu::Vec3f(0.739104f, -0.306146f, 0.3f),  .Normal = shu::Vec3f( 0.449390f, -0.186143f,  0.873727f)},
    {.Pos = shu::Vec3f(0.739104f, 0.306147f, 0.3f),   .Normal = shu::Vec3f( 0.449390f,  0.186143f,  0.873727f)},
    {.Pos = shu::Vec3f(0.923880f, -0.382683f, 0.1f),  .Normal = shu::Vec3f( 0.894836f, -0.324418f,  0.306629f)},
    {.Pos = shu::Vec3f(0.923879f, 0.382683f, 0.1f),   .Normal = shu::Vec3f( 0.893481f,  0.370092f,  0.254407f)},
    {.Pos = shu::Vec3f(0.923880f, -0.382683f, 0.0f),  .Normal = shu::Vec3f( 0.847502f, -0.389233f, -0.360885f)},
    {.Pos = shu::Vec3f(0.923879f, 0.382683f, 0.0f),   .Normal = shu::Vec3f( 0.833464f,  0.345232f, -0.431455f)},
    {.Pos = shu::Vec3f(0.707107f, -0.707106f, 0.1f),  .Normal = shu::Vec3f( 0.663197f, -0.663196f,  0.346903f)},
    {.Pos = shu::Vec3f(0.707107f, 0.707107f, 0.1f),   .Normal = shu::Vec3f( 0.618896f,  0.618896f,  0.483670f)},
    {.Pos = shu::Vec3f(0.4f, 0.0f, 0.4f),             .Normal = shu::Vec3f( 0.078455f,  0.000000f,  0.996918f)},
    {.Pos = shu::Vec3f(0.070711f, 0.070711f, -1.0f),  .Normal = shu::Vec3f( 0.432809f,  0.432809f, -0.790793f)},
    {.Pos = shu::Vec3f(0.707107f, 0.707107f, 0.0f),   .Normal = shu::Vec3f( 0.686734f,  0.686734f, -0.238315f)},
    {.Pos = shu::Vec3f(0.306147f, 0.739104f, 0.3f),   .Normal = shu::Vec3f( 0.186143f,  0.449390f,  0.873727f)},
    {.Pos = shu::Vec3f(0.382683f, 0.923880f, 0.1f),   .Normal = shu::Vec3f( 0.403346f,  0.862143f,  0.306629f)},
    {.Pos = shu::Vec3f(0.382683f, 0.923880f, 0.0f),   .Normal = shu::Vec3f( 0.324045f,  0.874504f, -0.360885f)},
    {.Pos = shu::Vec3f(0.282843f, 0.282843f, 0.4f),   .Normal = shu::Vec3f( 0.124864f,  0.124864f,  0.984285f)},
    {.Pos = shu::Vec3f(0.0f, 1.0f, 0.0f),             .Normal = shu::Vec3f(-0.042307f,  0.956989f, -0.287022f)},
    {.Pos = shu::Vec3f(-0.306147f, 0.739103f, 0.3f),  .Normal = shu::Vec3f(-0.186143f,  0.449389f,  0.873727f)},
    {.Pos = shu::Vec3f(-0.382684f, 0.923879f, 0.1f),  .Normal = shu::Vec3f(-0.324418f,  0.894836f,  0.306630f)},
    {.Pos = shu::Vec3f(-0.382684f, 0.923879f, 0.0f),  .Normal = shu::Vec3f(-0.389234f,  0.847502f, -0.360885f)},
    {.Pos = shu::Vec3f(-0.707107f, 0.707107f, 0.1f),  .Normal = shu::Vec3f(-0.663196f,  0.663196f,  0.346903f)},
    {.Pos = shu::Vec3f(0.0f, 0.4f, 0.4f),             .Normal = shu::Vec3f( 0.000000f,  0.141410f,  0.989951f)},
    {.Pos = shu::Vec3f(0.070711f, 0.070711f, -1.0f),  .Normal = shu::Vec3f(-0.367435f,  0.367435f, -0.854391f)},
    {.Pos = shu::Vec3f(-0.707107f, 0.707107f, 0.0f),  .Normal = shu::Vec3f(-0.659751f,  0.659751f, -0.359802f)},
    {.Pos = shu::Vec3f(-0.739104f, 0.306147f, 0.3f),  .Normal = shu::Vec3f(-0.449390f,  0.186143f,  0.873727f)},
    {.Pos = shu::Vec3f(-0.923880f, 0.382683f, 0.1f),  .Normal = shu::Vec3f(-0.894836f,  0.324418f,  0.306629f)},
    {.Pos = shu::Vec3f(-0.923880f, 0.382683f, 0.0f),  .Normal = shu::Vec3f(-0.847502f,  0.389233f, -0.360885f)},
    {.Pos = shu::Vec3f(-1.0f, 0.0f, 0.1f),            .Normal = shu::Vec3f(-0.913661f, -0.036001f,  0.404880f)},
    {.Pos = shu::Vec3f(-0.282843f, 0.282843f, 0.4f),  .Normal = shu::Vec3f(-0.099992f,  0.099992f,  0.989951f)},
    {.Pos = shu::Vec3f(-0.1f, 0.0f, -1.0f),           .Normal = shu::Vec3f(-0.519632f, -0.000000f, -0.854391f)},
    {.Pos = shu::Vec3f(-0.739103f, -0.306147f, 0.3f), .Normal = shu::Vec3f(-0.449390f, -0.186143f,  0.873727f)},
    {.Pos = shu::Vec3f(-0.923879f, -0.382684f, 0.1f), .Normal = shu::Vec3f(-0.852848f, -0.353262f,  0.384521f)},
    {.Pos = shu::Vec3f(-0.923879f, -0.382684f, 0.0f), .Normal = shu::Vec3f(-0.878557f, -0.363911f, -0.309365f)},
    {.Pos = shu::Vec3f(-0.707106f, -0.707107f, 0.1f), .Normal = shu::Vec3f(-0.671512f, -0.620600f,  0.404880f)},
    {.Pos = shu::Vec3f(-0.4f, 0.0f, 0.4f),            .Normal = shu::Vec3f(-0.141410f, -0.000000f,  0.989951f)},
    {.Pos = shu::Vec3f(-0.070711f, -0.070711f, -1.0f),.Normal = shu::Vec3f(-0.367435f, -0.367435f, -0.854391f)},
    {.Pos = shu::Vec3f(-0.707106f, -0.707107f, 0.0f), .Normal = shu::Vec3f(-0.646778f, -0.706610f, -0.287022f)},
    {.Pos = shu::Vec3f(-0.306146f, -0.739104f, 0.3f), .Normal = shu::Vec3f(-0.186143f, -0.449390f,  0.873727f)},
    {.Pos = shu::Vec3f(-0.382683f, -0.923880f, 0.1f), .Normal = shu::Vec3f(-0.403346f, -0.862143f,  0.306629f)},
    {.Pos = shu::Vec3f(-0.382683f, -0.923880f, 0.0f), .Normal = shu::Vec3f(-0.324044f, -0.874504f, -0.360885f)},
    {.Pos = shu::Vec3f(0.1f, -1.0f, 0.1f),            .Normal = shu::Vec3f(-0.036000f, -0.913661f,  0.404880f)},
    {.Pos = shu::Vec3f(-0.282843f, -0.282843f, 0.4f), .Normal = shu::Vec3f(-0.099992f, -0.099992f,  0.989951f)},
    {.Pos = shu::Vec3f(0.0f, -0.1f, -1.0f),           .Normal = shu::Vec3f( 0.000001f, -0.519632f, -0.854390f)},
    {.Pos = shu::Vec3f(0.1f, -1.0f, 0.0f),            .Normal = shu::Vec3f( 0.042308f, -0.956989f, -0.287022f)},
    {.Pos = shu::Vec3f(0.306147f, -0.739103f, 0.3f),  .Normal = shu::Vec3f( 0.186144f, -0.449390f,  0.873727f)},
    {.Pos = shu::Vec3f(0.382684f, -0.923879f, 0.1f),  .Normal = shu::Vec3f( 0.324419f, -0.894836f,  0.306629f)},
    {.Pos = shu::Vec3f(0.382684f, -0.923879f, 0.0f),  .Normal = shu::Vec3f( 0.389234f, -0.847502f, -0.360885f)},
    {.Pos = shu::Vec3f(0.0f, -0.4f, 0.4f),            .Normal = shu::Vec3f( 0.000000f, -0.141410f,  0.989951f)},
    {.Pos = shu::Vec3f(0.070711f, -0.070711f, -1.0f), .Normal = shu::Vec3f( 0.432810f, -0.432809f, -0.790792f)},
    {.Pos = shu::Vec3f(0.707107f, -0.707106f, 0.0f),  .Normal = shu::Vec3f( 0.659751f, -0.659751f, -0.359802f)},
    {.Pos = shu::Vec3f(0.282843f, -0.282843f, 0.4f),  .Normal = shu::Vec3f( 0.124864f, -0.124864f,  0.984285f)}
};

#if 0
Shu::vec3f RandomCubePositions[] = {Shu::Vec3f(0.0f, 0.0f, 0.0f),    Shu::Vec3f(2.0f, 5.0f, -15.0f),
                                    Shu::Vec3f(-1.5f, -2.2f, -2.5f), Shu::Vec3f(-3.8f, -2.0f, -12.3f),
                                    Shu::Vec3f(2.4f, -0.4f, -3.5f),  Shu::Vec3f(-1.7f, 3.0f, -7.5f),
                                    Shu::Vec3f(1.3f, -2.0f, -2.5f),  Shu::Vec3f(1.5f, 2.0f, -2.5f),
                                    Shu::Vec3f(1.5f, 0.2f, -1.5f),   Shu::Vec3f(-1.3f, 1.0f, -1.5f)};
#endif

void
shoora_primitive_collection::GenerateCircleMesh(shoora_mesh *Mesh, shoora_vertex_info *VerticesMemory,
                                                u32 *IndicesMemory, u32 &RunningVertexCount,
                                                u32 &RunningIndexCount)
{
    Mesh->VertexOffset = RunningVertexCount;
    Mesh->IndexOffset = RunningIndexCount;

    shoora_mesh_filter *MeshFilter = &Mesh->MeshFilter;

    MeshFilter->VertexCount = CIRCLE_PRIMITIVE_RESOLUTION + 1;
    MeshFilter->Vertices = (shoora_vertex_info *)VerticesMemory + RunningVertexCount;
    MeshFilter->Vertices[0].Pos = {0.0f, 0.0f, 1.0f};

    f32 AngleStep = (2.0f * SHU_PI) / CIRCLE_PRIMITIVE_RESOLUTION;
    f32 Radius = 1.0f;
    for(u32 Index = 0;
        Index < CIRCLE_PRIMITIVE_RESOLUTION;
        ++Index)
    {
        f32 xPos = shu::CosRad(AngleStep * Index);
        f32 yPos = shu::SinRad(AngleStep * Index);

        MeshFilter->Vertices[Index+1] = {xPos, yPos, 1.0f};
    }

    MeshFilter->IndexCount = CIRCLE_PRIMITIVE_RESOLUTION*3;
    MeshFilter->Indices = IndicesMemory + RunningIndexCount;

    for(u32 Index = 1;
        Index < CIRCLE_PRIMITIVE_RESOLUTION;
        ++Index)
    {
        u32 t = 3*(Index - 1);
        MeshFilter->Indices[t + 0] = Index;
        MeshFilter->Indices[t + 1] = 0;
        MeshFilter->Indices[t + 2] = Index + 1;
    }

    u32 NextPos = (3*(CIRCLE_PRIMITIVE_RESOLUTION - 1));
    MeshFilter->Indices[NextPos] = CIRCLE_PRIMITIVE_RESOLUTION;
    MeshFilter->Indices[NextPos + 1] = 0;
    MeshFilter->Indices[NextPos + 2] = 1;

    RunningIndexCount += MeshFilter->IndexCount;
    RunningVertexCount += MeshFilter->VertexCount;
}

void
shoora_primitive_collection::GenerateUVSphereMesh()
{
#if 1
    // TODO: Dont rely on Loading Meshes using GLTF!!
    // LoadModel(&UVSphereModel, "meshes/primitives/uv_sphere.glb");
    // LoadModel(&UVSphereModel, "meshes/primitives/quad_sphere_uv.glb");
    // LoadModel(&UVSphereModel, "meshes/primitives/ico_sphere_lowres.glb");
    LoadModel(&UVSphereModel, "meshes/primitives/ico_sphere_1subd.glb");
#else
    // TODO: Do Custom Implementation here!
#endif

    MeshesGenerated = true;
}

shoora_mesh_filter shoora_primitive_collection::GetPrimitiveInfo(u32 Type)
{
    shoora_mesh_filter Result = {};

    switch((shoora_mesh_type) Type)
    {
        /*
        case shoora_mesh_type::CIRCLE:
        {
            Result.Vertices = ;
            Result.VertexCount = ;
            Result.Indices = ;
            Result.IndexCount = ;
        }
        break;
        */

        case shoora_mesh_type::TRIANGLE:
        {
            Result.Vertices = TriangleVertices;
            Result.VertexCount = ARRAY_SIZE(TriangleVertices);
            Result.Indices = TriangleIndices;
            Result.IndexCount = ARRAY_SIZE(TriangleIndices);
        } break;

        case shoora_mesh_type::RECT_2D:
        {
            Result.Vertices = RectVertices;
            Result.VertexCount = ARRAY_SIZE(RectVertices);
            Result.Indices = RectIndices;
            Result.IndexCount = ARRAY_SIZE(RectIndices);
        } break;

        case shoora_mesh_type::CUBE:
        {
            Result.Vertices = CubeVertices;
            Result.VertexCount = ARRAY_SIZE(CubeVertices);
            Result.Indices = CubeIndices;
            Result.IndexCount = ARRAY_SIZE(CubeIndices);
        } break;

        case shoora_mesh_type::LINE:
        {
            Result.Vertices = LineVertices;
            Result.VertexCount = ARRAY_SIZE(LineVertices);
            Result.Indices = nullptr;
            Result.IndexCount = 0;
        } break;

        case shoora_mesh_type::CONVEX_DIAMOND:
        {
            Result.Vertices = DiamondVertices;
            Result.VertexCount = ARRAY_SIZE(DiamondVertices);
            Result.Indices = DiamondIndices;
            Result.IndexCount = ARRAY_SIZE(DiamondIndices);
        } break;

        case shoora_mesh_type::SPHERE:
        {
            ASSERT(MeshesGenerated);
            Result = UVSphereModel.MeshFilter;
        } break;

        SHU_INVALID_DEFAULT;
    }

    return Result;
}

i32
shoora_primitive_collection::GetTotalVertexCount()
{
    ASSERT(MeshesGenerated);
    i32 Result = (CIRCLE_PRIMITIVE_RESOLUTION + 1) + ARRAY_SIZE(CubeVertices) + ARRAY_SIZE(TriangleVertices) +
                 ARRAY_SIZE(RectVertices) + ARRAY_SIZE(LineVertices) + UVSphereModel.MeshFilter.VertexCount +
                 ARRAY_SIZE(DiamondVertices);
    return Result;
}

i32
shoora_primitive_collection::GetTotalIndexCount()
{
    ASSERT(MeshesGenerated);
    i32 Result = (CIRCLE_PRIMITIVE_RESOLUTION * 3) + ARRAY_SIZE(CubeIndices) + ARRAY_SIZE(TriangleIndices) +
                 ARRAY_SIZE(RectIndices) + UVSphereModel.MeshFilter.IndexCount + ARRAY_SIZE(DiamondIndices);
    return Result;
}

void
shoora_primitive_collection::Cleanup()
{
    ASSERT(UVSphereModel.MeshFilter.Vertices != nullptr &&
           UVSphereModel.MeshFilter.Indices != nullptr);
    free(UVSphereModel.MeshFilter.Vertices);
    free(UVSphereModel.MeshFilter.Indices);

    if(UVSphereModel.Nodes) { free(UVSphereModel.Nodes); }
    if(UVSphereModel.Materials) { free(UVSphereModel.Materials); }
    if(UVSphereModel.Textures) { free(UVSphereModel.Textures); }

}
