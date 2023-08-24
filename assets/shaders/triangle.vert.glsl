#version 450

layout(location = 0) in vec2 InPos;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec2 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	vec3 Color;
} ubo;

layout(location = 0) out vec3 OutUniformColor;
layout(location = 1) out vec3 OutVertexColor;
layout(location = 2) out vec2 OutUV;

void main()
{
	OutUniformColor = ubo.Color;
	OutVertexColor = InColor;
	OutUV = InUV;
	gl_Position = vec4(InPos, 0.0, 1.0);
}