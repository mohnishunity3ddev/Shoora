#version 450

float
GetAttenuation(float d)
{
    float Kc = 1.;
    float Kl = .7;
    float Kq = 1.8;

#if 0
    if(d > 0 && d <= 7.)
    {
        Kl = .7;
        Kq = 1.8;
    }
    else if(d > 7. && d <= 13.)
    {
        Kl = .35;
        Kq = .44;
    }
    else if(d > 13. && d <= 20.)
    {
        Kl = .22;
        Kq = .2;
    }
    else if(d > 20. && d <= 32.)
    {
        Kl = .14;
        Kq = .07;
    }
    else if(d > 32. && d <= 50.)
    {
        Kl = .09;
        Kq = .032;
    }
    else if(d > 50. && d <= 65.)
    {
        Kl = .07;
        Kq = .017;
    }
    else if(d > 65. && d <= 100.)
    {
        Kl = .045;
        Kq = .0075;
    }
    else if(d > 100. && d <= 160.)
    {
        Kl = .027;
        Kq = .0028;
    }
    else if(d > 160. && d <= 200.)
    {
        Kl = .022;
        Kq = .0019;
    }
    else if(d > 200. && d <= 325.)
    {
        Kl = .014;
        Kq = .0007;
    }
    else if(d > 325. && d <= 600.)
    {
        Kl = .007;
        Kq = .0002;
    }
    else if(d > 600. && d <= 3250.)
    {
        Kl = .0014;
        Kq = .00007;
    }
    else if(d > 3250.)
    {
        Kl = .0007;
        Kq = .000035;
    }
#endif

    float Attenuation = (1. / (Kc + Kl*d + Kq*d*d));
    return Attenuation;
}

#define PI 3.14159
#define DEG_TO_RAD(Angle) (PI / 180.)*Angle
#define AMBIENT_STRENGTH .1

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

struct SpotLight
{
    bool IsOn;

    vec3 Pos;
    vec3 Color;
    vec3 Direction;

    float InnerCutoffAngles;
    float OuterCutoffAngles;
    float Intensity;
};

layout(set = 2, binding = 0) uniform FragUniform
{
    PointLight PointLights[4];
    SpotLight SpotLight;

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

vec3
CalculatePointLight(PointLight Point, float Attenuation, vec3 NormalWS,
                    vec3 DiffuseTex, vec3 SpecTex)
{
    vec3 LightDir = normalize(Point.Pos - FSIn.FragPosWS);

    // Ambient
    vec3 Ambient = (AMBIENT_STRENGTH*DiffuseTex) * Point.Color;

    // Diffuse
    float NDotL = max(dot(NormalWS, LightDir), 0.);
    vec3 Diffuse = (NDotL * DiffuseTex) * Point.Color;

    // Specular
    vec3 ViewDir = normalize(FragUBO.CamPosWS - FSIn.FragPosWS);
    vec3 HalfwayDir = normalize(ViewDir + LightDir);
    float Spec = pow(max(dot(HalfwayDir, NormalWS), 0.), 128);
    vec3 Specular = (Spec * SpecTex.rgb) * Point.Color * NDotL;

    vec3 Result = (Ambient + Diffuse + Specular)*Attenuation;
    return Result;
}

vec3
CalculateSpotLight(SpotLight Spot, float InnerCutoff, float OuterCutoff, float Attenuation, vec3 NormalWS,
                   vec3 DiffuseTex, vec3 SpecTex)
{
    vec3 Ambient = (AMBIENT_STRENGTH*DiffuseTex) * Spot.Color;
    Ambient *= Attenuation;

    vec3 LightDir = normalize(Spot.Pos - FSIn.FragPosWS);
    float Theta = dot(LightDir, -Spot.Direction);

    //? If this dInner is greater than zero, then it means fragment is inside the inner spotlight citcle that we
    //? get.
    float SpotEnable = smoothstep(OuterCutoff, InnerCutoff, Theta);

    // Diffuse
    float NDotL = max(dot(NormalWS, LightDir), 0.);
    vec3 Diffuse = (NDotL*DiffuseTex) * Spot.Color;
    Diffuse *= Attenuation*SpotEnable;

    // Specular
    vec3 ViewDir = normalize(FragUBO.CamPosWS - FSIn.FragPosWS);
    vec3 HalfwayDir = normalize(ViewDir + LightDir);
    float Spec = pow(max(dot(HalfwayDir, NormalWS), 0.), 128);
    vec3 Specular = (Spec * SpecTex.rgb) * Spot.Color * NDotL;
    Specular *= Attenuation*SpotEnable;

    vec3 Result = (Ambient + Diffuse + Specular);
    return Result;
}

void
main()
{
    vec4 DiffuseTex = texture(DIFFUSE_TEX(Textures), FSIn.UV);
    vec4 SpecularTex = texture(SPECULAR_TEX(Textures), FSIn.UV);

    vec4 NormalTex = texture(NORMAL_TEX(Textures), FSIn.UV);
    vec3 NormalWS = normalize((2.*NormalTex.rgb - 1.) * FSIn.TBN);

    vec3 Result = vec3(0.);

    for(int i = 0; i < 4; ++i)
    {
        PointLight PtLight = FragUBO.PointLights[i];
        float PointDist = distance(PtLight.Pos, FSIn.FragPosWS);
        float PointAttenuation = PtLight.Intensity*GetAttenuation(PointDist);

        Result += CalculatePointLight(PtLight, PointAttenuation, NormalWS, DiffuseTex.rgb, SpecularTex.rgb);
    }

    SpotLight Spot = FragUBO.SpotLight;
    float SpotLightInnerCutoff = cos(DEG_TO_RAD(Spot.InnerCutoffAngles));
    float SpotLightOuterCutoff = cos(DEG_TO_RAD(Spot.OuterCutoffAngles));
    float SpotDist = distance(Spot.Pos, FSIn.FragPosWS);
    float SpotAttenuation = Spot.Intensity*GetAttenuation(SpotDist);

    Result += CalculateSpotLight(Spot, SpotLightInnerCutoff, SpotLightOuterCutoff, SpotAttenuation, NormalWS,
                                 DiffuseTex.rgb, SpecularTex.rgb);

    FragColor = vec4(Result, 1.);
}