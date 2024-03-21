#include "signed_volumes.h"

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
