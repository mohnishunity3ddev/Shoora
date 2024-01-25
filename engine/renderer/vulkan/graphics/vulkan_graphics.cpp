#include "vulkan_graphics.h"
#include <mesh/database/mesh_database.h>
#include <utils/utils.h>

static VkCommandBuffer GlobalCommandBuffer;
static VkPipelineLayout GlobalPipelineLayout;

struct data
{
    Shu::mat4f Model;
    Shu::vec3f Color = {1, 1, 1};
};

void
shoora_graphics::UpdateCmdBuffer(const VkCommandBuffer &CmdBuffer)
{
    GlobalCommandBuffer = CmdBuffer;
}

void
shoora_graphics::UpdatePipelineLayout(const VkPipelineLayout &PipelineLayout)
{
    GlobalPipelineLayout = PipelineLayout;
}

void
shoora_graphics::DrawLine(const Shu::vec2f P0, const Shu::vec2f P1, u32 ColorU32, f32 Thickness)
{
#if 0
    Shu::vec2f p0 = MouseToScreenSpace(P0);
    Shu::vec2f p1 = MouseToScreenSpace(P1);
#endif
    Shu::vec2f l = P1 - P0;

    Shu::vec3f Pos = Shu::Vec3f((P0 + P1) * 0.5f, 1.0f);
    Shu::vec3f Scale = Shu::Vec3f(l.Magnitude(), Thickness, 1.0f);

    shoora_mesh *Line = shoora_mesh_database::GetMesh(shoora_mesh_type::RECT_2D);
    Shu::mat4f Model = Shu::TRS(Pos, Scale, l.GetSlopeAngleInDegrees(), Shu::Vec3f(0.0f, 0.0f, 1.0f));

    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Line->MeshFilter.IndexCount, 1, Line->IndexOffset, Line->VertexOffset, 0);
}

void
shoora_graphics::DrawRect(i32 X, i32 Y, u32 Width, u32 Height, u32 ColorU32)
{
    Shu::mat4f Model = Shu::Mat4f(1.0f);
    Shu::Scale(Model, Shu::Vec3f((f32)Width * 0.5f, (f32)Height * 0.5f, 1.0f));
    Shu::Translate(Model, Shu::Vec3f(X, Y, 0.0f));
    
    shoora_mesh *RectMesh = shoora_mesh_database::GetMesh(shoora_mesh_type::RECT_2D);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, RectMesh->MeshFilter.IndexCount, 1, RectMesh->IndexOffset,
                     RectMesh->VertexOffset, 0);
}

void
shoora_graphics::DrawCircle(Shu::vec2f pos, f32 radius, u32 ColorU32)
{
    Shu::mat4f Model = Shu::Mat4f(1.0f);
    Shu::Scale(Model, Shu::Vec3f(radius, radius, 1.0f));
    Shu::Translate(Model, Shu::Vec3f(pos, 0.0f));

    shoora_mesh *Mesh = shoora_mesh_database::GetMesh(shoora_mesh_type::CIRCLE);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Mesh->MeshFilter.IndexCount, 1, Mesh->IndexOffset,
                     Mesh->VertexOffset, 0);
}

void
shoora_graphics::DrawSpring(const Shu::vec2f &startPos, const Shu::vec2f &endPos, f32 restLength, f32 thickness,
                        i32 nDivisions, u32 Color)
{
    ASSERT(nDivisions > 0);

    Shu::vec2f l = (endPos - startPos);
    f32 length = l.Magnitude();
    Shu::vec2f unitVector = Shu::Normalize(l);
    Shu::vec2f perp = Shu::Normalize(Shu::Vec2f(-unitVector.y, unitVector.x));
    i32 sign = -1;

    f32 offset = length / (f32)nDivisions;
    Shu::vec2f p0 = startPos;

    for (i32 DivIndex = 0; DivIndex < nDivisions; ++DivIndex)
    {
        Shu::vec2f t = startPos + unitVector * (offset * (DivIndex + 1.0f));
        Shu::vec2f p1 = t + perp * (thickness * 0.5f * sign);

        DrawLine(p0, p1, Color, 3.0f);

        p0 = p1;
        sign *= -1;
    }
}

void
shoora_graphics::Draw(u32 IndexCount, u32 IndexOffset, i32 VertexOffset)
{
    vkCmdDrawIndexed(GlobalCommandBuffer, IndexCount, 1, IndexOffset, VertexOffset, 0);
}