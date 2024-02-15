#include "constraint.h"
#include <math/linear_equations_solver.h>

Shu::matN<f32, 6>
constraint_2d::GetInverseMassMatrix() const
{
    Shu::matN<f32, 6> Result;
#if 0
    Result.Zero();

    Result.Data[0][0] = A->InvMass;
    Result.Data[1][1] = A->InvMass;
    Result.Data[2][2] = A->InverseInertiaTensor;
    Result.Data[3][3] = B->InvMass;
    Result.Data[4][4] = B->InvMass;
    Result.Data[5][5] = B->InverseInertiaTensor;
#endif
    return Result;
}

Shu::vecN<f32, 6>
constraint_2d::GetVelocities() const
{
    Shu::vecN<f32, 6> Result;

#if 0
    ASSERT(A != nullptr && B != nullptr);

    Result[0] = A->Velocity.x;
    Result[1] = A->Velocity.y;
    Result[2] = A->AngularVelocity;
    Result[3] = B->Velocity.x;
    Result[4] = B->Velocity.y;
    Result[5] = B->AngularVelocity;
#endif
    return Result;
}

