#include "StdAfx.h"
#include <math.h>
#include "math.h"

vec3::vec3(double a,double b, double c)
{
	x = a;
	y = b;
	z = c;
}

vec3 vec3::operator+(vec3 &q)
{
	return(vec3(x+q.x,y+q.y,z+q.z));
}

// diferencia de vectores
vec3 vec3::operator-(vec3 &q)
{
	return(vec3(x-q.x,y-q.y,z-q.z));
}

// escalar x vector
vec3 vec3::operator*(float k)
{
	return(vec3(k*x,k*y,k*z));
}


float vec3::length()
{
	return(sqrt(fabs(x*x+y*y+z*z)));
}

void vec3::normalize()
{
	float m = length();
	if(fabs(m)>0.001)
	{
		x/=m;
		y/=m;
		z/=m;
	}
}

void vec3::rotar_xz(float an)
{
	float xt=x*cos(an)-z*sin(an); 
	float zt=x*sin(an)+z*cos(an);
	x = xt;
	z = zt;
}

void vec3::rotar_xz(vec3 O, float an)
{
	*this = *this - O;
	float xt=x*cos(an)-z*sin(an); 
	float zt=x*sin(an)+z*cos(an);
	x = xt;
	z = zt;
	*this = *this + O;

}


void vec3::rotar(vec3 o,vec3 eje,float theta)
{
	float a = o.x;
	float b = o.y;
	float c = o.z;
	float u = eje.x;
	float v = eje.y;
	float w = eje.z;

	float u2 = u*u;
	float v2 = v*v;
	float w2 = w*w;
	float cosT = cos(theta);
	float sinT = sin(theta);
	float l2 = u2 + v2 + w2;
	float l =  sqrt(l2);

	if(l2 < 0.000000001)		// el vector de rotacion es casi nulo
		return;

	float xr = a*(v2 + w2) + u*(-b*v - c*w + u*x + v*y + w*z) 
		+ (-a*(v2 + w2) + u*(b*v + c*w - v*y - w*z) + (v2 + w2)*x)*cosT
		+ l*(-c*v + b*w - w*y + v*z)*sinT;
	xr/=l2;

	float yr = b*(u2 + w2) + v*(-a*u - c*w + u*x + v*y + w*z) 
		+ (-b*(u2 + w2) + v*(a*u + c*w - u*x - w*z) + (u2 + w2)*y)*cosT
		+ l*(c*u - a*w + w*x - u*z)*sinT;
	yr/=l2;

	float zr = c*(u2 + v2) + w*(-a*u - b*v + u*x + v*y + w*z) 
		+ (-c*(u2 + v2) + w*(a*u + b*v - u*x - v*y) + (u2 + v2)*z)*cosT
		+ l*(-b*u + a*v - v*x + u*y)*sinT;
	zr/=l2;

	x = xr;
	y = yr;
	z = zr;
}


vec3 cross( vec3 u , vec3 v)
{
	return vec3(u.y*v.z-u.z*v.y , u.z*v.x-u.x*v.z , u.x*v.y-u.y*v.x);
}

float dot( vec3 u , vec3 v)
{
	return u.x*v.x+u.y*v.y+u.z*v.z;	
}


vec4::vec4(double a,double b, double c,double d)
{
	x = a;
	y = b;
	z = c;
	w = d;
}


mat4::mat4(	double a11,double a12, double a13,double a14,
		   double a21,double a22, double a23,double a24,
		   double a31,double a32, double a33,double a34,
		   double a41,double a42, double a43,double a44)

{
	m[0] = a11;		m[1] = a21;		m[2] = a31;		m[3] = a41;
	m[4] = a12;		m[5] = a22;		m[6] = a32;		m[7] = a42;
	m[8] = a13;		m[9] = a23;		m[10] = a33;	m[11] = a43;
	m[12] = a14;	m[13] = a24;	m[14] = a34;	m[15] = a44;
}


mat4 mat4::fromBase(vec3 N, vec3 U,vec3 V)
{
	return mat4(		
		N.x,	U.x,	V.x,	0,
		N.y,	U.y,	V.y,	0,
		N.z,	U.z,	V.z,	0,
		0,	0,	0,	1);

}


mat4 mat4::RotateX(double an)
{
	double c = cos(an);
	double s = sin(an);
	return mat4(		
		1,	0,	0,	0,
		0,	c,	-s,	0,
		0,	s,	c,	0,
		0,	0,	0,	1);
}

mat4 mat4::RotateY(double an)
{
	double c = cos(an);
	double s = sin(an);
	return mat4(		
		c,	0,	s,	0,
		0,	1,	0,	0,
		-s,	0,	c,	0,
		0,	0,	0,	1);
}

mat4 mat4::RotateZ(double an)
{
	double c = cos(an);
	double s = sin(an);
	return mat4(		
		c,	-s,	0,	0,
		s,	c,	0,	0,
		0,	0,	1,	0,
		0,	0,	0,	1);
}


mat4 mat4::operator*(mat4 B)
{
	mat4 C;
	// 1er fila
	C.m[0] = m[0]*B.m[0] + m[4]*B.m[1] + m[8]*B.m[2] + m[12]*B.m[3];
	C.m[4] = m[0]*B.m[4] + m[4]*B.m[5] + m[8]*B.m[6] + m[12]*B.m[7];
	C.m[8] = m[0]*B.m[8] + m[4]*B.m[9] + m[8]*B.m[10] + m[12]*B.m[11];
	C.m[12] = m[0]*B.m[12] + m[4]*B.m[13] + m[8]*B.m[14] + m[12]*B.m[15];

	// 2da fila
	C.m[1] = m[1]*B.m[0] + m[5]*B.m[1] + m[9]*B.m[2] + m[13]*B.m[3];
	C.m[5] = m[1]*B.m[4] + m[5]*B.m[5] + m[9]*B.m[6] + m[13]*B.m[7];
	C.m[9] = m[1]*B.m[8] + m[5]*B.m[9] + m[9]*B.m[10] + m[13]*B.m[11];
	C.m[13] = m[1]*B.m[12] + m[5]*B.m[13] + m[9]*B.m[14] + m[13]*B.m[15];

	// 3era fila
	C.m[2] = m[2]*B.m[0] + m[6]*B.m[1] + m[10]*B.m[2] + m[14]*B.m[3];
	C.m[6] = m[2]*B.m[4] + m[6]*B.m[5] + m[10]*B.m[6] + m[14]*B.m[7];
	C.m[10] = m[2]*B.m[8] + m[6]*B.m[9] + m[10]*B.m[10] + m[14]*B.m[11];
	C.m[14] = m[2]*B.m[12] + m[6]*B.m[13] + m[10]*B.m[14] + m[14]*B.m[15];

	// 4ta fila
	C.m[3] = m[3]*B.m[0] + m[7]*B.m[1] + m[11]*B.m[2] + m[15]*B.m[3];
	C.m[7] = m[3]*B.m[4] + m[7]*B.m[5] + m[11]*B.m[6] + m[15]*B.m[7];
	C.m[11] = m[3]*B.m[8] + m[7]*B.m[9] + m[11]*B.m[10] + m[15]*B.m[11];
	C.m[15] = m[3]*B.m[12] + m[7]*B.m[13] + m[11]*B.m[14] + m[15]*B.m[15];

	return C;
}


void swap(float *a,float *b)
{
	float c = *a;
	*a = *b;
	*b = c;
}



// helper interseccion rayo - esfera
bool intersect(vec3 orig , vec3 dir, vec3 center,float radio) 
{ 
	vec3 L = orig-center;
	float B = 2*dot(dir, L);
	float C = dot(L,L)-radio*radio;
	float disc = B*B - 4*C;
	if(disc>=0)
	{
		float t0 = (-B-sqrt(disc))/2;
		float t1 = (-B+sqrt(disc))/2;
		if(t0>t1)
			swap(&t0,&t1);
		// t0 < t1 
		if(fabs(t0-15)<1 || fabs(t1-15)<1)
			return true;
	}
	return false;

} 
