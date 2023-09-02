#version 450

layout(set = 1, binding = 0) uniform sampler2D Textures[3];
#define DIFFUSE_TEX(TexArray) TexArray[0]
#define NORMAL_TEX(TexArray) TexArray[1]
#define SPECULAR_TEX(TexArray) TexArray[2]

layout(set = 2, binding = 0) uniform FragUniform
{
    vec3 LightPos;
    vec3 LightColor;

    vec3 ObjectColor;
} FragUBO;

layout(location = 0) in vec3 InVertexColor;
layout(location = 1) in vec2 InUV;
layout(location = 2) in vec3 InVertexNormalWorldSpace;
layout(location = 3) in vec3 InFragPosWorldSpace; // interpolated

layout(location = 0) out vec4 FragColor;
void main()
{
    vec4 texColor = texture(DIFFUSE_TEX(Textures), InUV);
    vec3 ObjColor = texColor.rgb*FragUBO.ObjectColor;

    float AmbientStrength = .1f;
    vec3 Ambient = AmbientStrength*FragUBO.LightColor;

    vec3 Normal = normalize(InVertexNormalWorldSpace);
    vec3 LightDir = normalize(FragUBO.LightPos - InFragPosWorldSpace);

    float NDotL = dot(Normal, LightDir);
    vec3 Diffuse = NDotL*texColor.rgb*FragUBO.LightColor;

    vec3 Result = Diffuse + Ambient;

    FragColor = vec4(Result, texColor.a);
}