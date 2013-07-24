#include "stdafx.h"
#include "/dev/graphics/texture.h"


#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

CBaseTexture::CBaseTexture()
{
	strcpy(name, "");
}


CBaseTexture::~CBaseTexture()
{
	Release();
}

void CBaseTexture::Release()
{
	strcpy(name,"");
}

// ------------------------------------------------------------------------------------------------
CDX11Texture::CDX11Texture() : CBaseTexture()
{
	resourceView = NULL;
}


CDX11Texture::~CDX11Texture()
{
	Release();
}

void CDX11Texture::Release()
{
	CBaseTexture::Release();
	SAFE_RELEASE(resourceView);
}


// ------------------------------------------------------------------------------------------------
CDX9Texture::CDX9Texture() : CBaseTexture()
{
	g_pTexture = NULL;
}


CDX9Texture::~CDX9Texture()
{
	Release();
}

void CDX9Texture::Release()
{
	CBaseTexture::Release();
	SAFE_RELEASE(g_pTexture);
}
