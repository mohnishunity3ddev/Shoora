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
    Shu::quat Rotation;

    Shu::vec3f LinearVelocity;
    Shu::vec3f Acceleration;

    Shu::vec3f AngularVelocity;

    // Angular motion
    // f32 RotationRadians;
    // f32 AngularVelocity;
    // f32 AngularAcceleration;

    f32 CoeffRestitution;

    Shu::vec3f SumForces;
    f32 SumTorques;

    f32 FrictionCoeff;
    f32 Mass;
    f32 InvMass;

    Shu::mat3f InertiaTensor; // Moment of inertia.
    Shu::mat3f InverseInertiaTensor; // Inverse of moment of inertia.

    Shu::vec3f Scale;
    Shu::vec3f Color;

    std::unique_ptr<shoora_shape> Shape;

    shoora_body() = default;
    shoora_body(const shoora_body &other) = delete;
    shoora_body &operator=(const shoora_body &other) = delete;
    shoora_body(const Shu::vec3f &Color, const Shu::vec3f &InitPos, f32 Mass, f32 Restitution,
                std::unique_ptr<shoora_shape> Shape, Shu::vec3f eulerAngles = Shu::Vec3f(0.0f));
    shoora_body(shoora_body &&other) noexcept;
    shoora_body &operator=(shoora_body &&other) noexcept;
    ~shoora_body();

    b32 CheckIfClicked(const Shu::vec2f &ClickedWorldPos);
    void KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor);

    // NOTE: returns true if the body is static. Meaning it has infinite mass.
    b32 IsStatic() const;
    
    void UpdateWorldVertices();

    Shu::vec3f WorldToLocalSpace(const Shu::vec3f &PointWS) const;
    Shu::vec3f LocalToWorldSpace(const Shu::vec3f &PointLS) const;
    Shu::mat3f GetInverseInertiaTensorWS() const;

    Shu::vec3f GetCenterOfMassLS() const;
    Shu::vec3f GetCenterOfMassWS() const;

    void Draw();
    void DrawWireframe(const Shu::mat4f &model, f32 thickness, u32 color);

    void ApplyImpulseLinear(const Shu::vec3f &LinearImpulse);
    void ApplyImpulseAngular(const Shu::vec3f &AngularImpulse);
    void ApplyImpulseAtPoint(const Shu::vec3f &Impulse, const Shu::vec3f &ImpulsePointWS);

    void AddForce(const Shu::vec3f &Force);
    void AddTorque(f32 Torque);

    void ClearForces();
    void ClearTorques();

    void IntegrateForces(const f32 deltaTime);
    void Update(const f32 deltaTime);
};

#define BODY_H
#endif // BODY_H