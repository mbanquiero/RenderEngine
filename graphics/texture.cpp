#include "stdafx.h"
#include "/dev/graphics/texture.h"


#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

CTexture::CTexture()
{
	imgWidth = imgHeight = 0;
	g_pTexture = NULL;
	strcpy(name, "");
}


CTexture::~CTexture()
{
	Release();
}

void CTexture::Release()
{
	strcpy(name,"");
	SAFE_RELEASE(g_pTexture);
}

