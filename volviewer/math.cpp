#include "StdAfx.h"
#include <math.h>
#include "math.h"


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

