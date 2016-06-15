varying vec3 vTexCoord;

void main()
{	
	vTexCoord = gl_TextureMatrix[0]*gl_MultiTexCoord0;
	gl_Position = gl_Vertex;
}

