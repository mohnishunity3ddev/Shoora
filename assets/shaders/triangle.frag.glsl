#version 450

layout(location = 0) in vec3 VertexColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(1., .5, .2, 1.0);
}