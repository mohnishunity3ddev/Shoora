#include "collision.h"
#include <mesh/primitive/geometry_primitive.h>

b32
collision2d::IsColliding(shoora_body *A, shoora_body *B, contact &Contact)
{
    b32 Result = false;

    b32 isBodyACircle = (A->Shape->GetType() == shoora_primitive_type::CIRCLE);
    b32 isBodyBCircle = (B->Shape->GetType() == shoora_primitive_type::CIRCLE);
    if(isBodyACircle && isBodyBCircle)
    {
        Result = IsCollidingCircleCircle(A, B, Contact);
    }
    else
    {
        b32 isBodyAPolygon = (A->Shape->GetType() == POLYGON || A->Shape->GetType() == RECT_2D);
        b32 isBodyBPolygon = (B->Shape->GetType() == POLYGON || B->Shape->GetType() == RECT_2D);

        if(isBodyAPolygon && isBodyBPolygon)
        {
            Result = IsCollidingPolygonPolygon(A, B, Contact);
        }
    }

    return Result;
}

b32
collision2d::IsCollidingCircleCircle(shoora_body *A, shoora_body *B, contact &Contact)
{
    shoora_shape_circle *CircleA = (shoora_shape_circle *)A->Shape.get();
    shoora_shape_circle *CircleB = (shoora_shape_circle *)B->Shape.get();

    Shu::vec3f DistanceAB = B->Position - A->Position;
    const f32 RadiusSum = CircleA->Radius + CircleB->Radius;

    b32 IsColliding = (DistanceAB.SqMagnitude() <= (RadiusSum*RadiusSum));
    if(IsColliding)
    {
        // TODO)): There is a collision here. Handle collision contact info here.
        Contact.A = A;
        Contact.B = B;

        Contact.Normal = Shu::Normalize(DistanceAB);

        Contact.Start = B->Position - (Contact.Normal * CircleB->Radius);
        Contact.End = A->Position + (Contact.Normal * CircleA->Radius);

        Contact.Depth = (Contact.End - Contact.Start).Magnitude();
    }

    return IsColliding;
}

b32
collision2d::IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B, contact &Contact)
{
    auto *PolyA = (shoora_shape_polygon *)A->Shape.get();
    auto *PolyB = (shoora_shape_polygon *)B->Shape.get();

    Shu::vec2f AxisA, PointA;
    Shu::vec2f AxisB, PointB;
    f32 abSeparation = PolyA->FindMinSeparation(PolyB, AxisA, PointB);
    if(abSeparation >= 0.0f)
    {
        return false;
    }
    f32 baSeparation = PolyB->FindMinSeparation(PolyA, AxisB, PointA);
    if(baSeparation >= 0.0f)
    {
        return false;
    }

    Contact.A = A;
    Contact.B = B;

    if (abSeparation > baSeparation)
    {
        Contact.Normal = Shu::Vec3f(AxisA.Normal(), 0.0f);
        Contact.Depth = -abSeparation;
        Contact.Start = Shu::Vec3f(PointB, 0.0f);
        Contact.End = Contact.Start + Contact.Normal*Contact.Depth;
    }
    else
    {
        Contact.Normal = Shu::Vec3f(-AxisB.Normal(), 0.0f);
        Contact.Depth = -baSeparation;
        Contact.End = Shu::Vec3f(PointA, 0.0f);
        Contact.Start = Contact.End - Contact.Normal*Contact.Depth;
    }

    return true;
}
