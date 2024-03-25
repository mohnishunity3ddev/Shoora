#include "vulkan_graphics.h"
#include <mesh/database/mesh_database.h>
#include <math/math_trig.h>

static VkCommandBuffer GlobalCommandBuffer;
static VkPipelineLayout GlobalPipelineLayout;

struct data
{
    shu::mat4f Model;
    shu::vec3f Color = {1, 1, 1};
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

VkCommandBuffer &
shoora_graphics::GetCmdBuffer()
{
    return GlobalCommandBuffer;
}

VkPipelineLayout &
shoora_graphics::GetPipelineLayout()
{
    return GlobalPipelineLayout;
}

void
shoora_graphics::DrawLine2D(const shu::vec2f P0, const shu::vec2f P1, u32 ColorU32, f32 Thickness)
{
#if 0
    Shu::vec2f p0 = MouseToScreenSpace(P0);
    Shu::vec2f p1 = MouseToScreenSpace(P1);
#endif
    shu::vec2f l = P1 - P0;

    shu::vec3f Pos = shu::Vec3f((P0 + P1) * 0.5f, 1.0f);
    shu::vec3f Scale = shu::Vec3f(l.Magnitude(), Thickness, 1.0f);

    shoora_mesh *Line = shoora_mesh_database::GetMesh(shoora_mesh_type::RECT_2D);
    shu::mat4f Model = shu::TRS(Pos, Scale, l.GetSlopeAngleInDegrees(), shu::Vec3f(0.0f, 0.0f, 1.0f));

    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Line->MeshFilter.IndexCount, 1, Line->IndexOffset, Line->VertexOffset, 0);
}

void
shoora_graphics::DrawCubeWireframe(const shu::vec3f &v000, const shu::vec3f &v100, const shu::vec3f &v110, const shu::vec3f &v010,
                                   const shu::vec3f &v001, const shu::vec3f &v101, const shu::vec3f &v111, const shu::vec3f &v011,
                                   u32 ColorU32, f32 Thickness)
{
    // Front Face
    DrawLine3D(v000, v100, ColorU32, Thickness);
    DrawLine3D(v100, v110, ColorU32, Thickness);
    DrawLine3D(v110, v010, ColorU32, Thickness);
    DrawLine3D(v010, v000, ColorU32, Thickness);

    // Front-Back Connecting Edges.
    DrawLine3D(v000, v001, ColorU32, Thickness);
    DrawLine3D(v100, v101, ColorU32, Thickness);
    DrawLine3D(v110, v111, ColorU32, Thickness);
    DrawLine3D(v010, v011, ColorU32, Thickness);

    // Back Face
    DrawLine3D(v001, v101, ColorU32, Thickness);
    DrawLine3D(v101, v111, ColorU32, Thickness);
    DrawLine3D(v111, v011, ColorU32, Thickness);
    DrawLine3D(v011, v001, ColorU32, Thickness);
}

static f32 _angle = 0.0f;

shu::vec3f
calculateCircumcenter(const shu::vec3f &vA, const shu::vec3f &vB, const shu::vec3f &vC)
{
    // Calculate the midpoints of two sides of the triangle
    shu::vec3f midAB = (vA + vB) / 2.0f;
    shu::vec3f midAC = (vA + vC) / 2.0f;

    // Calculate the normal to the triangle plane
    shu::vec3f normal = shu::Cross(vB - vA, vC - vA);
    normal = shu::Normalize(normal);

    // Calculate the perpendicular bisectors
    shu::vec3f bisectorAB = shu::Cross(normal, vB - vA);
    shu::vec3f bisectorAC = shu::Cross(normal, vC - vA);

    // Calculate the intersection point of the bisectors
    shu::vec3f circumcenter = midAB +
                              bisectorAB * shu::Dot(midAC - midAB, bisectorAC) / shu::Dot(bisectorAB, bisectorAC);

    return circumcenter;
}

void
translateTriangleToOrigin(shu::vec3f &vA, shu::vec3f &vB, shu::vec3f &vC)
{
    shu::vec3f circumcenter = calculateCircumcenter(vA, vB, vC);
    vA -= circumcenter;
    vB -= circumcenter;
    vC -= circumcenter;
}

void
shoora_graphics::DrawTriangle(const shu::vec3f &A, const shu::vec3f &B, const shu::vec3f &C)
{
#if 0
    DrawLine3D(A, B, colorU32::Proto_Green);
    DrawLine3D(B, C, colorU32::Proto_Green);
    DrawLine3D(C, A, colorU32::Proto_Green);
#endif

    shu::mat4f Model = shu::Mat4f(1.0f);
    shu::vec3f xAxis = C - A;
    shu::vec3f yAxis = B - A;
    shu::vec3f zAxis = yAxis.Cross(xAxis);
    Model.Row0 = shu::Vec4f(xAxis, 0);
    Model.Row1 = shu::Vec4f(yAxis, 0);
    Model.Row2 = shu::Vec4f(zAxis, 0);
    Model.Row3 = shu::Vec4f(A, 1);

    shoora_mesh *TriangleMesh = shoora_mesh_database::GetMesh(shoora_mesh_type::TRIANGLE);

    data Value = {.Model = Model, .Color = GetColor(colorU32::Blue)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data),
                       &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, TriangleMesh->MeshFilter.IndexCount, 1, TriangleMesh->IndexOffset,
                     TriangleMesh->VertexOffset, 0);
}

void
shoora_graphics::DrawCube(const shu::vec3f &Position, const u32 ColorU32, const f32 Scale)
{
    shu::mat4f Model = shu::Mat4f(1.0f);
    Model = shu::Scale(Model, shu::Vec3f(Scale));
    Model = shu::Translate(Model, Position);

    // shu::mat4f Model = shu::TRS(Position, shu::Vec3f(Scale), shu::QuatIdentity());

    shoora_mesh *Cube = shoora_mesh_database::GetMesh(shoora_mesh_type::CUBE);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data),
                       &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Cube->MeshFilter.IndexCount, 1, Cube->IndexOffset, Cube->VertexOffset, 0);
}

void
shoora_graphics::DrawLine3D(const shu::vec3f &P0, const shu::vec3f &P1, u32 ColorU32, f32 Thickness)
{
    shu::vec3f Diff = P1 - P0;
    shu::vec3f Direction = shu::Normalize(Diff);

    // NOTE: The problem is to align the cube mesh's local right vector to the direction given by the two points.
    // That means we need a quaternion which rotates the right vector so that it becomes the direction vector given
    // by the passed in two points. Axis of the rotation is always perpendicular to the two vectors. Angle to
    // rotate by is the angle between the two vectors.
    shu::vec3f RightVector = shu::Vec3f(1, 0, 0);
    f32 Angle = shu::CosInverse(RightVector.Dot(Direction)) * RAD_TO_DEG;
    shu::vec3f Axis = RightVector.Cross(Direction);
    shu::quat Rotation = shu::QuatAngleAxisDeg(Angle, Axis);

    shu::vec3f Pos = (P0 + P1) * 0.5f;
    shu::vec3f Scale = shu::Vec3f(Diff.Magnitude(), Thickness, Thickness);

    shoora_mesh *Cube = shoora_mesh_database::GetMesh(shoora_mesh_type::CUBE);
    shu::mat4f Model = shu::TRS(Pos, Scale, Rotation);

    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Cube->MeshFilter.IndexCount, 1, Cube->IndexOffset, Cube->VertexOffset, 0);
}

void
shoora_graphics::DrawRect(i32 X, i32 Y, u32 Width, u32 Height, u32 ColorU32)
{
    shu::mat4f Model = shu::Mat4f(1.0f);
    shu::Scale(Model, shu::Vec3f((f32)Width * 0.5f, (f32)Height * 0.5f, 1.0f));
    shu::Translate(Model, shu::Vec3f(X, Y, 0.0f));

    shoora_mesh *RectMesh = shoora_mesh_database::GetMesh(shoora_mesh_type::RECT_2D);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, RectMesh->MeshFilter.IndexCount, 1, RectMesh->IndexOffset,
                     RectMesh->VertexOffset, 0);
}

void
shoora_graphics::DrawCircle(shu::vec2f pos, f32 radius, u32 ColorU32)
{
    shu::mat4f Model = shu::Mat4f(1.0f);
    shu::Scale(Model, shu::Vec3f(radius, radius, 1.0f));
    shu::Translate(Model, shu::Vec3f(pos, 0.0f));

    shoora_mesh *Mesh = shoora_mesh_database::GetMesh(shoora_mesh_type::CIRCLE);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(data), &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Mesh->MeshFilter.IndexCount, 1, Mesh->IndexOffset,
                     Mesh->VertexOffset, 0);
}

void
shoora_graphics::DrawSphere(shu::vec3f pos, f32 radius, u32 ColorU32)
{
    shu::mat4f Model = shu::Mat4f(1.0f);
    shu::Scale(Model, shu::Vec3f(radius));
    shu::Translate(Model, pos);

    shoora_mesh *Mesh = shoora_mesh_database::GetMesh(shoora_mesh_type::SPHERE);
    data Value = {.Model = Model, .Color = GetColor(ColorU32)};

    vkCmdPushConstants(GlobalCommandBuffer, GlobalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data),
                       &Value);
    vkCmdDrawIndexed(GlobalCommandBuffer, Mesh->MeshFilter.IndexCount, 1, Mesh->IndexOffset, Mesh->VertexOffset,
                     0);
}

void
shoora_graphics::DrawSpring(const shu::vec2f &startPos, const shu::vec2f &endPos, f32 restLength, f32 thickness,
                        i32 nDivisions, u32 Color)
{
    ASSERT(nDivisions > 0);

    shu::vec2f l = (endPos - startPos);
    f32 length = l.Magnitude();
    shu::vec2f unitVector = shu::Normalize(l);
    shu::vec2f perp = shu::Normalize(shu::Vec2f(-unitVector.y, unitVector.x));
    i32 sign = -1;

    f32 offset = length / (f32)nDivisions;
    shu::vec2f p0 = startPos;

    for (i32 DivIndex = 0; DivIndex < nDivisions; ++DivIndex)
    {
        shu::vec2f t = startPos + unitVector * (offset * (DivIndex + 1.0f));
        shu::vec2f p1 = t + perp * (thickness * 0.5f * sign);

        DrawLine2D(p0, p1, Color, 3.0f);

        p0 = p1;
        sign *= -1;
    }
}

void
shoora_graphics::Draw(u32 IndexCount, u32 IndexOffset, i32 VertexOffset)
{
    vkCmdDrawIndexed(GlobalCommandBuffer, IndexCount, 1, IndexOffset, VertexOffset, 0);
}