uniform sampler3D s_texture0;
uniform vec3 iLookFrom;
uniform vec3 iViewDir;
uniform vec3 iDx;
uniform vec3 iDy;
uniform float voxel_step0;
uniform float voxel_step;
uniform int game_status;
uniform float time;

varying vec3 vTexCoord;

const float cant_total = 50.0;


// transfer function
vec4 transfer(float I)
{
	vec4 clr;
	float t0 = 0.3;
	float t1 = 0.7;
	if(I<t0)
		clr = vec4(0.0 , 0.0 , I/t0 , 1.0);
	else
	if(I<t1)
		clr = vec4(0.0 , (I-t0)/(t1-t0) , 0.0 , 1.0);
	else
		clr = vec4((I-t1)/(1-t1),0.0 , 0.0 , 1.0);
	return clr;
	
}


float3 tex3d(vec3 pos)
{
	pos += vec3(128.0,128.0,128.0);
	//return pos.x>=0 && pos.x<=256 && pos.y>=0 && pos.y<=256 && pos.z>=0 && pos.z<=256 ? 
		//	float3(1.,1.,1.) : float3(0.,0.,0.);
		
	float k = 1./256.0;
	//float I = texture3D(s_texture0,pos.xzy*k).r;
	//return transfer(I);
	return texture3D(s_texture0,pos.xyz*k).rgb;

}

void main()
{
	vec2 uv = vTexCoord.xy;

	voxel_step0 = 10;
	voxel_step = 40;
	
	// computo la direccion del rayo
	// D = N + Dy*y + Dx*x;
	
	vec3 rd = normalize(iViewDir + iDy*uv.y + iDx*uv.x);
	vec3 ro = iLookFrom + rd*voxel_step0;
	float3 S = vec3(0.0,0.0,0.0);
	float k = 1.;

	// ray marching
	for (int i = 0; i < cant_total; i++) {
		S += tex3d(ro)*k;
		ro += rd*voxel_step;
		k*=k;
	}
	S /= cant_total;
	S*=7.;
	
	gl_FragColor = vec4( S, 1.0 );
	
}


