

#include "glew.h"
#include <gl/GL.h>
#include "math.h"


#pragma once


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
	void loadShaders(char *vs,char *fs,GLhandleARB *vs_main,GLhandleARB *fs_main,GLhandleARB *shader_prog);

	void initFonts();
	void renderText(int px, int py,char *text);

	HDC m_hDC;
	HGLRC m_hglContext;
	CTexture tex;
	float fps;
	float time;
	float elapsed_time;

	vec3 lookFrom;
	vec3 viewDir;
	vec3 U,V;
	float voxel_step0;
	float voxel_step;

	float vel_tras;
	int game_status;
	float timer_catch;
	int cant_capturados;

	GLint fbWidth;
	GLint fbHeight;


	GLhandleARB vs_main,fs_main,shader_prog;
	GLhandleARB vs2_main,fs2_main,shader_prog2;

	// fonts
	st_glyph glyphs[255];


};

extern char *textFileRead(char *fn);


