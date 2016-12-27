uniform sampler3D s_texture0;
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
	float ep = 0.01;
	gl_FragColor = texture3D(s_texture0, vTexCoord);
	if(gl_FragColor.r < ep)
		discard;
		
	gl_FragColor.a *= gl_FragColor.a ;
/*	if( (abs(vTexCoord.x)<ep && abs(vTexCoord.y)<ep) || (abs(vTexCoord.x)<ep && abs(vTexCoord.z)<ep)
			|| (abs(vTexCoord.y)<ep && abs(vTexCoord.z)<ep) ||
		(abs(vTexCoord.x-1)<ep && abs(vTexCoord.y-1)<ep) || (abs(vTexCoord.x-1)<ep && abs(vTexCoord.z-1)<ep)
			|| (abs(vTexCoord.y-1)<ep && abs(vTexCoord.z-1)<ep))			
			gl_FragColor.rgba = 1;
*/
}

