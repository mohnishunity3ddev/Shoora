#include "math_matrix_transforms.h"


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

        f32 Nx = AxisNorm.x;
        f32 Ny = AxisNorm.y;
        f32 Nz = AxisNorm.z;

        mat4f RotateMat;
        RotateMat.Row0 = {Nx*Nx*InvCos + Cos,       Nx*Ny*InvCos + Nz*Sin,     Nx*Nz*InvCos - Ny*Sin,   0};
        RotateMat.Row1 = {Nx*Ny*InvCos - Nz*Sin,    Ny*Ny*InvCos + Cos,        Ny*Nz*InvCos + Nx*Sin,   0};
        RotateMat.Row2 = {Nx*Nz*InvCos + Ny*Sin,    Ny*Nz*InvCos - Nx*Sin,     Nz*Nz*InvCos + Cos,      0};
        RotateMat.Row3 = {0,                        0,                         0,                       1};

        Mat *= RotateMat;
        return Mat;
    }

    mat4f
    LookAt(const vec3f &CamPos, const vec3f &LookingTowards, const vec3f WorldUp, mat4f &M)
    {
        vec3f Front = Normalize(LookingTowards - CamPos);
        vec3f Right = Normalize(Cross(WorldUp, Front));
        vec3f Up    = Cross(Front, Right);

        M.Row0 = Vec4f( Right.x,             Right.y,          Right.z,             0);
        M.Row1 = Vec4f( Up.x,                Up.y,             Up.z,                0);
        M.Row2 = Vec4f( Front.x,             Front.y,          Front.z,             0);
        M.Row3 = Vec4f(-Dot(CamPos, Right), -Dot(CamPos, Up), -Dot(CamPos, Front),  1);

        return M;
    }

    mat4f
    Perspective(f32 FOV, f32 Aspect, f32 Near, f32 Far)
    {
        ASSERT(Aspect > 0.0f);

        f32 const TanHalfFov = Shu::Tan(FOV / 2);

        mat4f Result = Mat4f(0.0f);
        Result.m00 = 1.0f / (Aspect * TanHalfFov);
        Result.m11 = 1.0f / TanHalfFov;
        Result.m22 = Far / (Far - Near);
        Result.m23 = 1.0f;
        Result.m32 = -(Far * Near) / (Far - Near);
        return Result;
    }
} 