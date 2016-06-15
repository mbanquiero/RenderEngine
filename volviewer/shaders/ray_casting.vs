varying vec3 vTexCoord;

void main()
{	
	vTexCoord = gl_Vertex.xyz;
	gl_Position = gl_Vertex;
}
