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
    *AttributeCount = 2;

    VkVertexInputAttributeDescription *PositionAttribute = &Attributes[0];
    PositionAttribute->location = 0;
    PositionAttribute->binding = 0;
    PositionAttribute->format = VK_FORMAT_R32G32_SFLOAT;
    PositionAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexPos);

    VkVertexInputAttributeDescription *ColorAttribute = &Attributes[1];
    ColorAttribute->location = 1;
    ColorAttribute->binding = 0;
    ColorAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    ColorAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, VertexColor);
}
