#include "vulkan_vertex_definitions.h"

VkVertexInputBindingDescription
GetVertexBindingDescription()
{
    VkVertexInputBindingDescription BindingDesc;

    BindingDesc.binding = 0;
    BindingDesc.stride = sizeof(shoora_vertex_info);
    BindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return BindingDesc;
}

void
GetVertexAttributeDescriptions(VkVertexInputAttributeDescription *Attributes, u32 *AttributeCount)
{
    *AttributeCount = 4;

    VkVertexInputAttributeDescription *PositionAttribute = &Attributes[0];
    PositionAttribute->location = 0;
    PositionAttribute->binding = 0;
    PositionAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    PositionAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexPos);

    VkVertexInputAttributeDescription *NormalAttribute = &Attributes[1];
    NormalAttribute->location = 1;
    NormalAttribute->binding = 0;
    NormalAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    NormalAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexNormal);

    VkVertexInputAttributeDescription *ColorAttribute = &Attributes[2];
    ColorAttribute->location = 2;
    ColorAttribute->binding = 0;
    ColorAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    ColorAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexColor);

    VkVertexInputAttributeDescription *UVAttribute = &Attributes[3];
    UVAttribute->location = 3;
    UVAttribute->binding = 0;
    UVAttribute->format = VK_FORMAT_R32G32_SFLOAT;
    UVAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexUV);
}
