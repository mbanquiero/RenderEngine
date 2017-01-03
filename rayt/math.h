
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
	vec3 operator+(const vec3 &q);
	vec3 operator*(float k);
	float operator [](int i) {return i==0?x : i==1? y : z;};
	//float operator [](int i) {return (&x)[i];};
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
	vec4 operator*(float k);
	vec4 operator+(const vec4 &q);

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


// chequeo si un bounding box esta dentro de otro(o parcialmente adentro)
bool box_overlap(vec3 Amin,vec3 Amax,vec3 Bmin,vec3 Bmax );


inline void swap(float *a,float *b)
{
	float c = *a;
	*a = *b;
	*b = c;
}
void sort(float *v,int cant);

//SSE
#include <iostream>



inline __m128 sse_cross(__m128 a, __m128 b)
{
	return _mm_sub_ps(
		_mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))), 
		_mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)))
		);
}


// esta mal
inline __m128 cross_4shuffles(__m128 a, __m128 b)
{
	__m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 a_zxy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 b_zxy = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 1));
	__m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx));
}

// no mejora 
inline __m128 cross_3shuffles(__m128 a, __m128 b)
{
	__m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 c = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
	return _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
}


#define M_LOG2E 1.44269504088896340736 //log2(e)

inline long double log2(const long double x){
	return  log(x) * M_LOG2E;
}

inline int int_floor(double x) { 
	return (int)(x+100000) - 100000; 
}

inline float unit_clamp(double x) { 
	x+=100000;
	return x-(int)x; 
}

int upper_power2(int x);

inline BYTE clamp_to_color(float x) { x*=255;return x<0?0:x>255?255:x;};

inline float saturate(float x) { return x<0?0 : x>1? 1 : x;};
inline vec3 saturate(vec3 v) { return vec3(saturate(v.x), saturate(v.y),saturate(v.z));};

inline vec3 reflect(vec3 i,vec3 n) {return i - n*(2*dot(i, n));};


