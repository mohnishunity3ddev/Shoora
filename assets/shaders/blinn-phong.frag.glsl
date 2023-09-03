#version 450

layout(set = 1, binding = 0) uniform sampler2D Textures[3];

#define DIFFUSE_TEX(TexArray) TexArray[0]
#define NORMAL_TEX(TexArray) TexArray[1]
#define SPECULAR_TEX(TexArray) TexArray[2]

// TODO)): Make this a shader variant.
#define MATERIAL_VIEWER 0

#if MATERIAL_VIEWER
struct Material
{
    float AmbientStrength;
    vec3 Diffuse;
    vec3 Specular;
    float Shininess;
};
#endif

layout(set = 2, binding = 0) uniform FragUniform
{
    vec3 LightPos;
    vec3 LightColor;
    vec3 CamPosWS;
    vec3 ObjectColor;

#if MATERIAL_VIEWER
    Material Mat;
#endif

} FragUBO;

layout(location = 0) in vec3 InVertexColor;
layout(location = 1) in vec2 InUV;
layout(location = 2) in vec3 InVertexNormalWS;
layout(location = 3) in vec3 InFragPosWS; // interpolated

layout(location = 0) out vec4 FragColor;

void
main()
{
#if MATERIAL_VIEWER
    Material Mat = FragUBO.Mat;

    vec3 Normal = normalize(InVertexNormalWS);
    vec3 LightDir = normalize(FragUBO.LightPos - InFragPosWS);

    // ambient
    vec3 Ambient = FragUBO.LightColor*Mat.AmbientStrength;

    // Diffuse
    float NDotL = max(dot(Normal, LightDir), 0.);
    vec3 Diffuse = (NDotL*Mat.Diffuse) * FragUBO.LightColor;

    // Specular
    vec3 ViewDir = normalize(FragUBO.CamPosWS - InFragPosWS);
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float Spec = pow(max(dot(ReflectDir, ViewDir), 0.), Mat.Shininess);
    vec3 Specular = (Spec*Mat.Specular) * FragUBO.LightColor;

    vec3 Result = (Diffuse + Ambient + Specular);
    FragColor = vec4(Result, 1.);
#else
    vec4 DiffuseTex = texture(DIFFUSE_TEX(Textures), InUV);
    vec4 SpecularTex = texture(SPECULAR_TEX(Textures), InUV);

    vec3 Normal = normalize(InVertexNormalWS);
    vec3 LightDir = normalize(FragUBO.LightPos - InFragPosWS);

    // Ambient
    float AmbientStrength = .1f;
    vec3 Ambient = (AmbientStrength*DiffuseTex.rgb) * FragUBO.LightColor;

    // Diffuse
    float NDotL = max(dot(Normal, LightDir), 0.);
    vec3 Diffuse = (NDotL*DiffuseTex.rgb) * FragUBO.LightColor;

    // Specular
    vec3 ViewDir    = normalize(FragUBO.CamPosWS - InFragPosWS);
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float Spec      = pow(max(dot(ReflectDir, ViewDir), 0.), 64);
    vec3 Specular   = (Spec*SpecularTex.rgb) * FragUBO.LightColor;

    vec3 Result = (Diffuse + Ambient + Specular);
    FragColor = vec4(Result, 1.);
#endif

}