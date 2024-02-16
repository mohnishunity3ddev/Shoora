#if !defined(CONTACT_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"

#define MAX_CONTACT_COUNT 4
// NOTE: This is the struct which contains collision info between two bodies A and B.
// Start, End: The start and end positions of the collision overlap.
// Normal: Normal at the point of collision contact.
// Depth: Penetration depth of the collision between the two bodies A and B.
struct contact
{
    shoora_body *ReferenceBodyA;
    shoora_body *IncidentBodyB;

    Shu::vec3f ReferenceContactPointA;
    Shu::vec3f IncidentContactPointB;

    Shu::vec3f Normal;
    f32 Depth;
    f32 TimeOfImpact;

    void ResolvePenetration();
    void ResolveCollision();
};

#define CONTACT_H
#endif // CONTACT_H