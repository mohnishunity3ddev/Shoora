#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec3 Color;

layout(location = 0) out vec3 OutColor;


const vec2 vertices[3] =
{
	vec2( 1.0, 0.0),
	vec2(-1.0, 0.0),
	vec2( 0.5, 0.5)
};

void main()
{
	OutColor = Color;
	gl_Position = vec4(Pos, 0.0, 1.0);
}