#if !defined(VULKAN_DRAW_H)

#include <defines.h>
#include <math/math.h>
#include <utils/utils.h>
#include <volk/volk.h>


struct shoora_graphics
{
    static void UpdateCmdBuffer(const VkCommandBuffer &CmdBuffer);
    static void UpdatePipelineLayout(const VkPipelineLayout &PipelineLayout);
    static VkCommandBuffer &GetCmdBuffer();
    static VkPipelineLayout &GetPipelineLayout();

    static void DrawCube(const shu::vec3f &Position, const u32 ColorU32 = colorU32::Proto_Red, const f32 Scale = 1.0f);
    static void DrawCubeWireframe(const shu::vec3f &v000, const shu::vec3f &v100, const shu::vec3f &v110,
                                  const shu::vec3f &v010, const shu::vec3f &v001, const shu::vec3f &v101,
                                  const shu::vec3f &v111, const shu::vec3f &v011, u32 ColorU32 = colorU32::Green,
                                  f32 Thickness = 0.01f);
    static void DrawLine3D(const shu::vec3f &P0, const shu::vec3f &P1, u32 ColorU32 = colorU32::Green, f32 Thickness = 0.05f);
    static void DrawLine2D(const shu::vec2f P0, const shu::vec2f P1, u32 ColorU32, f32 Thickness);
    static void DrawRect(i32 X, i32 Y, u32 Width, u32 Height, u32 ColorU32);
    static void DrawCircle(shu::vec2f pos, f32 radius, u32 ColorU32);
    static void DrawSphere(shu::vec3f pos, f32 radius, u32 ColorU32);
    static void DrawSpring(const shu::vec2f &startPos, const shu::vec2f &endPos, f32 restLength, f32 thickness,
                           i32 nDivisions, u32 Color);

    static void Draw(u32 IndexCount, u32 IndexOffset, i32 VertexOffset);
};

#define VULKAN_DRAW_H
#endif