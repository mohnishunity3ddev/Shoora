#if !defined(MATH_BARYCENTRIC_H)
#define MATH_BARYCENTRIC_H

#include <defines.h>
#include "math_vector.h"

namespace shu
{
// NOTE: A, B, C should be in clockwise winding order for positive triangle area.
struct triangle
{
    vec3f A;
    vec3f B;
    vec3f C;
};

// NOTE: Barycentric stuff
// TODO: NOT TESTED
vec3f BarycentricCramersMethod(const triangle &Tri, const vec3f &P);

vec3f BarycentricProjectionMethod(const triangle &Tri, const vec3f &P);
vec3f BarycentricCrossMethod(const triangle &Tri, const vec3f &P);
void TestBarycentric();

// NOTE: Triangle Area Stuff
f32 TriangleArea3D(const triangle &Tri);
f32 TriangleArea3D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C);
f32 TriangleArea3DHerons(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C);

f32 TriangleArea2D(const shu::vec2f &A, const shu::vec2f &B, const shu::vec2f &C);
f32 DoubleTriangleArea2D(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
}; // namespace shu

#endif