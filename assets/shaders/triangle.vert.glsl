#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec3 Color;

layout(location = 0) out vec3 OutColor;

void main()
{
	OutColor = Color;
	gl_Position = vec4(Pos, 0., 1.);
}