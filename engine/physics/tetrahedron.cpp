#include "tetrahedron.h"

f32
TetrahedronVolume(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C, const shu::vec3f &D)
{
    const shu::vec3f AD = D - A;
    const shu::vec3f BD = D - B;
    const shu::vec3f CD = D - C;

    // NOTE: Scalar Triple Product of the edges of a tetrahedron gives 6 times the volume of the tetrahedron.
    f32 Numer = shu::ScalarTripleProduct(AD, BD, CD);
    f32 Volume = SHU_ABSOLUTE(Numer / 6.0f);

    return Volume;
}

shu::vec3f
CalculateCenterOfMassTetrahedron(const shu::vec3f *Points, i32 NumPoints,
                                 const tri_t *Triangles, i32 NumTriangles, memory_arena *Arena)
{
    ASSERT(NumPoints > 0);
    ASSERT(NumTriangles > 0);

    if(Arena == nullptr) { Arena = GetArena(MEMTYPE_GLOBAL); }

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    size_t MemSize = MEGABYTES(1);
    void *Memory = ShuAllocate_(TempMem.Arena, MemSize);
    freelist_allocator Freelist{Memory, MemSize};

    shoora_dynamic_array<shu::vec3f> CenterOfMasses{&Freelist, NumTriangles};
    shoora_dynamic_array<f32> Volumes{&Freelist, NumTriangles};
    f32 TotalVolume = 0.0f;

    shu::vec3f Centerish = shu::Vec3f(0.0f);
    for(i32 i = 0; i < NumPoints; ++i)
    {
        Centerish += Points[i];
    }
    Centerish *= 1.0f / (f32)(NumPoints);

    for (i32 i = 0; i < NumTriangles; ++i)
    {
        const tri_t &Triangle = Triangles[i];

        const shu::vec3f &PointA = Centerish;
        const shu::vec3f &PointB = Points[Triangle.A];
        const shu::vec3f &PointC = Points[Triangle.B];
        const shu::vec3f &PointD = Points[Triangle.C];

        const shu::vec3f CenterOfMassSimplex = (PointA + PointB + PointC + PointD) * 0.25f;
        const f32 Volume = TetrahedronVolume(PointA, PointB, PointC, PointD);

        CenterOfMasses.emplace_back(CenterOfMassSimplex);
        Volumes.emplace_back(Volume);

        TotalVolume += Volume;
    }

    shu::vec3f CenterOfMass = shu::Vec3f(0.0f);
    for (i32 i = 0; i < CenterOfMasses.size(); ++i)
    {
        CenterOfMass += CenterOfMasses[i] * Volumes[i];
    }
    CenterOfMass *= 1.0f / TotalVolume;

    EndTemporaryMemory(TempMem);

    return CenterOfMass;
}

// IMPORTANT: NOTE: Refer to this paper here: https://thescipub.com/pdf/jmssp.2005.8.11.pdf
shu::mat3f
InertiaTensorTetrahedron(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C, const shu::vec3f &D)
{
    const shu::vec3f Points[] = {A, B, C, D};

    shu::mat3f Mat;
    Mat.Rows[0] = shu::Vec3f(B.x-A.x, C.x-A.x, D.x-A.x);
    Mat.Rows[1] = shu::Vec3f(B.y-A.y, C.y-A.y, D.y-A.y);
    Mat.Rows[2] = shu::Vec3f(B.z-A.z, C.z-A.z, D.z-A.z);
    const f32 DeterminantJ = SHU_ABSOLUTE(Mat.Determinant());

    const f32 Density = 1.0f;
    const f32 Mu = Density;

    f32 xx = 0.0f, yy = 0.0f, zz = 0.0f;
    f32 xy = 0.0f, xz = 0.0f, yz = 0.0f;

    for (i32 i = 0; i < 4; ++i)
    {
        for (i32 j = 0; j < 4; ++j)
        {
            // Diagonals
            xx += Points[i].x * Points[j].x;
            yy += Points[i].y * Points[j].y;
            zz += Points[i].z * Points[j].z;

            // Off-Diagonals
            xy += Points[i].x * Points[j].y + Points[j].x * Points[i].y;
            xz += Points[i].x * Points[j].z + Points[j].x * Points[i].z;
            yz += Points[i].y * Points[j].z + Points[j].y * Points[i].z;
        }
    }
    const f32 a = Mu * DeterminantJ * (yy + zz) / 60.0f;
    const f32 b = Mu * DeterminantJ * (xx + zz) / 60.0f;
    const f32 c = Mu * DeterminantJ * (xx + yy) / 60.0f;
    const f32 aprime = Mu * DeterminantJ * yz / 120.0f;
    const f32 bprime = Mu * DeterminantJ * xz / 120.0f;
    const f32 cprime = Mu * DeterminantJ * xy / 120.0f;

    shu::mat3f InertiaTensor;
    InertiaTensor.Rows[0] = shu::Vec3f(a, -cprime, -bprime);
    InertiaTensor.Rows[1] = shu::Vec3f(-cprime, b, -aprime);
    InertiaTensor.Rows[2] = shu::Vec3f(-bprime, -aprime, c);

    return InertiaTensor;
}

shu::mat3f
CalculateIntertiaTensorTetrahedron(const shu::vec3f *Points, i32 NumPoints, const tri_t *Triangles,
                                   i32 NumTriangles, const shu::vec3f &CenterOfMass)
{
    shu::mat3f InertiaTensor = shu::Mat3f(0.0f);
    f32 TotalVolume = 0.0f;

    for (i32 i = 0; i < NumTriangles; ++i)
    {
        const tri_t &Triangle = Triangles[i];

        const shu::vec3f A = shu::Vec3f(0.0f);
        const shu::vec3f B = Points[Triangle.A] - CenterOfMass;
        const shu::vec3f C = Points[Triangle.B] - CenterOfMass;
        const shu::vec3f D = Points[Triangle.C] - CenterOfMass;

        shu::mat3f Tensor = InertiaTensorTetrahedron(A, B, C, D);
        InertiaTensor += Tensor;

        const f32 Volume = TetrahedronVolume(A, B, C, D);
        TotalVolume += Volume;
    }

    InertiaTensor *= 1.0f / TotalVolume;
    return InertiaTensor;
}
