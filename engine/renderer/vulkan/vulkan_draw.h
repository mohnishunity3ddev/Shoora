#if !defined(VULKAN_DRAW_H)

#include <defines.h>
#include <volk/volk.h>
#include <math/math.h>

struct shoora_draw
{
    static void DrawLine(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout, const Shu::vec2f P0,
                         const Shu::vec2f P1, u32 ColorU32, f32 Thickness);
    static void DrawRect(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout, i32 X, i32 Y,
                         u32 Width, u32 Height, u32 ColorU32);
    static void DrawCircle(VkCommandBuffer CmdBuffer, const VkPipelineLayout pipelineLayout, Shu::vec2f pos,
                           f32 radius, u32 ColorU32);
    static void DrawSpring(VkCommandBuffer CmdBuffer, const VkPipelineLayout &pipelineLayout,
                           const Shu::vec2f &startPos, const Shu::vec2f &endPos, f32 restLength, f32 thickness,
                           i32 nDivisions, u32 Color);

    static void Draw(const VkCommandBuffer &CmdBuffer, u32 IndexCount, u32 IndexOffset, i32 VertexOffset);
};

#define VULKAN_DRAW_H
#endif