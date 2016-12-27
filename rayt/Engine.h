#pragma once
#include "math.h"

#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;


struct face3d
{
	vec3 v[3];				// posicion de los vertices
	float tu[3],tv[3];		// tex coords
	vec3 n[3];				// normal x vertice
	int layer;				// sub-set
	// auxiliares
	vec3 pmin;
	vec3 pmax;

};

// datos en el punto de interseccion
struct ip_data
{
	vec3 ip;		// intersection point
	float t;		// distancia al punto de interseccion
	float bc_b;		// barycentric B
	float bc_g;		// barycentric G
	int nro_face;
};


// octree
struct st_node
{
	vec3 p_min;
	vec3 p_max;
	int deep;
	st_node *p[8];
	int cant_f;
	int *p_list;
};

#define MAX_DEEP 10
#define MAX_FACE_X_NODE 10


// kdree
struct kd_node
{
	vec3 p_min;
	vec3 p_max;
	int deep;
	float split;
	int split_plane;
	kd_node *p_left , *p_right;
	int cant_f;
	int *p_list;
};


class CEngine
{
public:

	vec3 LF;				// look from point
	vec3 LA;				// look at point
	vec3 VUP;				// look up vector
	vec3 N;					// viewing direction
	vec3 V;
	vec3 U;

	float fov;					// field of view angle (~120 grados)
	float W;					// Screen size
	float H;
	
	vec3 Dx;				// Ray direction
	vec3 Dy;

	// lista de caras
	int cant_faces;
	face3d *F;

	vec3 bb_min , bb_max;

	// colores layers
	COLORREF clr_layer[8];

	CEngine();
	~CEngine();

	void render(CDC *pDC);
	bool interseccion(vec3 p,vec3 d,ip_data *I);

	// precalculos x face
	void precalc();

	// Octree
	st_node *p_root;
	void renderOctree(CDC *pDC);
	bool interseccionB(vec3 p,vec3 d,ip_data *I);
	st_node *createOctreeNodeOld(vec3 pmin, vec3 dim,int deep);
	st_node *createOctreeNode(vec3 pmin, vec3 dim,int deep);
	void deleteOctreeNode(st_node *p_node);
	int countOctreeNodes(st_node *p_node);
	void createOctree();


	// Kd-tree
	int max_deep;
	kd_node *kd_tree;
	kd_node *createKDTreeNode(vec3 pmin, vec3 pmax,int deep,int cant_f,int *p_list);
	void createKDTree(); 
	void deleteKDTreeNode(kd_node *p_node);
	void renderKDTree(CDC *pDC);
	bool interseccionKDTree(vec3 p,vec3 d,ip_data *I);
	int debugKDTree(CDC *pDC,kd_node *p_node,int x,int y);



	void initFromTest();
	void quadTest();
	void createBox(vec3 pos,vec3 dim);
	void loadMesh();


};


int triangle_intersection( vec3 V1,  // Triangle vertices
						  vec3 V2,
						  vec3 V3,
						  vec3 O,  //Ray origin
						  vec3 D,  //Ray direction
						  float* out , float *U, float *V);

