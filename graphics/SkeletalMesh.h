#pragma once
#pragma pack(1)
#include "mesh.h"

#define MAX_BONES		26
struct SKELETAL_MESH_VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
	D3DXVECTOR3 tangent;
	D3DXVECTOR3 binormal;
	D3DXVECTOR4 blendWeights;
	D3DXVECTOR4 blendIndices;
};


class CSkeletalBone
{
	public:

		int id;
		char name[32];
		int parentId;
		D3DXVECTOR3 startPosition;
		D3DXQUATERNION startRotation;
		D3DXMATRIX matLocal;
		D3DXMATRIX matFinal;
		D3DXMATRIX matInversePose;
		CSkeletalBone();
		void Clean();
		void computeMatLocal();
};

#define MAX_FRAMES_X_BONE		30
#define MAX_ANIMATION_X_MESH	30

// notacion usada:
// un frame es una pos mas una rotacion que determinan una transformacion aplicada a un hueso en particular
struct st_bone_frame
{
	int nro_frame;
	D3DXVECTOR3 Position;
	D3DXQUATERNION Rotation;
};

// un conjunto de frames asociados a l mismo hueso es una animacion para el hueso
struct st_bone_animation
{
	int cant_frames;
	st_bone_frame frame[MAX_FRAMES_X_BONE];
};

// y esta clase agrupa a todas las animaciones de c/u de los huesos. 
class CSkeletalAnimation
{

	public:
		char name[32];
		int cant_bones;
		int cant_frames;
		int frame_rate;
		st_bone_animation bone_animation[MAX_BONES];

		// auxiliares para el xml parser
		int bone_id;
		int nro_frame;


		CSkeletalAnimation();
		bool CreateFromFile(char *fname);
		virtual bool ParserXMLLine(char *buffer);



};


// binding entre vertices huesos
struct vertexWeight
{
	int boneIndex[4];
	float weight[4];
};


class CBaseSkeletalMesh : public CBaseMesh
{
public:
	int cant_bones;
	CSkeletalBone bones[MAX_BONES];

	// Datos para el skining
	vertexWeight *verticesWeights;

	int cant_animaciones;
	CSkeletalAnimation *animacion[MAX_ANIMATION_X_MESH];
	bool animating;
	bool playLoop;
	float currentTime;
	float currentFrame;
	int currentAnimation;
	float frameRate;
	float animationTimeLenght;

	//Matrices final de transformacion de cada ueso
	D3DXMATRIX matBoneSpace[MAX_BONES];


	// Data auxiliar para el xml parser
	FLOAT *binormals;
	FLOAT *tangents;
	vertexWeight *aux_verticesWeights;



	CBaseSkeletalMesh();
	virtual ~CBaseSkeletalMesh();
	virtual void SetVertexDeclaration() = 0;
	virtual void SetShaders() = 0;
	virtual void Draw() = 0;


	virtual bool LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename);
	virtual char ParserXMLLine(char *buffer);
	virtual bool CreateFromXMLData() = 0;

	// animaciones
	bool LoadAnimation(char *fname);
	void initAnimation(int nro_animacion, bool con_loop = true, float userFrameRate=0);
	void setupSkeleton();
	void updateMeshVertices();
	void updateAnimation();
	void updateSkeleton();
	int getCurrentFrameBone(st_bone_animation *boneFrames, float currentFrame);

};


class CDX11SkeletalMesh : public CBaseSkeletalMesh
{
public:

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	CDX11SkeletalMesh();
	virtual ~CDX11SkeletalMesh();
	virtual void Release();
	virtual void SetVertexDeclaration();
	virtual void SetShaders();
	virtual void Draw();
	virtual void DrawSubset(int i);
	virtual bool CreateFromXMLData();

};



class CDX9SkeletalMesh : public CBaseSkeletalMesh
{
public:

	LPDIRECT3DVERTEXBUFFER9 m_vertexBuffer;	
	LPDIRECT3DINDEXBUFFER9 m_indexBuffer;	

	CDX9SkeletalMesh();
	virtual ~CDX9SkeletalMesh();
	virtual void Release();
	virtual void SetVertexDeclaration();
	virtual void SetShaders();
	virtual void Draw();
	virtual void DrawSubset(int i);
	virtual bool CreateFromXMLData();

};