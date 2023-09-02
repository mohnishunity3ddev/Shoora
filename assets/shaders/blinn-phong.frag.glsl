#version 450

layout(set = 1, binding = 0) uniform sampler2D Textures[3];
#define DIFFUSE_TEX(TexArray) TexArray[0]
#define NORMAL_TEX(TexArray) TexArray[1]
#define SPECULAR_TEX(TexArray) TexArray[2]

layout(set = 2, binding = 0) uniform FragUniform
{
    vec3 LightPos;
    vec3 LightColor;
    vec3 CamPosWS;
    vec3 ObjectColor;
} FragUBO;

layout(location = 0) in vec3 InVertexColor;
layout(location = 1) in vec2 InUV;
layout(location = 2) in vec3 InVertexNormalWS;
layout(location = 3) in vec3 InFragPosWS; // interpolated

layout(location = 0) out vec4 FragColor;
void main()
{
    vec4 texColor = texture(DIFFUSE_TEX(Textures), InUV);
    vec3 ObjColor = texColor.rgb*FragUBO.ObjectColor;

    vec3 Normal = normalize(InVertexNormalWS);
    vec3 LightDir = normalize(FragUBO.LightPos - InFragPosWS);

    // Ambient
    float AmbientStrength = .1f;
    vec3 Ambient = AmbientStrength*FragUBO.LightColor;

    // Diffuse
    float NDotL = max(dot(Normal, LightDir), 0.);
    vec3 Diffuse = NDotL*FragUBO.LightColor;

    // Specular
    float SpecularStrength = .25;
    vec3 ViewDir = normalize(FragUBO.CamPosWS - InFragPosWS);
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float Spec = pow(max(dot(ReflectDir, ViewDir), 0.), 64);
    vec3 Specular = Spec*FragUBO.LightColor;

    vec3 Result = (Diffuse + Ambient + Specular)*texColor.rgb;
    

    FragColor = vec4(Result, texColor.a);
}