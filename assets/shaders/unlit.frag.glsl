#version 450

layout(location = 0) in vec3 InColor;
layout(location = 1) in vec2 InUV;

layout(set = 1, binding = 0) uniform sampler2D DiffuseMap;

layout(location = 0) out vec4 FragColor;
void main()
{
    // vec4 color = texture(DiffuseMap, InUV) * vec4(InColor, 1.0);
    vec4 color = vec4(InColor, 0.1);
    FragColor = vec4(color.rgb, color.a);
}