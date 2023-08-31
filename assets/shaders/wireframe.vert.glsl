#version 450

layout(set = 0, binding = 0) uniform UniformBuffer
{
    layout(row_major) mat4 Model;
    layout(row_major) mat4 View;
    layout(row_major) mat4 Projection;

    vec3 Color;
} ubo;

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec2 InUV;

void main()
{
    vec4 Pos = vec4(InPos, 1.)*ubo.Model*ubo.View*ubo.Projection;
    gl_Position = Pos;
}