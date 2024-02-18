#include "collision.h"
#include <mesh/database/mesh_database.h>

#define SHU_CCD_ON 1

b32
collision::IsColliding(shoora_body *A, shoora_body *B, const f32 DeltaTime, contact *Contacts, i32 &ContactCount)
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
        Result = IsCollidingCircleCircle(A, B, Contacts, ContactCount);
    }
    else if(isBodyAPolygon && isBodyBPolygon)
    {
        Result = IsCollidingPolygonPolygon(A, B, Contacts, ContactCount);
    }
    else if(isBodyACircle && isBodyBPolygon)
    {
        Result = IsCollidingPolygonCircle(B, A, Contacts, ContactCount, true);
    }
    else if(isBodyAPolygon && isBodyBCircle)
    {
        Result = IsCollidingPolygonCircle(A, B, Contacts, ContactCount);
    }
    else if(isBodyASphere && isBodyBSphere)
    {
        Result = IsCollidingSphereSphere(A, B, DeltaTime, Contacts, ContactCount);
    }

    return Result;
}

b32
RaySphereHit(const Shu::vec3f &RayStart, const Shu::vec3f &RayDir, const Shu::vec3f SphereCenter,
             const f32 SphereRadius, f32 &t0, f32 &t1)
{
    b32 Result = false;

    Shu::vec3f CA = SphereCenter - RayStart;
    f32 a = RayDir.Dot(RayDir);
    f32 b = -2.0f * CA.Dot(RayDir);
    f32 c = CA.Dot(CA) - SphereRadius * SphereRadius;

    f32 Discriminant = (b*b) - (4.0f*a*c);
    if(Discriminant >= 0.0f)
    {
        f32 DiscriminantRoot = sqrtf(Discriminant);
        f32 invA = 1.0f / (2.0f * a);

        t0 = (-b - DiscriminantRoot) * invA;
        t1 = (-b + DiscriminantRoot) * invA;
        Result = true;
    }

    return Result;
}

// NOTE: Used in Continuous Collision Detection between two spheres
// The idea here is to check for collisions in between frames. Cast a ray from sphereA into the direction going
// towards sphereB. If the ray intersects, that is the timeOfImpact(in between t0 and t1). Ignore if the toi is
// less than t0(colliding in the past?) and greater than t1(not colliding in the current frame but in the future
// frames).
// *The way to check if a moving sphereA is colliding with sphereB is you take a ray originating at the center of
// *sphereA in the direction of its velocity and check if its colliding with a sphere whose radius is the sum of the
// *radii of both the spheres(Minkowski Sum)
b32
SphereSphereCCD(const shoora_shape_sphere *SphereA, const shoora_shape_sphere *SphereB, const Shu::vec3f &PosA,
                const Shu::vec3f &PosB, const Shu::vec3f &VelA, const Shu::vec3f &VelB, const f32 deltaTime,
                Shu::vec3f &ReferenceHitPointA, Shu::vec3f &IncidentHitPointB, f32 &TimeOfImpact)
{
    Shu::vec3f RayStart = PosA;
    Shu::vec3f RayEnd = PosA + VelA*deltaTime;
    Shu::vec3f RayDir = RayEnd - RayStart;

    // NOTE: If the ray is too short, sphere ray collision will be unreliable. Do a simpler check whether ray and
    // sphere are colliding already.
    f32 t0 = 0.0f;
    f32 t1 = 0.0f;
    const f32 lEpsilon = 0.001f;
    if(RayDir.SqMagnitude() < lEpsilon*lEpsilon)
    {
        const Shu::vec3f AB = PosB - PosA;
        const f32 TotalRadius = SphereA->Radius + SphereB->Radius + lEpsilon;
        // The sphere centers are too far. distance is more than the sum of their radii. if true, then we are sure
        // that they are not colliding.
        if(AB.SqMagnitude() > TotalRadius * TotalRadius)
        {
            return false;
        }
    }
    else if(!RaySphereHit(PosA, RayDir, PosB, SphereA->Radius + SphereB->Radius, t0, t1))
    {
        return false;
    }

    // t0 and t1 will be in the range [0, 1). Make them in the range [0, dt).
    // the t values will be our time of Impact as a fraction of t*deltaTime.
    //* if t comes out to be 0.5, that means the spheres are colliding haflway through this frame.
    t0 *= deltaTime;
    t1 *= deltaTime;

    // if the max t value is negative meaning the collision took place between the spheres directly opposite to the
    // direction of the ray that we have taken, meaning the collision took place in the past, then we say the
    // sphers are not colliding currently in this frame.
    if(t1 < 0.0f)
    {
        return false;
    }

    // Get the earliest possible time of impact between the spheres.
    TimeOfImpact = (t0 < 0.0f) ? 0.0f : t0;

    // this means the spheres are colliding at a time which exceeds the delta Time. Since this is the current
    // deltaTime frame, that means they are not colliding in the current frame but in some other future frame. We
    // don't handle the collision here and leave it future frames to return a collision. This method only computes
    // collisions within the current frame, since this is outside, we return false.
    if(TimeOfImpact > deltaTime)
    {
        return false;
    }

    // NOTE: Here, toi says after this fraction of the deltaTime has passed in the current frame, the sphere's
    // touch each other. What is the position of the two spheres at this point?
    Shu::vec3f newPosA = PosA + VelA*TimeOfImpact;
    Shu::vec3f newPosB = PosB + VelB*TimeOfImpact;
    Shu::vec3f collisionNormal = Shu::Normalize(newPosB - newPosA);

    // NOTE: The Collision Contact Points.
    ReferenceHitPointA = newPosA + collisionNormal*SphereA->Radius;
    IncidentHitPointB = newPosB - collisionNormal*SphereB->Radius;
    return true;
}

b32
collision::IsCollidingSphereSphere(shoora_body *A, shoora_body *B, const f32 DeltaTime, contact *Contacts,
                                   i32 &ContactCount)
{
    b32 Result = false;
    ContactCount = 0;

    shoora_shape_sphere *SphereA = (shoora_shape_sphere *)A->Shape.get();
    shoora_shape_sphere *SphereB = (shoora_shape_sphere *)B->Shape.get();

    contact *Contact = &Contacts[0];
    *Contact = {};

#if SHU_CCD_ON
    Shu::vec3f PosA = A->Position;
    Shu::vec3f PosB = B->Position;

    Shu::vec3f VelA = A->LinearVelocity;
    Shu::vec3f VelB = B->LinearVelocity;

    if(SphereSphereCCD(SphereA, SphereB, A->Position, B->Position, A->LinearVelocity, B->LinearVelocity,
                       DeltaTime, Contact->ReferenceHitPointA, Contact->IncidentHitPointB, Contact->TimeOfImpact))
    {
        // NOTE: There was collision within the current frame.
        Contact->ReferenceBodyA = A;
        Contact->IncidentBodyB = B;

        // Update the rigidbodies by only moving forward a fraction of the current deltaTime. This fraction within
        // the current frame is given by the timeOfImpact. This is done to calculate velocities and positions of
        // the bodies at that point in time within the frame.
        A->Update(Contact->TimeOfImpact);
        B->Update(Contact->TimeOfImpact);

        Contact->ReferenceHitPointA_LocalSpace = A->WorldToLocalSpace(Contact->ReferenceHitPointA);
        Contact->IncidentHitPointB_LocalSpace = B->WorldToLocalSpace(Contact->IncidentHitPointB);

        Contact->Normal = Shu::Normalize(B->Position - A->Position);

        // Rewind the time back to the start of the frame.
        A->Update(-Contact->TimeOfImpact);
        B->Update(-Contact->TimeOfImpact);

        // Get the collision depth.
        Shu::vec3f ab = B->Position - A->Position;
        Contact->Depth = SHU_ABSOLUTE(ab.Magnitude() - (SphereA->Radius + SphereB->Radius));
        ++ContactCount;
        Result = true;
    }
#else
    Shu::vec3f AB = B->Position - A->Position;

    f32 RadiusSum = SphereA->Radius + SphereB->Radius;
    f32 DistSquared = AB.SqMagnitude();

    if(DistSquared <= RadiusSum*RadiusSum)
    {
        Contact->ReferenceBodyA = A;
        Contact->IncidentBodyB = B;

        Contact->Normal = Shu::Normalize(AB);
        Contact->ReferenceHitPointA = B->Position - Contact->Normal*SphereB->Radius;
        Contact->IncidentHitPointB = A->Position + Contact->Normal*SphereA->Radius;
        Contact->Depth = (Contact->IncidentHitPointB - Contact->ReferenceHitPointA).Magnitude();

        ++ContactCount;
        Result = true;
    }
#endif

    return Result;
}

b32
collision::IsCollidingCircleCircle(shoora_body *A, shoora_body *B, contact *Contacts, i32 &ContactCount)
{
    shoora_shape_circle *CircleA = (shoora_shape_circle *)A->Shape.get();
    shoora_shape_circle *CircleB = (shoora_shape_circle *)B->Shape.get();

    Shu::vec3f DistanceAB = B->Position - A->Position;
    const f32 RadiusSum = CircleA->Radius + CircleB->Radius;

    b32 IsColliding = (DistanceAB.SqMagnitude() <= (RadiusSum*RadiusSum));
    contact *Contact = &Contacts[0];
    if(IsColliding)
    {
        // TODO)): There is a collision here. Handle collision contact info here.
        Contact->ReferenceBodyA = A;
        Contact->IncidentBodyB = B;

        Contact->Normal = Shu::Normalize(DistanceAB);

        Contact->ReferenceHitPointA = B->Position - (Contact->Normal * CircleB->Radius);
        Contact->IncidentHitPointB = A->Position + (Contact->Normal * CircleA->Radius);

        Contact->Depth = (Contact->IncidentHitPointB - Contact->ReferenceHitPointA).Magnitude();
        ++ContactCount;
    }

    return IsColliding;
}

b32
collision::IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B, contact *Contacts, i32 &ContactCount)
{
    ContactCount = 0;

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
        contact *Contact = Contacts + i;
        *Contact = {};
        Shu::vec2f ClippedVertex = ClippedPoints[i];
        f32 Proj = (ClippedVertex - ReferenceVertex).Dot(ReferenceFaceNormal);
        if(Proj <= 0)
        {
            Contact->ReferenceBodyA = A;
            Contact->IncidentBodyB = B;
            Contact->Normal = Shu::Vec3f(ReferenceFaceNormal, 0.0f);
            Contact->ReferenceHitPointA = Shu::Vec3f(ClippedVertex, 0.0f);
            Contact->IncidentHitPointB = Contact->ReferenceHitPointA + Contact->Normal * (-Proj);
            if(baSeparation >= abSeparation)
            {
                SWAP(Contact->ReferenceHitPointA, Contact->IncidentHitPointB);
                Contact->Normal = -Contact->Normal;
            }

            ++ContactCount;
        }
    }

    return true;
}

b32
collision::IsCollidingPolygonCircle(shoora_body *Polygon, shoora_body *Circle, contact *Contacts,
                                    i32 &ContactCount, b32 Invert)
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

    contact *Contact = &Contacts[0];
    *Contact = {};
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
                Contact->Depth = Radius - CenterToVertDistance;
                Contact->IncidentHitPointB = Shu::Vec3f(MinVertex.xy, 0.0f);
                Contact->Normal = Shu::Vec3f(Shu::Normalize(CircleCenter - MinVertex.xy), 0.0f);
                Contact->ReferenceHitPointA = Contact->IncidentHitPointB - Contact->Normal*Contact->Depth;
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
                    Contact->Depth = Radius - CenterToNextVertDistance;
                    Contact->IncidentHitPointB = Shu::Vec3f(MinNextVertex.xy, 0.0f);
                    Contact->Normal = Shu::Vec3f(Shu::Normalize(CircleCenter - MinNextVertex.xy), 0.0f);
                    Contact->ReferenceHitPointA = Contact->IncidentHitPointB - Contact->Normal*Contact->Depth;
                }
            }
            else
            {
                // IMPORTANT: NOTE: In the Middle region between MinVertex and MinNextVertex!
                auto DistanceFromClosestEdge = Shu::Dot((CircleCenter - MinVertex.xy), EdgeNormal);
                if(DistanceFromClosestEdge < Radius)
                {
                    IsCollidingResult = true;
                    Contact->Depth = Radius - DistanceFromClosestEdge;
                    Contact->Normal = Shu::Vec3f(EdgeNormal, 0.0f);
                    auto start = CircleCenter - Contact->Normal.xy * Radius;
                    Contact->ReferenceHitPointA = Shu::Vec3f(start, 0.0f);
                    Contact->IncidentHitPointB = Shu::Vec3f(start + Contact->Normal.xy*Contact->Depth, 0.0f);
                }
            }
        }
    }
    else
    {
        // IMPORTANT: NOTE: Circle is inside the polygon
        Contact->Depth = Radius + SHU_ABSOLUTE(MaxProjection);
        Contact->Normal = Shu::Vec3f(ClosestEdge.Normal(), 0.0f);
        Contact->ReferenceHitPointA = Shu::Vec3f(CircleCenter - Contact->Normal.xy*Radius, 0.0f);
        Contact->IncidentHitPointB = Shu::Vec3f(CircleCenter - Contact->Normal.xy*(MaxProjection), 0.0f);
        IsCollidingResult = true;
    }

    if(IsCollidingResult)
    {
        ASSERT(Contact->Depth >= 0.0f);
        if(!Invert)
        {
            Contact->ReferenceBodyA = Polygon;
            Contact->IncidentBodyB = Circle;
        }
        else
        {
            Contact->ReferenceBodyA = Circle;
            Contact->IncidentBodyB = Polygon;

            auto temp = Contact->ReferenceHitPointA;
            Contact->ReferenceHitPointA = Contact->IncidentHitPointB;
            Contact->IncidentHitPointB = temp;

            Contact->Normal = -Contact->Normal;
        }
        ++ContactCount;
    }

    return IsCollidingResult;
}
