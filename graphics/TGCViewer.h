#pragma once

#include "/dev/graphics/mesh.h"
#include "/dev/graphics/SkeletalMesh.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "/dev/graphics/RenderEngine.h"

// compatibilidad con TGCViewer

struct tgc_scene_mesh
{
	public:
		char mesh_id[255];
		D3DXVECTOR3 pos;
		int mat_id;
};

class CTGCXmlParser
{
	public:
		// helpers estaticos
		static D3DCOLORVALUE ParseXMLColor(char *buffer);
		static bool ParseIntStream(char *buffer,int *S,int count);
		static bool ParseFloatStream(char *buffer,float *S,int count);
		static D3DXVECTOR3 ParseXMLVector3(char *buffer);
		static D3DXVECTOR4 ParseXMLVector4(char *buffer);
};

class CTGCMeshParser : public CTGCXmlParser
{
	public:

		// Data auxiliar para el xml parser
		int xml_current_tag;
		int xml_current_layer;
		bool xml_multimaterial;
		int xml_mat_id;
		int xml_current_mat_id;

		int *coordinatesIdx;
		int *textCoordsIdx;
		int *matIds;
		int matIdsCount;

		char mesh_id[255];
		FLOAT *vertices;
		FLOAT *normals;
		FLOAT *texCoords;
		char path_texturas[MAX_PATH];

		CBaseMesh *M;

		// interface
		virtual bool LoadMesh(CBaseMesh *p_mesh,char *filename,char *mesh_name,int mesh_mat_id);
		virtual int LoadSceneHeader(char *filename,tgc_scene_mesh mesh_lst[]);

		// Parser x linea basico
		virtual char ParseXMLLine(char *buffer);

		// mesh data
		virtual void CreateMeshData();
	

};


class CTGCSkeletalMeshParser : public CTGCMeshParser
{
	public:
		FLOAT *binormals;
		FLOAT *tangents;
		vertexWeight *aux_verticesWeights;

		virtual bool LoadSkeletalMesh(CBaseSkeletalMesh *p_mesh,char *filename);
		virtual char ParseXMLLine(char *buffer);


		// mesh data
		virtual void CreateMeshData();

};


