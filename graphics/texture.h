#pragma once
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"


class CTexture
{
	public:
		int imgWidth , imgHeight;
		char name[MAX_PATH];
		LPDIRECT3DTEXTURE9      g_pTexture;
		CTexture();
		~CTexture();
		virtual void Release();
};


