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
    shu::vec3f Position;
    shu::quat Rotation;

    shu::vec3f LinearVelocity;
    shu::vec3f Acceleration;

    shu::vec3f AngularVelocity;

    // Angular motion
    // f32 RotationRadians;
    // f32 AngularVelocity;
    // f32 AngularAcceleration;

    f32 CoeffRestitution;

    shu::vec3f SumForces;
    f32 SumTorques;

    f32 FrictionCoeff;
    f32 Mass;
    f32 InvMass;

    shu::mat3f InertiaTensor; // Moment of inertia.
    shu::mat3f InverseInertiaTensor; // Inverse of moment of inertia.

    shu::vec3f Scale;
    shu::vec3f Color;

    std::unique_ptr<shoora_shape> Shape;

    shoora_body() = default;
    shoora_body(const shoora_body &other) = delete;
    shoora_body &operator=(const shoora_body &other) = delete;
    shoora_body(const shu::vec3f &Color, const shu::vec3f &InitPos, f32 Mass, f32 Restitution,
                std::unique_ptr<shoora_shape> Shape, shu::vec3f eulerAngles = shu::Vec3f(0.0f));
    shoora_body(shoora_body &&other) noexcept;
    shoora_body &operator=(shoora_body &&other) noexcept;
    ~shoora_body();

    b32 CheckIfClicked(const shu::vec2f &ClickedWorldPos);
    void KeepInView(const shu::rect2d &ViewBounds, f32 DampFactor);

    // NOTE: returns true if the body is static. Meaning it has infinite mass.
    b32 IsStatic() const;

    void UpdateWorldVertices();

    shu::vec3f WorldToLocalSpace(const shu::vec3f &PointWS) const;
    shu::vec3f LocalToWorldSpace(const shu::vec3f &PointLS) const;
    shu::mat3f GetInverseInertiaTensorWS() const;

    shu::vec3f GetCenterOfMassLS() const;
    shu::vec3f GetCenterOfMassWS() const;

    void Draw();
    void DrawWireframe(const shu::mat4f &model, f32 thickness, u32 color);

    void ApplyImpulseLinear(const shu::vec3f &LinearImpulse);
    void ApplyImpulseAngular(const shu::vec3f &AngularImpulse);
    void ApplyImpulseAtPoint(const shu::vec3f &Impulse, const shu::vec3f &ImpulsePointWS);

    void AddForce(const shu::vec3f &Force);
    void AddTorque(f32 Torque);

    void ClearForces();
    void ClearTorques();

    void IntegrateForces(const f32 deltaTime);
    void Update(const f32 deltaTime);
};

#define BODY_H
#endif // BODY_H