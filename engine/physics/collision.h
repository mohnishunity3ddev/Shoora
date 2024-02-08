#if !defined(COLLISION_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"
#include "contact.h"

struct collision2d
{
    static b32 IsColliding(shoora_body *A, shoora_body *B, arr<contact> &Contacts);
    static b32 IsCollidingCircleCircle(shoora_body *A, shoora_body *B,  arr<contact> &Contacts);
    // NOTE: How to get multiple contacts using SAT. Check this:
    // https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf
    static b32 IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B,  arr<contact> &Contacts);
    static b32 IsCollidingPolygonCircle(shoora_body *A, shoora_body *B,  arr<contact> &Contacts, b32 Invert = false);
};

#define COLLISION2D_H
#endif // COLLISION2D_H