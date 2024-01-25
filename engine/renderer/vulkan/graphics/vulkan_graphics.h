#if !defined(VULKAN_DRAW_H)

#include <defines.h>
#include <volk/volk.h>
#include <math/math.h>

struct shoora_graphics
{
    static void UpdateCmdBuffer(const VkCommandBuffer &CmdBuffer);
    static void UpdatePipelineLayout(const VkPipelineLayout &PipelineLayout);
    static VkCommandBuffer &GetCmdBuffer();
    static VkPipelineLayout &GetPipelineLayout();


    static void DrawLine(const Shu::vec2f P0, const Shu::vec2f P1, u32 ColorU32, f32 Thickness);
    static void DrawRect(i32 X, i32 Y, u32 Width, u32 Height, u32 ColorU32);
    static void DrawCircle(Shu::vec2f pos, f32 radius, u32 ColorU32);
    static void DrawSpring(const Shu::vec2f &startPos, const Shu::vec2f &endPos, f32 restLength, f32 thickness,
                           i32 nDivisions, u32 Color);

    static void Draw(u32 IndexCount, u32 IndexOffset, i32 VertexOffset);
};

#define VULKAN_DRAW_H
#endif