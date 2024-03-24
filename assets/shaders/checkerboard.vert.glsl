#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InColor;
layout(location = 3) in vec2 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    layout(row_major) mat4 View;
    layout(row_major) mat4 Projection;
}
ubo;

layout(push_constant) uniform PushConstantBlock
{
    layout(row_major) mat4 Model;
    vec3 Color;
}
PushConstant;

layout(location = 0) out vec3 OutColor;
layout(location = 1) out vec2 OutUV;
layout(location = 2) out vec4 OutModelPos;
layout(location = 3) out vec3 OutNormal;

void
main()
{
    OutColor = PushConstant.Color;
    // OutColor = InColor;
    // OutUV = InUV*PushConstant.Model[0][0];
    // TODO: Pass in the UV Scale using a uniform. Remove this!
    OutUV = InUV;
    float xx = PushConstant.Model[0][0];
    float yy = PushConstant.Model[1][1];
    if (xx > 1.)
    {
        OutUV.x = InUV.x * 5.;
        // OutUV.x = InUV.x * (xx*2);
    }
    if (yy > 1.)
    {
        OutUV.y = InUV.y * 5.;
        // OutUV.y = InUV.y * (yy*2);
    }

    mat4 MVP = PushConstant.Model * ubo.View * ubo.Projection;

    OutNormal = (vec4(InNormal, 0.0) * PushConstant.Model).xyz;
    // OutNormal = (vec4(InNormal, 1.0)).xyz;
    OutNormal = normalize(OutNormal);
    OutNormal = (OutNormal * .5) + .5;

    OutModelPos = vec4(InPos, 1.) * PushConstant.Model;

    vec4 Pos = vec4(InPos, 1.) * MVP;

    gl_Position = Pos;
}