#if !defined(EPA_H)
#define EPA_H

#include <defines.h>
#include <math/math.h>
#include <containers/dynamic_array.h>
#include "shape/shape.h"
#include "gjk.h"

f32 EPA_Expand(const shoora_body *A, const shoora_body *B, const f32 Bias, const gjk_point SimplexPoints[4],
               shu::vec3f &PointOnA, shu::vec3f &PointOnB);

#endif // EPA_H