#if !defined(COLLISION_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"
#include "contact.h"

struct collision2d
{
    static b32 IsColliding(shoora_body *A, shoora_body *B, arr<contact> &Contacts);

  private:
    static b32 IsCollidingCircleCircle(shoora_body *A, shoora_body *B,  arr<contact> &Contacts);
    static b32 IsCollidingPolygonPolygon(shoora_body *A, shoora_body *B,  arr<contact> &Contacts);
    static b32 IsCollidingPolygonCircle(shoora_body *A, shoora_body *B,  arr<contact> &Contacts, b32 Invert = false);
    static b32 IsCollidingSphereSphere(shoora_body *A, shoora_body *B, arr<contact> &Contacts);
};

#define COLLISION2D_H
#endif // COLLISION2D_H