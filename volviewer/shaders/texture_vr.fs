uniform sampler3D s_texture0;
uniform vec3 pos;
uniform vec3 iViewDir;
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
	/*
	vec3 pos_anomalia = vec3(0,0,0);
	vec3 q = (vTexCoord-0.5)* 256;
	float s = dot(q-pos,iViewDir);
	float dist = length(pos + iViewDir*s - q);
	if( s>=0 && s<75 && dist <5)
	{
		gl_FragColor = vec4(0.5,0.5,1,1);
	}
	else
	{
		dist = length(pos-q);
		if( dist<10)
			gl_FragColor = vec4(0.5,0.5,1,1-dist/10);
		else
		{
			dist = length(pos_anomalia-q);
			if( dist<10)
				gl_FragColor = vec4(1,0.5,0.5,1-dist/10);
			else
			{
				gl_FragColor = texture3D(s_texture0, vTexCoord);
			//	gl_FragColor.a *= gl_FragColor.a;
				gl_FragColor.a *= gl_FragColor.a * 0.1;
			}
		}
	}
	*/
	
	vec3 q = (vTexCoord-0.5)* 256;
	float s = dot(q-pos,iViewDir);
	float dist = length(pos + iViewDir*s - q);
	if( s>=0 && s<75 && dist <5)
	{
		gl_FragColor = vec4(0.5,0.5,1,1);
	}
	else
	{
		dist = length(pos-q);
		if( dist<10)
			gl_FragColor = vec4(0.5,0.5,1,1-dist/10);
		else
		{
			gl_FragColor = texture3D(s_texture0, vTexCoord);
			if(gl_FragColor.r>0.9)
				gl_FragColor.a = 0.07;
			else
				gl_FragColor.a *= gl_FragColor.a * 0.1;
		}
	}
}

