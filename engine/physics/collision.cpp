#include "collision.h"
#include <mesh/primitive/geometry_primitive.h>

b32
collision2d::IsColliding(shoora_body *A, shoora_body *B, contact &Contact)
{
    b32 Result = false;

    b32 isBodyACircle = (A->Shape->GetType() == shoora_primitive_type::CIRCLE);
    b32 isBodyBCircle = (B->Shape->GetType() == shoora_primitive_type::CIRCLE);
    b32 isBodyAPolygon = (A->Shape->GetType() == POLYGON_2D || A->Shape->GetType() == RECT_2D);
    b32 isBodyBPolygon = (B->Shape->GetType() == POLYGON_2D || B->Shape->GetType() == RECT_2D);

    if(isBodyACircle && isBodyBCircle)
    {
        Result = IsCollidingCircleCircle(A, B, Contact);
    }
    else if(isBodyAPolygon && isBodyBPolygon)
    {
        Result = IsCollidingPolygonPolygon(A, B, Contact);
    }
    else if(isBodyACircle && isBodyBPolygon)
    {
        Result = IsCollidingPolygonCircle(B, A, Contact);
    }
    else if(isBodyAPolygon && isBodyBCircle)
    {
        Result = IsCollidingPolygonCircle(A, B, Contact);
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

b32
collision2d::IsCollidingPolygonCircle(shoora_body *Polygon, shoora_body *Circle, contact &Contact)
{
    b32 IsCollidingResult = false;

    ASSERT(Polygon->Shape->GetType() == shoora_primitive_type::POLYGON_2D || shoora_primitive_type::RECT_2D);
    ASSERT(Circle->Shape->GetType() == shoora_primitive_type::CIRCLE);

    shoora_shape_polygon *Poly  = (shoora_shape_polygon *)Polygon->Shape.get();
    shoora_shape_circle *Circ = (shoora_shape_circle *)Circle->Shape.get();
    f32 Radius = Circ->GetDim().x;
    Shu::vec2f CircleCenter = Circle->Position.xy;
    Shu::vec2f PolyCenter = Polygon->Position.xy;

    // NOTE: Check for the edge which is closest to the center of the circle.
    Shu::vec3f MinVertex;
    Shu::vec3f MinNextVertex;
    b32 isInside = true;
    Shu::vec2f ClosestEdge;
    f32 MaxProjection = SHU_FLOAT_MIN;
    for(i32 i = 0; i < Poly->VertexCount; ++i)
    {
        auto CurrentEdge = Poly->GetEdgeAt(i).xy;
        auto EdgeNormal = CurrentEdge.Normal();
        auto CurrentVertex = Poly->WorldVertices[i];
        auto CurrentNextVertex = Poly->WorldVertices[(i + 1) % Poly->VertexCount];

        auto VertexToCircleCenter = Circle->Position - CurrentVertex;

        auto Projection = Shu::Dot(VertexToCircleCenter.xy, EdgeNormal);
        // NOTE: There is only one vertexToCircleCenter in the polygon whose dot product will be positive(which
        // will be the closest, everyone else will give negative projections). It would also mean that the circle
        // center is outside the circle.
        if(Projection > 0.0f)
        {
            isInside = false;
            ClosestEdge = CurrentEdge;
            MinVertex = CurrentVertex;
            MinNextVertex = CurrentNextVertex;
            break;
        }
        else
        {
            // NOTE: In this case we want to find the closest edge of the polygon to the circle center which is
            // inside the polygon
            // Since all projections will be negative in this case, to find the closest edge, we have to find the
            // projection which has the least negative value (in other words the closest to an edge).
            if(Projection > MaxProjection)
            {
                MaxProjection = Projection;
                MinVertex = CurrentVertex;
                MinNextVertex = CurrentNextVertex;
                ClosestEdge = CurrentEdge;
            }
        }
    }

    if(!isInside)
    {
        auto EdgeNormal = ClosestEdge.Normal();

        auto CurrCenter = Shu::Normalize(CircleCenter - MinVertex.xy);
        auto CurrCenterPerpEdge = Shu::Dot(CurrCenter, EdgeNormal) * CurrCenter;
        auto CurrCenterParrEdge = CurrCenter - CurrCenterPerpEdge;
        auto CurrDot = Shu::Dot(CurrCenterParrEdge, ClosestEdge);

        auto NextCenter = Shu::Normalize(CircleCenter - MinNextVertex.xy);
        auto NextCenterPerpEdge = Shu::Dot(NextCenter, EdgeNormal) * NextCenter;
        auto NextCenterParrEdge = NextCenter - NextCenterPerpEdge;
        auto NextDot = Shu::Dot(NextCenterParrEdge, ClosestEdge);

        if(CurrDot < 0.0f)
        {
            // IMPORTANT: NOTE: To the LEFT of MinVertex
            f32 CenterToVertDistance = (CircleCenter - MinVertex.xy).Magnitude();
            if(CenterToVertDistance < (Radius))
            {
                IsCollidingResult = true;
                Contact.Depth = Radius - CenterToVertDistance;
                Contact.Start = Shu::Vec3f(MinVertex.xy, 0.0f);
                Contact.Normal = Shu::Vec3f(Shu::Normalize(MinVertex.xy - CircleCenter), 0.0f);
                Contact.End = Contact.Start + Contact.Normal*Contact.Depth;
            }
        }
        else if(NextDot > 0.0f)
        {
            // IMPORTANT: NOTE: To the RIGHT of MinNextVertex
            f32 CenterToNextVertDistance = (CircleCenter - MinNextVertex.xy).Magnitude();
            if(CenterToNextVertDistance < (Radius))
            {
                IsCollidingResult = true;
                Contact.Depth = Radius - CenterToNextVertDistance;
                Contact.Start = Shu::Vec3f(MinNextVertex.xy, 0.0f);
                Contact.Normal = Shu::Vec3f(Shu::Normalize(MinNextVertex.xy - CircleCenter), 0.0f);
                Contact.End = Contact.Start + Contact.Normal*Contact.Depth;
            }
        }
        else
        {
            // IMPORTANT: NOTE: In the Middle region between MinVertex and MinNextVertex!
            auto DistanceFromClosestEdge = Shu::Dot((CircleCenter - MinVertex.xy), EdgeNormal);
            if(DistanceFromClosestEdge < Radius)
            {
                IsCollidingResult = true;
                Contact.Depth = Radius - DistanceFromClosestEdge;
                Contact.Normal = Shu::Vec3f(-EdgeNormal, 0.0f);
                auto end = CircleCenter + Contact.Normal.xy * Radius;
                Contact.End = Shu::Vec3f(end, 0.0f);
                Contact.Start = Shu::Vec3f(end - Contact.Normal.xy * Contact.Depth, 0.0f);
            }
        }
    }
    else
    {
        // IMPORTANT: NOTE: Circle is inside the polygon
        Contact.Normal = Shu::Vec3f(-ClosestEdge.Normal(), 0.0f);
        Contact.Start = Shu::Vec3f(CircleCenter + Contact.Normal.xy*MaxProjection, 0.0f);
        Contact.End = Shu::Vec3f(CircleCenter + Contact.Normal.xy*Radius, 0.0f);
        Contact.Depth = Radius + ABSOLUTE(MaxProjection);
        IsCollidingResult = true;
    }

    if(IsCollidingResult)
    {
        // ASSERT(Contact.Depth > 0.0f);
        Contact.A = Circle;
        Contact.B = Polygon;
    }

    return IsCollidingResult;
}
