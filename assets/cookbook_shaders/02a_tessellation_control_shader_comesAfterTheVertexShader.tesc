// There are 3 stages for tesselation alone.
// -> The first one is called a control tessellation stage. It is used to set up parameters that control how the
//    tesellation is performed. We do this by writing a tesellation control shader that specifies values of
//    tessellation factors.
// -> These are optional in Vulkan
// -> If we are using tessellation, we need to enable the tessellationFeature when we query the
//    physicalDeviceFeatures.
//

#version 450

layout(vertices = 3) out;

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 3.0;
        gl_TessLevelOuter[0] = 3.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 5.0;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}