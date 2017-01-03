#include "StdAfx.h"
#include <math.h>
#include "math.h"

vec3::vec3(float a,float b, float c)
{
	x = a;
	y = b;
	z = c;
}

vec3 vec3::operator+(const vec3 &q)
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

// A,B,C vectores columna
mat3::mat3(vec3 A,vec3 B,vec3 C)
{
	m[0] = A.x;
	m[3] = A.y;
	m[6] = A.z;

	m[1] = B.x;
	m[4] = B.y;
	m[7] = B.z;

	m[2] = C.x;
	m[5] = C.y;
	m[8] = C.z;
}

mat3::mat3(	float a11,float a12, float a13,
		   float a21,float a22, float a23,
		   float a31,float a32, float a33)
{
	m[0] = a11;		m[1] = a12;		m[2] = a13;
	m[3] = a21;		m[4] = a22;		m[5] = a23;
	m[6] = a31;		m[7] = a32;		m[8] = a33;
}


float mat3::det()
{
	return m[0]*m[4]*m[8] + m[1]*m[5]*m[6] + m[3]*m[7]*m[2] - (m[2]*m[4]*m[6] + m[3]*m[1]*m[8] + m[7]*m[5]*m[0]);
}



float mat3::det(vec3 B,int col)
{
	float D=0;
	switch(col)
	{
	case 1:

		D = B.x*m[4]*m[8] + m[1]*m[5]*B.z + B.y*m[7]*m[2] -(m[2]*m[4]*B.z + B.y*m[1]*m[8] + m[7]*m[5]*B.x);
		break;
	case 2:
		D = m[0]*B.y*m[8] + B.x*m[5]*m[6] + m[3]*B.z*m[2] - (m[2]*B.y*m[6] + m[3]*B.x*m[8] + B.z*m[5]*m[0]);
		break;
	case 3:
		D = m[0]*m[4]*B.z + m[1]*B.y*m[6] + m[3]*m[7]*B.x -(B.x*m[4]*m[6] + m[3]*m[1]*B.z+ m[7]*B.y*m[0]);
		break;
	}
	return D ;
}

// Multiplicar un vector3d x una matriz de 3x3 da como resultado
// un vector3d
vec3 mat3::operator*(vec3 p)
{
	vec3 r;
	r.x = p.x*m[0] + p.y*m[1]+ p.z*m[2];
	r.y = p.x*m[3] + p.y*m[4]+ p.z*m[5];
	r.z = p.x*m[6] + p.y*m[7]+ p.z*m[8];
	return r;
}




vec4::vec4(float a,float b, float c,float d)
{
	x = a;
	y = b;
	z = c;
	w = d;
}

// escalar x vector
vec4 vec4::operator*(float k)
{
	return(vec4(k*x,k*y,k*z,k*w));
}

vec4 vec4::operator+(const vec4 &q)
{
	return(vec4(x+q.x,y+q.y,z+q.z,w+q.w));
}

mat4::mat4(	float a11,float a12, float a13,float a14,
		   float a21,float a22, float a23,float a24,
		   float a31,float a32, float a33,float a34,
		   float a41,float a42, float a43,float a44)

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


mat4 mat4::RotateX(float an)
{
	float c = cos(an);
	float s = sin(an);
	return mat4(		
		1,	0,	0,	0,
		0,	c,	-s,	0,
		0,	s,	c,	0,
		0,	0,	0,	1);
}

mat4 mat4::RotateY(float an)
{
	float c = cos(an);
	float s = sin(an);
	return mat4(		
		c,	0,	s,	0,
		0,	1,	0,	0,
		-s,	0,	c,	0,
		0,	0,	0,	1);
}

mat4 mat4::RotateZ(float an)
{
	float c = cos(an);
	float s = sin(an);
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
		return true;
	}
	return false;

} 


 
// chequeo si un bounding box esta dentro de otro(o parcialmente adentro)
bool box_overlap(vec3 Amin,vec3 Amax,vec3 Bmin,vec3 Bmax )
{
	if(Amin.x>Bmax.x) return false;
	if(Amin.y>Bmax.y) return false;
	if(Amin.z>Bmax.z) return false;

	if(Amax.x<Bmin.x) return false;
	if(Amax.y<Bmin.y) return false;
	if(Amax.z<Bmin.z) return false;

	return true;			// overlap
}


void sort(float *v,int cant)
{
	for(int i=0;i<cant-1;++i)
		for(int j=i+1;j<cant;++j)
			if(v[j]<v[i])
				swap(&v[j],&v[i]);
}


/*
struct Ray
{
	float ox, oy, oz, ow;
	float dx, dy, dz, dw;
};

struct PrecomputedTriangle
{
	float nx, ny, nz, nd;
	float ux, uy, uz, ud;
	float vx, vy, vz, vd;
};

struct Hit
{
	float px, py, pz, pw;
	float t, u, v;
};

const float int_coef_arr[4] = { -1, -1, -1, 1 };

const __m128 int_coef = _mm_load_ps(helper);

bool Intersect(const Ray &r,
			   const PrecomputedTriangle &p, Hit &h)
{
	const __m128 o = _mm_load_ps(&r.ox);
	const __m128 d = _mm_load_ps(&r.dx);
	const __m128 n = _mm_load_ps(&p.nx);
	const __m128 det = _mm_dp_ps(n, d, 0x7f);
	const __m128 dett = _mm_dp_ps(
		_mm_mul_ps(int_coef, n), o, 0xff);
	const __m128 oldt = _mm_load_ss(&h.t);
	if((_mm_movemask_ps(_mm_xor_ps(dett,
		_mm_sub_ss(_mm_mul_ss(oldt, det), dett)))&1) == 0)
	{
		const __m128 detp = _mm_add_ps(_mm_mul_ps(o, det),
			_mm_mul_ps(dett, d));
		const __m128 detu = _mm_dp_ps(detp,
			_mm_load_ps(&p.ux), 0xf1);
		if((_mm_movemask_ps(_mm_xor_ps(detu,
			_mm_sub_ss(det, detu)))&1) == 0)
		{
			const __m128 detv = _mm_dp_ps(pt,
				_mm_load_ps(&p.vx), 0xf1));
			if((_mm_movemask_ps(_mm_xor_ps(detv,
				_mm_sub_ss(det, _mm_add_ss(detu, detv))))&1) == 0)
			{
				const __m128 inv_det = inv_ss(det);
				_mm_store_ss(&h.t, _mm_mul_ss(dett, inv_det));
				_mm_store_ss(&h.u, _mm_mul_ss(detu, inv_det));
				_mm_store_ss(&h.v, _mm_mul_ss(detv, inv_det));
				_mm_store_ps(&h.px, _mm_mul_ps(detp,
					_mm_shuffle_ps(inv_det, inv_det, 0)));
				return true;
			}
		}
	}
	return false;
}
*/

int upper_power2(int x)
{
	int power = 1;
	while(power < x)
		power*=2;
	return power;
}

