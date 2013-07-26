#include "stdafx.h"
#include "/dev/graphics/dx11device.h"
#include <d3dcompiler.h>
#include "/dev/graphics/renderengine.h"
#include "/dev/graphics/xstring.h"

bool IsSkeletalMesh(char *fname);

CDX11Device::CDX11Device()
{
	dev = NULL;
}

CDX11Device::~CDX11Device()
{
	if(dev)
		CleanD3D();
}



// Inicializa el DirectX
void  CDX11Device::InitD3D(HWND hWnd)
{
	HRESULT hr;
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);


	// Preparo el back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	dev->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);

	D3D11_TEXTURE2D_DESC desc;
	pBackBuffer->GetDesc(&desc);
	pBackBuffer->Release();
	screenWidth = desc.Width;
	screenHeight = desc.Height;
	pBackBuffer->Release();

	// El zbuffer 
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = scd.SampleDesc.Count;		// OJO QUE SI NO ES EL MISMO NO CAMINA
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	hr = dev->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);

	// Stencil buffer
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create the depth stencil state.
	hr = dev->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	devcon->OMSetDepthStencilState(m_depthStencilState, 1);
	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = dev->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);

	// Ahora si, Pongo el back buffer como render target
	devcon->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	dev->CreateRasterizerState(&rasterDesc, &m_rasterState);
	devcon->RSSetState(m_rasterState);

	// Inicializo el Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = screenWidth;
	viewport.Height = screenHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	devcon->RSSetViewports(1, &viewport);

	// transformation pipeline
	fov = (float)D3DX_PI / 4.0f;
	aspect_ratio = (float)screenWidth / (float)screenHeight;
	near_plane = 50;
	far_plane = 50000;

	// los shaders de direct11 piden que las matrices vayan transpuestas
	transponer_matrices = true;
}

void CDX11Device::CleanD3D()
{
	SAFE_RELEASE(m_sampleState);
	SAFE_RELEASE(m_matrixBuffer);
	SAFE_RELEASE(m_lightBuffer);
	SAFE_RELEASE(m_boneBuffer);
	SAFE_RELEASE(pVS);
	SAFE_RELEASE(pPS);
	SAFE_RELEASE(pSkeletalMeshVS);
	SAFE_RELEASE(pSkeletalMeshPS);
	SAFE_RELEASE(swapchain);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_depthStencilState);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_rasterState);
	SAFE_RELEASE(dev);
	SAFE_RELEASE(devcon);
}



void CDX11Device::InitPipeline()
{
	HRESULT hr;
	// load and compile the two shaders
	ID3D10Blob *VS,*SkeletalMeshVS,*PS,*SkeletalMeshPS ;
	ID3D10Blob* errorMessage;

	// Vertesh shader comun y corriente
	if(FAILED(D3DX11CompileFromFile("/dev/graphics/shaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, &errorMessage, 0)))
	{
		AfxMessageBox((char *)errorMessage->GetBufferPointer());
		SAFE_RELEASE(errorMessage);
	}

	// Vertesh shader skeletal mesh
	if(FAILED(D3DX11CompileFromFile("/dev/graphics/shaders.hlsl", 0, 0, "SkeletalVShader", "vs_5_0", 0, 0, 0, &SkeletalMeshVS, &errorMessage, 0)))
	{
		AfxMessageBox((char *)errorMessage->GetBufferPointer());
		SAFE_RELEASE(errorMessage);
	}

	// Pixel shader comun y corriente
	DWORD dwShaderFlags = 0;
	if(FAILED(D3DX11CompileFromFile("/dev/graphics/shaders.hlsl", NULL, NULL, "PShader", "ps_5_0", 
		dwShaderFlags, 0, NULL, &PS, &errorMessage, NULL)))
	{
		AfxMessageBox((char *)errorMessage->GetBufferPointer());
		SAFE_RELEASE(errorMessage);
	}
	// Pixel shader skeletal mesh
	if(FAILED(D3DX11CompileFromFile("/dev/graphics/shaders.hlsl", NULL, NULL, "SkeletalPShader", "ps_5_0", 
		dwShaderFlags, 0, NULL, &SkeletalMeshPS, &errorMessage, NULL)))
	{
		AfxMessageBox((char *)errorMessage->GetBufferPointer());
		SAFE_RELEASE(errorMessage);
	}


	// encapsulate both shaders into shader objects
	hr = dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	hr = dev->CreateVertexShader(SkeletalMeshVS->GetBufferPointer(), SkeletalMeshVS->GetBufferSize(), NULL, &pSkeletalMeshVS);
	hr = dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
	hr = dev->CreatePixelShader(SkeletalMeshPS->GetBufferPointer(), SkeletalMeshPS->GetBufferSize(), NULL, &pSkeletalMeshPS);

	// set the shader objects
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	int numElements = sizeof(ied) / sizeof(ied[0]);
	hr = dev->CreateInputLayout(ied, numElements, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	
	D3D11_INPUT_ELEMENT_DESC ied2[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	numElements = sizeof(ied2) / sizeof(ied2[0]);
	hr = dev->CreateInputLayout(ied2, numElements, SkeletalMeshVS->GetBufferPointer(), SkeletalMeshVS->GetBufferSize(), &pLayoutSkeletalMesh);


	// Constant buffer de matrices
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.ByteWidth = sizeof(MatrixBufferType);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	dev->CreateBuffer(&BufferDesc, NULL, &m_matrixBuffer);

	// Constant buffer de lighting
	BufferDesc.ByteWidth = sizeof(LightBufferType);
	dev->CreateBuffer(&BufferDesc, NULL, &m_lightBuffer);

	// Constant buffer matrices de animacion
	BufferDesc.ByteWidth = sizeof(boneBufferType);
	hr = dev->CreateBuffer(&BufferDesc, NULL, &m_boneBuffer);

	// Sampler de textura
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Create the texture sampler state.
	dev->CreateSamplerState(&samplerDesc, &m_sampleState);

}


void CDX11Device::InitTransformPipeline()
{
	// Matrices de Transformacion
	D3DXMatrixPerspectiveFovLH(&m_Proj, fov, aspect_ratio, near_plane, far_plane);
	D3DXMatrixIdentity(&m_World);
	// ojo, las matrices en el shader van transpuestas!! 
	D3DXMatrixTranspose(&m_World, &m_World);
	D3DXMatrixTranspose(&m_Proj, &m_Proj);
}



void CDX11Device::BeginRenderFrame(D3DXMATRIX matView)
{
	// Limpio la pantalla y el depth buffer
	devcon->ClearRenderTargetView(m_renderTargetView, D3DXCOLOR(0.94f, 0.94f, 0.94f, 1));
	devcon->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	// Actualizo la matriz de view
	m_View = matView;
	// ojo que en el shader la matriz va transpuesta
	D3DXMatrixTranspose(&m_View, &m_View);

	// Set the sampler state in the pixel shader.
	devcon->PSSetSamplers(0, 1, &m_sampleState);

	// Constant buffer de la luz
	SetShaderLighting();


}

void CDX11Device::EndRenderFrame()
{
	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);

}


void CDX11Device::SetShaderTransform(D3DXVECTOR3 lookFrom)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	// Lock the constant buffer so it can be written to.
	devcon->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->world = m_World;
	dataPtr->view = m_View;
	dataPtr->projection = m_Proj;

	// Transpuesta inversa del world, (para transformar direcciones)		
	D3DXMATRIXA16 m_TransposeInvWorld, matAux;
	FLOAT det;
	D3DXMatrixTranspose(&matAux,&m_World);
	D3DXMatrixInverse(&m_TransposeInvWorld,&det,&matAux);
	dataPtr->TransInvWorld = m_TransposeInvWorld;

	// ojo, que como la matriz va transpuesta en el shader, hay que multiplicar al reves!, asi me ahorro transponer
	dataPtr->worldViewProj = m_Proj*m_View*m_World;
	dataPtr->worldView = m_View*m_World;
	dataPtr->lookFrom = lookFrom;
	// Unlock the constant buffer.
	devcon->Unmap(m_matrixBuffer, 0);
	// Finanly set the constant buffer in the vertex shader with the updated values.
	devcon->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	devcon->PSSetConstantBuffers(0, 1, &m_matrixBuffer);
}

void CDX11Device::SetShaderLighting()
{
	// Lighting    ----------------------------------------------------
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	LightBufferType * dataPtr;
	float phi = 2;
	float theta = 0.95;
	devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (LightBufferType*)mappedResource.pData;
	dataPtr->Pos = D3DXVECTOR3(0,2500,0);
	dataPtr->Dir = D3DXVECTOR3(0,-1,0);
	dataPtr->Color = D3DXVECTOR3(1,1,1);
	dataPtr->Phi = cos(phi/2.0);
	dataPtr->Theta = cos(theta/2.0);
	devcon->Unmap(m_lightBuffer, 0);
	devcon->PSSetConstantBuffers(1, 1, &m_lightBuffer);
}


CBaseTexture *CDX11Device::CreateTexture(char *fname)
{
	CDX11Texture *p_texture = NULL;
	ID3D11ShaderResourceView *texture = NULL;
	D3DX11CreateShaderResourceViewFromFile(dev, fname, NULL, NULL, &texture, NULL);
	if(texture!=NULL)
	{
		p_texture = new CDX11Texture();
		strcpy(p_texture->name , fname);
		p_texture->resourceView = texture;
	}
	return (CBaseTexture *)p_texture;
}


CBaseMesh *CDX11Device::LoadMesh(CRenderEngine *p_engine, char *fname)
{
	CBaseMesh *p_mesh = NULL;
	char ext[4];
	que_extension(fname,ext);
	bool ok;
	if(ext[0]=='y' || ext[0]=='Y')
	{
		// Cargo un archivo fomrato .Y 
		p_mesh = new CDX11Mesh;
		ok = ((CDX11Mesh *)p_mesh)->LoadFromFile(p_engine,this,fname);
	}
	else
	{
		// Cargo un archivo formato xml
		if(IsSkeletalMesh(fname))
		{
			// Skeletal mesh
			p_mesh = new CDX11SkeletalMesh;
			ok = ((CDX11SkeletalMesh*)p_mesh)->LoadFromXMLFile(p_engine,this,fname);
		}
		else
		{
			// Mesh comun 
			p_mesh = new CDX11Mesh;
			ok = ((CDX11Mesh *)p_mesh)->LoadFromXMLFile(p_engine,this,fname);
		}
	}

	if(!ok)
		SAFE_DELETE(p_mesh);			// y p_mesh queda en NULL
	return p_mesh;
}


CBaseMesh *CDX11Device::LoadMeshFromXmlFile(CRenderEngine *p_engine, char *fname,char *mesh_name,int mat_id)
{
	CBaseMesh *p_mesh = NULL;
	p_mesh = new CDX11Mesh;
	bool ok = ((CDX11Mesh *)p_mesh)->LoadFromXMLFile(p_engine,this,fname,mesh_name,mat_id);

	if(!ok)
		SAFE_DELETE(p_mesh);			// y p_mesh queda en NULL

	return p_mesh;
}



// Helper, para saber si es un xml dato es un skeletal o no
bool IsSkeletalMesh(char *fname)
{
	bool rta = false;
	FILE *fp = fopen(fname,"rt");
	if(fp)
	{
		char buffer[255];
		fgets(buffer,sizeof(buffer),fp);
		if(strncmp(buffer,"<tgcSkeletalMesh>",17)==0)
			rta = true;
		fclose(fp);
	}
	return rta;
}

