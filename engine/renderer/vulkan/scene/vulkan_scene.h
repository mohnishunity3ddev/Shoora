#if !defined(VULKAN_SCENE_H)

#include <defines.h>
#include <math/math.h>
#include <physics/body.h>
#include <containers/dynamic_array.h>

struct shoora_scene
{
  private:
    shoora_dynamic_array<shoora_body> Bodies;

  public:
    shoora_scene()
    {
        Bodies.reserve(32);
    }

    ~shoora_scene()
    {
        LogWarnUnformatted("shoora scene destructor called!\n");
    }

    void AddBox(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution, f32 InitialRotation = 0);
    void AddCircle(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution, f32 InitialRotation = 0);
    void AddPolygon(const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution, f32 InitialRotation = 0);

    i32 GetBodyCount();
    shoora_body *GetBodies();
};

#define VULKAN_SCENE_H
#endif // VULKAN_SCENE_H