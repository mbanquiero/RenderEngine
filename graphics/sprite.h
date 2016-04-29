#pragma once
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"

class CRenderEngine;
class CDevice;

#define MAX_BATCHED_SPRITES		256

struct sprite_batch
{
	CTexture *p_texture;
	CRect srcRect;
	CRect destRect;
	bool text;
	D3DXCOLOR color;
};

struct QUAD_VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 texture;
};

class CSprite
{
public:

	CDevice *device;

	int screenWidth, screenHeight;
	int m_vertexCount, m_indexCount;
	int cant_sprites;
	sprite_batch sprite[MAX_BATCHED_SPRITES];

	LPDIRECT3DVERTEXBUFFER9 m_vertexBuffer;	
	LPDIRECT3DINDEXBUFFER9 m_indexBuffer;	

	CSprite();
	virtual ~CSprite();
	virtual bool Create(CDevice *p_device);
	virtual void Release();

	virtual bool UpdateQuadPosition(int i);
	virtual void SetTextColor(D3DXCOLOR text_color);
	virtual bool Begin();
	virtual bool Flush();
	virtual bool End();

	virtual bool Draw(CTexture *p_texture,CRect *srcRect = NULL,CRect *destRect = NULL,bool text=false,
		D3DXCOLOR text_color=D3DXCOLOR(0,0,0,0));

};


