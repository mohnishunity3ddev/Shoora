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

struct PointLight
{
    vec3 Pos;
    vec3 Color;
    float Intensity;
};

layout(set = 2, binding = 0) uniform FragUniform
{
    PointLight Light;

    vec3 CamPosWS;
    vec3 ObjectColor;

#if MATERIAL_VIEWER
    Material Mat;
#endif

} FragUBO;

layout(location = 0) in FS_IN
{
    vec3 Color;
    vec2 UV;
    vec3 FragPosWS; // interpolated
    mat3 TBN;
} FSIn;

layout(location = 0) out vec4 FragColor;

float
GetAttenuation(float d)
{
    float Kc = 1.;
    float Kl = .7;
    float Kq = 1.8;

    if(d > 0 && d <= 7.)
    {
        Kl = .7; Kq = 1.8;
    }
    else if (d > 7. && d <= 13.)
    {
        Kl = .35; Kq = .44;
    }
    else if (d > 13. && d <= 20.)
    {
        Kl = .22;
        Kq = .2;
    }
    else if (d > 20. && d <= 32.)
    {
        Kl = .14;
        Kq = .07;
    }
    else if (d > 32. && d <= 50.)
    {
        Kl = .09;
        Kq = .032;
    }
    else if (d > 50. && d <= 65.)
    {
        Kl = .07;
        Kq = .017;
    }
    else if (d > 65. && d <= 100.)
    {
        Kl = .045;
        Kq = .0075;
    }
    else if (d > 100. && d <= 160.)
    {
        Kl = .27;
        Kq = .0028;
    }
    else if (d > 160. && d <= 200.)
    {
        Kl = .022;
        Kq = .0019;
    }
    else if (d > 200. && d <= 325.)
    {
        Kl = .014;
        Kq = .0007;
    }
    else if (d > 325. && d <= 600.)
    {
        Kl = .007;
        Kq = .0002;
    }
    else if (d > 600. && d <= 3250.)
    {
        Kl = .0014;
        Kq = .00007;
    }
    else if (d > 3250.)
    {
        Kl = 0.0005;
        Kq = 0.00001;
    }

    float attenuation = (1. / (Kc + Kl*d + Kq*d*d));
    return attenuation;
}

void
main()
{
    vec4 DiffuseTex = texture(DIFFUSE_TEX(Textures), FSIn.UV);
    vec4 SpecularTex = texture(SPECULAR_TEX(Textures), FSIn.UV);
    vec4 NormalTex = texture(NORMAL_TEX(Textures), FSIn.UV);

    vec3 NormalWS = normalize((2.*NormalTex.rgb - 1.)*FSIn.TBN);

    PointLight light = FragUBO.Light;

    vec3 LightDir = normalize(light.Pos - FSIn.FragPosWS);

    // Ambient
    float AmbientStrength = .1f;
    vec3 Ambient = (AmbientStrength*DiffuseTex.rgb) * light.Color;

    float d = distance(light.Pos, FSIn.FragPosWS);
    float attenFactor = light.Intensity*GetAttenuation(d);

    // Diffuse
    float NDotL = max(dot(NormalWS, LightDir), 0.);
    vec3 Diffuse = (NDotL*DiffuseTex.rgb)*light.Color;
    Diffuse *= attenFactor;

    // Specular
    vec3 ViewDir    = normalize(FragUBO.CamPosWS - FSIn.FragPosWS);
    vec3 HalfwayDir = normalize(ViewDir + LightDir);
    float Spec      = pow(max(dot(HalfwayDir, NormalWS), 0.), 128);
    vec3 Specular   = (Spec*SpecularTex.rgb) * light.Color * NDotL;
    Specular *= attenFactor;

    vec3 Result = (Diffuse + Ambient + Specular);
    FragColor = vec4(Result, 1.);
}