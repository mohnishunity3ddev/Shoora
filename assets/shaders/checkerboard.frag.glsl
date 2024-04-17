#version 450

layout(location = 0) in vec3 InColor;
layout(location = 1) in vec2 InUV;
layout(location = 2) in vec4 ModelPos;
layout(location = 3) in vec3 ModelNormal;

layout(set = 1, binding = 0) uniform sampler2D DiffuseMap;

layout(location = 0) out vec4 FragColor;

vec3 lightDir = vec3(1, 1, 1);

vec3
GetColorFromPositionAndNormal(in vec3 worldPosition, in vec3 normal)
{
    const float pi = 3.141519;

    vec3 scaledPos = worldPosition.xyz * pi * 2.0;
    vec3 scaledPos2 = worldPosition.xyz * pi * 2.0 / 10.0 + vec3(pi / 4.0);
    float s = cos(scaledPos2.x) * cos(scaledPos2.y) * cos(scaledPos2.z); // [-1,1] range
    float t = cos(scaledPos.x) * cos(scaledPos.y) * cos(scaledPos.z);    // [-1,1] range

    vec3 colorMultiplier = vec3(0.5, 0.5, 1);
    if (abs(normal.x) > abs(normal.y) && abs(normal.x) > abs(normal.z))
    {
        colorMultiplier = vec3(1, 0.5, 0.5);
    }
    else if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z))
    {
        colorMultiplier = vec3(0.5, 1, 0.5);
    }

    t = ceil(t * 0.9);
    s = (ceil(s * 0.9) + 3.0) * 0.25;
    vec3 colorB = vec3(0.85, 0.85, 0.85);
    vec3 colorA = vec3(1, 1, 1);
    vec3 finalColor = mix(colorA, colorB, t) * s;

    return colorMultiplier * finalColor;
}

void
main()
{
#if 0
    float dx = 0.25;
    float dy = 0.25;
    vec3 colorMultiplier = vec3(0.0, 0.0, 0.0);
    for (float y = 0.0; y < 1.0; y += dy)
    {
        for (float x = 0.0; x < 1.0; x += dx)
        {
            vec4 samplePos = ModelPos + dFdx(ModelPos) * x + dFdy(ModelPos) * y;
            colorMultiplier += GetColorFromPositionAndNormal(samplePos.xyz, ModelNormal) * dx * dy;
        }
    }

    vec4 finalColor;
    finalColor.rgb = colorMultiplier.rgb;
    finalColor.a = 1.0;
#endif

    vec3 ld = normalize(lightDir);
    float d = dot(ModelNormal, ld);
    vec3 c = d*InColor;

    // FragColor = vec4(ModelNormal, 1.);
    // FragColor = finalColor;
    FragColor = vec4(c, 0.5);
}