#version 450	

const vec3 vertices[3] = {
	vec3(  0,      0.5f, 0),
	vec3(  0.5f,  -0.5f, 0),
	vec3( -0.5f,  -0.5f, 0)

};

void main() 
{
	int vI = gl_VertexIndex;

	gl_Position = vec4(vertices[vI], 1);
}