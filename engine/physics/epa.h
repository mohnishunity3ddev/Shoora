#if !defined(EPA_H)
#define EPA_H

#include <defines.h>
#include <math/math.h>
#include <containers/dynamic_array.h>
#include "shape/shape.h"
#include "gjk.h"

#define EPA_DEBUG 1

#if EPA_DEBUG
struct epa_debug_result
{
    b32 DoubleSided = false;
    gjk_point *GJKPoints;

    i32 NewPointIndex = -1;
    shu::vec3f NormalDir = shu::Vec3f(0.0f);

    tri_t Triangles[128];
    i32 TriangleCount = 0;

    edge_t DanglingEdges[64];
    i32 EdgeCount = 0;
};
#endif

f32 EPA_Expand(const shoora_body *A, const shoora_body *B, const f32 Bias, const gjk_point SimplexPoints[4],
               shu::vec3f &PointOnA, shu::vec3f &PointOnB);

#endif // EPA_H