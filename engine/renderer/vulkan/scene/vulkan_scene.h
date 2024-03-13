#if !defined(VULKAN_SCENE_H)

#include <containers/dynamic_array.h>
#include <defines.h>
#include <math/math.h>
#include <physics/body.h>
#include <physics/broadphase.h>
#include <physics/constraint.h>


struct shoora_scene
{
  private:
    b32 SceneAddBegin = false, SceneAddEnd = false;

  public:
    shoora_dynamic_array<shoora_body> Bodies;
    shoora_dynamic_array<constraint_2d *> Constraints2D;
    shoora_dynamic_array<penetration_constraint_2d> PenetrationConstraints2D;

  public:
    shoora_scene();
    ~shoora_scene();

    void AddMeshToScene(const shu::vec3f *vPositions, u32 vCount);

    void AddConstraint2D(constraint_2d *Constraint);
    i32 GetConstraints2DCount();

    // shoora_body *AddBody(const shoora_body &Body);
    shoora_body *AddBody(shoora_body &&Body);

    shoora_body *AddCubeBody(const shu::vec3f &Pos, const shu::vec3f &Scale, u32 ColorU32, f32 Mass,
                             f32 Restitution, const shu::vec3f &EulerAngles = shu::Vec3f(0.0f));
    shoora_body *AddSphereBody(const shu::vec3f &Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                               const shu::vec3f &EulerAngles = shu::Vec3f(0.0f));
    shoora_body *AddBoxBody(const shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                            const shu::vec3f &EulerAngles = shu::Vec3f(0.0f));
    shoora_body *AddCircleBody(const shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                               const shu::vec3f &EulerAngles = shu::Vec3f(0.0f));
    shoora_body *AddPolygonBody(const u32 MeshId, const shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                                const shu::vec3f &EulerAngles = shu::Vec3f(0.0f), f32 Scale = 1.0f);
    shoora_body *AddDiamondBody(const shu::vec3f &Pos, const shu::vec3f &Scale, u32 ColorU32, f32 Mass,
                                f32 Restitution, const shu::vec3f &EulerAngles);

    i32 GetBodyCount();
    shoora_body *GetBodies();
    shoora_body *GetBody(i32 Index);

    void UpdateInput(const shu::vec2f &CurrentMouseWorldPos);

    void PhysicsUpdate(f32 dt, b32 DebugMode);

    void Draw(b32 Wireframe);
    void DrawAxes(shu::rect2d &Rect);
};

#define VULKAN_SCENE_H
#endif // VULKAN_SCENE_H