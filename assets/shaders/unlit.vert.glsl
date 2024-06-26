#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InColor;
layout(location = 3) in vec2 InUV;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    layout(row_major) mat4 View;
    layout(row_major) mat4 Projection;
} ubo;

layout(push_constant) uniform PushConstantBlock
{
    layout(row_major) mat4 Model;
    vec3 Color;
} PushConstant;

layout(location = 0) out vec3 OutColor;
layout(location = 1) out vec2 OutUV;

void
main()
{
    OutColor = PushConstant.Color;
    // OutUV = InUV*PushConstant.Model[0][0];
    // TODO: Pass in the UV Scale using a uniform. Remove this!
    OutUV = InUV;
    float xx = PushConstant.Model[0][0];
    float yy = PushConstant.Model[1][1];
    if( xx > 1.) {
        OutUV.x = InUV.x * 5.;
        // OutUV.x = InUV.x * (xx*2);
    }
    if(yy > 1.) {
        OutUV.y = InUV.y * 5.;
        // OutUV.y = InUV.y * (yy*2);
    }

    mat4 MVP = PushConstant.Model*ubo.View*ubo.Projection;
    vec4 Pos = vec4(InPos, 1.)*MVP;
    gl_Position = Pos;
}