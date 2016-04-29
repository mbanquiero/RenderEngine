uniform sampler3D s_texture;

varying vec3 vTexCoord;

// transfer function
vec4 transfer(float I)
{
	vec4 clr;
	float s = 0.75;
	if(I<0.3)
		clr = vec4(s , s , 1.0 , 1.0);
	else
	if(I<0.7)
		clr = vec4(s , 1.0 , s,1.0);
	else
		clr = vec4(1.0 , s,s,1.0) ;
		
	return clr * I * 1.5;
}

void main()
{
	float ambient = 0.5;
	float diffuse = 0.8;
	float I = texture3D(s_texture, vTexCoord).r;
	float gx = texture3D(s_texture, vTexCoord+vec3(1.0/256.0, 0.0 , 0.0)).r;
	float gy = texture3D(s_texture, vTexCoord+vec3(0.0, 1.0/256.0, 0.0)).r;
	float gz = texture3D(s_texture, vTexCoord+vec3(0.0 , 0.0, 1.0/256.0)).r;
	vec3 N = normalize(vec3(gx,gy,gz));
	vec3 L = normalize(vTexCoord*256.0 - vec3(100.0 , 300.0 , 10.0));
	gl_FragColor = transfer(I*(ambient + abs(dot(N,L))*diffuse));
	//gl_FragColor = texture3D(s_texture, vTexCoord) * (ambient + abs(dot(N,L))*diffuse);
	
}
