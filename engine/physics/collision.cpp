#include "collision.h"

b32
collision2d::IsColliding(shoora_body *A, shoora_body *B, contact &Contact)
{
    b32 Result = false;


    b32 isBodyACircle = (A->Shape->Type == shoora_primitive_type::CIRCLE);
    b32 isBodyBCircle = (B->Shape->Type == shoora_primitive_type::CIRCLE);
    if(isBodyACircle && isBodyBCircle)
    {
        Result = IsCollidingCircleCircle(A, B, Contact);
    }
    else
    {
        // TODO)): To implement other types of colliders
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
