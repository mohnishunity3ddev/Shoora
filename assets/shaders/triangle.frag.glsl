#version 450

layout(set = 1, binding = 0) uniform sampler2D ImageSamplers[3];

#define DIFFUSE_TEX(Sampler) Sampler[0]
#define NORMAL_TEX(Sampler) Sampler[1]
#define SPECULAR_TEX(Sampler) Sampler[2]

layout(location = 0) in vec3 InUniformColor;
layout(location = 1) in vec3 InVertexColor;
layout(location = 2) in vec2 InUV;

layout(location = 0) out vec4 outColor;
void main()
{
    vec4 texColor = texture(NORMAL_TEX(ImageSamplers), InUV);

    outColor = vec4(texColor.rgb*InUniformColor, texColor.a);
}