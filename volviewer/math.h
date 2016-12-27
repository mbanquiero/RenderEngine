
#pragma once

#define M_PI		3.14159265359 
#define M_PI_2		1.570796326795
#define M_2PI		6.28318530718 

class vec3
{
public:
	double x,y,z;

	vec3(double a=0,double b=0, double c=0);
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
void swap(float *a,float *b);

class vec4
{
public:
	double x,y,z,w;

	vec4(double a=0,double b=0, double c=0,double d=0);

};


class mat4
{
	public:
		double m[16];
		mat4(double a11=0,double a12=0, double a13=0,double a14=0,
			double a21=0,double a22=0, double a23=0,double a24=0,
			double a31=0,double a32=0, double a33=0,double a34=0,
			double a41=0,double a42=0, double a43=0,double a44=0);

		static mat4 RotateX(double an);
		static mat4 RotateY(double an);
		static mat4 RotateZ(double an);
		static mat4 fromBase(vec3 N, vec3 U,vec3 V);

		mat4 operator*(mat4 B);

};

bool intersect(vec3 orig , vec3 dir, vec3 center,float radio) ;

