

#include "glew.h"
#include <gl/GL.h>
#include "math.h"


#pragma once

extern bool distance_map;


class CTexture
{
public:
	CTexture();
	virtual ~CTexture();
	bool CreateFromFile(LPCTSTR lpDataFile_i, int nWidth_i, int nHeight_i, int nSlices_i );
	bool CreateFromTest(int n,int nWidth_i, int nHeight_i, int nSlices_i );
	void Box(BYTE *buff, int x0,int y0,int z0,int x1,int y1,int z1);

	int dx , dy, dz , id;
};


struct st_glyph
{
	POINT *lpPt;
	BYTE  *lpB;
	int nNumPoints;
};


class CRenderEngine
{
public:
	CRenderEngine(void);
	virtual ~CRenderEngine(void);

	bool Initialize( HDC hContext_i);
	void Resize( int nWidth_i, int nHeight_i );
	void Render();
	void RayCasting();
	void TextureVR();
	void Release();
	void setShaders();
	void initFonts();
	void renderText(int px, int py,char *text);

	bool ray_casting;
	HDC m_hDC;
	HGLRC m_hglContext;
	float dist;
	float escale;
	float an_x,an_y,an_z;			// rotacion = angulos de euler
	CTexture tex[4];
	float fps;
	float time;
	float elapsed_time;

	vec3 lookFrom;
	vec3 lookAt;
	vec3 viewDir;
	float voxel_opacity;
	float voxel_step0;
	float voxel_step;

	GLhandleARB vs_main,fs_main,shader_prog;

	// fonts
	st_glyph glyphs[255];


};

extern char *textFileRead(char *fn);


