#include "math_barycentric.h"

namespace shu
{
// NOTE: Calculate Barycentric coordinates of a point P, wrt a triange ABC
// (u,v,w) are the barycentric ccoordinates.
// P = uA + vB + wC  and u+v+w = 1(definition of barycentric coordinates)
// P = A + u(B - A) + v(C - A)
// P - A = u(B - A) + v(C - A)
// v2 = u*v0 + v*v1
// u*v0.v0 + v*v1.v0 = v2.v0 ---- equation 1
// u*v0.v1 + v*v1.v1 = v2.v1 ---- equation 2
// these two are system of linear equations which can be solved using Cramer's rule.
// and solve for u and v. w then can be solved using w = 1-u-v
// TODO: NOT TESTED
vec3f
BarycentricCramersMethod(const triangle &Tri, const vec3f &P)
{
    vec3f Result = {};

    vec3f v0 = Tri.C - Tri.A;
    vec3f v1 = Tri.B - Tri.A;
    vec3f v2 = P - Tri.A;

    f32 d00 = v0.Dot(v0);
    f32 d01 = v0.Dot(v1);
    f32 d11 = v1.Dot(v1);
    f32 d02 = v0.Dot(v2);
    f32 d12 = v1.Dot(v2);

    f32 denom = d00 * d11 - d01 * d01;
    ASSERT(!NearlyEqual(denom, 0.0f));

    f32 uNorm = d02 * d11 - d01 * d12;
    f32 vNorm = d00 * d12 - d02 * d01;

    f32 u = uNorm / denom;
    f32 v = vNorm / denom;
    f32 w = 1.0f - u - v;

    Result = {u, v, w};

    return Result;
}

// NOTE: P divides the triangle(if it is inside the triangle) into three triangle.
// then P(u,v,w) barycentric can be found using u=Area(PBC)/Area(ABC), v=Area(PAC)/Area(ABC),
// w=Area(PAB)/Area(ABC)
// Here, ABC, PBC, PAC and PAB are in clockwise winding order we are assuming.

// NOTE: Barycentric coordinates do not change for the 3D triangle even though the triangle was projected
// on a 2D plane. So we figure the normal of the triangle, get the greatest component of the normal, and we
// pick out that axis to calculate area of all projected triangles. If normal.x is greatest, we drop the x
// component, we project triangles on to the yz plane and then calculate area of triangle ratios to get the
// barycentric coordinates.
vec3f
BarycentricProjectionMethod(const triangle &Tri, const vec3f &p)
{

    auto a = Tri.A;
    auto b = Tri.B;
    auto c = Tri.C;

    auto ab = b - a;
    auto bc = c - b;
    auto ca = a - c;

    // 2 * Area of ABC. Also the normal
    vec3f n = shu::Cross(ab, bc);

#if _SHU_DEBUG
    vec3f norm = shu::Normalize(n);
    f32 dist = (p - b).Dot(norm);
    ASSERT(NearlyEqual(dist, 0.0f, 0.001f));
#endif

    f32 numeratorU, numeratorV, oneOverDenom;

    f32 nX = SHU_ABSOLUTE(n.x);
    f32 nY = SHU_ABSOLUTE(n.y);
    f32 nZ = SHU_ABSOLUTE(n.z);

    // NOTE: X Component of the triangle normal is largest, project the triangle onto the YZ Plane and
    // calculate the areas.
    if (nX >= nY && nX >= nZ)
    {
        numeratorU = DoubleTriangleArea2D(p.y, p.z, b.y, b.z, c.y, c.z); // (2*Area(PBC) projected on to YZ Plane)
        numeratorV = DoubleTriangleArea2D(p.y, p.z, c.y, c.z, a.y, a.z); // (2*Area(PCA) projected on to YZ Plane)
        oneOverDenom = 1.0f / n.x; // n.x = (2*Area(ABC) Projected onto YZ Plane)
    }
    else if (nY >= nX && nY >= nZ) // XZ Plane
    {
        numeratorU = DoubleTriangleArea2D(p.x, p.z, b.x, b.z, c.x, c.z); // (2*Area(PBC) projected on to YZ Plane)
        numeratorV = DoubleTriangleArea2D(p.x, p.z, c.x, c.z, a.x, a.z); // (2*Area(PCA) projected on to YZ Plane)
        oneOverDenom = -1.0f / n.y; // n.y = (2*Area(ABC) Projected onto YZ Plane)
    }
    else if (nZ >= nX && nZ >= nY)  // XY Plane
    {
        numeratorU = DoubleTriangleArea2D(p.x, p.y, b.x, b.y, c.x, c.y); // (2*Area(PBC) projected on to YZ Plane)
        numeratorV = DoubleTriangleArea2D(p.x, p.y, c.x, c.y, a.x, a.y); // (2*Area(PCA) projected on to YZ Plane)
        oneOverDenom = 1.0f / n.z; // n.z = (2*Area(ABC) Projected onto YZ Plane)
    }

    f32 u = numeratorU * oneOverDenom;
    f32 v = numeratorV * oneOverDenom;
    f32 w = 1.0f - u - v;

#if _SHU_DEBUG
    auto test = u * a + v * b + w * c;
    ASSERT(NearlyEqual(test.x, p.x, 0.001f));
    ASSERT(NearlyEqual(test.y, p.y, 0.001f));
    ASSERT(NearlyEqual(test.z, p.z, 0.001f));
#endif

    shu::vec3f Result = {u, v, w};
    return Result;
}

// NOTE: Area of triangle can also be 0.5f*(edge1.Cross(edge2))
vec3f
BarycentricCrossMethod(const triangle &Tri, const vec3f &p)
{
    auto a = Tri.A;
    auto b = Tri.B;
    auto c = Tri.C;

    shu::vec3f ab = b - a;
    shu::vec3f bc = c - b;
    shu::vec3f ca = a - c;

    shu::vec3f n = ab.Cross(bc);

#if _SHU_DEBUG
    auto norm = shu::Normalize(n);
    f32 dist = (p - b).Dot(norm);
    ASSERT(NearlyEqual(dist, 0.0f, 0.001f));
#endif

    shu::vec3f ap = p - a;
    shu::vec3f bp = p - b;
    shu::vec3f cp = p - c;

    f32 abc = shu::Dot(ab.Cross(bc), n); // * 0.5f;
    f32 pbc = shu::Dot(bc.Cross(cp), n); // * 0.5f;
    f32 pca = shu::Dot(ca.Cross(ap), n); // * 0.5f;
    // f32 pab = shu::Dot(ab.Cross(bp), n); // * 0.5f;

    f32 u = pbc / abc;
    f32 v = pca / abc;
    // f32 w = pab / abc;
    f32 w = 1.0 - u - v;

#if _SHU_DEBUG
    auto test = u * a + v * b + w * c;
    ASSERT(NearlyEqual(test.x, p.x, 0.001f));
    ASSERT(NearlyEqual(test.y, p.y, 0.001f));
    ASSERT(NearlyEqual(test.z, p.z, 0.001f));
#endif

    shu::vec3f Result = {u, v, w};
    return Result;
}

void
TestBarycentric()
{
    auto a = shu::Vec3f(2, -5, 7);
    auto b = shu::Vec3f(3, 1, -9);
    auto c = shu::Vec3f(-4, 5, 10);
    shu::triangle Tri = {a, b, c};
    shu::vec3f P = shu::Vec3f(-0.21f, 1.36f, 2.69f);

    auto b1 = shu::BarycentricProjectionMethod(Tri, P);
    auto b2 = shu::BarycentricCrossMethod(Tri, P);

    i32 x = 0;
}

f32
TriangleArea3D(const triangle &Tri)
{
    f32 Result = 0.0f;
    Result = TriangleArea3D(Tri.A, Tri.B, Tri.C);

    return Result;
}

f32
TriangleArea3D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    f32 Result = 0.0f;

    vec3f AB = B - A;
    vec3f AC = C - A;

    vec3f ABCrossAC = AB.Cross(AC);
    vec3f n = shu::Normalize(ABCrossAC);

    Result = ABCrossAC.Dot(n) * 0.5f;

    return Result;
}

f32
TriangleArea3DHerons(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    auto ab = B - A;
    auto bc = C - B;
    auto ca = A - C;

    auto l1 = ab.Magnitude();
    auto l2 = bc.Magnitude();
    auto l3 = ca.Magnitude();

    auto s = (l1 + l2 + l3) / 2;

    f32 Area = sqrtf(s * (s - l1) * (s - l2) * (s - l3));
    return Area;
}

f32
TriangleArea2D(const shu::vec2f &A, const shu::vec2f &B, const shu::vec2f &C)
{
    f32 Result = 0.0f;

    Result = (B.x - A.x) * (C.y - B.y) - (C.x - B.x) * (B.y - A.y);
    Result *= 0.5f;

    return Result;
}

f32
DoubleTriangleArea2D(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    f32 Result = 0.0f;

    Result = (x2 - x1) * (y3 - y2) - (x3 - x2) * (y2 - y1);
    return Result;
}
}; // namespace shu
