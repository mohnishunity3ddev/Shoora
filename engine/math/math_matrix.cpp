#include "math_matrix.h"

namespace Shu
{

mat4f
RotateGimbalLock(mat4f &Mat, const vec3f &Axis, f32 AngleInDegrees)
{
    f32 Cos = Shu::Cos(AngleInDegrees);
    f32 InvCos = 1.0f - Cos;
    f32 Sin = Shu::Sin(AngleInDegrees);
    f32 InvSin = 1.0f - Sin;

    vec3f AxisNorm = Normalize(Axis);
    
    f32 Rx = AxisNorm.x;
    f32 Ry = AxisNorm.y;
    f32 Rz = AxisNorm.z;

    mat4f RotateMat;
    RotateMat.Row0 = {Cos + Rx*Rx*InvCos,       Rx*Ry*InvCos - Rz*Sin,     Rx*Rz*InvCos + Ry*Sin,   0};
    RotateMat.Row1 = {Ry*Rx*InvCos + Rz*Sin,    Cos + Ry*Ry*InvCos,        Ry*Rz*InvCos - Rx*Sin,   0};
    RotateMat.Row2 = {Rz*Rx*InvCos - Ry*Sin,    Rz*Ry*InvCos + Rx*Sin,     Cos + Rz*Rz*InvCos,      0};
    RotateMat.Row3 = {0, 0, 0, 1};

    Mat *= RotateMat;

    return Mat;
}
}