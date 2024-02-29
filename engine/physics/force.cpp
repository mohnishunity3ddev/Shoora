#include "force.h"


// NOTE: Drag Formula
// Drag = 0.5f*(density)*(Kd)*(Area)*(magnitude(v)^2)*(-v.direction())
shu::vec2f
force::GenerateDragForce(const shoora_body *Body, f32 DragCoefficient)
{
    // NOTE: Unexpected force otherwise.
    // ASSERT(DragCoefficient < 0.1f);

    shu::vec2f DragForce = shu::Vec2f(0, 0);

    if(Body->LinearVelocity.SqMagnitude() > 0)
    {
        shu::vec3f VelocityNormalized = shu::Normalize(Body->LinearVelocity);
        shu::vec2f DragDirection = shu::Vec2f(VelocityNormalized)*-1.0f;

        f32 DragMagnitude = DragCoefficient * Body->LinearVelocity.SqMagnitude();

        DragForce = DragDirection * DragMagnitude;
    }

    return DragForce;
}

shu::vec2f
force::GenerateFrictionForce(const shoora_body *Body, f32 FrictionCoefficient)
{
    shu::vec2f FrictionForce = shu::Vec2f(0.0f);

    shu::vec3f VelocityNormalized = shu::Normalize(Body->LinearVelocity);
    shu::vec2f FrictionDir = shu::Vec2f(VelocityNormalized) * -1.0f;

    f32 FrictionMagnitude = FrictionCoefficient;

    FrictionForce = FrictionDir * FrictionMagnitude;

    return FrictionForce;
}

shu::vec2f
force::GenerateGravitationalForce(const shoora_body *A, const shoora_body *B, f32 G, f32 minDistance, f32 maxDistance)
{
    shu::vec2f d = shu::ToVec2(B->Position - A->Position);
    f32 dSquared = d.SqMagnitude();

    dSquared = ClampToRange(dSquared, minDistance, maxDistance);

    shu::vec2f AttractionDir = shu::Normalize(d);
    f32 AttractionMagnitude = G * (A->Mass * B->Mass) / dSquared;

    shu::vec2f AttractionForce = AttractionDir * AttractionMagnitude;

    return AttractionForce;
}

shu::vec2f
force::GenerateSpringForce(const shoora_body *p, const shu::vec2f &Anchor, f32 RestLength, f32 k)
{
    shu::vec2f pos = shu::ToVec2(p->Position);
    shu::vec2f d = pos - Anchor;

    f32 displacement = d.Magnitude() - RestLength;

    shu::vec2f SpringDirection = shu::Normalize(d);
    f32 SpringMagnitude = -k * displacement;

    shu::vec2f SpringForce = SpringDirection * SpringMagnitude;
    return SpringForce;
}

shu::vec2f
force::GenerateSpringForce(const shoora_body *pA, const shoora_body *pB, f32 RestLength, f32 k)
{
    shu::vec2f d = shu::ToVec2(pA->Position - pB->Position);

    f32 displacement = d.Magnitude() - RestLength;

    shu::vec2f SpringDirection = shu::Normalize(d);
    f32 SpringMagnitude = -k * displacement;

    shu::vec2f SpringForce = SpringDirection * SpringMagnitude;

    return SpringForce;
}
