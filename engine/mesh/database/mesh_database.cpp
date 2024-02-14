#include "mesh_database.h"

#include <mesh/primitive/geometry_primitive.h>
#include <renderer/vulkan/vulkan_debug.h>

static const Shu::vec3f *RunningPolygonVertexPositions[MAX_MESH_COUNT];
static u32 RunningPolygonVertexCounts[MAX_MESH_COUNT];
static u32 RunningVertexCount = 0, RunningIndexCount = 0;
static void *Memory;
static shoora_vertex_info *Vertices;
static u32 *Indices;

static shoora_vulkan_device *RenderDevice;
static shoora_vulkan_buffer vBuffer;
static shoora_vulkan_buffer iBuffer;

static b32 Begin;
static u32 TotalVertexCount, TotalIndexCount;

static shoora_mesh Meshes[MAX_MESH_COUNT];
static u32 TotalMeshCount;

// Primitives
static shoora_mesh *_LineMesh;
static shoora_mesh *_Rect2DMesh;
static shoora_mesh *_CircleMesh;
static shoora_mesh *_CubeMesh;
static shoora_mesh *_UVSphereMesh;
static u32 PrimitiveCount;

static shoora_mesh *CustomPolyMeshes;
static u32 CustomPolyCount;

static void Initialize();

void
shoora_mesh_database::MeshDbBegin(shoora_vulkan_device *Device)
{
    Begin = true;
    RenderDevice = Device;
}

void
shoora_mesh_database::AddPolygonMeshToDb(const Shu::vec3f *vPositions, i32 vCount)
{
    ASSERT(vCount >= 3 && "Polygons must have a minimum of three vertices");
    ASSERT(Begin && (TotalMeshCount + 1) <= MAX_MESH_COUNT);
    RunningPolygonVertexPositions[CustomPolyCount] = vPositions;
    RunningPolygonVertexCounts[CustomPolyCount] = vCount;
    CustomPolyCount++;
}

void shoora_mesh_database::MeshDbEnd()
{
    Begin = false;
    Initialize();
}

void
Initialize()
{
    shoora_primitive_collection::GenerateUVSphereMesh();

    // Get the required memory size to hold all the meshes to be used in the scene
    // that is the total number of vertices for primitives and any other custom polygon ones.
    TotalVertexCount = shoora_primitive_collection::GetTotalVertexCount();
    TotalIndexCount = shoora_primitive_collection::GetTotalIndexCount();
    for (i32 i = 0; i < CustomPolyCount; ++i)
    {
        auto vCount = RunningPolygonVertexCounts[i];
        TotalVertexCount += vCount;
        u32 nTriangles = vCount - 2;
        TotalIndexCount += nTriangles*3;
    }
    size_t RequiredSize = TotalVertexCount*sizeof(shoora_vertex_info) + TotalIndexCount*sizeof(u32);
    Memory = malloc(RequiredSize);
    memset(Memory, 0, RequiredSize);
    Vertices = (shoora_vertex_info *)Memory;
    Indices = (u32 *)(Vertices + TotalVertexCount);

    // NOTE: Primitives first
    _CircleMesh = &Meshes[TotalMeshCount++];
    _CubeMesh = &Meshes[TotalMeshCount++];
    _Rect2DMesh = &Meshes[TotalMeshCount++];
    _LineMesh = &Meshes[TotalMeshCount++];
    _UVSphereMesh = &Meshes[TotalMeshCount++];

    PrimitiveCount = TotalMeshCount;

    _CircleMesh->Type = shoora_mesh_type::CIRCLE;
    shoora_primitive_collection::GenerateCircleMesh(_CircleMesh, Vertices + RunningVertexCount,
                                                    Indices + RunningIndexCount, RunningVertexCount,
                                                    RunningIndexCount);

    auto UVSphereInfo = shoora_primitive_collection::GetPrimitiveInfo((u32)shoora_mesh_type::SPHERE);
    shoora_mesh *UVSphereMesh = _UVSphereMesh;
    UVSphereMesh->Type = shoora_mesh_type::SPHERE;
    UVSphereMesh->VertexOffset = RunningVertexCount;
    UVSphereMesh->IndexOffset = RunningIndexCount;
    shoora_mesh_filter *UVSphereMeshFilter = &UVSphereMesh->MeshFilter;
    UVSphereMeshFilter->Vertices = Vertices + RunningVertexCount;
    UVSphereMeshFilter->VertexCount = UVSphereInfo.VertexCount;
    UVSphereMeshFilter->Indices = Indices + RunningIndexCount;
    UVSphereMeshFilter->IndexCount = UVSphereInfo.IndexCount;
    memcpy(UVSphereMeshFilter->Vertices, UVSphereInfo.Vertices, UVSphereMeshFilter->VertexCount * sizeof(shoora_vertex_info));
    memcpy(UVSphereMeshFilter->Indices, UVSphereInfo.Indices, UVSphereMeshFilter->IndexCount * sizeof(u32));
    RunningVertexCount += UVSphereMeshFilter->VertexCount;
    RunningIndexCount += UVSphereMeshFilter->IndexCount;


    auto CubeInfo = shoora_primitive_collection::GetPrimitiveInfo((u32)shoora_mesh_type::CUBE);
    shoora_mesh *CubeMesh = _CubeMesh;
    CubeMesh->Type = shoora_mesh_type::CUBE;
    CubeMesh->VertexOffset = RunningVertexCount;
    CubeMesh->IndexOffset = RunningIndexCount;
    shoora_mesh_filter *CubeMeshFilter = &CubeMesh->MeshFilter;
    CubeMeshFilter->Vertices = Vertices + RunningVertexCount;
    CubeMeshFilter->VertexCount = CubeInfo.VertexCount;
    CubeMeshFilter->Indices = Indices + RunningIndexCount;
    CubeMeshFilter->IndexCount = CubeInfo.IndexCount;
    memcpy(CubeMeshFilter->Vertices, CubeInfo.Vertices, CubeMeshFilter->VertexCount * sizeof(shoora_vertex_info));
    memcpy(CubeMeshFilter->Indices, CubeInfo.Indices, CubeMeshFilter->IndexCount * sizeof(u32));
    RunningVertexCount += CubeMeshFilter->VertexCount;
    RunningIndexCount += CubeMeshFilter->IndexCount;

    auto RectInfo = shoora_primitive_collection::GetPrimitiveInfo((u32)shoora_mesh_type::RECT_2D);
    shoora_mesh *RectMesh = _Rect2DMesh;
    RectMesh->Type = shoora_mesh_type::RECT_2D;
    RectMesh->VertexOffset = RunningVertexCount;
    RectMesh->IndexOffset = RunningIndexCount;
    shoora_mesh_filter *RectMeshFilter = &RectMesh->MeshFilter;
    RectMeshFilter->Vertices = Vertices + RunningVertexCount;
    RectMeshFilter->VertexCount = RectInfo.VertexCount;
    RectMeshFilter->Indices = Indices + RunningIndexCount;
    RectMeshFilter->IndexCount = RectInfo.IndexCount;
    memcpy(RectMeshFilter->Vertices, RectInfo.Vertices, RectMeshFilter->VertexCount * sizeof(shoora_vertex_info));
    memcpy(RectMeshFilter->Indices, RectInfo.Indices, RectMeshFilter->IndexCount * sizeof(u32));
    RunningVertexCount += RectMeshFilter->VertexCount;
    RunningIndexCount += RectMeshFilter->IndexCount;

    auto LineInfo = shoora_primitive_collection::GetPrimitiveInfo((u32)shoora_mesh_type::LINE);
    shoora_mesh *LineMesh = _LineMesh;
    LineMesh->Type = shoora_mesh_type::LINE;
    LineMesh->VertexOffset = RunningVertexCount;
    LineMesh->IndexOffset = -1;
    shoora_mesh_filter *LineMeshFilter = &LineMesh->MeshFilter;
    LineMeshFilter->Vertices = Vertices + RunningVertexCount;
    LineMeshFilter->VertexCount = LineInfo.VertexCount;
    LineMeshFilter->Indices = nullptr;
    LineMeshFilter->IndexCount = LineInfo.IndexCount;
    memcpy(LineMeshFilter->Vertices, LineInfo.Vertices, LineMeshFilter->VertexCount * sizeof(shoora_vertex_info));
    RunningVertexCount += LineMeshFilter->VertexCount;
    RunningIndexCount += LineMeshFilter->IndexCount;

    // Storing Custom meshes here
    CustomPolyMeshes = Meshes + PrimitiveCount;
    for (i32 i = 0; i < CustomPolyCount; ++i)
    {
        auto *VPositions = RunningPolygonVertexPositions[i];
        auto VCount = RunningPolygonVertexCounts[i];

        auto *VertexDest = Vertices + RunningVertexCount;
        auto *IndexDest = Indices + RunningIndexCount;
        for (i32 j = 0; j < VCount; ++j)
        {
            VertexDest[j].Pos = VPositions[j];
        }

        i32 numTriangles = (VCount - 2);
        auto ICount = numTriangles * 3;
        for(i32 tI = 1; tI <= numTriangles; ++tI)
        {
            auto s = 3 * (tI - 1);
            IndexDest[s] = 0;
            IndexDest[s+1] = tI;
            IndexDest[s+2] = tI+1;
        }

        shoora_mesh *PolyMesh = &Meshes[TotalMeshCount++];
        PolyMesh->VertexOffset = RunningVertexCount;
        PolyMesh->IndexOffset = RunningIndexCount;
        PolyMesh->Type = shoora_mesh_type::POLYGON_2D;
        shoora_mesh_filter *PolyMeshFilter = &PolyMesh->MeshFilter;
        PolyMeshFilter->Vertices = VertexDest;
        PolyMeshFilter->VertexCount = VCount;
        PolyMeshFilter->Indices = IndexDest;
        PolyMeshFilter->IndexCount = ICount;


        RunningVertexCount += VCount;
        RunningIndexCount += ICount;
    }

    CreateVertexBuffers(RenderDevice, Vertices, TotalVertexCount, Indices, TotalIndexCount,
                        &vBuffer, &iBuffer);

    shoora_primitive_collection::Cleanup();

#ifdef _SHU_DEBUG
    // NOTE: This is how we name vulkan objects. Easier to read validation errors if errors are there!
    LogInfo("This is a debug build brother!\n");
    SetObjectName(RenderDevice, (u64)vBuffer.Handle, VK_OBJECT_TYPE_BUFFER, "Mesh Database Vertex Buffer");
    SetObjectName(RenderDevice, (u64)iBuffer.Handle, VK_OBJECT_TYPE_BUFFER, "Mesh Database Index Buffer");
#elif defined(_SHU_RELEASE)
    LogInfo("This is a RELEASE build brother!\n");
#endif
}

shoora_mesh *
shoora_mesh_database::GetMesh(shoora_mesh_type Type)
{
    shoora_mesh *Result = nullptr;

    switch (Type)
    {
        case shoora_mesh_type::CIRCLE: { Result = _CircleMesh; } break;
        case shoora_mesh_type::SPHERE: { Result = _UVSphereMesh; } break;
        case shoora_mesh_type::RECT_2D: { Result = _Rect2DMesh; } break;
        case shoora_mesh_type::CUBE: { Result = _CubeMesh; } break;
        case shoora_mesh_type::LINE: { Result = _LineMesh; } break;
        SHU_INVALID_DEFAULT;
    }

    ASSERT(Result != nullptr);
    return Result;
}

shoora_mesh_filter *
shoora_mesh_database::GetMeshFilter(shoora_mesh_type Type)
{
    auto *Mesh = GetMesh(Type);
    return &Mesh->MeshFilter;
}

shoora_mesh_filter *
shoora_mesh_database::GetCustomMeshFilter(i32 MeshId)
{
    ASSERT((MeshId >= 0) && (MeshId < CustomPolyCount));

    shoora_mesh_filter *MeshFilter = &CustomPolyMeshes[MeshId].MeshFilter;

    return MeshFilter;
}

VkBuffer *
shoora_mesh_database::GetVertexBufferHandlePtr()
{
    VkBuffer *Result = &vBuffer.Handle;
    return Result;
}

VkBuffer
shoora_mesh_database::GetIndexBufferHandle()
{
    VkBuffer Result = iBuffer.Handle;
    return Result;
}

void
shoora_mesh_database::Destroy()
{
    if (TotalVertexCount == 0 || TotalIndexCount == 0)
    {
        return;
    }

    TotalVertexCount = 0;
    TotalIndexCount = 0;
    free(Memory);
    Vertices = nullptr;
    Indices = nullptr;
    DestroyVertexBuffer(RenderDevice, &vBuffer, &iBuffer);
}
