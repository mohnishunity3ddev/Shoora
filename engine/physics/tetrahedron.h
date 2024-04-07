#if !defined(TETRAHEDRON_H)
#define TETRAHEDRON_H

#include <defines.h>
#include <math/math.h>
#include <containers/dynamic_array.h>

struct tri_t
{
    i32 A;
    i32 B;
    i32 C;

    inline void
    Validate()
    {
#if !_SHU_DEBUG
        ASSERT(!"This should not be called in a NON Debug Build!");
#endif
        ASSERT(this->A != this->B);
        ASSERT(this->A != this->C);
        ASSERT(this->B != this->C);
    }
};

struct edge_t
{
    i32 A;
    i32 B;

    b32
    operator==(const edge_t &Other) const
    {
        b32 Result = ((A == Other.A && B == Other.B) || (A == Other.B && B == Other.A));
        return Result;
    }

    inline void
    Validate()
    {
#if !_SHU_DEBUG
        ASSERT(!"This should not be called in a NON Debug Build!");
#endif
        ASSERT(this->A != this->B);
    }
};

shu::vec3f CalculateCenterOfMassTetrahedron(const shu::vec3f *Points, i32 NumPoints, const tri_t *Triangles,
                                            i32 NumTriangles, memory_arena *Arena);
shu::mat3f CalculateIntertiaTensorTetrahedron(const shu::vec3f *Points, i32 NumPoints, const tri_t *Triangles,
                                              i32 NumTriangles, const shu::vec3f &CenterOfMass);

#endif // TETRAHEDRON_H