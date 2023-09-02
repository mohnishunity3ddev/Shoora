#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InColor;
layout(location = 3) in vec2 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	layout(row_major) mat4 Model;
	layout(row_major) mat4 View;
	layout(row_major) mat4 Projection;
} ubo;

layout(location = 0) out vec3 OutVertexColor;
layout(location = 1) out vec2 OutUV;
layout(location = 2) out vec3 OutVertexNormalWorldSpace;
layout(location = 3) out vec3 OutFragPosWorldSpace; // interpolated

void main()
{
	OutVertexColor = InColor;
	OutUV = InUV;

	mat4 ModelView = ubo.Model*ubo.View;

	mat3 NormalMatrix = mat3(transpose(inverse(ubo.Model)));
	OutVertexNormalWorldSpace = InNormal*NormalMatrix;
	OutFragPosWorldSpace = vec3(vec4(InPos, 1.)*ubo.Model);

	vec4 Pos = vec4(InPos, 1.)*ModelView*ubo.Projection;
	gl_Position = Pos;
}