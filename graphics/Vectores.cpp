
#include "stdafx.h"
#include<stdio.h>
#include<math.h>
#include"vectores.h"
#include<afxpriv.h>

void swap(int *a,int *b)
{
	int x = *a;
	*a = *b;
	*b = x;
}

void swap(long *a,long *b)
{
	long x = *a;
	*a = *b;
	*b = x;
}

void swap(double *a,double *b)
{
	double c = *a;
	*a = *b;
	*b = c;
}

void swap(float *a,float *b)
{
	float c = *a;
	*a = *b;
	*b = c;
}

void swap(char *a,char *b)
{
	char c = *a;
	*a = *b;
	*b = c;
}



// retorna 1 si es positivo -1 si es negativo
char sign(float n)
{
	return(n<0?-1:1);
}

// Devuelve el entero mas cercano a x
int round(float x)
{
	int rta;
	int a = (int)floor(x);
	int b = a + 1;
	// luego
	// a <= x <= b
	if(fabs(a-x) < fabs(b-x))
		rta = a;
	else
		rta = b;

	return rta;
}

// redondea a 2 decimales
float round2(float x)
{
	char buffer[255];
	sprintf(buffer,"%10.2f",round(x*100)/100.0);
	return atof(buffer);
}

// redondea a 1 decimales
float round1(float x)
{
	char buffer[255];
	sprintf(buffer,"%10.1f",round(x*10)/10.0);
	return atof(buffer);
}


// Redondea un angulo, de tal forma que si es casi ortogonal, quede ortogonal.
// pero cualquier otro angulo, con la precision maxima que se le pueda dar
float round_dir(float x)
{
	float rta = x;
	int angulo = round(x*10);
	if(angulo%900==0)
		rta = angulo/10;
	return rta;
}




Vector3::Vector3(float a,float b,float c)
{
	x=a;
	y=b;
	z=c;
}

Vector3::Vector3(Vector2 p,float c)
{
	x = p.x;
	y = p.y;
	z = c;
}

Vector3::Vector3(D3DXVECTOR3 v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}


Vector3 Vector3::operator=(Vector3 p)
{
	x=p.x;
	y=p.y;
	z=p.z;
	return(*this);
}

Vector3 Vector3::operator=(Vector2 p)
{
	x=p.x;
	y=p.y;
	z=0;
	return(*this);
}

void Vector3::SetValue(float a,float b,float c)
{
	x=a;
	y=b;
	z=c;
}


void Vector3::grabar(FILE *fp)
{
	fprintf(fp,"%g\n",x);
	fprintf(fp,"%g\n",y);
	fprintf(fp,"%g\n",z);
}


void Vector3::cargar(FILE *fp)
{
	char buffer[80];
	fgets(buffer,80,fp);
	x=atof(buffer);
	fgets(buffer,80,fp);
	y=atof(buffer);
	fgets(buffer,80,fp);
	z=atof(buffer);
}

// rotacion x-y es sobre el eje z
void Vector3::rotar_xy(Vector3 o,float an)
{
	if(an==0)
		return;

	float ro=sqrt((x-o.x)*(x-o.x)+(y-o.y)*(y-o.y));
	float alfa;
	if((x-o.x)!=0)
		alfa=atan2(y-o.y,x-o.x);
	else
		alfa=(y-o.y)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	y=o.y+ro*sin(alfa);

	/*
	x=x*cos(an)+y*sin(an);
	y=y*cos(an)-x*sin(an); 
	*/
}

// rotacion xz es sobre el eje y
void Vector3::rotar_xz(Vector3 o,float an)
{
	if(an==0)
		return;

	float ro=sqrt((x-o.x)*(x-o.x)+(z-o.z)*(z-o.z));
	float alfa;
	if((x-o.x)!=0)
		alfa=atan2(z-o.z,x-o.x);
	else
		alfa=(z-o.z)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	z=o.z+ro*sin(alfa); 

	/*
	x=x*cos(an)-z*sin(an); 
	z=x*sin(an)+z*cos(an);
	*/
}

// rotacion zy es sobre el eje x
void Vector3::rotar_zy(Vector3 o,float an)
{
	if(an==0)
		return;
	float ro=sqrt((y-o.y)*(y-o.y)+(z-o.z)*(z-o.z));
	float alfa;
	if((y-o.y)!=0)
		alfa=atan2(z-o.z,y-o.y);
	else
		alfa=(z-o.z)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	y=o.y+ro*cos(alfa);
	z=o.z+ro*sin(alfa);

	/*
	y=y*cos(an)+z*sin(an);
	z=z*cos(an)-y*sin(an); 
	*/

}

// rotacion x-y es sobre el eje z
void Vector3::rotar_xy(float an)
{
	float xr = x*cos(an)-y*sin(an);
	float yr = y*cos(an)+x*sin(an); 
	x = xr;
	y = yr;
}


// rotacion xz es sobre el eje y
void Vector3::rotar_xz(float an)
{
	
	float xr=x*cos(an)+z*sin(an); 
	float zr=-x*sin(an)+z*cos(an);
	x = xr;
	z = zr;

}

// rotacion zy es sobre el eje x
void Vector3::rotar_zy(float an)
{
	float yr=y*cos(an)-z*sin(an);
	float zr=z*cos(an)+y*sin(an); 
	y = yr;
	z = zr;
}


// Rotacion sobre un eje arbitrario
void Vector3::rotar(Vector3 o,Vector3 eje,float theta)
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

void Vector3::rotar(Vector3 o,float an_X,float an_Y,float an_Z)
{
	Vector3 eje_x = Vector3(1,0,0);
	Vector3 eje_y = Vector3(0,1,0);
	Vector3 eje_z = Vector3(0,0,1);
	Vector3 O = Vector3(0,0,0);

	// primero roto en el eje Y
	rotar(o,eje_y,an_Y);
	// y roto los ejes x,z tambien, la primer rotacion se puede hacer
	// directamente, pues los ejes todavia no estan rotados
	eje_x.rotar_xz(an_Y);
	eje_z.rotar_xz(an_Y);

	// sin embargo las siguientes tienen que ser sobre los ejes ya rotados
	// Ahoro roto sobre el eje X 
	rotar(o,eje_x,an_X);		// el pto pp dicho
	// y los ejes 
	eje_z.rotar(O,eje_x,an_X);

	// Y ahora roto sobre el eje Z 
	rotar(o,eje_z,an_Z);		// el pto pp dicho

}



void Vector3::rotar_inv(Vector3 o,float an_Z,float an_X,float an_Y)
{
	Vector3 eje_x = Vector3(1,0,0);
	Vector3 eje_y = Vector3(0,1,0);
	Vector3 eje_z = Vector3(0,0,1);
	Vector3 O = Vector3(0,0,0);
	
	// Revierto la rotacion en Y 
	rotar(o,eje_y,-an_Y);		// el pto pp dicho
	// y roto los ejes x,y tambien
	eje_x.rotar_xz(-an_Y);
	eje_z.rotar_xz(-an_Y);

	// Revierto la rot. sobre el eje X (plano zy)
	rotar(o,eje_x,-an_X);		// el pto pp dicho
	eje_z.rotar(O,eje_x,-an_X);

	// y revierto la rot en el eje Z (plano_xy)
	rotar(o,eje_z,-an_Z);
	


}


float Vector3::distancia(Vector3 q)
{
	float dx=x-q.x;
	float dy=y-q.y;
	float dz=z-q.z;
	return(sqrt(fabs(dx*dx+dy*dy+dz*dz)));
}

float Vector3::mod()
{
	return(sqrt(fabs(x*x+y*y+z*z)));
}

float Vector3::mod_xy()
{
	return(sqrt(fabs(x*x+y*y)));
}



void Vector3::normalizar()
{
	float m = mod();
	if(m!=0)
	{
		x/=m;
		y/=m;
		z/=m;
	}
}


// suma de vectores
Vector3 Vector3::operator+(Vector3 &q)
{
	return(Vector3(x+q.x,y+q.y,z+q.z));
}

// diferencia de vectores
Vector3 Vector3::operator-(Vector3 &q)
{
	return(Vector3(x-q.x,y-q.y,z-q.z));
}

// escalar x vector
Vector3 Vector3::operator*(float k)
{
	return(Vector3(k*x,k*y,k*z));
}

// producto vectorial
Vector3 Vector3::operator*(Vector3 &q)
{
	float a=y*q.z-z*q.y;
	float b=z*q.x-x*q.z;
	float c=x*q.y-y*q.x;
	return(Vector3(a,b,c));
}

// producto escalar
float Vector3::operator>>(Vector3 &q)
{
	float rta=x*q.x+y*q.y+z*q.z;
	return(rta);
}




// ----------------------------------------------
// 	VECTORES EN 2 DIMENSIONES
// ----------------------------------------------
Vector2::Vector2(float a,float b)
{
	x=a;
	y=b;
}

Vector2::Vector2(POINT p)
{
	x = p.x;
	y = p.y;
}


Vector2 Vector2::operator=(Vector2 p)
{
	x=p.x;
	y=p.y;
	return(*this);
}


void Vector2::grabar(FILE *fp)
{
	fprintf(fp,"%10.2f",x);
	fprintf(fp,"%10.2f",y);
}


void Vector2::cargar(FILE *fp)
{
	char buffer[50];
	fgets(buffer,11,fp);
	x=atof(buffer);
	fgets(buffer,11,fp);
	y=atof(buffer);
}

void Vector2::rotar(Vector2 o,float an)
{
	float ro=sqrt((x-o.x)*(x-o.x)+(y-o.y)*(y-o.y));
	float alfa;
	if((x-o.x)!=0)
		alfa=atan2(y-o.y,x-o.x);
	else
		alfa=(y-o.y)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	y=o.y+ro*sin(alfa);
}


void Vector2::rotar(float an)
{
	float xp=x*cos(an)-y*sin(an);
	float yp=x*sin(an)+y*cos(an);
	x=xp;
	y=yp;
}



float Vector2::distancia(Vector2 q)
{
	float dx=x-q.x;
	float dy=y-q.y;
	return(sqrt(fabs(dx*dx+dy*dy)));
}


float Vector2::mod()
{
	return(sqrt(fabs(x*x+y*y)));
}

// suma de vectores
Vector2 Vector2::operator+(Vector2 &q)
{
	return(Vector2(x+q.x,y+q.y));
}

// diferencia de vectores
Vector2 Vector2::operator-(Vector2 &q)
{
	return(Vector2(x-q.x,y-q.y));
}

// escalar x vector
Vector2 Vector2::operator*(float k)
{
	return(Vector2(k*x,k*y));
}

// producto escalar
float Vector2::operator>>(Vector2 &q)
{
	return(x*q.x+y*q.y);
}


// retorna un vector normal a si mismo 
Vector2 Vector2::normal()
{
	return(Vector2(-y,x));
}


float Vector2::angulo()
{
	float rta;
	if(x!=0)
		rta = atan2(y,x);
	else
		rta = y>=0?M_PI/2:-M_PI/2;
	return(rta);

}


// devuelve el angulo entre cero y 2pi
float Vector2::angulo2()
{
	float rta = angulo();
	if(rta<0)
		rta+=2*M_PI;
	return rta;
}

//----------------------------------------------------
// MATRIZ 2x2
//----------------------------------------------------

Matrix2x2::Matrix2x2(	float a,float b,float c,float d )
{
	a11=a;
	a12=b;
	a21=c;
	a22=d;
}

float Matrix2x2::det()
{
	float rta = a11*a22 - a12*a21;
	return rta;
}


float Matrix2x2::X(Vector2 B)
{
	float x = (a22*B.x-a12*B.y)/det();
	return x;
}

float Matrix2x2::Y(Vector2 B)
{
	float y = (a11*B.y-a21*B.x)/det();
	return y;
}


	
void Vector2::normalizar()
{
	float m = mod();
	if(m!=0)
	{
		float k = 1/mod();
		*this=*this*k;
	}
}

void Vector2::ortonormalizar()
{
	float ep = 0.00001f; 
	
	if(fabs(x)<ep)
		x = 0;
	if(fabs(y)<ep)
		y = 0;
	
	if(fabs(x-1)<ep)
		x = 1;
	if(fabs(y-1)<ep)
		y = 1;

	if(fabs(x+1)<ep)
		x = -1;
	if(fabs(y+1)<ep)
		y = -1;
}


//----------------------------------------------
Matrix3x3::Matrix3x3(Vector3 A,Vector3 B,Vector3 C)
{
	a11 = A.x;
	a21 = A.y;
	a31 = A.z;

	a12 = B.x;
	a22 = B.y;
	a32 = B.z;

	a13 = C.x;
	a23 = C.y;
	a33 = C.z;
}


Matrix3x3::Matrix3x3(	float a,float b, float c,float d,float e, float f,float g,float h, float i)
{
	a11=a;
	a12=b;
	a13=c;
	a21=d;
	a22=e;
	a23=f;
	a31=g;
	a32=h;
	a33=i;
}

float Matrix3x3::det()
{
	float rta=a11*a22*a33 + a12*a23*a31 + a21*a32*a13 -
			(a13*a22*a31 + a21*a12*a33 + a32*a23*a11);
	return rta;
}


Matrix3x3 Matrix3x3::operator=(Matrix3x3 M)
{
	a11=M.a11;
	a12=M.a12;
	a13=M.a13;
	
	a21=M.a21;
	a22=M.a22;
	a23=M.a23;

	a31=M.a31;
	a32=M.a32;
	a33=M.a33;
	
	return *this;
}



float Matrix3x3::det(Vector3 B,int col)
{
	float D=0;
	switch(col)
	{
		case 1:

			D = B.x*a22*a33 + a12*a23*B.z + B.y*a32*a13 -
			(a13*a22*B.z + B.y*a12*a33 + a32*a23*B.x);

			//D=TMatriz3x3(	B.x,a12,a13,
								//B.y,a22,a23,
								//B.z,a32,a33	).det();
			break;
		case 2:
			D = a11*B.y*a33 + B.x*a23*a31 + a21*B.z*a13 -
			(a13*B.y*a31 + a21*B.x*a33 + B.z*a23*a11);

			//D=TMatriz3x3(	a11,B.x,a13,
								//a21,B.y,a23,
								//a31,B.z,a33	).det();
			break;
		case 3:
			D = a11*a22*B.z + a12*B.y*a31 + a21*a32*B.x -
			(B.x*a22*a31 + a21*a12*B.z+ a32*B.y*a11);

			//D=TMatriz3x3(	a11,a12,B.x,
								//a21,a22,B.y,
								//a31,a32,B.z	).det();
			break;
	}
	return D;
}

// Multiplicar un vector3d x una matriz de 3x3 da como resultado un vector3d
// Matriz x Vector
Vector3 Matrix3x3::operator*(Vector3 p)
{
	Vector3 r;
	r.x = p.x*a11 + p.y*a12 + p.z*a13;
	r.y = p.x*a21 + p.y*a22 + p.z*a23;
	r.z = p.x*a31 + p.y*a32 + p.z*a33;
	return r;
}

// ojo este es al revez :
// Vector x Matriz 
Vector3 Vector3 ::operator*(Matrix3x3 &M)
{
	Vector3 r;
	r.x = x*M.a11 + y*M.a21 + z*M.a31;
	r.y = x*M.a12 + y*M.a22 + z*M.a32;
	r.z = x*M.a13 + y*M.a23 + z*M.a33;
	return r;
}


void Vector3::swap_xy()
{
	float aux = x;
	x = y;
	y = aux;
}



Matrix3x3 Matrix3x3::inversa()
{

	Matrix3x3 Adj;
	float det_A = det();
	if(det_A!=0)
	{
		float k = 1/det_A;

		Adj.a11 = (a22*a33-a32*a23) * k;
		Adj.a21 = -(a21*a33-a31*a23)* k;
		Adj.a31 = (a21*a32-a31*a22)* k;

		Adj.a12 = -(a12*a33-a32*a13)*k;
		Adj.a22 = (a11*a33-a31*a13)*k;
		Adj.a32 = -(a11*a32-a31*a12)*k;

		Adj.a13 = (a12*a23-a22*a13)*k;
		Adj.a23 = -(a11*a23-a21*a13)*k;
		Adj.a33 = (a11*a22-a21*a12)*k;
	}
	return Adj;
}

Matrix3x3 Matrix3x3::operator*(Matrix3x3 B)
{
	Matrix3x3 C;
	C.a11 = a11*B.a11+a12*B.a21+a13*B.a31;
	C.a12 = a11*B.a12+a12*B.a22+a13*B.a32;
	C.a13 = a11*B.a13+a12*B.a23+a13*B.a33;

	C.a21 = a21*B.a11+a22*B.a21+a23*B.a31;
	C.a22 = a21*B.a12+a22*B.a22+a23*B.a32;
	C.a23 = a21*B.a13+a22*B.a23+a23*B.a33;

	C.a31 = a31*B.a11+a32*B.a21+a33*B.a31;
	C.a32 = a31*B.a12+a32*B.a22+a33*B.a32;
	C.a33 = a31*B.a13+a32*B.a23+a33*B.a33;

	return C;
}
	


// helpers, devuelve las coordenadas baricentricas de un pto p
Vector2 barycentric(Vector3 v1,Vector3 v2,Vector3 v3,Vector3 p)
{
	float b,g;

	float A = v1.x - v3.x;
	float B = v2.x - v3.x;
	float C = v3.x - p.x;

	float D = v1.y - v3.y;
	float E = v2.y - v3.y;
	float F = v3.y - p.y;

	float G = v1.z - v3.z;
	float H = v2.z - v3.z;
	float I = v3.z - p.z;

	if(A==0 && B==0)
	{
		swap(&A,&D);
		swap(&B,&E);
		swap(&C,&F);
	}


	b = (B*(F+I) - C*(E+H)) / (A*(E+H) - B*(D+G));
	g = (A*(F+I) - C*(D+G)) / (B*(D+G) - A*(E+H));


	return Vector2(b,g);
}



BOOL interseccion_2rectas(Vector2 p0,Vector2 dir_0,
							Vector2 p1,Vector2 dir_1,Vector2 *Ip)
{
	BOOL rta = FALSE;
	Matrix2x2 M(	dir_0.x,	-dir_1.x,
					dir_0.y,	-dir_1.y	
				);

	if(fabs(M.det())>0.000001f)
	{
		float t = M.X(Vector2(p1.x-p0.x,p1.y-p0.y));
		*Ip = p0 + dir_0*t;
		rta = TRUE;
	}

	return rta;

}


BOOL interseccion_2rectas(Vector2 p0,Vector2 dir_0,
						  Vector2 p1,Vector2 dir_1,float *t)
{
	BOOL rta = FALSE;
	Matrix2x2 M(	dir_0.x,	-dir_1.x,
		dir_0.y,	-dir_1.y	
		);
	
	if(fabs(M.det())>0.000001f)
	{
		*t = M.X(Vector2(p1.x-p0.x,p1.y-p0.y));
		rta = TRUE;
	}
	
	return rta;
	
}

BOOL interseccion_2segmentos(Vector2 p0,Vector2 p1,Vector2 q0,Vector2 q1,
									Vector2 *Ip,BOOL extremos)
{
	BOOL rta = FALSE;
	// La recta p = p0+t*p
	Vector2 p = p1-p0;
	// La recta q = q0+k*q
	Vector2 q = q1-q0;

	float EPSILON_DBL = 0.000001f;
	Matrix2x2 M(p.x,-q.x,p.y,-q.y);
	if(fabs(M.det())>EPSILON_DBL)
	{
		float t = M.X(Vector2(q0.x-p0.x,q0.y-p0.y));
		float k = M.Y(Vector2(q0.x-p0.x,q0.y-p0.y));
		float ep = extremos?-EPSILON_DBL:EPSILON_DBL;
		// si incluye los extremos t,k E [0,1], si no debe ser t,k E (0,1)
		// con lo cual el ep es negativo si incluye los extremos 
		if(t>ep && t<1-ep && k>ep && k<1-ep)
		{
			*Ip = p0+p*t;
			rta = TRUE;
		}
	}
	
	return rta;
}

float interseccion_2segmentos(Vector2 p0,Vector2 p1,Vector2 q0,Vector2 q1,BOOL extremos)
{
	float rta = -1;
	// La recta p = p0+t*p
	Vector2 p = p1-p0;
	// La recta q = q0+k*q
	Vector2 q = q1-q0;

	float EPSILON_DBL = 0.000001f;
	Matrix2x2 M(p.x,-q.x,p.y,-q.y);
	if(fabs(M.det())>EPSILON_DBL)
	{
		float t = M.X(Vector2(q0.x-p0.x,q0.y-p0.y));
		float k = M.Y(Vector2(q0.x-p0.x,q0.y-p0.y));
		float ep = extremos?-EPSILON_DBL:EPSILON_DBL;
		// si incluye los extremos t,k E [0,1], si no debe ser t,k E (0,1)
		// con lo cual el ep es negativo si incluye los extremos 
		if(t>ep && t<1-ep && k>ep && k<1-ep)
		{
			//*Ip = p0+p*t;
			rta = t;
		}
	}
	
	return rta;
}


// esta el pto pt adentro del triangulo
BOOL pto_inside_tri(Vector2 pt,Vector2 p0,Vector2 p1,Vector2 p2)
{
	BOOL rta = FALSE;
	// verifico si el pto esta adentro del triangulo
	Vector2 B = barycentric(p0,p1,p2,pt);
	// Check if point is in triangle
	if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
		rta = TRUE;
	return rta;
}


// esta el pto pt adentro del poligono
BOOL pto_inside_poly(Vector2 pt,Vector2 P[])
{
	BOOL rta = FALSE;
	// verifico si el pto esta adentro del triangulo
	Vector2 B = barycentric(P[0],P[1],P[2],pt);
	// Check if point is in triangle
	if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
		rta = TRUE;
	else
	{
		// todavia puede esta adentro del otro tiangulo
		Vector2 B = barycentric(P[0],P[2],P[3],pt);
		// Check if point is in triangle
		if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
			rta = TRUE;
	}

	return rta;
}

// Devuelve t = - (n.p0 + D) / n.rd 
// representa la interseccion entre la recta R = p0 + t*rd y 
// el plano de normal N, N*(x,y,z) = -plano_d
// 
// o bien -1 si no hay interseccion
float interseccion_recta_plano(Vector3 p0,Vector3 rd,Vector3 N,float plano_D)
{
	float t = -1;
	float disc = N>>rd;
	if(fabs(disc)>0.0001f)
		// hay interseccion
		t = - ((N>>p0) + plano_D)/disc;
	return t;
}



// Clase helper de geometrias. 
CPerimetro::CPerimetro()
{
	cant_ptos = 0;
	H = 0;
}

CPerimetro::CPerimetro(Vector2 *P,int cant)
{

	cant_ptos = cant;
	memcpy(Pt,P,sizeof(Vector2)*cant);
	memset(idata,0,sizeof(idata));
	H = 0;
}

// cierra el perimetro
int CPerimetro::cerrar()
{
	if((Pt[cant_ptos-1]-Pt[0]).mod()>0.001f)
	{
		// la figura no era cerrada, la cierro
		Pt[cant_ptos] = Pt[0];
		idata[cant_ptos] = idata[0];
		++cant_ptos;
	}
	return cant_ptos;
}


void CPerimetro::add_pt(Vector2 p0,DWORD id)
{
	Pt[cant_ptos] = p0;
	idata[cant_ptos] = id;
	++cant_ptos;
}


// Devuelve la primer interseccion de la recta p0+k*v con el perimetro
int CPerimetro::interseccion(Vector2 p0,Vector2 v,Vector2 *Ip)
{
	// para simular una recta, calculo el punto p1, bien alejado de p0
	float min_dist = 100000;
	Vector2 p1 = p0 + v*100000;
	
	int rta = -1;
	float ep= 0.001f;
	int cant = 0;
	for(int t=0;t<cant_ptos-1;++t)
	{
		Vector2 pc;
		if(interseccion_2segmentos(p0,p1,Pt[t],Pt[t+1],&pc,FALSE))
		{
			float d = (pc - p0).mod();
			if(d<min_dist)
			{
				*Ip = pc;
				min_dist = d;
				rta = t;
			}
		}
	}
	
	return rta;

}


int CPerimetro::intersecciones(Vector2 p0,Vector2 p1,Vector2 *Ip)
{
	float ep= 0.001f;
	int cant = 0;
	for(int t=0;t<cant_ptos-1;++t)
	{
		Vector2 pc;
		if(interseccion_2segmentos(p0,p1,Pt[t],Pt[t+1],&pc,FALSE))
		{
			if(Ip)
				Ip[cant++] = pc;		// cuenta y almacena los ptos de interseccion
			else
				++cant;					// solo cuenta
		}
	}
	return cant;
}

BOOL CPerimetro::inside(Vector2 p0)
{
	//TODO: chequear que la direccion no sea muy parecida a ninguna otra
	// En teoria podria tomar una direccion arbitraria, pero para evitar que haya una interseccion infinita
	// (es decir que todo un segmento coincida), me conviene tomar una direccion unica que no se repita en todo 
	// el perimetro. Usualmente tenmos dir verticales, horizontales y a 45 grados
	int cant = intersecciones(p0,p0+Vector2(112320,51210));
	return cant%2==0?FALSE:TRUE;
}

BOOL CPerimetro::inside(CPerimetro *Q)
{
	// devuelve true si todos los puntos del perimetro Q estan dentro this
	// Q se supone cerrado 
	BOOL rta = TRUE;
	for(int i = 0;i<Q->cant_ptos-1 && rta; ++i)
		if(!inside(Q->Pt[i]))
			rta = FALSE;

	if(rta)
	{
		// verifico que cada segmento de Q no se salga de this
		for(int i = 0;i<Q->cant_ptos-1 && rta; ++i)
			if(intersecciones(Q->Pt[i],Q->Pt[i+1],NULL)!=0)
				rta = FALSE;
	}

	return rta;
}


BOOL CPerimetro::outside(CPerimetro *Q)
{
	// devuelve true si todos los puntos del perimetro Q estan fuera this
	// Q se supone cerrado 
	BOOL rta = TRUE;
	for(int i = 0;i<Q->cant_ptos-1 && rta; ++i)
		if(inside(Q->Pt[i]))
			rta = FALSE;
		
		if(rta)
		{
			// verifico que cada ningun de Q no se entra al this
			for(int i = 0;i<Q->cant_ptos-1 && rta; ++i)
				if(intersecciones(Q->Pt[i],Q->Pt[i+1],NULL)!=0)
					rta = FALSE;
		}
		
		return rta;
}

// triangulo i,i+1,i+2, es una oreja? 
// tiene que haber 2 lados afuera y uno adentro
BOOL CPerimetro::es_oreja(int i)
{
	// Caso general
	BOOL oreja = FALSE;
	Vector2 p0 = Pt[i];
	Vector2 p1 = Pt[i+2];
	// i) no tiene ninguna interseccion con el perimetro
	// (excluyendo las intersecciones con sigo mismo, 
	// (es decir hay que excluir las que dan p0, y p1
	BOOL hay_interseccion = FALSE;
	float ep = 0.01f;		// este epsilon esta en milimetros, no tiene que ser tan tan pequeño
	Vector2 pc;
	for(int t=0;t<cant_ptos-1 && !hay_interseccion;++t)
	{
		if(interseccion_2segmentos(p0,p1,Pt[t],Pt[t+1],&pc,TRUE))
			// hay interseccion, pero la excluyo si esta sobre p0 o p1
			if((pc-p0).mod()>ep && (pc-p1).mod()>ep)
				hay_interseccion = TRUE;
	}

	// ii) si no hay interseccion, para que sea una oreja
	// hay que verificar que el pto intermedio esta adentro del poligono
	if(!hay_interseccion)
		oreja = inside((p0+p1)*0.5);

	return oreja;
}


void CPerimetro::delete_pt(int i)
{
	// borra un nuevo pto de control en la pos i
	for(int t=i;t<cant_ptos;t++)
	{
		Pt[t] = Pt[t+1];
		idata[t] = idata[t+1];
	}

	cant_ptos--;
}


// Genera la triangulacion usando el metodo de las orejas
// devuelve la cantidad de puntos, cada 3 puntos forman un face (oreja) 
// OJO, esta rutina elimina el perimetro a la vez que devuelve su triangulacion
int CPerimetro::triangular(Vector2 Q[])
{
	int cant = 0;
	int t;
	while(cant_ptos>3)
	{
		BOOL oreja = FALSE;
		if(cant_ptos>4)
		{
			// busco la primer oreja (tiene que haber al menos una)
			for(t=0;t<cant_ptos-3 && !oreja;++t)
				oreja = es_oreja(t);
			
			if(!oreja)
			{
				for(t=0;t<cant_ptos-3 && !oreja;++t)
					oreja = es_oreja(t);
				break;
			}
		}
		else
		{
			oreja = TRUE;
			t = 1;
		}
		
		// meto la oreja
		if(oreja)
		{
			--t;		// reposiciono t
			
			if(oreja!=2)
			{
				Q[cant++] = Pt[t];
				Q[cant++] = Pt[t+1];
				Q[cant++] = Pt[t+2];
			}

			// elimino el vertice t+1
			delete_pt(t+1);
		}
	}

	return cant;
}


void CPerimetro::offset(float *ds)
{
	// Armo una estructura de segmentos
	Vector2 p0[1000];
	Vector2 dir[1000];
	for(int t=0;t<cant_ptos-1;++t)
	{
		// agrego el segmento t = desde t hasta el t+1
		p0[t] = Pt[t];
		dir[t] = Pt[t+1] - Pt[t];
		dir[t].normalizar();
	}

	// los muevo hacia la direccion adentro o afuera, segun el signo del offset
	float desp_offset = ds[0];		// offset 
	for(int t=0;t<cant_ptos-1;++t)
	{
		if(t)
		{
			// Valido el offset: si el angulo entre el segmento actual y el anterior
			// no es suf. grande, el offset tampoco puede variar mucho: 
			// eso se aplica a curvas...
			float cosa = dir[t] >> dir[t-1];
			if(cosa<0.9)
				// cosa==1 si en angulo entre los 2 vectores es cero, 
				// necesito que haya un cierto angulo para que tenga sentido el offset
				// como el angulo es suficiente, cambio el offset
				desp_offset = ds[t];
			// de lo contrario, sigue con el offset anterior, hasta que haya un cambio
			// un poco mas bruzco de direccion
		}

		Vector2 dir_a = dir[t].normal();
		p0[t] = p0[t] + dir_a*desp_offset;
	}

	for(int t=0;t<cant_ptos-2;++t)
		// La interseccion entre el segmento t y el t+1 se da en el punto t+1 del offset
		if(!interseccion_2rectas(p0[t],dir[t],p0[t+1],dir[t+1],&Pt[t+1]))
			// rectas paralelas? puede ser que el los segmentos esten contiguos;
			// x----x----x
			Pt[t+1] = p0[t+1];

	// nos falta la interseccion entre el ultimo segmento y el primero
	if(!interseccion_2rectas(p0[cant_ptos-2],dir[cant_ptos-2],p0[0],dir[0],&Pt[0]))
		Pt[0] = p0[0];

	// y como la figura es cerrada, el ultimo punto es = al primero
	Pt[cant_ptos-1] = Pt[0];

}


void CPerimetro::explotar(float *r,float da)
{
	CPerimetro Q;
	for(int i=0;i<cant_ptos-1;++i)
	{
		// Agrego el punto
		Q.add_pt(Pt[i],idata[i]);
		// si tiene radio agrego el arco
		if(r[i]>1 )
		{
			// frente curvo 
			Vector2 p0 = Pt[i];
			Vector2 p1 = Pt[i+1];
			Vector2 N = (p1-p0).normal();
			N.normalizar();
			Vector2 p2 = (p0 + p1)*0.5 + N*r[i];
			Vector2 aux[100];
			int cant = arc3p(p0,p1,p2,aux,da);
			for(int t=0;t<cant;++t)
				Q.add_pt(aux[t],idata[i]);
		}
	}
	Q.cerrar();
	*this = Q;
}

// helper, devuelve true si los puntos p0,p1, y p2 estan sobre la misma linea
BOOL colinales(Vector2 p0,Vector2 p1,Vector2 p2,float ep)
{
	// calculo el area del triangulo p0,p1,p2
	float A = p0.x * (p1.y - p2.y) + p1.x * (p2.y - p0.y) + p2.x * (p0.y - p1.y);
	BOOL rta;
	if(fabs(A)<ep)
		rta = TRUE;
	else
		rta = FALSE;

	return rta;
	//return A<ep?TRUE:FALSE;

}

// Operacion de restar perimetros
// Luego de substraer el perimetro Q, this queda sin hueco interno, y el perimetro original es 
// igual a {this} UNION {resto}, que es el perimetro que devuelve la funcion 
CPerimetro *CPerimetro::substract(CPerimetro *Q)
{
	// i- Q esta afuera de this, no tengo lo que hacer
	if(outside(Q))
		return NULL;

	// ii- Q esta totalmente incluido en this
	// +---------------+
	// |               |
	// |   q2    q3    |
	// |    +-----+    |
	// |    |     |    |
	// |    |     |    |
	// |    +-----+    |
	// |   q1    q0    |
	// |               |
	// +---------------+
	if(inside(Q))
		return substractInside(Q);


	// caso particular esquina (el primer punto esta adentro del perimetro)
	if(Q->inside(Pt[0]))
		return substractEsquina(Q);



	//iii- Q tiene una parte adentro y otra afuera
	// es decir tiene toca contra el borde
	return substractBorde(Q);

}




CPerimetro *CPerimetro::substractInside(CPerimetro *Q)
{
	// Para la resta de poligonos, se supone que 
	// i) el perimetro Q esta sentido contrario al this
	// ii) el perimetro Q esta totalmente incluido en this
	
	// 0                 3
	// +-----------------+
	// |                 |
	// |                 |
	// |                 |
	// |   q2    q3      |
	// |    +-----+      |
	// |    |     |      |
	// |    |     |      |
	// |    +-----+      |
	// |   q1    q0      |
	// +----Y-----X------+
	// 1   py     px      2
	// s1                s1+1

	
	// Este metodo divide el perimetro en 2 perimetros, es amigable con la triangulacion.

	// el perimetro original 
	
	// 0                 3
	// +-----------------+
	// |                 |
	// |                 |
	// |                 |
	// |   q2    q3      |
	// |    +-----+      |
	// |    |     |      |
	// |    |     |      |
	// |    |     |      |
	// |    |            |
	// +----Y     X------+
	// 1   py     px      2
	// s1                s1+1
	

	// y el resto:
	//     q1     q0
	//      +-----+       
	//      |     |
	//      Y-----X       
	//     py     px     
	// 

	// si bien la rutina es general, en la practica se usa en el contexto de mesadas con bachas.
	// en ese caso, conviene que el perimetro que resto, que corresponde a la bacha, este generado de 
	// tal forma que el resto quede adelante de la mesada. Eso es para que luego el resto no se vuelva a procesar
	// aun cuando el algortimo se lo banca, pero es menos sencible a errores de redondeo si el resto es mas pequeño


	CPerimetro *R = NULL;		// Resto

	// calculo n, la normal al primer punto de Q 
	// n apunta para afuera de Q hacia el limite de this
	Vector2 n = Q->Pt[1] - Q->Pt[0];
	n.normalizar();
	n.rotar(-M_PI_2);
	
	// con n, genero un recta y calculo la interseccion con el this
	Vector2 px,py;
	int s1;
	if( (s1=interseccion(Q->Pt[0],n,&px))!=-1 && interseccion(Q->Pt[1],n,&py)==s1) 
	{
	
		// Genero el perimetro Resto de la operacion
		R = new CPerimetro();
		R->H = H;
		R->add_pt(Q->Pt[0],Q->idata[0]);
		R->add_pt(Q->Pt[1],Q->idata[1]);
		R->add_pt(py,9999);
		R->add_pt(px,9999);
		R->cerrar();


		// La recta s1 , s1+1, la voy a reemplazar por 
		// s1 , Y, q2, q3, ... qn-1 ,X , s1+1
		// Agrego q = {Y, q2,q3, ... qn-1, X} a continuacion de s1

		// Verifico si los puntos  SY = {q2,q1,Y} son colinales
		// idem con SX = {q3,q0,X}

		// |   q2    q3      |
		// |    +-----+      |
		// |    |     |      |
		// |    |     |      |
		// |    +-----+      |
		// |   q1    q0      |
		// |    |     |      |
		// |    |     |      |
		// +----Y-----X------+

		// nota: en una bacha de 4 puntos, como es cerrada Q->cant_ptos = 5
		// Q->Pt[4] = Q->Pt[0]
		// Sea la cantidad real de puntos: 
		int cant = Q->cant_ptos - 1;
		// el punto 3 del grafico seria Q->Pt[cant-1]
		BOOL sy_colineal = colinales(Q->Pt[2],Q->Pt[1],py,5);
		BOOL sx_colineal = colinales(Q->Pt[cant-1],Q->Pt[0],px,5);

		// si alguno de los segmentos de salida SX o SY no son colineales,
		// tengo que agregar mas puntos
		if(!sx_colineal)
			cant++;
		if(!sy_colineal)
			cant++;
		// Ejemplo

		// |   q2       q3   |
		// |    +--------+   |
		// |    |       /    |
		// |    |      /     |
		// |    +-----+      |
		// |   q1    q0      |
		// |    |     |      |
		// |    |     |      |
		// +----Y-----X------+

		// de un lado puede terminar {q2,Y}
		// pero del otro precisa {q3,q0,X} 


		for(int i=cant_ptos-1;i>s1;--i)
		{
			Pt[i+cant] = Pt[i];
			idata[i+cant] = idata[i];

		}

		// Primero va el punto Y 
		int t = s1+1;
		Pt[t] = py;
		idata[t] = 9999;
		t++;
		if(!sy_colineal)
		{
			// si no son colineales agrego el punto q1
			Pt[t] = Q->Pt[1];
			idata[t] = Q->idata[1];
			t++;
		}
		
		// Ahora viene la secuencia pp dicha: {q2,q3...qn-1}
		for(int i=2;i<Q->cant_ptos-1;++i)
		{
			Pt[t] = Q->Pt[i];
			idata[t] = Q->idata[i];
			t++;
		}

		// Y el fin de la secuencia SX hacia el punto X
		if(!sx_colineal)
		{
			// si no son colineales agrego el punto q0
			Pt[t] = Q->Pt[0];
			idata[t] = Q->idata[0];
			t++;
		}

		// finalmente el punto X
		Pt[t] = px;
		idata[t] = 9999;

		cant_ptos+=cant;
	}
	else
	{
		// Error, la salida no es limpia
		int s1 = interseccion(Q->Pt[0],n,&px);
		int s2 = interseccion(Q->Pt[1],n,&py);
		int error = 1;

	}

	return R;
}

CPerimetro *CPerimetro::substractEsquina(CPerimetro *Q)
{
	// caso particular. El cero de this esta adentro de Q 
	// de momento Q tiene solo 4 puntos 


	//   q0      q1
	//   +-------+
	//   |       |
	//   |   0   |              3
	//   |   +---Y-------------+
	//   |   |   |             |
	//   +---X---+             |
	//   q3  |   q2            |
	//       |                 |
	//       |                 |
	//       +-----------------+
	//         1                 2


	Vector2 X;
	Vector2 dir_s = Pt[1] - Pt[0];
	dir_s.normalizar();
	Q->interseccion(Pt[0],dir_s,&X);

	Vector2 Y;
	Vector2 dir_e = Pt[cant_ptos-2] - Pt[0];
	dir_e.normalizar();
	Q->interseccion(Pt[0],dir_e,&Y);

	Pt[0] = X;
	Pt[cant_ptos-1] = Y;
	add_pt(Q->Pt[2]);
	cerrar();

	return NULL;
}



CPerimetro *CPerimetro::substractBorde(CPerimetro *Q)
{
	// Para la resta de poligonos, se supone que 
	// i) el perimetro Q esta sentido contrario al this
	// ii) el perimetro Q esta parcialmente incluido en this

	
	// 0                 3
	// +-----------------+
	// |                 |
	// |   e1     s0     |
	// |    +-----+      |
	// |    |     |      |
	// +----Y-----X------+
	// 1    |     |      2
	// te   |     |       
	//      +-----+       
	//     e0     s1
	

	// Busco el primer punto que traspase el perimetro this desde afuera 
	// hacia adentro, ese va a ser el camino de entrada E = {e0,e1}
	BOOL hay_entrada = FALSE;
	int e;
	// tiene que ser que pt[i] outside y pt[i+1] insdide
	int i =0;
	for(i=0;i<Q->cant_ptos-1 && !hay_entrada;++i)
		if(!inside(Q->Pt[i]) && inside(Q->Pt[i+1]))
		{
			hay_entrada = TRUE;
			e = i;
		}

	if(!hay_entrada)
		return NULL;		// Error

	// Voy armando el resultado en un perimetro auxiliar
	CPerimetro M;
	M.H = H;

	// Caclculo el punto de entrada Y
	Vector2 Y;
	Vector2 dir_e = Q->Pt[e+1] - Q->Pt[e];
	dir_e.normalizar();
	int te = interseccion(Q->Pt[e],dir_e,&Y);

	// Ahora voy a insetar en M todos los puntos de this hasta llegar a te inclusive
	for(i=0;i<=te;++i)
		M.add_pt(Pt[i],idata[i]);

	// Path interno = { Y,e1,.....s0.X} 
	M.add_pt(Y);

	// busco el camino de salida S = {s0,s1} a la vez que voy complentando el path R
	// paso al siguiente
	i = e+1;
	BOOL hay_salida = FALSE;
	int s;
	while(i<Q->cant_ptos-1 && !hay_salida)
		if(inside(Q->Pt[i]) && !inside(Q->Pt[i+1]))
		{
			hay_salida = TRUE;
			s = i;
		}
		else
		{
			// meto el punto en el path R y sigo buscando la salida
			M.add_pt(Q->Pt[i]);
			++i;
		}
		
	if(!hay_salida)
		return NULL;		// Error

	// Caclculo el punto de salida X
	Vector2 X;
	Vector2 dir_s = Q->Pt[s+1] - Q->Pt[s];
	dir_s.normalizar();
	int ts = interseccion(Q->Pt[s],dir_s,&X);
	// completo el path con la salida
	M.add_pt(Q->Pt[s]);
	M.add_pt(X);

	// completo con el resto de los puntos de this, desde ts+1 inclusive hacia el fin
	for(i=ts+1;i<cant_ptos;++i)
		M.add_pt(Pt[i],idata[i]);

	// piso el perimetro
	*this = M;

	// y devuelvo NULL pues no hay resto en este caso
	return NULL;	
}


	
	
// que no necesarimanete pertenece a la curva
int arc3p(Vector2 p0,Vector2 p1,Vector2 p2,Vector2 *Pt,float da)
{
	// Calculo la posicion del centro
	// Projecto la pos. p2 sobre la recta de simetria
	Vector2 n = (p1-p0).normal();
	n.normalizar();
	// pm = proyeccion del pto p2 sobre la recta n
	Vector2 O = (p0+p1)*0.5;
	Vector2 pm = O + n*(n>>(p2-O));
	Vector2 pc;
	
	// d = curvatura del arco = distancia desde el punto p1, hasta la linea que une los ptos p0,p2
	// cuando es cero los ptos p0,p1,y p2 estan alineados en la misma recta, la distancia d esta en mm
	float d = n>>(pm-O);
	if(fabs(d)<=0.1 || (p0-p1).mod()<1)
		return 0;		// no tiene puntos intermedios

	int i = 0;
	// Caso general: 3darc
	float a = (p1-p0).mod()/2;
	float b = (a*a - d*d)/(2*d);
	pc = O - n*b;
	
	BOOL inverso = FALSE;
	if(d>0)
	{
		Vector2 paux = p0;
		p0 = p1;
		p1 = paux;
		d = -d;
		inverso = TRUE;
	}
	
	float radio = (p1-pc).mod();
	float alfa_0 = (p0-pc).angulo();
	float alfa_1 = (p1-pc).angulo();
	if(alfa_0>alfa_1)
		alfa_1+=2*M_PI;
	
	if(inverso)
	{
		// curva negativa
		float alfa = alfa_1;
		while(alfa>alfa_0+2*da)
		{
			alfa-=da;
			Pt[i++] = pc + Vector2(cos(alfa),sin(alfa))*radio;
		}
	}
	else
	{
		// curva positiva
		float alfa = alfa_0;
		while(alfa<alfa_1-2*da)
		{
			alfa+=da;
			Pt[i++] = pc + Vector2(cos(alfa),sin(alfa))*radio;
		}
	}

	// devuelve la cantidad de puntos generados
	return i;
}



Vector3 ComputeCuadraticBezier(Vector3 P0,Vector3 P1,Vector3 P2,float t)
{
	Vector3 Q0 = P0 *(1-t) + P1*t;
	Vector3 Q1 = P1 *(1-t) + P2*t;
	Vector3 Bt = Q0 *(1-t) + Q1*t;
	return Bt;
}
