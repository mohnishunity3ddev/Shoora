#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec3 Color;

layout(binding = 0) uniform UniformBuffer
{
	vec3 Color;
} ubo;

layout(location = 0) out vec3 OutColor;

void main()
{
	OutColor = ubo.Color;
	gl_Position = vec4(Pos, 0.0, 1.0);
}