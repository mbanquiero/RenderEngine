#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/Sprite.h"


CSprite::CSprite()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	cant_sprites = 0;
}

CSprite::~CSprite()
{
	Release();
}

void CSprite::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}


// Agrega el sprite a la lista de batches sprites
bool CSprite::Draw(CTexture *p_texture,CRect *srcRect,CRect *destRect,bool text,D3DXCOLOR text_color)
{
	if(cant_sprites>= MAX_BATCHED_SPRITES)
		return false;

	sprite[cant_sprites].p_texture = p_texture;
	sprite[cant_sprites].text = text;
	sprite[cant_sprites].color = text_color;

	if(srcRect!=NULL)
	{
		sprite[cant_sprites].srcRect = *srcRect;
	}
	else
	{
		// tomo toda la pantalla
		sprite[cant_sprites].srcRect.left = 0;
		sprite[cant_sprites].srcRect.right = screenWidth;
		sprite[cant_sprites].srcRect.top = 0;
		sprite[cant_sprites].srcRect.bottom = screenHeight;
	}

	if(destRect!=NULL)
	{
		sprite[cant_sprites].destRect = destRect;
	}
	else
	{
		// toma toda la pantalla
		sprite[cant_sprites].destRect.left = 0;
		sprite[cant_sprites].destRect.right = screenWidth;
		sprite[cant_sprites].destRect.top = 0;
		sprite[cant_sprites].destRect.bottom = screenHeight;
	}
	++cant_sprites;

	return true;
}

bool CSprite::End()
{
	bool rta = Flush();
	device->SetZEnabled(true);
	device->SetAlphaBlendEnabled(false);
	return rta;
}



bool CSprite::Create(CDevice *p_device)
{
	device = p_device;
	screenWidth = p_device->screenWidth;
	screenHeight = p_device->screenHeight;

	// Creo el vertex e index buffer
	QUAD_VERTEX * vertices;
	DWORD * indices;
	HRESULT result;
	int i;

	// genero un Quad con 2 triangulos (6 vertices)
	vertices = new QUAD_VERTEX[m_indexCount = m_vertexCount = 6];
	if(!vertices)
	{
		return false;
	}

	indices = new DWORD[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// indice trivial
	memset(vertices, 0, (sizeof(QUAD_VERTEX) * m_vertexCount));
	for(i=0; i<m_indexCount; i++)
	{
		indices[i] = i;
	}


	// create the vertex buffer
	UINT size = sizeof(QUAD_VERTEX) * m_vertexCount;
	if( FAILED( p_device->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	QUAD_VERTEX *p_gpu_vb;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&p_gpu_vb, 0 ) ) )
		return false;
	memcpy(p_gpu_vb,vertices,size);
	m_vertexBuffer->Unlock();

	// Index buffer
	size = sizeof(unsigned long) * m_indexCount;
	if( FAILED( p_device->g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
		return false;

	unsigned long *p_gpu_ib;
	if( FAILED( m_indexBuffer->Lock( 0, size, (void**)&p_gpu_ib, 0 ) ) )
		return false;
	memcpy(p_gpu_ib,indices,size);
	m_indexBuffer->Unlock();


	delete [] vertices;
	delete [] indices;

	return true;
}


bool CSprite::UpdateQuadPosition(int i)
{
	float left, right, top, bottom;				// positions
	float tleft, tright, ttop, tbottom;			// tex coords
	QUAD_VERTEX *vertices;
	QUAD_VERTEX *verticesPtr;
	HRESULT result;

	// paso a screen space (-1,1)
	left = 2 * (float)sprite[i].destRect.left / (float)screenWidth - 1;
	right = 2 * (float)sprite[i].destRect.right / (float)screenWidth - 1;
	top = 1- 2 * (float)sprite[i].destRect.top / (float)screenHeight ;
	bottom = 1 - 2 * (float)sprite[i].destRect.bottom / (float)screenHeight;

	// paso a coords de textura (0,1)
	float imgWidth = sprite[i].p_texture->imgWidth;
	float imgHeight = sprite[i].p_texture->imgHeight;
	tleft = (float)sprite[i].srcRect.left / (float)imgWidth;
	tright = (float)sprite[i].srcRect.right / (float)imgWidth;
	ttop = (float)sprite[i].srcRect.top / (float)imgHeight;
	tbottom = (float)sprite[i].srcRect.bottom / (float)imgHeight;

	// Create the vertex array.
	vertices = new QUAD_VERTEX[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// First triangle.
	vertices[0].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[0].texture = D3DXVECTOR2(tleft, ttop);

	vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = D3DXVECTOR2(tright, tbottom);

	vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = D3DXVECTOR2(tleft, tbottom);

	// Second triangle.
	vertices[3].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[3].texture = D3DXVECTOR2(tleft, ttop);

	vertices[4].position = D3DXVECTOR3(right, top, 0.0f);  // Top right.
	vertices[4].texture = D3DXVECTOR2(tright, ttop);

	vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = D3DXVECTOR2(tright, tbottom);

	if( FAILED( m_vertexBuffer->Lock( 0, m_vertexCount*sizeof(QUAD_VERTEX), (void**)&verticesPtr, 0 ) ) )
		return false;
	memcpy(verticesPtr,vertices,m_vertexCount*sizeof(QUAD_VERTEX));
	m_vertexBuffer->Unlock();

	delete [] vertices;
	vertices = 0;

	return true;
}

bool CSprite::Begin()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = device->g_pd3dDevice;
	HRESULT hr;
	// Seteo el index y el vertex buffer
	hr = g_pd3dDevice->SetStreamSource( 0, m_vertexBuffer, 0, sizeof(QUAD_VERTEX));
	hr = g_pd3dDevice->SetIndices(m_indexBuffer);

	// Vertex Declaration
	hr = g_pd3dDevice->SetVertexDeclaration(device->m_pSpriteVertexDeclaration);

	// Desactivo el zbuffer 
	device->SetZEnabled(false);

	// habilito el alpha blend
	device->SetAlphaBlendEnabled(true);

	// Reseteo la cantidad de batches sprites
	cant_sprites = 0;

	return true;
}


void CSprite::SetTextColor(D3DXCOLOR text_color)
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = device->g_pd3dDevice;
	device->g_pEffectStandard->SetValue("m_FontColor",&text_color,sizeof(D3DXCOLOR));
}


bool CSprite::Flush()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = device->g_pd3dDevice;
	device->g_pEffect = device->g_pEffectStandard;

	// Recorro la lista de batches sprites
	for(int i=0 ;i<cant_sprites ;++i)
	{
		// setel el pixel shader, o bien para dibujar sprites pp dichos o bien para dibujar font
		if(sprite[i].text)
		{
			// pixel shader para dibujar textos
			device->g_pEffectStandard->SetTechnique("RenderText");
			// Seteo el color del texto 
			SetTextColor(sprite[i].color);
		}
		else
		{
			// pixel shader para dibujar imagenes
			device->g_pEffectStandard->SetTechnique("RenderSprite");
		}

		// Actualizo la posicion del sprite en la pantalla
		UpdateQuadPosition(i);

		// Set shader texture resource in the pixel shader.
		device->g_pEffect->SetTexture("g_Texture", sprite[i].p_texture->g_pTexture);

		// por fin dibujo el quad pp dicho
		UINT cPasses;
		device->g_pEffect->Begin(&cPasses, 0);
		device->g_pEffect->BeginPass(0);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,6,0,2);
		device->g_pEffect->EndPass();
		device->g_pEffect->End();
	}

	return true;
}


