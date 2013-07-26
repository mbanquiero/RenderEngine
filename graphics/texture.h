#pragma once
#include <d3d11.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"


class CBaseTexture
{
	public:
		char name[MAX_PATH];
		CBaseTexture();
		~CBaseTexture();
		virtual void Release();
};



class CDX11Texture : public CBaseTexture
{
public:
	ID3D11ShaderResourceView *resourceView;

	CDX11Texture();
	~CDX11Texture();
	virtual void Release();
};


class CDX9Texture : public CBaseTexture
{
public:
	LPDIRECT3DTEXTURE9      g_pTexture;
	CDX9Texture();
	~CDX9Texture();
	virtual void Release();
};



