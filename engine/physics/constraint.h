#if !defined(CONSTRAINT_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"
#include "contact.h"

#define WARM_STARTING 1

struct constraint_3d
{
  public:
    // NOTE: Calculate the Jacobian.
    virtual void PreSolve(const f32 dt) {}
    virtual void Solve() {}
    virtual void PostSolve() {}

    constraint_3d() = default;
    virtual ~constraint_3d() = default;

  protected:
    shu::matN<f32, 12> GetInverseMassMatrix() const;
    shu::vecN<f32, 12> GetVelocities() const;
    void ApplyImpulses(const shu::vecN<f32, 12> &Impulses);


  public:
    // NOTE: The anchor point where the bodies are to be fixed at in the local space of the corresponding rigid
    // bodies.
    shu::vec3f AnchorPointLS_A; // The anchor point in A's Local Space
    shu::vec3f AnchorPointLS_B; // The anchor point in B's Local Space
    shoora_body *A;
    shoora_body *B;

    shu::vec3f rA; // The axis of the the anchor point in A.
    shu::vec3f rB; // The axis of the the anchor point in B.
};

struct constraint_2d
{
    shoora_body *A;
    shoora_body *B;

    // NOTE: The anchor point where the bodies are to be fixed at in the local space of the corresponding rigid
    // bodies.
    shu::vec2f AnchorPointLS_A; // The anchor point in A's Local Space
    shu::vec2f AnchorPointLS_B; // The anchor point in B's Local Space

    constraint_2d() = default;
    virtual ~constraint_2d() = default;

    shu::matN<f32, 6> GetInverseMassMatrix() const;
    shu::vecN<f32, 6> GetVelocities() const;

    virtual void PreSolve(const f32 dt) {}
    virtual void Solve() {}
    virtual void PostSolve() {}
};

struct joint_constraint_3d : public constraint_3d
{
    joint_constraint_3d() : constraint_3d()
    {
        this->Jacobian.Zero();
#if WARM_STARTING
        this->PreviousFrameLambda.Zero();
#endif
        this->Baumgarte = 0.0f;
    }

    void PreSolve(const f32 dt) override;
    void Solve() override;
    void PostSolve() override;

  private:
    shu::matMN<f32, 1, 12> Jacobian;
#if WARM_STARTING
    shu::vecN<f32, 1> PreviousFrameLambda;
#endif
    // NOTE: The Stabilization Factor. Bias to correct positional error(constraint error).
    f32 Baumgarte;
};

struct joint_constraint_2d : public constraint_2d
{
  private:
    shu::matMN<f32, 6, 1> Jacobian;
    shu::vecN<f32, 1> CachedLambda;
    // NOTE: Baumgarte Stabilization Factor.
    f32 Bias;

  public:
    joint_constraint_2d();
    joint_constraint_2d(shoora_body *a, shoora_body *b, const shu::vec2f &anchorPointWS);

    // NOTE: This is where the Warm starting takes place to limit the number of solver iterations in the Solve()
    // method.
    virtual void PreSolve(const f32 dt) override;
    // NOTE: This is where we solve the constraint by finding out the impulse which must be applied to the bodies
    // so that the constraint that is breaking is resolved.
    virtual void Solve() override;
    virtual void PostSolve() override;
};

struct penetration_constraint_2d : public constraint_2d
{
  private:
    // There are two columns here since we have two sets. One for the collision resolution along the normal.
    // And the other for Friction.
    shu::matMN<f32, 6, 2> Jacobian;

    shu::vecN<f32, 2> CachedLambda;

    // Baumgarte Syabilization Factor.
    f32 Bias;

    // Normal direction of the penetration in A's local Space.
    shu::vec2f Normal;

    // Friction Coefficient between the two penetrating bodies.
    f32 Friction;

  public:
    penetration_constraint_2d();
    penetration_constraint_2d(shoora_body *a, shoora_body *b, const contact &contact);


    virtual void PreSolve(const f32 dt) override;
    virtual void Solve() override;
    virtual void PostSolve() override;
};

#define CONSTRAINT_H
#endif // CONSTRAINT_H