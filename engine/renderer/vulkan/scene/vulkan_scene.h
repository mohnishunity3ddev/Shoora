#if !defined(VULKAN_SCENE_H)

#include <defines.h>
#include <math/math.h>
#include <physics/body.h>
#include <containers/dynamic_array.h>

struct shoora_scene
{
  private:
    shoora_dynamic_array<shoora_body> Bodies;
    b32 SceneHasBegun, SceneEnded;

    void CheckCollisions(b32 ShowContacts);

  public:
    shoora_scene();
    ~shoora_scene();

    void AddMeshToScene(const Shu::vec3f *vPositions, u32 vCount);

    void AddBoxBody(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution, f32 InitialRotation = 0);
    void AddCircleBody(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution, f32 InitialRotation = 0);
    void AddPolygonBody(const u32 MeshId, const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution,
                        f32 InitialRotation, f32 Scale = 1.0f);

    i32 GetBodyCount();
    shoora_body *GetBodies();

    void UpdateInput(const Shu::vec2f &CurrentMouseWorldPos);

    void PhysicsUpdate(f32 dt, b32 ShowContacts);

    void Draw(b32 Wireframe);
    void DrawAxes(Shu::rect2d &Rect);
};

#define VULKAN_SCENE_H
#endif // VULKAN_SCENE_H