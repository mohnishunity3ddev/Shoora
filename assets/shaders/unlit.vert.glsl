#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InColor;
layout(location = 3) in vec3 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    layout(row_major) mat4 Model;
    layout(row_major) mat4 View;
    layout(row_major) mat4 Projection;
} ubo;

void
main()
{
    mat4 MVP = ubo.Model*ubo.View*ubo.Projection;

    vec4 Pos = vec4(InPos, 1.)*MVP;
    gl_Position = Pos;
}