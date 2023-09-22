#version 450

layout(set = 1, binding = 0) uniform sampler2D DiffuseMap;
layout(set = 1, binding = 1) uniform sampler2D NormalMap;
layout(set = 1, binding = 2) uniform sampler2D MetallicMap;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inViewVec;
layout(location = 4) in vec3 inLightVec;
layout(location = 5) in vec4 inTangent;

layout(location = 0) out vec4 outFragColor;

layout(constant_id = 0) const bool ALPHA_MASK = false;
layout(constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0f;

void
main()
{
    vec4 color = texture(DiffuseMap, inUV) * vec4(inColor, 1.0);
    vec4 metallicSample = texture(MetallicMap, inUV);

    float metallic = metallicSample.b;
    float roughness = metallicSample.g;

    if (ALPHA_MASK)
    {
        if (color.a < ALPHA_MASK_CUTOFF)
        {
            discard;
        }
    }

    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent.xyz);
    vec3 B = cross(inNormal, inTangent.xyz) * inTangent.w;
    mat3 TBN = mat3(T, B, N);
    N = TBN * normalize(texture(NormalMap, inUV).xyz * 2.0 - vec3(1.0));

    float ambient = 0.1;
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 H = normalize(L + V);
    vec3 diffuse = max(dot(N, L), ambient).rrr;
    float specular = pow(max(dot(N, H), 0.0), 256);

    outFragColor = vec4(diffuse*color.rgb + specular, color.a);
}