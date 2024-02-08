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
    f32 RotationRadians;
    f32 AngularVelocity;
    f32 AngularAcceleration;
    f32 CoeffRestitution;
    
    Shu::vec3f SumForces;
    f32 SumTorques;

    f32 FrictionCoeff;
    f32 Mass;
    f32 InvMass;
    f32 I; // Moment of inertia.
    f32 InvI; // Inverse of moment of inertia.

    Shu::vec3f Scale;
    Shu::vec3f Color;

    std::unique_ptr<shoora_shape> Shape;

    shoora_body() = default;
    shoora_body(const shoora_body &other) = delete;
    shoora_body &operator=(const shoora_body &other) = delete;
    shoora_body(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Mass, f32 Restitution,
                std::unique_ptr<shoora_shape> Shape, f32 InitialRotationInRadians = 0.0f);
    shoora_body(shoora_body &&other) noexcept;
    shoora_body &operator=(shoora_body &&other) noexcept;
    ~shoora_body();

    b32 CheckIfClicked(const Shu::vec2f &ClickedWorldPos);
    void KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor);

    // NOTE: returns true if the body is static. Meaning it has infinite mass.
    b32 IsStatic() const;

    void UpdateWorldVertices();

    Shu::vec2f WorldToLocalSpace(const Shu::vec2f &PointWS) const;
    Shu::vec2f LocalToWorldSpace(const Shu::vec2f &PointLS) const;

    void Draw();
    void DrawWireframe(const Shu::mat4f &model, f32 thickness, u32 color);

    void ApplyImpulseLinear(const Shu::vec2f &Impulse);
    void ApplyImpulseAngular(float Impulse);
    void ApplyImpulseAtPoint(const Shu::vec2f &Impulse, const Shu::vec2f &R);

    void AddForce(const Shu::vec2f &Force);
    void AddTorque(f32 Torque);

    void ClearForces();
    void ClearTorques();

    void IntegrateLinear(f32 DeltaTime);
    void IntegrateAngular(f32 DeltaTime);

    void IntegrateForces(const f32 deltaTime);
    void IntegrateVelocities(const f32 deltaTime);
};

#define BODY_H
#endif // BODY_H