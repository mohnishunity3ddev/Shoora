#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec2 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	layout(row_major) mat4 Model;
	layout(row_major) mat4 View;
	layout(row_major) mat4 Projection;

	vec3 Color;
} ubo;

layout(location = 0) out vec3 OutUniformColor;
layout(location = 1) out vec3 OutVertexColor;
layout(location = 2) out vec2 OutUV;
layout(location = 3) out float OutDepth;

void main()
{
	OutUniformColor = ubo.Color;
	OutVertexColor = InColor;
	OutUV = InUV;
	vec4 pos = vec4(InPos, 1.)*ubo.Model*ubo.View*ubo.Projection;
	gl_Position = pos;
	OutDepth = pos.z / pos.w;
}