#pragma once

#include <math.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"

#define M_PI	3.1415926535f
#define M_PI_2  3.1415926535f/2.0f
#define EPSILON	0.0001f

#define IGUAL(x,y) fabs((x)-(y))<EPSILON?TRUE:FALSE

void swap(int *a,int *b);
void swap(long *a,long *b);
void swap(double *a,double *b);
void swap(float *a,float *b);
void swap(char *a,char *b);

char sign(float n);				// retorna 1 si es positivo -1 si es negativo
int round(float x);				// Devuelve el entero mas cercano a x
float round_dir(float x);		// Redondea un angulo
float round2(float x);
float round1(float x);

class Matrix3x3;
class Matrix2x2;
class Vector2;
class Vector3;

#pragma warning( disable : 4244 )

class Vector2
{
	public:
		float x,y;

		Vector2(float a=0,float b=0);
		Vector2(POINT p);
		Vector2 operator=(Vector2 p);
		virtual void rotar(Vector2 o,float an);
		virtual void rotar(float an);
		float distancia(){return(sqrt(x*x+y*y));};
		float distancia(Vector2 q);
		// operaciones entre vectores y escalres
		Vector2 operator-(Vector2 &q);
		Vector2 operator+(Vector2 &q);
		Vector2 operator*(float k);
		float operator>>(Vector2 &q);
		float mod();			// modulo
		float angulo();			// angulo -pi y pi
		float angulo2();			// angulo 0 y 2pi
		Vector2 normal();
		void normalizar();
		void ortonormalizar();
		operator CPoint() const { return CPoint(x,y);};
		virtual Vector2 yx() { return Vector2(y,x);};
		float coords(int i){return i==0?x:y;};

		// entrada / salida
		virtual void grabar(FILE *fp);
		virtual void cargar(FILE *fp);
};


class Matrix2x2
{
	public:
		float a11;
		float a12;
		float a21;
		float a22;
		Matrix2x2(	float a=0,float b=0,
						float c=0,float d=0);
		virtual float det();
		virtual float X(Vector2 B);
		virtual float Y(Vector2 B);

};


class Vector3
{
	public:
		float x,y,z;

		Vector3(float a=0,float b=0,float c=0);
		Vector3(Vector2 p,float c =0);
		Vector3(D3DXVECTOR3 v);

		Vector3 operator=(Vector3 p);
		Vector3 operator=(Vector2 p);
		virtual void SetValue(float a,float b,float c);
		virtual void rotar_xy(Vector3 o,float an);
		virtual void rotar_xz(Vector3 o,float an);
		virtual void rotar_zy(Vector3 o,float an);
		virtual void rotar_xy(float an);
		virtual void rotar_xz(float an);
		virtual void rotar_zy(float an);
		virtual void rotar(Vector3 o,Vector3 eje,float an);
		virtual void rotar(Vector3 o,float an_x,float an_y,float an_z);
		virtual void rotar_inv(Vector3 o,float an_x,float an_y,float an_z);
		float distancia(){return(sqrt(x*x+y*y+z*z));};
		float distancia(Vector3 q);
		// operaciones entre vectores y escalres
		Vector3 operator-(Vector3 &q);
		Vector3 operator+(Vector3 &q);
		Vector3 operator*(float k);
		Vector3 operator*(Vector3 &q);
		Vector3 operator*(Matrix3x3 &M);

		float operator>>(Vector3 &q);
		float mod();			// modulo
		float mod_xy();		// distancia plana (sobre x,y)
		void normalizar();
		void swap_xy();

		Vector2 pxy(){return Vector2(x,y);};
		Vector2 pxz(){return Vector2(x,z);};
		Vector2 pyx(){return Vector2(y,x);};
		Vector2 pyz(){return Vector2(y,z);};
		float coords(int i){return i==0?x:i==1?y:z;};

		// entrada / salida
		virtual void grabar(FILE *fp);
		virtual void cargar(FILE *fp);

		operator D3DXVECTOR3() const { return D3DXVECTOR3(x,y,z);};

};



class Matrix3x3
{
	public:
		float a11;
		float a12;
		float a13;
		float a21;
		float a22;
		float a23;
		float a31;
		float a32;
		float a33;
		Matrix3x3(	float a=0,float b=0, float c=0,float d=0,float e=0, float f=0,float g=0,float h=0, float i=0);
		Matrix3x3(	Vector3 A,Vector3 B,Vector3 C);
		Matrix3x3 operator=(Matrix3x3 M);
		virtual float det();
		virtual float det(Vector3 B,int col);
		virtual Matrix3x3 inversa();
		Vector3 operator*(Vector3 p);
		Matrix3x3 operator*(Matrix3x3 B);

};



class CBox
{
	public:
	Vector3 A;
	Vector3 B;
	CBox(Vector3 a=Vector3(0,0,0),Vector3 b=Vector3(0,0,0))
	{
		A=a;
		B=b;
	}
};

// helpers geometrias
Vector2 barycentric(Vector3 v1,Vector3 v2,Vector3 v3,Vector3 p);

BOOL interseccion_2rectas(Vector2 p0,Vector2 dir_0,
						  Vector2 p1,Vector2 dir_1,Vector2 *Ip);
BOOL interseccion_2rectas(Vector2 p0,Vector2 dir_0,
						  Vector2 p1,Vector2 dir_1,float *t);

BOOL interseccion_2segmentos(Vector2 p0,Vector2 p1,Vector2 q0,Vector2 q1,
									Vector2 *Ip,BOOL extremos=TRUE);

float interseccion_2segmentos(Vector2 p0,Vector2 p1,Vector2 q0,Vector2 q1,BOOL extremos=TRUE);
BOOL pto_inside_tri(Vector2 pt,Vector2 p0,Vector2 p1,Vector2 p2);

BOOL pto_inside_poly(Vector2 pt,Vector2 P[]);


float interseccion_recta_plano(Vector3 p0,Vector3 rd,Vector3 N,float plano_D);

BOOL colinales(Vector2 p0,Vector2 p1,Vector2 p2,float ep=0.1);		


// contorno en 2d
class CPerimetro
{
	public:
		int cant_ptos;
		Vector2 Pt[1000];
		DWORD idata[1000];
		float H;

		// Creacion
		CPerimetro();
		CPerimetro(Vector2 *P,int cant);
		
		// edicion de puntos
		virtual int cerrar();		// cierra la figura y devuelve la cant. de puntos
		virtual void delete_pt(int i);
		virtual void add_pt(Vector2 p0,DWORD id=0);
		
		// Triangulacion
		virtual int intersecciones(Vector2 p0,Vector2 p1,Vector2 *Ip=NULL);
		virtual int interseccion(Vector2 p0,Vector2 v,Vector2 *Ip=NULL);
		virtual BOOL inside(Vector2 p0);
		virtual BOOL inside(CPerimetro *Q);
		virtual BOOL outside(CPerimetro *Q);
		virtual BOOL es_oreja(int i);
		virtual int triangular(Vector2 Q[]);


		// Transformar
		virtual void offset(float *ds);		// offsets
		virtual void explotar(float *r,float da = M_PI/32);		// explotar curvas

		// GC
		virtual CPerimetro *substract(CPerimetro *Q);
		virtual CPerimetro *substractInside(CPerimetro *Q);
		virtual CPerimetro *substractBorde(CPerimetro *Q);
		virtual CPerimetro *substractEsquina(CPerimetro *Q);
		

};

int arc3p(Vector2 p0,Vector2 p1,Vector2 p2,Vector2 *Pt,float da = M_PI/32);	// Genera un arco que pasa x 3 puntos

Vector3 ComputeCuadraticBezier(Vector3 P0,Vector3 P1,Vector3 P2,float t);


