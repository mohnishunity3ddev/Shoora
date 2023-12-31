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

VkVertexInputAttributeDescription
GetVertexAttributeDescription(u32 BindingIndex, u32 Location, VkFormat Format, u32 Offset)
{
    VkVertexInputAttributeDescription Desc;
    Desc.location = Location;
    Desc.binding = BindingIndex;
    Desc.format = Format;
    Desc.offset = Offset;

    return Desc;
}

void
GetVertexAttributeDescriptions(VkVertexInputAttributeDescription *Attributes, u32 *AttributeCount)
{
#if CALCULATE_BITANGENT
    *AttributeCount = 6;
#else
    *AttributeCount = 5;
#endif

    VkVertexInputAttributeDescription *PositionAttribute = &Attributes[0];
    PositionAttribute->location = 0;
    PositionAttribute->binding = 0;
    PositionAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    PositionAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, Pos);

    VkVertexInputAttributeDescription *NormalAttribute = &Attributes[1];
    NormalAttribute->location = 1;
    NormalAttribute->binding = 0;
    NormalAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    NormalAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, Normal);

    VkVertexInputAttributeDescription *ColorAttribute = &Attributes[2];
    ColorAttribute->location = 2;
    ColorAttribute->binding = 0;
    ColorAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    ColorAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, Color);

    VkVertexInputAttributeDescription *UVAttribute = &Attributes[3];
    UVAttribute->location = 3;
    UVAttribute->binding = 0;
    UVAttribute->format = VK_FORMAT_R32G32_SFLOAT;
    UVAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, UV);

    VkVertexInputAttributeDescription *TangentAttribute = &Attributes[4];
    TangentAttribute->location = 4;
    TangentAttribute->binding = 0;
    TangentAttribute->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    TangentAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, Tangent);

    // TODO)): Do we need to calculate the bitangent in the CPU since we are already calculating this in the shaders.
#if CALCULATE_BITANGENT
    VkVertexInputAttributeDescription *BiTangentAttribute = &Attributes[5];
    BiTangentAttribute->location = 5;
    BiTangentAttribute->binding = 0;
    BiTangentAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
    BiTangentAttribute->offset = (u32)OFFSET_OF(shoora_vertex_info, BiTangent);
#endif
}
