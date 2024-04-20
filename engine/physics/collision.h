#if !defined(COLLISION_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"
#include "contact.h"

// TODO: Remove this since I have added contact manifolds which keeps track of multiple contacts.
#define MaxContactCountPerPair 1

struct collision
{
    static b32 IsColliding(shoora_body *A, shoora_body *B, const f32 DeltaTime, contact *Contacts,
                           i32 &ContactCount);

  private:
    static b32 IsCollidingCircleCircle(shoora_body *A, shoora_body *B,  contact *Contacts, i32 &ContactCount);
    static b32 IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B, contact *Contacts, i32 &ContactCount);
    static b32 IsCollidingPolygonCircle(shoora_body *A, shoora_body *B, contact *Contacts,
                                        i32 &ContactCount, b32 Invert = false);
    // A is the reference body here, B is the one which is incident(colliding) with A.
    static b32 IsCollidingSphereSphere(shoora_body *A, shoora_body *B, const f32 DeltaTime, contact *Contacts,
                                       i32 &ContactCount);

    static b32 IsCollidingConvex(shoora_body *A, shoora_body *B, f32 DeltaTime, contact *Contacts, i32 &ContactCount);
};

#define COLLISION2D_H
#endif // COLLISION2D_H