varying vec3 vTexCoord;

void main()
{	
	//vTexCoord = gl_Vertex.xyz;
	vTexCoord = gl_MultiTexCoord0;
	gl_Position = gl_Vertex;
}
