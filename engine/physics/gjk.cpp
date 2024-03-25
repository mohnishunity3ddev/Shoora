#include "gjk.h"

f32 EPA_Expand(const shoora_body *A, const shoora_body *B, const f32 Bias, const gjk_point SimplexPoints[4],
               shu::vec3f &PointOnA, shu::vec3f &PointOnB);

shu::vec2f
SignedVolume1D(const shu::vec3f &s1, const shu::vec3f &s2)
{
    shu::vec2f Result;

    shu::vec3f ab = s2 - s1;
    shu::vec3f ao = shu::Vec3f(0.0f) - s1;
    shu::vec3f pp = s1 + ab * (ao.Dot(ab) / ab.SqMagnitude());

    i32 MaxAxis = 0;
    f32 MaxMagnitude = ab.x;
    for(i32 i = 1; i < 3; ++i)
    {
        f32 m = s2[i] - s1[i];
        // NOTE: Absolute Value of the projection.
        if(m*m > MaxMagnitude*MaxMagnitude)
        {
            MaxAxis = i;
            MaxMagnitude = m;
        }
    }

    f32 a = s1[MaxAxis];
    f32 b = s2[MaxAxis];
    f32 p = pp[MaxAxis];

    f32 c1 = p - a;
    f32 c2 = b - p;

    // NOTE: c2 represents how close origin is to point b. if c2 is small, origin is closer to point b.
    // meaning the factor that is multiplied with a for barycentric coordinate should be low. which would mean the
    // point needs less of a and more of b since its closer to b
    // In other words, point p if represented by this eq: t*a + (1-t)*b. and c2 is low(meaning closer to b), then
    // t*a should be low. so barycentric would be (c2, c1) which means p = c2*a + c1*b where c1 + c2 = 1.
    if ((p > a && p < b) || (p > b && p < a))
    {
        Result.x = c2 / MaxMagnitude;
        Result.y = c1 / MaxMagnitude;
        return Result;
    }

    if (((a <= b) && (p <= a)) || (a >= b) && (p >= a)) {
        Result.x = 1.0f;
        Result.y = 0.0f;
        return Result;
    }

    return shu::Vec2f(0.0f, 1.0f);
}

shu::vec2f
SignedVolume1D_Optimized(const shu::vec3f &A, const shu::vec3f &B)
{
    shu::vec3f AP = shu::Vec3f(0.0f) - A;
    shu::vec3f AB = B - A;

    f32 ProjTimesABLength = shu::Dot(AP, AB);
    f32 ABLengthSquared = shu::Dot(AB, AB);

    f32 t = (ProjTimesABLength / ABLengthSquared);
    t = ClampToRange(t, 0.0f, 1.0f);

    shu::vec2f baryCoord = shu::Vec2f(1.0f - t, t);

    return baryCoord;
}

i32 CompareSigns(f32 a, f32 b)
{
    if(((a > 0.0f) && (b > 0.0f)) || (a < 0.0f) && (b < 0.0f))
    {
        return 1;
    }

    return 0;
}

shu::vec3f
SignedVolume2D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    shu::vec3f ab = B - A;
    shu::vec3f ac = C - A;

    shu::vec3f Normal = ab.Cross(ac);
    shu::vec3f pProj = Normal * (A.Dot(Normal) / Normal.SqMagnitude());

    // NOTE: Find the axes with the largest projected area.
    // NOTE: We could also do this by taking the normal and seeing which component of the normal has the highest
    // absolute value. if Normal.x is highest, YZ is the max area axis. This is the long more verbose form.
    i32 MaxAxis = 0;
    f32 MaxArea = 0.0f;
    for(i32 i = 0; i < 3; ++i)
    {
        i32 j = (i + 1) % 3;
        i32 k = (i + 2) % 3;

        shu::vec2f a = shu::Vec2f(A[j], A[k]);
        shu::vec2f b = shu::Vec2f(B[j], B[k]);
        shu::vec2f c = shu::Vec2f(C[j], C[k]);
        shu::vec2f abProj = b - a;
        shu::vec2f acProj = c - a;

        // NOTE: Signed area. Proportional to cross product of ab and ac
        f32 areaProj = abProj.x * acProj.y - abProj.y * acProj.x;
        if((areaProj * areaProj) > (MaxArea * MaxArea))
        {
            MaxAxis = i;
            MaxArea = areaProj;
        }
    }

    i32 x = (MaxAxis + 1) % 3;
    i32 y = (MaxAxis + 2) % 3;
    shu::vec2f v[3];
    v[0] = shu::Vec2f(A[x], A[y]);
    v[1] = shu::Vec2f(B[x], B[y]);
    v[2] = shu::Vec2f(C[x], C[y]);
    shu::vec2f p = shu::Vec2f(pProj[x], pProj[y]);

    // NOTE: Get signed areas of the projected triangles - PAB, PBC, PCA
    shu::vec3f Areas;
    for (i32 i = 0; i < 3; ++i)
    {
        i32 j = (i + 1) % 3;
        i32 k = (i + 2) % 3;

        shu::vec2f a = p;
        shu::vec2f b = v[j];
        shu::vec2f c = v[k];

        shu::vec2f ab = b - a;
        shu::vec2f ac = c - a;

        Areas[i] = ab.x*ac.y - ab.y*ac.x;
    }

    // NOTE: Signed Areas are all of the same sign of area ABC when the point is inside the triangle.
    f32 projAreaABC = MaxArea;
    if ((CompareSigns(projAreaABC, Areas[0]) > 0) &&
        (CompareSigns(projAreaABC, Areas[1]) > 0) &&
        (CompareSigns(projAreaABC, Areas[2]) > 0))
    {
        // NOTE: Bary coordinates of a point which is guaranteed to be inside the triangle.
        shu::vec3f Barycentric = Areas / projAreaABC;
        return Barycentric;
    }

    // NOTE: Point is outside the triangle projected ABC(or on one of the edges). In this case, point is projected
    // onto the edge/vertex of triangle ABC.
    f32 dist = SHU_FLOAT_MAX;
    shu::vec3f Barycentric = shu::Vec3f(1, 0, 0);
    shu::vec3f edgePts[3] = {A, B, C};
    for (i32 i = 0; i < 3; ++i)
    {
        i32 k = (i + 1) % 3;
        i32 l = (i + 2) % 3;

        shu::vec2f lineBaryCoords = SignedVolume1D_Optimized(edgePts[k], edgePts[l]);
        shu::vec3f pt = edgePts[k]*lineBaryCoords[0] + edgePts[l]*lineBaryCoords[1];

        // NOTE: We need to return the bary coordinates of the point closest to the triangle ABC
        if(pt.SqMagnitude() < dist)
        {
            dist = pt.SqMagnitude();
            Barycentric[i] = 0;
            Barycentric[k] = lineBaryCoords[0];
            Barycentric[l] = lineBaryCoords[1];
        }
    }

    return Barycentric;
}

shu::vec3f
SignedVolume2D_Optimized(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    shu::vec3f Point = shu::Vec3f(0.0f);

    shu::vec3f AB = B - A;
    shu::vec3f AC = C - A;
    shu::vec3f AP = Point - A;

    f32 d1 = shu::Dot(AB, AP);
    f32 d2 = shu::Dot(AC, AP);
    if (d1 <= 0.0f && d2 <= 0.0f)
    {
        // shu::vec3f closestPoint = A; // NOTE:
        shu::vec3f barycentric = shu::Vec3f(1, 0, 0);
        return barycentric;
    }

    shu::vec3f BP = Point - B;
    shu::vec3f BC = C - B;
    f32 d3 = shu::Dot(AB, BP);
    f32 d4 = shu::Dot(AC, BP);
    if (d3 >= 0.0f && d4 <= d3)
    {
        // shu::vec3f closestPoint = B; // NOTE:
        shu::vec3f barycentric = shu::Vec3f(0, 1, 0);
        return barycentric;
    }


    f32 vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
    {
        f32 t = d1 / (d1 - d3);
        // shu::vec3f closestPoint = A + t*AB; // NOTE:
        shu::vec3f barycentric = shu::Vec3f(1.0f - t, t, 0);

        return barycentric;
    }

    shu::vec3f CP = Point - C;
    f32 d5 = shu::Dot(AB, CP);
    f32 d6 = shu::Dot(AC, CP);
    if (d6 >= 0.0f && d5 <= d6)
    {
        // shu::vec3f closestPoint = C; // NOTE:
        shu::vec3f barycentric = shu::Vec3f(0, 0, 1);
        return barycentric;
    }

    f32 vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d6 <= 0.0f && d2 >= 0.0f)
    {
        f32 t = d2 / (d2 - d6);
        // shu::vec3f closestPoint = A + *C; // NOTE:
        shu::vec3f barycentric = shu::Vec3f(1.0f - t, 0, t);

        return barycentric;
    }

    f32 va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
    {
        float t = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        // shu::vec3f closestPoint = B + *C - B); // NOTE:
        shu::vec3f barycentric = shu::Vec3f(0, 1.0f - t, t);
        return barycentric;
    }

    float denom = 1.0f / (va + vb + vc);
    float v = vb*denom;
    float w = vc*denom;

    // shu::vec3f closestPoint = A + AB * v + AC * w; // NOTE:
    shu::vec3f barycentric = shu::Vec3f((1.0f-v-w), v, w);
    return barycentric;
}

shu::vec4f
SignedVolume3D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C, const shu::vec3f &D)
{
    shu::mat4f M;
    // NOTE: The Determinant of this matrix is 6.0f * Volume of Tetrahedron(ABCD)
    M.Row0 = shu::Vec4f(A.x,  B.x,  C.x,  D.x);
    M.Row1 = shu::Vec4f(A.y,  B.y,  C.y,  D.y);
    M.Row2 = shu::Vec4f(A.z,  B.z,  C.z,  D.z);
    M.Row3 = shu::Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

    shu::vec4f c4;
    // NOTE: Signed Volume of PBCD Since P is origin, so a 4x4 matrix gets collapsed to a 3x3 matrix since one
    // column of this 4x4 matrix would be [0 0 0] which is P(origin).
    c4.x = M.CoFactor(3, 0);
    // NOTE: Signed Volume (APCD)
    c4.y = M.CoFactor(3, 1);
    // NOTE: Signed Volume (ABPD)
    c4.z = M.CoFactor(3, 2);
    // NOTE: Signed Volume (ABCP)
    c4.w = M.CoFactor(3, 3);

    // NOTE: Signed Volume of the tetrahedron ABCD.
    const f32 DetM = c4.x + c4.y + c4.z + c4.w;
    // NOTE: Asserting theat the determinant is not zero, if this were the case then abcd would be coplanar and not
    // tetrahedron.
    ASSERT(!NearlyEqual(DetM, 0.0f));

    // NOTE: if all volumes pbcd, apcd, abpd, abcp have the same sign of volume with abcd, then P is inside the
    // tetrahedron abcd.
    if((CompareSigns(c4.x, DetM) > 0) && (CompareSigns(c4.y, DetM) > 0) &&
       (CompareSigns(c4.z, DetM) > 0) && (CompareSigns(c4.w, DetM) > 0))
    {
        // IMPORTANT: NOTE: If code reaches here, this would also mean that the origin is contained inside the
        // tetrahedron ABCD.
        f32 oneOverVolABCD = 1.0f / DetM;
        f32 u = c4.x * oneOverVolABCD;
        f32 v = c4.y * oneOverVolABCD;
        f32 w = c4.z * oneOverVolABCD;
        shu::vec4f baryCoords = shu::Vec4f(u, v, w, 1.0f - u - v - w);
        return baryCoords;
    }

    // NOTE: IF code reaches here, this means origin is not inside the tetrahedron, in this case, we need to
    // project the origin onto the closest face in the ABCD tetrahedron and return the barycentric coords of that
    // projected point on to one of its triangular faces.
    shu::vec4f baryCoords = shu::Vec4f(0, 0, 0, 0);
    f32 minDistSq = SHU_FLOAT_MAX;

    // NOTE: Cycle through the faces of the tetrahedron.
    shu::vec3f faceVertices[4];
    faceVertices[0] = A;
    faceVertices[1] = B;
    faceVertices[2] = C;
    faceVertices[3] = D;
    for (i32 i = 0; i < 4; ++i)
    {
        i32 j = (i + 1) % 4;
        i32 k = (i + 2) % 4;

        // NOTE: Barycentric coordinates of the point which comes after projecting the origin on this face of the
        // tetrahedron.
        shu::vec3f baryOrigin = SignedVolume2D_Optimized(faceVertices[i], faceVertices[j], faceVertices[k]);
        shu::vec3f projectedOrigin = baryOrigin.x*faceVertices[i] + baryOrigin.y*faceVertices[j] +
                                     baryOrigin.z*faceVertices[k];

        f32 projDistSquared = projectedOrigin.SqMagnitude();
        if(projDistSquared < minDistSq)
        {
            minDistSq = projDistSquared;

            baryCoords.ZeroOut();
            baryCoords[i] = baryOrigin[0];
            baryCoords[j] = baryOrigin[1];
            baryCoords[k] = baryOrigin[2];
        }
    }

    return baryCoords;
}

gjk_point
GJK_Support(const shoora_body *A, const shoora_body *B, shu::vec3f Dir, const f32 Bias)
{
    // NOTE: Calculates the support point on the monkowski difference convex shape of the two bodies.
    // Since GJK does not calculate the entire minkowski difference, it just gets a subset of the minkowski
    // difference vertices in specific directions that the algorithm chooses.
    // NOTE: You will get a support point on the Minkowski Difference by getting the support point on A in the
    // given direction, subtracted by the support point in body B in the opposite direction.
    gjk_point Point;

    Dir = shu::Normalize(Dir);
    // NOTE: Point on A furthest in the given Direction.
    Point.PointOnA = A->Shape->SupportPtWorldSpace(Dir,  A->Position, A->Rotation, Bias);
    // NOTE: Point on B furthest in the opposite Direction to the one given.
    Point.PointOnB = B->Shape->SupportPtWorldSpace(-Dir, B->Position, B->Rotation, Bias);

    // NOTE: Point on the Minkowski Difference convex shape furthest in the given direction.
    Point.MinkowskiPoint = Point.PointOnA - Point.PointOnB;

    return Point;
}

// NOTE: Project the origin onto the simplex to acquire the new search direction. Also checks if the origin is
// inside the simplex.
b32
SimplexSignedVolumes(gjk_point *Points, const i32 Num, shu::vec3f &NewDir, shu::vec4f &BaryCoords)
{
    const f32 Epsilon = 0.0001f * 0.0001f;
    BaryCoords.ZeroOut();

    b32 doesIntersect = false;
    switch(Num)
    {
        default:
        case 2:
        {
            shu::vec2f baryCoordsLine = SignedVolume1D_Optimized(Points[0].MinkowskiPoint, Points[1].MinkowskiPoint);
            shu::vec3f projectedPoint = baryCoordsLine.x * Points[0].MinkowskiPoint +
                                        baryCoordsLine.y * Points[1].MinkowskiPoint;

            NewDir = projectedPoint * -1.0f;
            // NOTE: If the 1-simplex line is too close to the origin to be intersecting.
            doesIntersect = (projectedPoint.SqMagnitude() < Epsilon);
            BaryCoords.x = baryCoordsLine.x;
            BaryCoords.y = baryCoordsLine.y;
        } break;

        case 3:
        {
            shu::vec3f baryCoordsTriangle = SignedVolume2D_Optimized(Points[0].MinkowskiPoint,
                                                                     Points[1].MinkowskiPoint,
                                                                     Points[2].MinkowskiPoint);
            shu::vec3f projectedPoint = baryCoordsTriangle.x * Points[0].MinkowskiPoint +
                                        baryCoordsTriangle.y * Points[1].MinkowskiPoint +
                                        baryCoordsTriangle.z * Points[2].MinkowskiPoint;

            NewDir = projectedPoint * -1.0f;
            doesIntersect = (projectedPoint.SqMagnitude() < Epsilon);
            BaryCoords.x = baryCoordsTriangle.x;
            BaryCoords.y = baryCoordsTriangle.y;
            BaryCoords.z = baryCoordsTriangle.z;
        } break;

        case 4:
        {
            shu::vec4f baryCoordsTetrahedron = SignedVolume3D(Points[0].MinkowskiPoint, Points[1].MinkowskiPoint,
                                                              Points[2].MinkowskiPoint, Points[3].MinkowskiPoint);
            shu::vec3f projectedPoint = baryCoordsTetrahedron.x * Points[0].MinkowskiPoint +
                                        baryCoordsTetrahedron.y * Points[1].MinkowskiPoint +
                                        baryCoordsTetrahedron.z * Points[2].MinkowskiPoint +
                                        baryCoordsTetrahedron.w * Points[3].MinkowskiPoint;

            NewDir = projectedPoint * -1.0f;
            doesIntersect = (projectedPoint.SqMagnitude() < Epsilon);
            BaryCoords.x = baryCoordsTetrahedron.x;
            BaryCoords.y = baryCoordsTetrahedron.y;
            BaryCoords.z = baryCoordsTetrahedron.z;
            BaryCoords.w = baryCoordsTetrahedron.w;
        } break;
    }

    return doesIntersect;
}

// NOTE: Tells whether the newPoint we select for simplex set is already there in the simple set.
b32
HasPoint(const gjk_point SimplexPoints[4], const gjk_point &NewPoint)
{
    const f32 precision = 1e-06f;

    for(i32 i = 0; i < 4; ++i)
    {
        shu::vec3f delta = SimplexPoints[i].MinkowskiPoint - NewPoint.MinkowskiPoint;
        if(delta.SqMagnitude() < (precision * precision))
        {
            return true;
        }
    }

    return false;
}

void
SortValids(gjk_point SimplexPoints[4], shu::vec4f &BaryCoords)
{
    b32 Valids[4];
    for(i32 i = 0; i < 4; ++i)
    {
        Valids[i] = true;
        if(NearlyEqual(BaryCoords[i], 0.0f))
        {
            Valids[i] = false;
        }
    }

    shu::vec4f validLambdas{};
    i32 validCount = 0;
    gjk_point validPoints[4];
    SHU_MEMZERO(validPoints, sizeof(gjk_point) * 4);
    for(i32 i = 0; i < 4; i++)
    {
        if(Valids[i])
        {
            validPoints[validCount] = SimplexPoints[i];
            validLambdas[validCount] = BaryCoords[i];
            validCount++;
        }
    }

    // Copy the valids back into simplexPoints
    for (i32 i = 0; i < 4; i++)
    {
        SimplexPoints[i] = validPoints[i];
        BaryCoords[i] = validLambdas[i];
    }
}

static i32
NumValids(const shu::vec4f &BaryCoords)
{
    i32 num = 0;
    for (i32 i = 0; i < 4; i++)
    {
        if (NearlyEqual(BaryCoords[i], 0.0f))
        {
            num++;
        }
    }
    return num;
}

#if GJK_DEBUG
#include <memory/memory.h>
#include <platform/platform.h>
#ifdef WIN32
#include "platform/windows/win_platform.h"
#endif
#include <renderer/vulkan/graphics/vulkan_graphics.h>

static b32 DebugInitialized = false;
static gjk_debug_result *DebugStepsArr;
static i32 DebugStepsCount = 0;
static i32 CurrentStep = 0;

void
InitializeDebug()
{
    if(DebugInitialized)
    {
        SHU_MEMZERO(DebugStepsArr, sizeof(gjk_debug_result) * DebugStepsCount);
        DebugStepsCount = 0;
        CurrentStep = 0;
        return;
    }

    memory_arena *Arena = GetArena(MEMTYPE_GLOBAL);
    size_t TotalSize = MEGABYTES(1);
    DebugStepsArr = (gjk_debug_result *)ShuAllocate_(Arena, TotalSize);
    DebugStepsCount = 0;
    DebugInitialized = true;
}

void
DebugSteps()
{
    if(Platform_GetKeyInputState(SU_ALPHABETKEYM, KeyState::SHU_KEYSTATE_PRESS)) {
        CurrentStep = (CurrentStep + 1) % DebugStepsCount;
    }
    if(Platform_GetKeyInputState(SU_ALPHABETKEYN, KeyState::SHU_KEYSTATE_PRESS)) {
        CurrentStep = (CurrentStep > 0) ? CurrentStep - 1 : 0;
    }

    gjk_debug_result DebugResult = DebugStepsArr[CurrentStep];

    shu::vec3f v0 = shu::Vec3f(-10, 10, -34);
    shu::vec3f v1 = shu::Vec3f(10, 0, 5);
    shu::vec3f v2 = shu::Vec3f(5, 123, 23);
    shoora_graphics::DrawTriangle(v0, v2, v1);
}
#endif

b32
GJK_DoesIntersect(const shoora_body *A, const shoora_body *B, const f32 Bias, shu::vec3f &PointOnA,
                  shu::vec3f &PointOnB)
{
#if GJK_DEBUG
    InitializeDebug();
#endif

    const shu::vec3f Origin = shu::Vec3f(0.0f);

    i32 NumPoints = 1;
    gjk_point SimplexPoints[4];
    SimplexPoints[0] = GJK_Support(A, B, shu::Vec3f(0, 0, 1), 0.0f);

#if GJK_DEBUG
    auto DebugResult = gjk_debug_result(NumPoints, SimplexPoints, {}, false, shu::Vec3f(1), true, false);
    DebugStepsArr[DebugStepsCount++] = (DebugResult);
#endif

    f32 ClosestDistance = 1e10f;
    b32 DoesContainOrigin = false;
    shu::vec3f NewDirection = SimplexPoints[0].MinkowskiPoint * -1.0f;

#if GJK_DEBUG
    DebugResult = gjk_debug_result(NumPoints, SimplexPoints, {}, false, NewDirection, true, false);
    DebugStepsArr[DebugStepsCount++] = (DebugResult);
#endif

    do
    {
        // NOTE: Get the new point to check on.
        gjk_point NewPoint = GJK_Support(A, B, NewDirection, 0.0f);

        // NOTE: If the new point is the same as a previous point, then we can't expand further. A and B don't
        // intersect.
        if(HasPoint(SimplexPoints, NewPoint)) { break; }

        SimplexPoints[NumPoints++] = NewPoint;
#if GJK_DEBUG
        DebugResult = gjk_debug_result(NumPoints, SimplexPoints, {}, false, NewDirection, true, false);
        DebugStepsArr[DebugStepsCount++] = (DebugResult);
#endif

        // NOTE: If the new point has not crossed the origin, then there is no way origin is contained in the
        // minkowski difference.
        f32 dot = NewDirection.Dot(NewPoint.MinkowskiPoint - Origin);
        if(dot < 0.0f) { break; }

        shu::vec4f ProjectedBaryCoords;
        DoesContainOrigin = SimplexSignedVolumes(SimplexPoints, NumPoints, NewDirection, ProjectedBaryCoords);

#if GJK_DEBUG
        DebugResult = gjk_debug_result(NumPoints, SimplexPoints, ProjectedBaryCoords, true, NewDirection, false,
                                       DoesContainOrigin);
        DebugStepsArr[DebugStepsCount++] = (DebugResult);
#endif

        if(DoesContainOrigin) { break; }

        // NOTE: Check that the new projection of the origin onto the simplex is closer than the previous one. So
        // that we can be rest assured that we are getting closer to the origin as we proceed... If we are not
        // getting closer to the origin, then A and B are not colliding.
        f32 Distance = NewDirection.SqMagnitude();
        if(Distance >= ClosestDistance) { break; }
        ClosestDistance = Distance;

        // NOTE: Use the lambdas that support the new search direction, and invalidate any points that don't
        // support it.
        // IMPORTANT: NOTE: Here, we explain why the DoesContainOrigin = NumPoints == 4 statement works below.
        // Say you have 4 points before calling SimplexSignedVolumes which projects the origin onto the simplex.
        // 4 points meaning you have a tetrahedron. Now, after projection if its found the origin to be inside the
        // tetrahedron, we don't even reach here. If its not, then projection will be on a triangle(max) which will
        // have a max of 3 barycentric coordinates and the 4th one will be zero. After calling these SortValids,
        // NumValids, my NumPoints which were 4 before get set to 3(a triangle). So in this case DoesContainOrigin
        // does not evaluate to true since NumPoints == 3.
        SortValids(SimplexPoints, ProjectedBaryCoords);
        NumPoints = NumValids(ProjectedBaryCoords);

#if GJK_DEBUG
        DebugResult = gjk_debug_result(NumPoints, SimplexPoints, ProjectedBaryCoords, true, NewDirection, true,
                                       DoesContainOrigin);
        DebugStepsArr[DebugStepsCount++] = (DebugResult);
#endif

        DoesContainOrigin = (NumPoints == 4);

    } while (!DoesContainOrigin);

#if GJK_DEBUG
    DebugSteps();
#endif

    if(!DoesContainOrigin)
    {
        return false;
    }

    // NOTE: Ensure that we have a 3-simplex. EPA needs a tetrahedron to work.
    if(NumPoints == 1)
    {
        shu::vec3f SearchDirection = SimplexPoints[0].MinkowskiPoint * -1.0f;
        gjk_point NewPoint = GJK_Support(A, B, SearchDirection, 0.0f);
        SimplexPoints[NumPoints++] = NewPoint;
    }
    if (NumPoints == 2)
    {
        shu::vec3f AB = SimplexPoints[1].MinkowskiPoint - SimplexPoints[0].MinkowskiPoint;
        shu::vec3f u, v;
        AB.GetOrtho(u, v);

        shu::vec3f NewDirection = u;
        gjk_point NewPoint = GJK_Support(A, B, NewDirection, 0.0f);
        SimplexPoints[NumPoints++] = NewPoint;
    }
    if(NumPoints == 3)
    {
        shu::vec3f AB = SimplexPoints[1].MinkowskiPoint - SimplexPoints[0].MinkowskiPoint;
        shu::vec3f BC = SimplexPoints[2].MinkowskiPoint - SimplexPoints[1].MinkowskiPoint;
        shu::vec3f Normal = AB.Cross(BC);

        shu::vec3f NewDirection = Normal;
        gjk_point NewPoint = GJK_Support(A, B, NewDirection, 0.0f);
        SimplexPoints[NumPoints++] = NewPoint;
    }

    // NOTE: Expand the Simplex by the Bias amount

    // NOTE: Get the center point of the simplex.
    shu::vec3f Average = shu::Vec3f(0.0f);
    for (i32 i = 0; i < 4; ++i)
    {
        Average += SimplexPoints[i].MinkowskiPoint;
    }
    Average *= 0.25f;

    // NOTE: Now expand the Simplex by the bias amount
    for (i32 i = 0; i < NumPoints; ++i)
    {
        gjk_point &Point = SimplexPoints[i];

        // NOTE: Ray from center to witness point.
        shu::vec3f Direction = Point.MinkowskiPoint - Average;
        Direction.Normalize();

        Point.PointOnA += Direction * Bias;
        Point.PointOnB -= Direction * Bias;
        Point.MinkowskiPoint = Point.PointOnA - Point.PointOnB;
    }

    // NOTE: Perform EPA Expansion to get the closest face on the Minkowski Difference
    // EPA_Expand(A, B, Bias, SimplexPoints, PointOnA, PointOnB);
    return true;
}

// NOTE: Returns the closest points between two bodies A and B using GJK
void
GJK_ClosestPoints(const shoora_body *A, const shoora_body *B, shu::vec3f &PointOnA, shu::vec3f &PointOnB)
{
    const shu::vec3f Origin = shu::Vec3f(0.0f);

    f32 ClosestDistance = 1e10f;
    const f32 Bias = 0.0f;

    i32 NumPoints = 1;
    gjk_point SimplexPoints[4];
    SimplexPoints[0] = GJK_Support(A, B, shu::Vec3f(1, 1, 1), Bias);

    shu::vec4f Lambdas = shu::Vec4f(1, 0, 0, 0);
    shu::vec3f NewDirection = SimplexPoints[0].MinkowskiPoint * -1.0f;

    do
    {
        // NOTE: Get the new point to check on.
        gjk_point NewPoint = GJK_Support(A, B, NewDirection, Bias);

        // NOTE: If the new point is the same as the previous point, we cannot expand any further.
        if(HasPoint(SimplexPoints, NewPoint))
        {
            break;
        }

        // NOTE: Add Point and get new direction.
        SimplexPoints[NumPoints++] = NewPoint;

        SimplexSignedVolumes(SimplexPoints, NumPoints, NewDirection, Lambdas);
        SortValids(SimplexPoints, Lambdas);
        NumPoints = NumValids(Lambdas);

        // NOTE: Check that the new projection of the origin onto the simplex is closer than the previous one.
        f32 Distance = NewDirection.SqMagnitude();
        if (Distance >= ClosestDistance)
        {
            break;
        }

        ClosestDistance = Distance;
    } while (NumPoints < 4);

    PointOnA.ZeroOut();
    PointOnB.ZeroOut();
    for(i32 i = 0; i < 4; ++i)
    {
        PointOnA += SimplexPoints[i].PointOnA * Lambdas[i];
        PointOnB += SimplexPoints[i].PointOnB * Lambdas[i];
    }
}

#if _SHU_DEBUG
void
TestSignedVolumeProjection()
{
    const shu::vec3f orgPts[4] = {shu::Vec3f(0, 0, 0), shu::Vec3f(1, 0, 0), shu::Vec3f(0, 1, 0), shu::Vec3f(0, 0, 1)};
    shu::vec3f pts[4];
    shu::vec4f lambdas;
    shu::vec3f v;

    for (int i = 0; i < 4; i++)
    {
        pts[i] = orgPts[i] + shu::Vec3f(1, 1, 1);
    }

    lambdas = SignedVolume3D(pts[0], pts[1], pts[2], pts[3]);
    v.ZeroOut();

    for (int i = 0; i < 4; i++)
    {
        v += pts[i] * lambdas[i];
    }

    LogInfo("lambdas: %.3f %.3f %.3f %.3f v: %.3f %.3f %.3f \n", lambdas.x, lambdas.y, lambdas.z, lambdas.w, v.x,
            v.y, v.z);

    for (int i = 0; i < 4; i++)
    {
        pts[i] = orgPts[i] + shu::Vec3f(-1, -1, -1) * 0.25f;
    }

    lambdas = SignedVolume3D(pts[0], pts[1], pts[2], pts[3]);
    v.ZeroOut();
    for (int i = 0; i < 4; i++)
    {
        v += pts[i] * lambdas[i];
    }
    LogInfo("lambdas: %.3f %.3f %.3f %.3f v: %.3f %.3f %.3f \n", lambdas.x, lambdas.y, lambdas.z, lambdas.w, v.x,
            v.y, v.z);

    for (int i = 0; i < 4; i++)
    {
        pts[i] = orgPts[i] + shu::Vec3f(-1, -1, -1);
    }
    lambdas = SignedVolume3D(pts[0], pts[1], pts[2], pts[3]);

    v.ZeroOut();
    for (int i = 0; i < 4; i++)
    {
        v += pts[i] * lambdas[i];
    }
    LogInfo("lambdas: %.3f %.3f %.3f %.3f v: %.3f %.3f %.3f \n", lambdas.x, lambdas.y, lambdas.z, lambdas.w, v.x,
            v.y, v.z);

    for (int i = 0; i < 4; i++)
    {
        pts[i] = orgPts[i] + shu::Vec3f(1, 1, -0.5f);
    }
    lambdas = SignedVolume3D(pts[0], pts[1], pts[2], pts[3]);

    v.ZeroOut();
    for (int i = 0; i < 4; i++)
    {
        v += pts[i] * lambdas[i];
    }
    LogInfo("lambdas: %.3f %.3f %.3f %.3f v: %.3f %.3f %.3f \n", lambdas.x, lambdas.y, lambdas.z, lambdas.w, v.x,
            v.y, v.z);

    pts[0] = shu::Vec3f(51.1996613f, 26.1989613f, 1.91339576f);
    pts[1] = shu::Vec3f(-51.0567360f, -26.0565681f, -0.436143428f);
    pts[2] = shu::Vec3f(50.8978920f, -24.1035538f, -1.04042661f);
    pts[3] = shu::Vec3f(-49.1021080f, 25.8964462f, -1.04042661f);
    lambdas = SignedVolume3D(pts[0], pts[1], pts[2], pts[3]);

    v.ZeroOut();
    for (int i = 0; i < 4; i++)
    {
        v += pts[i] * lambdas[i];
    }
    LogInfo("lambdas: %.3f %.3f %.3f %.3f v: %.3f %.3f %.3f \n", lambdas.x, lambdas.y, lambdas.z, lambdas.w, v.x,
            v.y, v.z);
}
#endif