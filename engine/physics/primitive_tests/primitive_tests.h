#if !defined(PRIMITIVE_TESTS_H)
#define PRIMITIVE_TESTS_H

#include <defines.h>
#include <math/math.h>

// Point-Normal form
// n.(X - P) = 0; where X is a point on the plane.
// n.X = x.P => n.X = d;
// d is the distance of the origin from the plane.
struct plane
{
    shu::vec3f Normal;
    f32 DistanceFromOrigin;
};

// NOTE: A line segment joined by A and B.
struct line_segment
{
    shu::vec3f A;
    shu::vec3f B;
};

struct aabb
{
    shu::vec3f Min;
    shu::vec3f Max;
};

struct obb
{
    shu::vec3f Center;
    shu::vec3f LocalAxes[3];
    shu::vec3f HalfExtents;
};

b32
IsZeroVector(const shu::vec3f &v){
    b32 Result = NearlyEqual(v.x, 0.0f, 0.0001f) &&
                 NearlyEqual(v.y, 0.0f, 0.0001f) &&
                 NearlyEqual(v.z, 0.0f, 0.0001f);

    return Result;
}

b32
AreCollinear(const shu::vec3f &p1, const shu::vec3f &p2, const shu::vec3f &p3)
{
    auto d1 = p2 - p1;
    auto d2 = p3 - p2;

    auto cross = d1.Cross(d2);

    b32 Result = IsZeroVector(cross);
    return Result;
}

plane
BuildPlane(const shu::vec3f &p1, const shu::vec3f &p2, const shu::vec3f &p3)
{
    ASSERT(!AreCollinear(p1, p2, p3));

    auto d1 = p2 - p1;
    auto d2 = p3 - p1;

    auto cross = d1.Cross(d2);

    plane Result;
    Result.Normal = shu::Normalize(cross);
    Result.DistanceFromOrigin = shu::Dot(Result.Normal, p1);

    return Result;
}

f32
DistancePointPlane(const shu::vec3f &Point, const plane &Plane)
{
    ASSERT(!IsZeroVector(Plane.Normal));
    f32 Result = (shu::Dot(Plane.Normal, Point) - Plane.DistanceFromOrigin) / shu::Dot(Plane.Normal, Plane.Normal);
    return Result;
}

shu::vec3f
ClosestPtPointPlane(const shu::vec3f &Point, const plane &Plane)
{
    f32 t = DistancePointPlane(Point, Plane);

    shu::vec3f Result = Point - t * Plane.Normal;
    return Result;
}

shu::vec3f
ClosestPtPointLineSegment(const line_segment &Line, const shu::vec3f &Point)
{
    shu::vec3f AP = Point - Line.A;
    shu::vec3f AB = Line.B - Line.A;

    f32 ProjTimesABLen = shu::Dot(AP, AB);
    f32 LenSquared = shu::Dot(AB, AB);

    f32 t = ProjTimesABLen / LenSquared;
    t = ClampToRange(t, 0.0f, 1.0f);

    // TODO: Remove this
    shu::vec2f baryCoord = shu::Vec2f(1.0f - t, t);
    LogInfo("Bary: %f, %f.\n", baryCoord.x, baryCoord.y);

    shu::vec3f Result = Line.A + t * (Line.B - Line.A);
    return Result;
}

f32
SqDistancePointLineSegment(const line_segment &Line, const shu::vec3f &Point)
{
    shu::vec3f ac = Point - Line.A;
    shu::vec3f ab = Line.B - Line.A;

    f32 Proj = ac.Dot(ab);
    f32 Result = 0.0f;
    if(Proj <= 0.0f)
    {
        Result = ac.Dot(ac);
    }
    else
    {
        f32 abSq = ab.Dot(ab);
        if(Proj >= abSq)
        {
            shu::vec3f bc = Line.B - Point;
            Result = bc.Dot(bc);
        }
        else
        {
            Result = ac.Dot(ac) - ((Proj*Proj) / abSq);
        }
    }

    return Result;
}

shu::vec3f
ClosestPtPointAABB(const shu::vec3f &Point, const aabb &AABB)
{
    shu::vec3f Result;
    for(i32 i = 0; i < 3; ++i)
    {
        f32 v = Point.E[i];
        if(v < AABB.Min.E[i]) v = AABB.Min.E[i];
        if(v > AABB.Max.E[i]) v = AABB.Max.E[i];
        Result.E[i] = v;
    }

    return Result;
}

f32
SqDistancePointAABB(const shu::vec3f &Point, const aabb &b)
{
    // NOTE: If Point is inside AABB, distance is zero.
    f32 Result = 0.0f;
    for(i32 i = 0; i < 3; ++i)
    {
        f32 v = Point.E[i];
        if (v < b.Min.E[i]) Result += (b.Min.E[i] - v) * (b.Min.E[i] - v);
        if (v > b.Max.E[i]) Result += (v - b.Max.E[i]) * (v - b.Max.E[i]);
    }

    return Result;
}

shu::vec3f
ClosestPtPointObb(const shu::vec3f &Point, const obb &b)
{
    shu::vec3f d = Point - b.Center;
    shu::vec3f Result = b.Center;

    for(i32 i = 0; i < 3; ++i)
    {
        const shu::vec3f axes = b.LocalAxes[i];
        ASSERT(axes.IsNormalized());
        f32 dist = d.Dot(axes);
        if(dist >  b.HalfExtents[i]) { dist = b.HalfExtents[i]; }
        else if(dist < -b.HalfExtents[i]) { dist = -b.HalfExtents[i]; }
        Result += dist * axes;
    }

    return Result;
}

f32
SqDistancePointObb(const shu::vec3f &Point, const obb &b)
{
#if 0
    shu::vec3f ClosestPoint = ClosestPtPointObb(Point, b);
    shu::vec3f v = Point - ClosestPoint;

    f32 Result = v.Dot(v);

    return Result;
#else
    f32 Result = 0.0f;
    shu::vec3f v = Point - b.Center;

    for(i32 i = 0; i < 3; ++i)
    {
        const shu::vec3f axes = b.LocalAxes[i];
        ASSERT(axes.IsNormalized());

        f32 dist = v.Dot(axes);
        f32 excess = 0.0f;

        if(dist > b.HalfExtents[i]) excess = dist - b.HalfExtents[i];
        else if(dist < -b.HalfExtents[i]) excess = dist + b.HalfExtents[i];

        Result += excess * excess;
    }

    return Result;

#endif
}

shu::vec3f
ClosestPtPointTriangle_Unoptimized(const shu::vec3f &Point, const shu::vec3f &A, const shu::vec3f &B,
                                   const shu::vec3f &C)
{
    shu::vec3f AB = B - A;
    shu::vec3f AC = C - A;
    shu::vec3f BC = C - B;

    // Voronoi Region A.
    shu::vec3f AP = Point - A;
    f32 APDotAB = AP.Dot(AB);
    f32 APDotAC = AP.Dot(AC);
    if(APDotAB <= 0.0f && APDotAC <= 0.0f) { return A; }

    // Vertex B
    shu::vec3f BP = Point - B;
    f32 BPDotBA = BP.Dot(A - B);
    f32 BPDotBC = BP.Dot(BC);
    if(BPDotBA <= 0.0f && BPDotBC <= 0.0f) { return B; }

    // Edge AB
    shu::vec3f N = AB.Cross(AC);
    f32 vc = shu::Dot(N, shu::Cross(AP, BP));
    if(vc <= 0.0f && APDotAB >= 0.0f && BPDotBA >= 0.0f)
    {
        f32 t = APDotAB / (APDotAB + BPDotBA);
        return A + t * AB;
    }

    // Vertex C
    shu::vec3f CP = Point - C;
    f32 CPDotCB = CP.Dot(B - C);
    f32 CPDotCA = CP.Dot(A - C);
    if(CPDotCB <= 0.0f && CPDotCA <= 0.0f) { return C; }

    // Edge BC
    f32 va = shu::Dot(N, shu::Cross(BP, CP));
    if(va <= 0.0f && BPDotBC >= 0.0f && CPDotCB >= 0.0f)
    {
        f32 t = BPDotBC / (BPDotBC + CPDotCB);
        return B + t * BC;
    }

    // Edge AC
    f32 vb = shu::Dot(N, shu::Cross(CP, AP));
    if(vb <= 0.0f && CPDotCA >= 0.0f && APDotAC >= 0.0f)
    {
        f32 t = APDotAC / (APDotAC + CPDotCA);
        return A + t * AC;
    }

    // Inside the triangle ABC
    f32 denom = 1.0f / (va + vb + vc);
    f32 u = va * denom;
    f32 v = vb * denom;
    f32 w = 1.0f - u - v;
    return u*A + v*B + w*C;
}

shu::vec3f
ClosestPtPointTriangle(const shu::vec3f &Point, const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    shu::vec3f AB = B - A;
    shu::vec3f AC = C - A;
    shu::vec3f AP = Point - A;

    // NOTE: Check if the point P is in the voronoi region of vertex A.
    f32 d1 = shu::Dot(AB, AP);
    f32 d2 = shu::Dot(AC, AP);
    if(d1 <= 0.0f && d2 <= 0.0f)
    {
        // NOTE: Closest Point on the Triangle ABC to point P is A. Barycentric coordinate (1, 0, 0).
        return A;
    }

    // NOTE: Check if the Point P is in the voronoi region of vertex B
    shu::vec3f BP = Point - B;
    shu::vec3f BC = C - B;
    f32 d3 = shu::Dot(AB, BP);
    f32 d4 = shu::Dot(AC, BP);
    // NOTE: d4 <= d3 means the bp vector is oriented more towards ab vector than the ac vector.
    if(d3 >= 0.0f && d4 <= d3)
    {
        // NOTE: Closest Point on the Triangle ABC to point P is B. Barycentric coordinate (0, 1, 0).
        return B;
    }

    // NOTE: Check if the Point P is in the edge region of AB, if so perform the projection P' on AB and P' will be
    // the closest point.

    // IMPORTANT: Explanation of how we got vc below.
    // d1 = ab.ap, d2 = ac.ap, d3 = ab.bp, d4 = ac.bp
    // To check for point P to be inside the edge region AB, we have to do a scalar triple product.
    // P is on or outside AB if n.(PA X PB) <= 0, where n is the normal of the triangle which is AB X AC
    // n.(PA X PB) is the signed area of PAB. and we know to get barycentric we do SignedArea(PAB) /
    // SignedArea(ABC) where n is normalized vector AB X AC.
    // barycentric coordinate wrt to a vertex is negative when the ratio of signed areas is negative. that is why
    // we check n.(PA X PB) to be negative.
    // Now, this ratio is n.(PA X PB) or (AB X AC).(PA X PB)
    // or in the form (a X b).(c X d), The Lagrange eq says this is equivalent to (a.c)(b.d) - (a.d)(b.c)
    // So n.(PA X PB) or (ab X ac).(ap X bp) = (ab.ap)*(ac.bp) - (ab.bp)*(ac.ap) |
    // d1 = ab.ap, d4 = ac.bp, d3 = ab.bp and d2 = ac.ap
    // So, n.(PA X PB) (which is the signed area of triangle PAB) evaluates to (d1*d4 - d3*d2).
    // Signed Area PAB corresponds to the barycentric coordinate of point C of Triangle ABC.
    f32 vc = d1*d4 - d3*d2;
    if(vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
    {
        // NOTE: The Closest Point is the projection of P onto the edge AB.
        // d1 - d3 here represents the length of the edge AB. d1 is the projection ap onto ab.
        f32 t = d1 / (d1 - d3);
        // NOTE: Barycentric coordinates: (1-t, t, 0)
        return A + t * AB;
    }

    // NOTE: Check if the point p is in the voronoi region of vertex C.
    shu::vec3f CP = Point - C;
    f32 d5 = shu::Dot(AB, CP);
    f32 d6 = shu::Dot(AC, CP);
    // NOTE: d5 <= d6 checks whether the vector cp is oriented more towards AC than AB.
    if(d6 >= 0.0f && d5 <= d6)
    {
        // NOTE: The closest point is vertex C of the triangle ABC
        // NOTE: Barycentric coordinates: (0, 0, 1)
        return C;
    }

    // NOTE: Check whether P is in the region of AC, if so, return the projection of P onto AC.
    // Here, we have to get the signed area of PAC to get the barycentric coordinate of a and c.(dont need b since
    // the projection onto ac does not depend on vertex b at all).
    // again n.(ap X ac) which is (ab X ac).(cp X ap) which is ab.cp * ac.ap - ab.ap * ac.cp
    // and this area corresponds to the barycentric of vertex b. and this evaluation needs to be negative if the
    // point is outside AC.
    f32 vb = d5*d2 - d1*d6;
    // NOTE: d6 <= 0.0f && d2 >= 0.0f is the voronoi region outside AC.
    if(vb <= 0.0f && d6 <= 0.0f && d2 >= 0.0f)
    {
        // NOTE: Return the projection of P on AC.
        // d2 - d6 is the sum of the tow projection of AP onto AC and CP onto AC. which together make up the length
        // of AC. The fraction t is the ratio of the projection AP makes onto AC over the total length AC.
        f32 t = d2 / (d2 - d6);
        // NOTE: Barycentric coordinates: (1-t, 0, t)
        return A + t*AC;
    }

    // NOTE: Check if the point P is the voronoi region of BC.
    f32 va = d3*d6 -d5*d4;
    // NOTE: d4 - d3 is positive: This means that bp is closer to edge ac than to edge ab. We need this condition
    // to be true if P is in the voronoi region of edge BC.
    // NOTE: d5 - d6 is positive: This means that cp is closer to the edge ab than to edge ac. We need this
    // condition also to be true if P is in the voronoi region outside edge bc.
    if(va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
    {
        // NOTE: How close bp is to the edge bc is decided by - by what amount is bp closer to ac than ab (d4 - d3)
        // NOTE: How close bp is to the edge bc is also decided by - by what amount is cp closer to ab than ac (d5 - d6)
        // NOTE: The ratio between these gives the weight along bc.
        float t = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        // NOTE: Barycentric coordinates: (0, 1-t, t)
        return B + t*(C - B);
    }

    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    // NOTE: u*a + v*b + w*c, u = va * denom = 1.0f - v - w
    // NOTE: Barycentric coordinates: (1-v-w, v, w)
    return A + AB*v + AC*w;
}

b32
PointOutsidePlane(const shu::vec3f &P, const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
    shu::vec3f Normal = shu::Cross(B - A, C - A);
    f32 Dot = shu::Dot(P - A, Normal);

    return Dot >= 0.0f;
}

shu::vec3f
ClosestPtPointTetrahedron(const shu::vec3f &P, const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C,
                          const shu::vec3f &D)
{
    shu::vec3f Result = P;
    f32 BestSqDistance = SHU_FLOAT_MAX;

    if(PointOutsidePlane(P, A, B, C))
    {
        shu::vec3f Closest = ClosestPtPointTriangle(P, A, B, C);
        f32 CurrentSqDistance = shu::Dot(Closest - P, Closest - P);
        if(CurrentSqDistance < BestSqDistance)
        {
            BestSqDistance = CurrentSqDistance;
            Result = Closest;
        }
    }

    if(PointOutsidePlane(P, C, B, D))
    {
        shu::vec3f Closest = ClosestPtPointTriangle(P, C, B, D);
        f32 CurrentSqDistance = shu::Dot(Closest - P, Closest - P);
        if(CurrentSqDistance < BestSqDistance)
        {
            BestSqDistance = CurrentSqDistance;
            Result = Closest;
        }
    }

    if(PointOutsidePlane(P, D, B, A))
    {
        shu::vec3f Closest = ClosestPtPointTriangle(P, D, B, A);
        f32 CurrentSqDistance = shu::Dot(Closest - P, Closest - P);
        if(CurrentSqDistance < BestSqDistance)
        {
            BestSqDistance = CurrentSqDistance;
            Result = Closest;
        }
    }

    if(PointOutsidePlane(P, A, C, D))
    {
        shu::vec3f Closest = ClosestPtPointTriangle(P, D, B, A);
        f32 CurrentSqDistance = shu::Dot(Closest - P, Closest - P);
        if(CurrentSqDistance < BestSqDistance)
        {
            BestSqDistance = CurrentSqDistance;
            Result = Closest;
        }
    }

    return Result;
}

// IMPORTANT: NOTE: Check concepts/obb-obb-sat folder for the intersection proof.
b32
ObbObbTestSeparatingAxis(const obb &A, const obb &B)
{
    ASSERT(A.LocalAxes[0].IsNormalized());
    ASSERT(A.LocalAxes[1].IsNormalized());
    ASSERT(A.LocalAxes[2].IsNormalized());
    ASSERT(B.LocalAxes[0].IsNormalized());
    ASSERT(B.LocalAxes[1].IsNormalized());
    ASSERT(B.LocalAxes[2].IsNormalized());

    // NOTE: Projected radii of obb a and b when projected onto separating axes.
    f32 ra, rb;
    // NOTE: Rotation matrix of B in Frame A. Rb * InverseRa = Rb * TransposeRa.
    shu::mat3f R, AbsR;

    for(i32 i = 0; i < 3; ++i)
    {
        for(i32 j = 0; j < 3; ++j)
        {
            R[i][j] = shu::Dot(A.LocalAxes[i], B.LocalAxes[j]);
        }
    }

    // NOTE: Translation Vector.
    shu::vec3f T = B.Center - A.Center;
    // T in A's Frame
    T = shu::Vec3f(T.Dot(A.LocalAxes[0]), T.Dot(A.LocalAxes[1]), T.Dot(A.LocalAxes[2]));

    // NOTE: Add in an epsion term to counteract cases where edges are parallel.
    // Check paper on obb by Goatschalk: http://gamma.cs.unc.edu/users/gottschalk/main.pdf
    for(i32 i = 0; i < 3; ++i)
    {
        for(i32 j = 0; j < 3; ++j)
        {
            AbsR[i][j] = SHU_ABSOLUTE(R[i][j]) + 0.000001f;
        }
    }

    // NOTE: Case 1. Separating Axis test using A's Face Normals.
    for(i32 i = 0; i < 3; ++i)
    {
        ra = A.HalfExtents[i];
        rb = B.HalfExtents[0]*AbsR[i][0] + B.HalfExtents[1]*AbsR[i][1] + B.HalfExtents[2]*AbsR[i][2];
        if((SHU_ABSOLUTE(T[0])) > (ra + rb))
            return false;
    }

    // NOTE: Case 2. Separating Axis test using B's Face Normals.
    for(int i = 0; i < 3; i++)
    {
        ra = A.HalfExtents[0]*AbsR[0][i] + A.HalfExtents[1]*AbsR[1][i] + A.HalfExtents[2]*AbsR[2][i];
        rb = B.HalfExtents[i];
        if(SHU_ABSOLUTE(T[0]*R[0][i] + T[1]*R[1][i] + T[2]*R[2][i]) > (ra + rb))
            return false;
    }

    // NOTE: Test axis L = A0 x B0
    ra = A.HalfExtents[1] * AbsR[2][0] + A.HalfExtents[2] * AbsR[1][0];
    rb = B.HalfExtents[1] * AbsR[0][2] + B.HalfExtents[2] * AbsR[0][1];
    if(SHU_ABSOLUTE(T[2] * R[1][0] - T[1] * R[2][0]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A0 x B1
    ra = A.HalfExtents[1] * AbsR[2][1] + A.HalfExtents[2] * AbsR[1][1];
    rb = B.HalfExtents[0] * AbsR[0][2] + B.HalfExtents[2] * AbsR[0][0];
    if(SHU_ABSOLUTE(T[2] * R[1][1] - T[1] * R[2][1]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A0 x B2
    ra = A.HalfExtents[1] * AbsR[2][2] + A.HalfExtents[2] * AbsR[1][2];
    rb = B.HalfExtents[0] * AbsR[0][1] + B.HalfExtents[1] * AbsR[0][0];
    if(SHU_ABSOLUTE(T[2] * R[1][2] - T[1] * R[2][2]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A1 x B0
    ra = A.HalfExtents[0] * AbsR[2][0] + A.HalfExtents[2] * AbsR[0][0];
    rb = B.HalfExtents[1] * AbsR[1][2] + B.HalfExtents[2] * AbsR[1][1];
    if(SHU_ABSOLUTE(T[0] * R[2][0] - T[2] * R[0][0]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A1 x B1
    ra = A.HalfExtents[0] * AbsR[2][1] + A.HalfExtents[2] * AbsR[0][1];
    rb = B.HalfExtents[0] * AbsR[1][2] + B.HalfExtents[2] * AbsR[1][0];
    if(SHU_ABSOLUTE(T[0] * R[2][1] - T[2] * R[0][1]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A1 x B2
    ra = A.HalfExtents[0] * AbsR[2][2] + A.HalfExtents[2] * AbsR[0][2];
    rb = B.HalfExtents[0] * AbsR[1][1] + B.HalfExtents[1] * AbsR[1][0];
    if(SHU_ABSOLUTE(T[0] * R[2][2] - T[2] * R[0][2]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A2 x B0
    ra = A.HalfExtents[0] * AbsR[1][0] + A.HalfExtents[1] * AbsR[0][0];
    rb = B.HalfExtents[1] * AbsR[2][2] + B.HalfExtents[2] * AbsR[2][1];
    if(SHU_ABSOLUTE(T[1] * R[0][0] - T[0] * R[1][0]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A2 x B1
    ra = A.HalfExtents[0] * AbsR[1][1] + A.HalfExtents[1] * AbsR[0][1];
    rb = B.HalfExtents[0] * AbsR[2][2] + B.HalfExtents[2] * AbsR[2][0];
    if(SHU_ABSOLUTE(T[1] * R[0][1] - T[0] * R[1][1]) > (ra + rb))
        return false;

    // NOTE: Test axis L = A2 x B2
    ra = A.HalfExtents[0] * AbsR[1][2] + A.HalfExtents[1] * AbsR[0][2];
    rb = B.HalfExtents[0] * AbsR[2][1] + B.HalfExtents[1] * AbsR[2][0];
    if(SHU_ABSOLUTE(T[1] * R[0][2] - T[0] * R[1][2]) > (ra + rb))
        return false;

    // Since no separating axis is found, the OBBs must be intersecting
    return true;
}

#endif