#include "collision.h"
#include <mesh/database/mesh_database.h>

b32
collision::IsColliding(shoora_body *A, shoora_body *B, arr<contact> &Contacts)
{
    if(A->IsStatic() && B->IsStatic()) {
        return false;
    }

    b32 Result = false;

    b32 isBodyACircle = (A->Shape->GetType() == shoora_mesh_type::CIRCLE);
    b32 isBodyAPolygon = (A->Shape->GetType() == POLYGON_2D || A->Shape->GetType() == RECT_2D);
    b32 isBodyASphere = (A->Shape->GetType() == shoora_mesh_type::SPHERE);

    b32 isBodyBCircle = (B->Shape->GetType() == shoora_mesh_type::CIRCLE);
    b32 isBodyBPolygon = (B->Shape->GetType() == POLYGON_2D || B->Shape->GetType() == RECT_2D);
    b32 isBodyBSphere = (B->Shape->GetType() == shoora_mesh_type::SPHERE);

    if(isBodyACircle && isBodyBCircle)
    {
        Result = IsCollidingCircleCircle(A, B, Contacts);
    }
    else if(isBodyAPolygon && isBodyBPolygon)
    {
        Result = IsCollidingPolygonPolygon(A, B, Contacts);
    }
    else if(isBodyACircle && isBodyBPolygon)
    {
        Result = IsCollidingPolygonCircle(B, A, Contacts, true);
    }
    else if(isBodyAPolygon && isBodyBCircle)
    {
        Result = IsCollidingPolygonCircle(A, B, Contacts);
    }
    else if(isBodyASphere && isBodyBSphere)
    {
        Result = IsCollidingSphereSphere(A, B, Contacts);
    }

    return Result;
}

b32
collision::IsCollidingSphereSphere(shoora_body *A, shoora_body *B, arr<contact> &Contacts)
{
    b32 Result = false;

    shoora_shape_sphere *SphereA = (shoora_shape_sphere *)A->Shape.get();
    shoora_shape_sphere *SphereB = (shoora_shape_sphere *)B->Shape.get();

    Shu::vec3f AB = B->Position - A->Position;

    f32 RadiusSum = SphereA->Radius + SphereB->Radius;
    f32 DistSquared = AB.SqMagnitude();

    if(DistSquared <= RadiusSum*RadiusSum)
    {
        contact Contact{};
        Contact.A = A;
        Contact.B = B;
        Contact.Normal = Shu::Normalize(AB);
        Contact.Start = B->Position - Contact.Normal*SphereB->Radius;
        Contact.End = A->Position + Contact.Normal*SphereA->Radius;
        Contact.Depth = (Contact.End - Contact.Start).Magnitude();
        Contacts.add(Contact);

        Result = true;
    }

    return Result;
}

b32
collision::IsCollidingCircleCircle(shoora_body *A, shoora_body *B, arr<contact> &Contacts)
{
    shoora_shape_circle *CircleA = (shoora_shape_circle *)A->Shape.get();
    shoora_shape_circle *CircleB = (shoora_shape_circle *)B->Shape.get();

    Shu::vec3f DistanceAB = B->Position - A->Position;
    const f32 RadiusSum = CircleA->Radius + CircleB->Radius;

    b32 IsColliding = (DistanceAB.SqMagnitude() <= (RadiusSum*RadiusSum));
    if(IsColliding)
    {
        // TODO)): There is a collision here. Handle collision contact info here.
        contact Contact;
        Contact.A = A;
        Contact.B = B;

        Contact.Normal = Shu::Normalize(DistanceAB);

        Contact.Start = B->Position - (Contact.Normal * CircleB->Radius);
        Contact.End = A->Position + (Contact.Normal * CircleA->Radius);

        Contact.Depth = (Contact.End - Contact.Start).Magnitude();

        Contacts.add(Contact);
    }

    // NOTE: Circle to Circle collision can produce a max of 1 contact.
    ASSERT(Contacts.size <= 1);

    return IsColliding;
}

b32
collision::IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B, arr<contact> &Contacts)
{
    auto *PolyA = (shoora_shape_polygon *)A->Shape.get();
    auto *PolyB = (shoora_shape_polygon *)B->Shape.get();

    i32 ReferenceEdgeIndexA = -1, ReferenceEdgeIndexB = -1;
    // NOTE: Support points are the corresponding vertex on the bodies which has the best penetration.
    Shu::vec2f SupportPointA, SupportPointB;
    // NOTE: This one is taking the body A as a reference body and b as the body that is incident on it.
    f32 abSeparation = PolyA->FindMinSeparation(PolyB, ReferenceEdgeIndexA, SupportPointB);
    ASSERT(ReferenceEdgeIndexA != -1);
    if(abSeparation >= 0.0f)
    {
        return false;
    }
    // NOTE: This one is taking the body B as a reference body and A as the body that is incident on it.
    f32 baSeparation = PolyB->FindMinSeparation(PolyA, ReferenceEdgeIndexB, SupportPointA);
    ASSERT(ReferenceEdgeIndexB != -1);
    if(baSeparation >= 0.0f)
    {
        return false;
    }

    // NOTE: Getting Multiple Contact points using Clipping(Erin Catto)
    // For more info: Check this - https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf Slide number 14
    // NOTE: Finding the Reference Edge.
    shoora_shape_polygon *ReferenceShape;
    shoora_shape_polygon *IncidentShape;
    i32 ReferenceEdgeIndex = -1;
    if(abSeparation > baSeparation)
    {
        // NOTE: Set A as the reference shape and B as the incident shape.
        ReferenceShape = PolyA;
        IncidentShape = PolyB;
        ReferenceEdgeIndex = ReferenceEdgeIndexA;
    }
    else
    {
        // NOTE: Set B as the reference shape and A as the incident shape.
        ReferenceShape = PolyB;
        IncidentShape = PolyA;
        ReferenceEdgeIndex = ReferenceEdgeIndexB;
    }
    ASSERT(ReferenceEdgeIndex != -1);
    Shu::vec2f ReferenceEdge = ReferenceShape->GetEdgeAt(ReferenceEdgeIndex).xy;

    // NOTE: Perform Clipping(Erin Catto)

    // NOTE: Finding the Incident Edge.
    i32 IncidentVertexIndex = IncidentShape->GetIncidentEdgeIndex(ReferenceEdge.Normal());
    ASSERT(IncidentVertexIndex != -1);
    i32 NextIncidentVertexIndex = IncidentShape->GetNextVertexIndex(IncidentVertexIndex);

    Shu::vec2f i0 = IncidentShape->WorldVertices[IncidentVertexIndex].xy;
    Shu::vec2f i1 = IncidentShape->WorldVertices[NextIncidentVertexIndex].xy;
    Shu::vec2f ContactPoints[2] = {i0, i1};
    Shu::vec2f ClippedPoints[2] = {i0, i1};
    for(i32 i = 0; i < ReferenceShape->VertexCount; ++i)
    {
        if(i == ReferenceEdgeIndex) continue;

        Shu::vec2f r0 = ReferenceShape->WorldVertices[i].xy;
        Shu::vec2f r1 = ReferenceShape->WorldVertices[((i + 1) % ReferenceShape->VertexCount)].xy;

        i32 NumClipped = ReferenceShape->ClipSegmentToLine(ContactPoints, ClippedPoints, r0, r1);
        if(NumClipped < 2)
        {
            break;
        }

        ContactPoints[0] = ClippedPoints[0];
        ContactPoints[1] = ClippedPoints[1];
    }

    Shu::vec2f ReferenceVertex = ReferenceShape->WorldVertices[ReferenceEdgeIndex].xy;
    Shu::vec2f ReferenceFaceNormal = ReferenceEdge.Normal();
    for (i32 i = 0; i < 2; ++i)
    {
        Shu::vec2f ClippedVertex = ClippedPoints[i];
        f32 Proj = (ClippedVertex - ReferenceVertex).Dot(ReferenceFaceNormal);
        if(Proj <= 0)
        {
            contact Contact;
            Contact.A = A;
            Contact.B = B;
            Contact.Normal = Shu::Vec3f(ReferenceFaceNormal, 0.0f);
            Contact.Start = Shu::Vec3f(ClippedVertex, 0.0f);
            Contact.End = Contact.Start + Contact.Normal * (-Proj);
            if(baSeparation >= abSeparation)
            {
                SWAP(Contact.Start, Contact.End);
                Contact.Normal = -Contact.Normal;
            }

            Contacts.add(Contact);
        }
    }

    return true;
}

b32
collision::IsCollidingPolygonCircle(shoora_body *Polygon, shoora_body *Circle, arr<contact> &Contacts, b32 Invert)
{
    b32 IsCollidingResult = false;

    ASSERT(Polygon->Shape->GetType() == shoora_mesh_type::POLYGON_2D || shoora_mesh_type::RECT_2D);
    ASSERT(Circle->Shape->GetType() == shoora_mesh_type::CIRCLE);

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
    contact Contact;
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
        auto CurrDot = Shu::Dot(CurrCenter, ClosestEdge);

        if(CurrDot < 0.0f)
        {
            // IMPORTANT: NOTE: To the LEFT of MinVertex
            f32 CenterToVertDistance = (CircleCenter - MinVertex.xy).Magnitude();
            if(CenterToVertDistance < (Radius))
            {
                IsCollidingResult = true;
                Contact.Depth = Radius - CenterToVertDistance;
                Contact.End = Shu::Vec3f(MinVertex.xy, 0.0f);
                Contact.Normal = Shu::Vec3f(Shu::Normalize(CircleCenter - MinVertex.xy), 0.0f);
                Contact.Start = Contact.End - Contact.Normal*Contact.Depth;
            }
        }
        else
        {
            auto NextCenter = Shu::Normalize(CircleCenter - MinNextVertex.xy);
            auto NextDot = Shu::Dot(NextCenter, -ClosestEdge);

            if(NextDot < 0.0f)
            {
                // IMPORTANT: NOTE: To the RIGHT of MinNextVertex
                f32 CenterToNextVertDistance = (CircleCenter - MinNextVertex.xy).Magnitude();
                if(CenterToNextVertDistance < (Radius))
                {
                    IsCollidingResult = true;
                    Contact.Depth = Radius - CenterToNextVertDistance;
                    Contact.End = Shu::Vec3f(MinNextVertex.xy, 0.0f);
                    Contact.Normal = Shu::Vec3f(Shu::Normalize(CircleCenter - MinNextVertex.xy), 0.0f);
                    Contact.Start = Contact.End - Contact.Normal*Contact.Depth;
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
                    Contact.Normal = Shu::Vec3f(EdgeNormal, 0.0f);
                    auto start = CircleCenter - Contact.Normal.xy * Radius;
                    Contact.Start = Shu::Vec3f(start, 0.0f);
                    Contact.End = Shu::Vec3f(start + Contact.Normal.xy*Contact.Depth, 0.0f);
                }
            }
        }
    }
    else
    {
        // IMPORTANT: NOTE: Circle is inside the polygon
        Contact.Depth = Radius + ABSOLUTE(MaxProjection);
        Contact.Normal = Shu::Vec3f(ClosestEdge.Normal(), 0.0f);
        Contact.Start = Shu::Vec3f(CircleCenter - Contact.Normal.xy*Radius, 0.0f);
        Contact.End = Shu::Vec3f(CircleCenter - Contact.Normal.xy*(MaxProjection), 0.0f);
        IsCollidingResult = true;
    }

    if(IsCollidingResult)
    {
        ASSERT(Contact.Depth >= 0.0f);
        if(!Invert)
        {
            Contact.A = Polygon;
            Contact.B = Circle;
        }
        else
        {
            Contact.A = Circle;
            Contact.B = Polygon;

            auto temp = Contact.Start;
            Contact.Start = Contact.End;
            Contact.End = temp;

            Contact.Normal = -Contact.Normal;
        }

        Contacts.add(Contact);
    }

    return IsCollidingResult;
}
