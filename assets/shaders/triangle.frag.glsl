#version 450

layout(set = 1, binding = 0) uniform sampler2D Sampler;

layout(location = 0) in vec3 InUniformColor;
layout(location = 1) in vec3 InVertexColor;
layout(location = 2) in vec2 InUV;
layout(location = 3) in float InDepth;

layout(location = 0) out vec4 outColor;
void main()
{
    vec4 texColor = texture(Sampler, InUV);
    outColor = vec4(texColor.rgb*InUniformColor, texColor.a);
    // vec3 col = vec3(InDepth,0,0);
    // outColor = vec4(col, 1);
}