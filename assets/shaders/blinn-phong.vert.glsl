#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InColor;
layout(location = 3) in vec2 InUV;
layout(location = 4) in vec3 InTangent;
layout(location = 5) in vec3 InBiTangent;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	layout(row_major) mat4 View;
	layout(row_major) mat4 Projection;
} ubo;

layout(push_constant) uniform PushConsts
{
	layout(row_major) mat4 Model;
} pushConsts;

layout(location = 0) out VS_OUT
{
	vec3 Color;
	vec2 UV;
	vec3 FragPosWS; // interpolated
	mat3 TBN;
} VSOut;

void main()
{
	VSOut.Color = InColor;
	VSOut.UV = InUV;

	mat4 ModelView = pushConsts.Model*ubo.View;

	vec3 TangentWS = normalize(vec3(vec4(InTangent, 0.)*pushConsts.Model));
	vec3 NormalWS = normalize(vec3(vec4(InNormal, 0.)*pushConsts.Model));

	TangentWS = normalize(TangentWS - dot(TangentWS, NormalWS)*NormalWS);

	vec3 BiTangentWS = cross(TangentWS, NormalWS);

	mat3 TBN = mat3(TangentWS, BiTangentWS, NormalWS);
	// NOTE: The above mat3 accepts columns as arguments. We want rows here.
	VSOut.TBN = transpose(TBN);

	// NOTE: This was when we were not using the TBN Matrix to transform the normals.
	// mat3 NormalMatrix = mat3(transpose(inverse(ubo.Model)));
	// OutVertexNormalWS = InNormal*NormalMatrix;
	VSOut.FragPosWS = vec3(vec4(InPos, 1.)*pushConsts.Model);

	vec4 Pos = vec4(InPos, 1.)*ModelView*ubo.Projection;
	gl_Position = Pos;
}