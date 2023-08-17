#if !defined(VULKAN_VERTEX_DEFINITIONS_H)
#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shoora_vertex_info
{
    vec2 VertexPos;
    vec3 VertexColor;
};

VkVertexInputBindingDescription GetVertexBindingDescription();
void GetVertexAttributeDescriptions(VkVertexInputAttributeDescription *Attributes, u32 *AttributeCount);

#define VULKAN_VERTEX_DEFINITIONS_H
#endif // VULKAN_VERTEX_DEFINITIONS_H