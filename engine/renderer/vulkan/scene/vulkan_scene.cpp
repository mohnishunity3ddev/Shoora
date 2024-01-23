#include "vulkan_scene.h"

#include <utils/utils.h>

void
shoora_scene::AddCircle(const Shu::vec2f Pos, u32 ColorU32, f32 Radius, f32 Mass, f32 Restitution, f32 InitialRotation)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_circle>(Radius), InitialRotation};
    Bodies.emplace_back(std::move(body));
}

void
shoora_scene::AddBox(const Shu::vec2f Pos, u32 ColorU32, f32 Width, f32 Height, f32 Mass, f32 Restitution,
                     f32 InitialRotation)
{
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::make_unique<shoora_shape_box>(Width, Height),
                     InitialRotation};
    Bodies.emplace_back(std::move(body));
}

void
shoora_scene::AddPolygon(const Shu::vec2f Pos, u32 ColorU32, f32 Mass, f32 Restitution, f32 InitialRotation)
{
    auto poly = std::make_unique<shoora_shape_polygon>(nullptr, 1);
    shoora_body body{GetColor(ColorU32), Pos, Mass, Restitution, std::move(poly), InitialRotation};
    Bodies.emplace_back(std::move(body));
}

i32
shoora_scene::GetBodyCount()
{
    return Bodies.size();
}

shoora_body *
shoora_scene::GetBodies()
{
    return Bodies.data();
}
