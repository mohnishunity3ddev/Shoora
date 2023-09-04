#if !defined(VULKAN_VERTEX_DEFINITIONS_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shoora_vertex_info
{
    Shu::vec3f Pos;
    Shu::vec3f Normal;
    Shu::vec3f Color;
    Shu::vec2f UV;
    Shu::vec3f Tangent;
    Shu::vec3f BiTangent;
};

VkVertexInputBindingDescription GetVertexBindingDescription();
void GetVertexAttributeDescriptions(VkVertexInputAttributeDescription *Attributes, u32 *AttributeCount);

#define VULKAN_VERTEX_DEFINITIONS_H
#endif // VULKAN_VERTEX_DEFINITIONS_H