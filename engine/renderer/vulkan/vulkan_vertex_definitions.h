#if !defined(VULKAN_VERTEX_DEFINITIONS_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

#define CALCULATE_BITANGENT 0

struct shoora_vertex_info
{
    shu::vec3f Pos;
    shu::vec3f Normal;
    shu::vec2f UV;
    shu::vec3f Color;
    shu::vec4f Tangent;
#if CALCULATE_BITANGENT
    Shu::vec3f BiTangent;
#endif
};

VkVertexInputBindingDescription GetVertexBindingDescription();
void GetVertexAttributeDescriptions(VkVertexInputAttributeDescription *Attributes, u32 *AttributeCount);
VkVertexInputAttributeDescription GetVertexAttributeDescription(u32 BindingIndex, u32 Location, VkFormat Format,
                                                                u32 Offset);

#define VULKAN_VERTEX_DEFINITIONS_H
#endif // VULKAN_VERTEX_DEFINITIONS_H