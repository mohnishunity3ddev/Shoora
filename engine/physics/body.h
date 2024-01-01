#if !defined(BODY_H)

#include "defines.h"
#include <math/math.h>
#include <mesh/primitive/geometry_primitive.h>
#include <mesh/mesh_filter.h>
#include <physics/shape.h>

#include <memory>

struct shoora_body
{
    b32 IsColliding = false;

    // linear motion
    Shu::vec3f Position;
    Shu::vec3f Velocity;
    Shu::vec3f Acceleration;

    // Angular motion
    f32 Rotation;
    f32 AngularVelocity;
    f32 AngularAcceleration;

    Shu::vec3f SumForces;
    f32 SumTorques;

    f32 Mass;
    f32 InvMass;
    f32 I; // Moment of inertia.
    f32 InvI; // Inverse of moment of inertia.

    Shu::vec3f Scale;
    Shu::vec3f Color;

    std::unique_ptr<shoora_shape> Shape;

    b32 CheckIfClicked(const Shu::vec2f &ClickedWorldPos);
    void KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor);

    void Initialize(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Mass, std::unique_ptr<shoora_shape> Shape);

    void AddForce(const Shu::vec2f &Force);
    void AddTorque(f32 Torque);

    void ClearForces();
    void ClearTorque();

    void IntegrateLinear(f32 DeltaTime);
    void IntegrateAngular(f32 DeltaTime);
};

#define BODY_H
#endif // BODY_H