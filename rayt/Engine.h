#pragma once
#include "math.h"
#include "texture.h"

#define MAX_TEXTURAS	250
#define MAX_MATERIALES	4096


#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;


struct st_material 
{
	int nro_texture;
	int bmp_k;
	vec3 Diffuse;
	vec3 Ambient;
	vec3 Specular;
	vec3 Emissive;
	float Power;
};


struct face3d
{
	vec3 v[3];				// posicion de los vertices
	float tu[3],tv[3];		// tex coords
	vec3 n[3];				// normal x vertice
	vec3 color[3];		// color x vertice
	int layer;				// sub-set
	// auxiliares
	vec3 pmin;
	vec3 pmax;
	// precomputed data: interseccion ray - tri
	vec3 e1;
	vec3 e2;

};

// version sse
struct sse_precomputed_face3d
{
	__m128 v0;
	__m128 e1;
	__m128 e2;

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


_MM_ALIGN16 class CEngine
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
	sse_precomputed_face3d *sse_F;

	vec3 bb_min , bb_max;

	// auxiliar para dibujar cosas en gdi
	int ox,oy;
	float ex,ey;
	// colores layers
	COLORREF clr_layer[8];

	// textura de prueba
	CTexture tx;
	CTexture texturas[MAX_TEXTURAS];
	int cant_texturas;
	// materiales
	int cant_mat;
	st_material materiales[MAX_MATERIALES];



	CEngine();
	~CEngine();

	void render(CDC *pDC);
	bool interseccion(vec3 p,vec3 d,ip_data *I);

	// precalculos x face
	void precalc();			// calculos que no dependen del rayo


	// Kd-tree
	int max_deep;
	kd_node *kd_tree;
	kd_node *createKDTreeNode(vec3 pmin, vec3 pmax,int deep,int cant_f,int *p_list);
	void createKDTree(); 
	void deleteKDTreeNode(kd_node *p_node);
	void renderKDTree(CDC *pDC);
	bool interseccionKDTree(vec3 p,vec3 d,ip_data *I);
	int debugKDTree(CDC *pDC,kd_node *p_node,int x,int y);
	float best_split(int eje,kd_node *p_node);
	int best_split(kd_node *p_node,float *best_s);



	void initFromTest();
	void quadTest();
	void createBox(vec3 pos,vec3 dim);
	void loadMesh();
	void loadScene();				// carga un archivo de 3ds (del exporter)


	int triangle_ray( int i,
		vec3 O,  //Ray origin
		vec3 D,  //Ray direction
		float* out , float *U, float *V);

	void plane_ray( int i,
		vec3 O,  //Ray origin
		vec3 D,  //Ray direction
		float *U, float *V);


	vec3 ray_O , ray_D;
	__m128 SSE_O;  //Ray origin
	__m128 SSE_D;  //Ray direction
	int triangle_ray_SSE( int i,float* out, float *U,float *V);


	// texturas
	int CreateTexture(const char *fname);


};

int triangle_intersection( vec3 V1,  // Triangle vertices
						  vec3 V2,
						  vec3 V3,
						  vec3 O,  //Ray origin
						  vec3 D,  //Ray direction
						  float* out , float *U, float *V);

bool box_intersection(vec3 A, vec3 B,
					  vec3 O,	//Ray origin
					  vec3 D,	//Ray direction
					  float* tn, float *tf,
					  float mint=0.001f , float maxt=1000000);


// helper para loader de 3ds
bool ld_vec3(FILE *fp,vec3 &v);
bool ld_tx_rec(FILE *fp, char *fname,float &k);
