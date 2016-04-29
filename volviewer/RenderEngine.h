

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
	void Release();
	void setShaders();
	void initFonts();
	void renderText(int px, int py,char *text);

	HDC m_hDC;
	HGLRC m_hglContext;
	float dist;
	float escale;
	float an_x,an_y,an_z;			// rotacion = angulos de euler
	CTexture tex;
	float fps;
	float time;
	float elapsed_time;

	GLhandleARB vs_main,fs_main,shader_prog;

	// fonts
	st_glyph glyphs[255];


};

extern char *textFileRead(char *fn);


