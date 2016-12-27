
#pragma once
#include <math.h>
#include <iostream>



#define M_PI		3.14159265359 
#define M_PI_2		1.570796326795
#define M_2PI		6.28318530718 

class vec3
{
public:
	float x,y,z;

	vec3(float a=0,float b=0, float c=0);
	vec3 operator-(vec3 &q);
	vec3 operator+(vec3 &q);
	vec3 operator*(float k);
	float length();
	void normalize();
	void rotar_xz(float an);
	void rotar_xz(vec3 O, float an);
	void rotar(vec3 o,vec3 eje,float theta);


};


vec3 cross( vec3 u , vec3 v);
float dot( vec3 u , vec3 v);


class mat3
{
public:
	float m[9];
	mat3(float a11=0,float a12=0, float a13=0,
		float a21=0,float a22=0, float a23=0,
		float a31=0,float a32=0, float a33=0);

	mat3(	vec3 A,vec3 B,vec3 C);
	virtual float det();
	virtual float det(vec3 B,int col);
	vec3 operator*(vec3 p);
};



class vec4
{
public:
	float x,y,z,w;

	vec4(float a=0,float b=0, float c=0,float d=0);

};


class mat4
{
	public:
		float m[16];
		mat4(float a11=0,float a12=0, float a13=0,float a14=0,
			float a21=0,float a22=0, float a23=0,float a24=0,
			float a31=0,float a32=0, float a33=0,float a34=0,
			float a41=0,float a42=0, float a43=0,float a44=0);

		static mat4 RotateX(float an);
		static mat4 RotateY(float an);
		static mat4 RotateZ(float an);
		static mat4 fromBase(vec3 N, vec3 U,vec3 V);

		mat4 operator*(mat4 B);

};

bool intersect(vec3 orig , vec3 dir, vec3 center,float radio) ;

int box_intersection(vec3 A, vec3 B,vec3 O,	vec3 D,	float* tnear , float *tfar);

// chequeo si un bounding box esta dentro de otro(o parcialmente adentro)
bool box_overlap(vec3 Amin,vec3 Amax,vec3 Bmin,vec3 Bmax );


inline void swap(float *a,float *b)
{
	float c = *a;
	*a = *b;
	*b = c;
}


//SSE
#include <iostream>

inline __m128 CrossProduct(__m128 a, __m128 b)
{
	return _mm_sub_ps(
		_mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))), 
		_mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)))
		);
}

inline vec3 fast_cross( vec3 u , vec3 v)
{
	float r[4];
	_mm_storeu_ps(r , CrossProduct(_mm_setr_ps(u.x, u.y, u.z, 0) , _mm_setr_ps(v.x, v.y, v.z, 0)));
	return vec3(r[0],r[1],r[2]);
}



