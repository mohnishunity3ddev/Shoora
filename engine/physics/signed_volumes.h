#if !defined(SIGNED_VOLUMES_H)
#define SIGNED_VOLUMES_H

#include <defines.h>
#include <math/math.h>

// NOTE: This is basically calculating the barycentric coordinate of the Origin with respect to the Line joining s2
// and s1. if vec2(b1, b2) is the barycentric coordinate of the origin with respect to this line segment, then
// origin can be seen as b1*s1 + b2*(s2 - s1) where b1 + b2 = 1(condition for barycentric coordinates). This can
// also be used to get the closest point on the line segment from the origin. if (b1, b2) is (0, 1) then closest
// point on line is s2 to the origin. if (b1, b2) is (1, 0) then closest point is s1 from the origin.
//
// NOTE: The way we do this here is we project the line onto x,y,z axis and see the greatest component. We project
// origin onto this max axis, and then find the factor of the projected origin about how far it is from both s1 and
// se as a factor and return that as the barycentric coordinate.
//
// NOTE: This function can also be seen as projecting the origin onto the line joining s1 and s2.
shu::vec2f SignedVolume1D(const shu::vec3f &A, const shu::vec3f &B);
//  NOTE: This one does the same thing as the unoptimized version above, only it does not involve finding the max
//  axis. This one just
shu::vec2f SignedVolume1D_Optimized(const shu::vec3f &A, const shu::vec3f &B);

// NOTE: This does the same but calculate barycentric for a triangle ABC. This one like the 1D case also projects
// the triangle on all 2d axes - xy, xz, yz and selects the axes with the largest projected area of the triangle.
// and then using signed area approach the barycentric coordinates are calculated.
shu::vec3f SignedVolume2D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C);
shu::vec3f SignedVolume2D_Optimized(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C);

// NOTE: This routine is used to calculate the barycentric coordinates of a tetrahedron.
// It is calculated as follows: A,B,C,D are points of the tetrahedron and we need to calculate the barycentric
// coordinates of any point P with respect to this tetrahedron. the barycentric of P is vec4(u,v,w,x) such that
// P.x = u*A.x + v*B.x + w*C.x + x*D.x
// P.y = u*A.y + v*B.y + w*C.y + x*D.y
// P.z = u*A.z + v*B.z + w*C.z + x*D.z
// u + v + w + x = 1 (sum of bary coordinates is 1).
// so, P - A = v(B - A) + w(C - A) + x(D - A)
// In Linear equation form this will become:
// [ a.x b.x c.x d.x ] [ u ] = [ P.x ]
// [ a.y b.y c.y d.y ] [ v ] = [ P.y ]
// [ a.z b.z c.z d.z ] [ w ] = [ P.z ]
// [  1   1   1   1  ] [ x ] = [  1  ]
// Solving this matrix using cramers rule solves for u,v,w,x
// u = Volume(pbcd) / Volume(abcd)
// v = Volume(apcd) / Volume(abcd)
// w = Volume(abpd) / Volume(abcd)
// x = Volume(abcp) / Volume(abcd)
// Also, the volume of tetrahedron ABCD is (1/6)*Determinant of this matrix:
// | a.x b.x c.x d.x |
// | a.y b.y c.y d.y |
// | a.z b.z c.z d.z |
// |  1   1   1   1  |
// Also, the volume of tetrahedron PBCD is (1/6)*Determinant of this matrix:
// | p.x b.x c.x d.x |
// | p.y b.y c.y d.y |
// | p.z b.z c.z d.z |
// |  1   1   1   1  |
// Also, the volume of tetrahedron APCD is (1/6)*Determinant of this matrix:
// | a.x p.x c.x d.x |
// | a.y p.y c.y d.y |
// | a.z p.z c.z d.z |
// |  1   1   1   1  |
// and so on
// NOTE: In our case, since we want to get bary coords of the origin, P = (0, 0, 0)
shu::vec4f SignedVolume3D(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C, const shu::vec3f &D);

void TestSignedVolumeProjection();

#endif // SIGNED_VOLUMES_H