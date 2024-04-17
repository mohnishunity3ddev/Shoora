#include "constraint.h"
#include <math/linear_equations_solver.h>

shu::matN<f32, 12>
constraint_3d::GetInverseMassMatrix() const
{
    // TODO: Make this a Sparse Matrix.
    shu::matN<f32, 12> InverseMassMatrix;

    InverseMassMatrix.Rows[0][0] = A->InvMass;
    InverseMassMatrix.Rows[1][1] = A->InvMass;
    InverseMassMatrix.Rows[2][2] = A->InvMass;

    shu::mat3f InverseInertiaA = A->GetInverseInertiaTensorWS();
    for(i32 i = 0; i < 3; ++i)
    {
        InverseMassMatrix.Rows[3 + i][3 + 0] = InverseInertiaA.Rows[i][0];
        InverseMassMatrix.Rows[3 + i][3 + 1] = InverseInertiaA.Rows[i][1];
        InverseMassMatrix.Rows[3 + i][3 + 2] = InverseInertiaA.Rows[i][2];
    }

    InverseMassMatrix.Rows[6][6] = B->InvMass;
    InverseMassMatrix.Rows[7][7] = B->InvMass;
    InverseMassMatrix.Rows[8][8] = B->InvMass;

    shu::mat3f InverseInertiaB = B->GetInverseInertiaTensorWS();
    for (i32 i = 0; i < 3; ++i)
    {
        InverseMassMatrix.Rows[9 + i][9 + 0] = InverseInertiaB.Rows[i][0];
        InverseMassMatrix.Rows[9 + i][9 + 1] = InverseInertiaB.Rows[i][1];
        InverseMassMatrix.Rows[9 + i][9 + 2] = InverseInertiaB.Rows[i][2];
    }

    return InverseMassMatrix;
}

shu::vecN<f32, 12>
constraint_3d::GetVelocities() const
{
    shu::vecN<f32, 12> Velocities;

    Velocities[0] = A->LinearVelocity.x;
    Velocities[1] = A->LinearVelocity.y;
    Velocities[2] = A->LinearVelocity.z;
    Velocities[3] = A->AngularVelocity.x;
    Velocities[4] = A->AngularVelocity.y;
    Velocities[5] = A->AngularVelocity.z;

    Velocities[6] = B->LinearVelocity.x;
    Velocities[7] = B->LinearVelocity.y;
    Velocities[8] = B->LinearVelocity.z;
    Velocities[9] = B->AngularVelocity.x;
    Velocities[10] = B->AngularVelocity.y;
    Velocities[11] = B->AngularVelocity.z;

    return Velocities;
}

void
constraint_3d::ApplyImpulses(const shu::vecN<f32, 12> &Impulses)
{
    shu::vec3f LinearImpulseA, AngularImpulseA;
    shu::vec3f LinearImpulseB, AngularImpulseB;

    LinearImpulseA.x = Impulses[0];
    LinearImpulseA.y = Impulses[1];
    LinearImpulseA.z = Impulses[2];
    AngularImpulseA.x = Impulses[3];
    AngularImpulseA.y = Impulses[4];
    AngularImpulseA.z = Impulses[5];

    LinearImpulseB.x = Impulses[6];
    LinearImpulseB.y = Impulses[7];
    LinearImpulseB.z = Impulses[8];
    AngularImpulseB.x = Impulses[9];
    AngularImpulseB.y = Impulses[10];
    AngularImpulseB.z = Impulses[11];
    
    A->ApplyImpulseLinear(LinearImpulseA);
    A->ApplyImpulseAngular(AngularImpulseA);
    B->ApplyImpulseLinear(LinearImpulseB);
    B->ApplyImpulseAngular(AngularImpulseB);
}

shu::matN<f32, 6>
constraint_2d::GetInverseMassMatrix() const
{
    shu::matN<f32, 6> Result;
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

shu::vecN<f32, 6>
constraint_2d::GetVelocities() const
{
    shu::vecN<f32, 6> Result;

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

