#version 450

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 Model;
    vec3 Color;
}
ubo;

layout(location = 0) in vec2 InPos;
layout(location = 1) in vec3 InColor;

void main()
{
    vec4 Pos = ubo.Model*vec4(InPos, 0, 1);
    gl_Position = Pos;
}