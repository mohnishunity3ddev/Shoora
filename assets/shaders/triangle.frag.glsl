#version 450

layout(set = 1, binding = 0) uniform sampler2D Textures[3];
#define DIFFUSE_TEX(TexArray) TexArray[0]
#define NORMAL_TEX(TexArray) TexArray[1]
#define SPECULAR_TEX(TexArray) TexArray[2]

layout(set = 2, binding = 0) uniform FragUniform
{
    vec3 DirLightDirection;
    vec3 DirLightColor;
} FragUBO;

layout(location = 0) in vec3 InUniformColor;
layout(location = 1) in vec3 InVertexColor;
layout(location = 2) in vec2 InUV;

layout(location = 0) out vec4 FragColor;
void main()
{
    vec4 texColor = texture(DIFFUSE_TEX(Textures), InUV);
    FragColor = vec4(texColor.rgb*FragUBO.DirLightColor, texColor.a);
}