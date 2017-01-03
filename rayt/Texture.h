#pragma once

#include "math.h"

#define MAX_MIP_MAP			16

struct st_rgba
{
	BYTE r,g,b,a;
};

struct st_surface
{
	int width,height;
	st_rgba *p_data;
};

class CTexture
{
public:
	int width,height;
	st_surface mipmap[MAX_MIP_MAP];
	st_surface sat;
	int cant_mipmap;
	char fname[MAX_PATH];			// debug

	CTexture();
	~CTexture();

	bool CreateFromFile(const char *file_name);

	//vec4 tex2d(float tu, float tv);
	vec4 tex2Dlod(float tu, float tv,int level=0);
	vec4 tex2Dgrad(float tu, float tv,float ddx,float ddy);
	//test
	void Draw(CDC *pDC, int l,int x,int y,int dx,int dy,float k);
	void DrawSurface(CDC *pDC, int l, int x,int y,int k);

};

void extension(char *file,char *ext);
