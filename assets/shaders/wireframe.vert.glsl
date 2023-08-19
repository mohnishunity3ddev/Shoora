#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec3 Color;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
}