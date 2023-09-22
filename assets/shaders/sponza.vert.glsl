#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec4 inTangent;

layout(set = 0, binding = 0) uniform UBOScene
{
    layout(row_major) mat4 projection;
    layout(row_major) mat4 view;
    vec4 lightPos;
    vec4 viewPos;
}
uboScene;

layout(push_constant) uniform PushConsts
{
    layout(row_major) mat4 model;
} primitive;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outViewVec;
layout(location = 4) out vec3 outLightVec;
layout(location = 5) out vec4 outTangent;

void
main()
{
    outNormal = inNormal;
    outColor = inColor;
    outUV = inUV;
    outTangent = inTangent;
    gl_Position = vec4(inPos.xyz, 1.0) * primitive.model * uboScene.view * uboScene.projection;
    
    outNormal = inNormal * mat3(primitive.model);
    vec4 pos = vec4(inPos, 1.0) * primitive.model;
    outLightVec = uboScene.lightPos.xyz - pos.xyz;
    outViewVec = uboScene.viewPos.xyz - pos.xyz;
}