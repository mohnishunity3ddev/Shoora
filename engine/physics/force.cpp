#include "force.h"


// NOTE: Drag Formula
// Drag = 0.5f*(density)*(Kd)*(Area)*(magnitude(v)^2)*(-v.direction())
Shu::vec2f
force::GenerateDragForce(const shoora_body *Body, f32 DragCoefficient)
{
    // NOTE: Unexpected force otherwise.
    // ASSERT(DragCoefficient < 0.1f);

    Shu::vec2f DragForce = Shu::Vec2f(0, 0);

    if(Body->LinearVelocity.SqMagnitude() > 0)
    {
        Shu::vec3f VelocityNormalized = Shu::Normalize(Body->LinearVelocity);
        Shu::vec2f DragDirection = Shu::Vec2f(VelocityNormalized)*-1.0f;

        f32 DragMagnitude = DragCoefficient * Body->LinearVelocity.SqMagnitude();

        DragForce = DragDirection * DragMagnitude;
    }

    return DragForce;
}

Shu::vec2f
force::GenerateFrictionForce(const shoora_body *Body, f32 FrictionCoefficient)
{
    Shu::vec2f FrictionForce = Shu::Vec2f(0.0f);

    Shu::vec3f VelocityNormalized = Shu::Normalize(Body->LinearVelocity);
    Shu::vec2f FrictionDir = Shu::Vec2f(VelocityNormalized) * -1.0f;

    f32 FrictionMagnitude = FrictionCoefficient;

    FrictionForce = FrictionDir * FrictionMagnitude;

    return FrictionForce;
}

Shu::vec2f
force::GenerateGravitationalForce(const shoora_body *A, const shoora_body *B, f32 G, f32 minDistance, f32 maxDistance)
{
    Shu::vec2f d = Shu::ToVec2(B->Position - A->Position);
    f32 dSquared = d.SqMagnitude();

    dSquared = ClampToRange(dSquared, minDistance, maxDistance);

    Shu::vec2f AttractionDir = Shu::Normalize(d);
    f32 AttractionMagnitude = G * (A->Mass * B->Mass) / dSquared;

    Shu::vec2f AttractionForce = AttractionDir * AttractionMagnitude;

    return AttractionForce;
}

Shu::vec2f
force::GenerateSpringForce(const shoora_body *p, const Shu::vec2f &Anchor, f32 RestLength, f32 k)
{
    Shu::vec2f pos = Shu::ToVec2(p->Position);
    Shu::vec2f d = pos - Anchor;

    f32 displacement = d.Magnitude() - RestLength;

    Shu::vec2f SpringDirection = Shu::Normalize(d);
    f32 SpringMagnitude = -k * displacement;

    Shu::vec2f SpringForce = SpringDirection * SpringMagnitude;
    return SpringForce;
}

Shu::vec2f
force::GenerateSpringForce(const shoora_body *pA, const shoora_body *pB, f32 RestLength, f32 k)
{
    Shu::vec2f d = Shu::ToVec2(pA->Position - pB->Position);

    f32 displacement = d.Magnitude() - RestLength;

    Shu::vec2f SpringDirection = Shu::Normalize(d);
    f32 SpringMagnitude = -k * displacement;

    Shu::vec2f SpringForce = SpringDirection * SpringMagnitude;

    return SpringForce;
}
