#version 450

layout(set = 0, binding = 1) uniform UniformBuffer
{
    vec3 Color;
} ubo;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(ubo.Color, 1.0);
}