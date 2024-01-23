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
                std::unique_ptr<shoora_shape> Shape, f32 InitialRotation = 0.0f);
    shoora_body(shoora_body &&other) noexcept
        : IsColliding(other.IsColliding), Position(std::move(other.Position)), Velocity(std::move(other.Velocity)),
          Acceleration(std::move(other.Acceleration)), RotationRadians(other.RotationRadians),
          AngularVelocity(other.AngularVelocity), AngularAcceleration(other.AngularAcceleration),
          CoeffRestitution(other.CoeffRestitution), SumForces(std::move(other.SumForces)),
          SumTorques(other.SumTorques), FrictionCoeff(other.FrictionCoeff), Mass(other.Mass),
          InvMass(other.InvMass), I(other.I), InvI(other.InvI), Scale(std::move(other.Scale)),
          Color(std::move(other.Color)), Shape(std::move(other.Shape))
    {
        // Ensure the moved-from object is in a valid state
        other.IsColliding = false;
        other.Shape = nullptr; // Assuming ownership transfer is desired
    }
    shoora_body &operator=(shoora_body &&other) noexcept
    {
        if (this != &other)
        {
            IsColliding = other.IsColliding;
            Position = std::move(other.Position);
            Velocity = std::move(other.Velocity);
            Acceleration = std::move(other.Acceleration);
            RotationRadians = other.RotationRadians;
            AngularVelocity = other.AngularVelocity;
            AngularAcceleration = other.AngularAcceleration;
            CoeffRestitution = other.CoeffRestitution;
            SumForces = std::move(other.SumForces);
            SumTorques = other.SumTorques;
            FrictionCoeff = other.FrictionCoeff;
            Mass = other.Mass;
            InvMass = other.InvMass;
            I = other.I;
            InvI = other.InvI;
            Scale = std::move(other.Scale);
            Color = std::move(other.Color);
            Shape = std::move(other.Shape);

            other.IsColliding = false;
            other.Shape = nullptr;
        }
        return *this;
    }

    ~shoora_body()
    {
        LogUnformatted("shoora_body desctructor called!\n");
    }

    b32 CheckIfClicked(const Shu::vec2f &ClickedWorldPos);
    void KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor);

    // NOTE: returns true if the body is static. Meaning it has infinite mass.
    b32 IsStatic() const;

    void UpdateWorldVertices();

    void Draw();
    void DrawWireframe(const Shu::mat4f &model, f32 thickness, u32 color);

    void ApplyImpulse(const Shu::vec2f &Impulse);
    void ApplyImpulse(const Shu::vec2f &Impulse, const Shu::vec2f &R);

    void AddForce(const Shu::vec2f &Force);
    void AddTorque(f32 Torque);

    void ClearForces();
    void ClearTorque();

    void IntegrateLinear(f32 DeltaTime);
    void IntegrateAngular(f32 DeltaTime);
};

#define BODY_H
#endif // BODY_H