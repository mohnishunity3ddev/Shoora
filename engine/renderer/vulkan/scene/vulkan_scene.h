#if !defined(VULKAN_SCENE_H)

#include <defines.h>
#include <math/math.h>
#include <physics/body.h>
#include <physics/constraint.h>
#include <containers/dynamic_array.h>

#include <vector>
struct shoora_scene
{
  private:
    shoora_dynamic_array<shoora_body> Bodies;
    // TODO: Use our dynamic array instead here.
    std::vector<constraint_2d *> Constraints2D;
    std::vector<penetration_constraint_2d> PenetrationConstraints2D;

    b32 SceneAddBegin = false, SceneAddEnd = false;
  public:
    shoora_scene();
    ~shoora_scene();

    void AddMeshToScene(const Shu::vec3f *vPositions, u32 vCount);

    void AddConstraint2D(constraint_2d *Constraint);
    i32 GetConstraints2DCount();

    shoora_body *AddBoxBody(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                            f32 InitialRotation = 0);
    shoora_body *AddCircleBody(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution,
                               f32 InitialRotation = 0);
    shoora_body *AddPolygonBody(const u32 MeshId, const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                                f32 InitialRotation, f32 Scale = 1.0f);

    i32 GetBodyCount();
    shoora_body *GetBodies();
    shoora_body *GetBody(i32 Index);

    void UpdateInput(const Shu::vec2f &CurrentMouseWorldPos);

    void PhysicsUpdate(f32 dt, b32 ShowContacts);

    void Draw(b32 Wireframe);
    void DrawAxes(Shu::rect2d &Rect);
};

#define VULKAN_SCENE_H
#endif // VULKAN_SCENE_H