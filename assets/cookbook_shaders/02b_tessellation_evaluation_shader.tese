// Provides the info on tessellated vertices(the new vertices generated by the tessellator) to the subsequent
// stages in the pipeline.
#version 450
layout(triangles, equal_spacing, cw) in;

void main()
{
    gl_Position = gl_in[0].gl_Position*gl_TessCoord.x +
                  gl_in[1].gl_Position*gl_TessCoord.y +
                  gl_in[2].gl_Position*gl_TessCoord.z;
}
