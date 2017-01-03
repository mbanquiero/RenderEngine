#include "stdafx.h"
#include "Engine.h"

// Agregar esto para debug 
//#define DEBUG_NEW new(__FILE__, __LINE__)
//#define new DEBUG_NEW

CEngine::CEngine()
{
	cant_faces = 0;
	F = NULL;
	sse_F = NULL;

	VUP = vec3(0,0,1);				// look up vector
	LF = vec3(-3000,-3000,1500);
	LA = vec3(0,0,0);
	fov = 0.9;
	W = 1280;
	H = 800;

	//W = 600;
	//H = 400;
	LF = vec3(3000,2000,250);
	LA = vec3(1000,2000,0);

	clr_layer[0] = RGB(255,0,0);
	clr_layer[1] = RGB(0,255,0);
	clr_layer[2] = RGB(0,0,255);
	clr_layer[3] = RGB(255,0,255);

	clr_layer[4] = RGB(255,100,0);
	clr_layer[5] = RGB(0,255,100);
	clr_layer[6] = RGB(100,100,255);
	clr_layer[7] = RGB(255,100,255);

	cant_mat = 0;
	cant_luces = 0;
	cant_texturas = 0;
	// creo la textura de test y un material standard
	//	tx.CreateFromFile("media/chess_small.png");
	//tx.CreateFromFile("media/chess_250.png");
	mat_std.nro_texture = CreateTexture("media/test_128.png");
	mat_std.bmp_k = 1;


	k_la = 0.5;			// luz ambiente
	k_ld = 1;			// luz difusa
	k_ls = 0.7;			// luz specular

}


CEngine::~CEngine()
{
	SAFE_DELETE(F);
	if(sse_F)
		_mm_free(sse_F);
	if(kd_tree!=NULL)
		deleteKDTreeNode(kd_tree);

}

void CEngine::initFromTest()
{

	loadScene();
//	quadTest();
//	createBox(vec3(0,0,500) , vec3(500,500,500));
//	createBox(vec3(0,700,500) , vec3(500,500,500));
//	createBox(vec3(0,1600,500) , vec3(500,500,500));
//	cant_luces = 1;
//	luces[0].Position = vec3(-500,-500,3000);


/*	F = new face3d[1000];
	createBox(vec3(0,0,0) , vec3(1500,1500,1500));

	LF = vec3(0,0,0);
	LA = vec3(1,1,1);
	*/

	/*
	F = new face3d[100000];
	createBox(vec3(-1200,-1000,0) , vec3(500,500,500));
	createBox(vec3(-1400,-1000,0) , vec3(500,500,500));
	createBox(vec3(-1000,-1000,0) , vec3(500,500,500));
	createBox(vec3(2000,2000,0) , vec3(500,500,500));

	createBox(vec3(-1200,-1000,500) , vec3(500,500,500));
	createBox(vec3(-1400,-1000,500) , vec3(500,500,500));
	createBox(vec3(-1000,-1000,500) , vec3(500,500,500));
	createBox(vec3(2000,2000,500) , vec3(500,500,500));
	*/

/*	for(int k=-3;k<3;++k)
		for(int i=-3;i<3;++i)
		for(int j=-3;j<3;++j)
		createBox(vec3(i*300,j*300,k*300) , vec3(100,100,100));
		*/

	precalc();
	createKDTree();
}

void CEngine::quadTest()
{
	F = new face3d[100];

	float k = 8;
	F[0].v[0] = vec3(0,0,0);
	F[0].tu[0] = 0;
	F[0].tv[0] = 0;
	F[0].color[0] = vec3(1,0,0);

	F[0].v[1] = vec3(0,4000,0);
	F[0].tu[1] = 0;
	F[0].tv[1] = k;
	F[0].color[1] = vec3(0,1,0);
	
	F[0].v[2] = vec3(4000,0,0);
	F[0].tu[2] = k;
	F[0].tv[2] = 0;
	F[0].color[2] = vec3(0,0,1);
	F[0].layer  = -1;


	F[1].v[0] = vec3(0,4000,0);
	F[1].tu[0] = 0;
	F[1].tv[0] = k;
	F[1].color[0] = vec3(0,1,0);

	F[1].v[1] = vec3(4000,4000,0);
	F[1].tu[1] = k;
	F[1].tv[1] = k;
	F[1].color[1] = vec3(1,0,0);

	F[1].v[2] = vec3(4000,0,0);
	F[1].tu[2] = k;
	F[1].tv[2] = 0;
	F[1].color[2] = vec3(0,0,1);
	F[1].layer  = -1;

	cant_faces = 2;


	

}

void CEngine::createBox(vec3 pos,vec3 dim)
{
	vec3 dx = vec3(dim.x,0,0);
	vec3 dy = vec3(0,dim.y,0);
	vec3 dz = vec3(0,0,dim.z);

	// Cara de abajo
	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dy;
	F[cant_faces].v[2] = pos + dx + dy;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 0;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dx + dy;
	F[cant_faces].v[2] = pos + dx;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 0;

	cant_faces++;

	
	// Cara de arriba
	F[cant_faces].v[0] = pos + dz;
	F[cant_faces].v[1] = pos + dy + dz;
	F[cant_faces].v[2] = pos + dx + dy + dz;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 0;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	F[cant_faces].v[0] = pos + dz;
	F[cant_faces].v[1] = pos + dx + dy + dz;
	F[cant_faces].v[2] = pos + dx+ dz;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 0;

	cant_faces++;


	// Cara de izquierda
	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dz;
	F[cant_faces].v[2] = pos + dx + dz;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 0;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dx + dz;
	F[cant_faces].v[2] = pos + dx;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 0;
	F[cant_faces].tv[2] = 1;

	cant_faces++;
	

	// Cara de derecha
	F[cant_faces].v[0] = pos + dy;
	F[cant_faces].v[1] = pos + dz + dy;
	F[cant_faces].v[2] = pos + dx + dz + dy; 
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 0;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	F[cant_faces].v[0] = pos + dy;
	F[cant_faces].v[1] = pos + dx + dz + dy;
	F[cant_faces].v[2] = pos + dx + dy;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 0;

	cant_faces++;

	// Cara de atras
	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dz;
	F[cant_faces].v[2] = pos + dy + dz;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 0;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dy + dz;
	F[cant_faces].v[2] = pos + dy;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 0;
	F[cant_faces].tv[2] = 1;

	cant_faces++;

	// Cara de adelante
	F[cant_faces].v[0] = pos + dx;
	F[cant_faces].v[1] = pos + dz + dx;
	F[cant_faces].v[2] = pos + dy + dz + dx;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 0;
	F[cant_faces].tu[2] = 1;
	F[cant_faces].tv[2] = 1;
	cant_faces++;

	F[cant_faces].v[0] = pos + dx;
	F[cant_faces].v[1] = pos + dy + dz + dx;
	F[cant_faces].v[2] = pos + dy + dx;
	F[cant_faces].layer = -1;
	F[cant_faces].tu[0] = 0;
	F[cant_faces].tv[0] = 0;
	F[cant_faces].tu[1] = 1;
	F[cant_faces].tv[1] = 1;
	F[cant_faces].tu[2] = 0;
	F[cant_faces].tv[2] = 1;
	cant_faces++;

}


// precalculos x face
void CEngine::precalc()
{

	// alloco memoria para los valores precomputados que usan sse (tiene que estar alineado a 16 bytes)
	sse_F =  (sse_precomputed_face3d*)_mm_malloc( sizeof(sse_precomputed_face3d) * cant_faces , 16);

	// bounding box scene
	bb_min.x = 100000;
	bb_min.y = 100000;
	bb_min.z = 100000;
	bb_max.x = -100000;
	bb_max.y = -100000;
	bb_max.z = -100000;

	for(int i=0;i<cant_faces;++i)
	{
		// bounding box face
		float min_x = 100000;
		float min_y = 100000;
		float min_z = 100000;
		float max_x = -100000;
		float max_y = -100000;
		float max_z = -100000;
		for(int j=0;j<3;++j)
		{
			float x = F[i].v[j].x;
			float y = F[i].v[j].y;
			float z = F[i].v[j].z;
			if(x<min_x)
				min_x = x;
			if(y<min_y)
				min_y = y;
			if(z<min_z)
				min_z = z;
			if(x>max_x)
				max_x = x;
			if(y>max_y)
				max_y = y;
			if(z>max_z)
				max_z = z;

			// scene bb
			if(x<bb_min.x)
				bb_min.x = x;
			if(y<bb_min.y)
				bb_min.y = y;
			if(z<bb_min.z)
				bb_min.z = z;
			if(x>bb_max.x)
				bb_max.x = x;
			if(y>bb_max.y)
				bb_max.y = y;
			if(z>bb_max.z)
				bb_max.z = z;


		}
		F[i].pmin = vec3(min_x , min_y , min_z);
		F[i].pmax = vec3(max_x , max_y , max_z);

		F[i].e1 = F[i].v[1] - F[i].v[0];
		F[i].e2 = F[i].v[2] - F[i].v[0];

		// precomputados para la version sse
		sse_F[i].v0 = _mm_setr_ps(F[i].v[0].x, F[i].v[0].y, F[i].v[0].z, 0);
		sse_F[i].e1 = _mm_setr_ps(F[i].e1.x, F[i].e1.y, F[i].e1.z, 0);
		sse_F[i].e2 = _mm_setr_ps(F[i].e2.x, F[i].e2.y, F[i].e2.z, 0);

	}
}




void CEngine::createKDTree()
{
	max_deep = (8 + 1.3f * log(cant_faces));

	// alloco memoria para SAH
	//splanes = new float[2*cant_faces];

	// creo un nodo con toda la escena
	int *p_list = new int[cant_faces];
	for(int i=0;i<cant_faces;++i)
		p_list[i] = i;
	kd_tree = createKDTreeNode(bb_min , bb_max , 0 , cant_faces , p_list);
}

kd_node *CEngine::createKDTreeNode(vec3 pmin, vec3 pmax,int deep,int cant_f,int *p_list)
{
	kd_node *p_node = new kd_node;
	memset(p_node, 0 , sizeof(kd_node));
	p_node->deep = deep;
	p_node->p_min = pmin;
	p_node->p_max = pmax;

	// creo un nodo leaf
	p_node->cant_f = cant_f;
	p_node->p_list = p_list;

	// si la cantidad de primitivas en el nodo es mayor a cierto limite y el deep no supera el maximo, pruebo dividir el nodo
	if(cant_f>=MAX_FACE_X_NODE && deep<max_deep)
	{
		// divido el nodo en 2:
		vec3 dim = pmax - pmin;
		vec3 Lmin , Lmax , Rmin, Rmax;
		int eje;
		float s;
		
		// version eje fijo: selecciono el eje en base a la direccion que mas extension tiene
		if(0)
		{
			if(fabs(dim.z)>=fabs(dim.x) && fabs(dim.z)>=fabs(dim.y))
				eje = 2;			// split Z
			else
			if(fabs(dim.x)>=fabs(dim.z) && fabs(dim.x)>=fabs(dim.y))
				eje = 0;			// splite X
			else
				eje = 1;			// split Z

			s = (pmin[eje] + pmax[eje]) / 2.0;
			//s = best_split(eje,p_node);
		}
		else
		{
			// version que prueba los 3 ejes: 
			eje = best_split(p_node , &s);
		}


		p_node->split = s;
		p_node->split_plane = eje;
		Lmin = pmin;
		Lmax = vec3(eje==0?s:pmax.x , eje==1?s:pmax.y , eje==2?s:pmax.z);
		Rmin = vec3(eje==0?s:pmin.x , eje==1?s:pmin.y , eje==2?s:pmin.z);
		Rmax = pmax;

		// clasifico las primitivas
		int cant_L = 0;
		int cant_R = 0;

		int *list_L = new int[cant_f];
		int *list_R = new int[cant_f];

		for(int i=0;i<cant_f;++i)
		{
			face3d *f = &F[p_list[i]];
			if(box_overlap(f->pmin , f->pmax , Lmin , Lmax))
				list_L[cant_L++] = p_list[i];
			if(box_overlap(f->pmin , f->pmax , Rmin , Rmax))
				list_R[cant_R++] = p_list[i];
		}

		// hago el nodo interior: 
		// libero la memoria original 
		if(p_node->p_list)
		{
			delete []p_node->p_list;
			p_node->p_list = NULL;
		}

		// creo los 2 nodos hijos
		p_node->p_left = createKDTreeNode(Lmin,Lmax,deep+1,cant_L,list_L);
		p_node->p_right = createKDTreeNode(Rmin,Rmax,deep+1,cant_R,list_R);

	}

	return p_node;
}

void CEngine::deleteKDTreeNode(kd_node *p_node)
{
	if(p_node->p_left)
		deleteKDTreeNode(p_node->p_left);
	if(p_node->p_right)
		deleteKDTreeNode(p_node->p_right);

	if(p_node->p_list)
		delete []p_node->p_list;

	SAFE_DELETE(p_node);

}

int CEngine::debugKDTree(CDC *pDC,kd_node *p_node,int x,int y)
{
	pDC->MoveTo(ox + p_node->p_min.x*ex , oy + p_node->p_min.y*ey);
	pDC->LineTo(ox + p_node->p_min.x*ex , oy + p_node->p_max.y*ey);
	pDC->LineTo(ox + p_node->p_max.x*ex , oy + p_node->p_max.y*ey);
	pDC->LineTo(ox + p_node->p_max.x*ex , oy + p_node->p_min.y*ey);
	pDC->LineTo(ox + p_node->p_min.x*ex , oy + p_node->p_min.y*ey);
	if(p_node->p_left!=NULL)
		debugKDTree(pDC,p_node->p_left,x+20,y);
	if(p_node->p_right!=NULL)
		debugKDTree(pDC,p_node->p_right,x+20,y);

	return 0 ;

	pDC->FillSolidRect(x,y,10,10,RGB(0,0,0));

	int dy = 15;
	int y0 = y;
	if(p_node->cant_f)
	{
		char saux[40];
		sprintf(saux,"%d",p_node->cant_f);
		pDC->TextOut(x,y,saux,strlen(saux));
	}
	//else
	{
		if(p_node->p_left!=NULL)
		{
			pDC->MoveTo(x,y);
			y+=dy;
			pDC->LineTo(x,y);
			pDC->LineTo(x+20,y);
			y += debugKDTree(pDC,p_node->p_left,x+20,y);
		}
		if(p_node->p_right!=NULL)
		{
			pDC->MoveTo(x,y0);
			y+=dy;
			pDC->LineTo(x,y);
			pDC->LineTo(x+20,y);
			y += debugKDTree(pDC,p_node->p_right,x+20,y);
		}
	}
	
	return y - y0;
}


void CEngine::render(CDC *pDC)
{

	pDC->FillSolidRect(0,0,W,H,RGB(255,0,255));

	// camara simple
	N = LA-LF;						// viewing direction
	N.normalize();
	V = cross(N,VUP);
	V.normalize();
	U = cross(V,N);
	double k = 2*tan(fov/2)/H;
	Dy = U*k;
	Dx = V*k;

	int x,y;
	for(y=0;y<H;++y)
	{
		for(x=0;x<W;++x)
		{
			//x = W/2;
			//y = H/2;
			// Direccion of each pixel
			vec3 D = N + Dy*(0.5*(H-2*y)) + Dx*(0.5*(2*x-W));
			vec3 nnD = D;			// no normalizado
			D.normalize();

			// Ecuacion del rayo
			// r = LF + k*D
			vec4 clr = shade(LF,D,nnD);
			BYTE r = clamp_to_color(clr.x);
			BYTE g = clamp_to_color(clr.y);
			BYTE b = clamp_to_color(clr.z);
			pDC->SetPixel(x,y,RGB(r,g,b));

		}
	}
}



vec4 CEngine::shade(vec3 p,vec3 d,vec3 nnD,int r_deep)
{
	ip_data I;
	vec4 clr = vec4(0,0,0,1);
	if(ray_intersects(p,d,&I))
	{

		float k1 = I.bc_b;
		float k2 = I.bc_g;
		float k0 = 1-k1-k2;
		int n = I.nro_face;
		face3d *P = &F[n];			// primitiva
		// interpolo las coord de texturas
		float tu = P->tu[0]*k0 + P->tu[1]*k1 + P->tu[2]*k2;
		float tv = P->tv[0]*k0 + P->tv[1]*k1 + P->tv[2]*k2;
		// interpolo las normales x vertices
		vec3 Normal = P->n[0]*k0 + P->n[1]*k1 + P->n[2]*k2;
		Normal.normalize();
				
		st_material *mat = P->layer>=0 ? &materiales[P->layer] : &mat_std;
		if(mat->nro_texture!=-1)
		{
			// texture
			// Gradiente en X
			vec3 Ddx = nnD + Dx;
			Ddx.normalize();

			float k0dx,k1dx,k2dx;
			plane_ray( n,LF,Ddx,&k1dx,&k2dx);
			k0dx = 1-k1dx-k2dx;
			float tudx = P->tu[0]*k0dx + P->tu[1]*k1dx + P->tu[2]*k2dx;
			float tvdx = P->tv[0]*k0dx + P->tv[1]*k1dx + P->tv[2]*k2dx;

			// Gradiente en Y
			vec3 Ddy = nnD + Dy;
			Ddy.normalize();
			float k0dy,k1dy,k2dy;
			plane_ray( n,LF,Ddy,&k1dy,&k2dy);
			k0dy = 1-k1dy-k2dy;
			float tudy = P->tu[0]*k0dy + P->tu[1]*k1dy + P->tu[2]*k2dy;
			float tvdy = P->tv[0]*k0dy + P->tv[1]*k1dy + P->tv[2]*k2dy;

			float ddx = vec3(tudx-tu, tvdx-tv , 0).length();
			float ddy = vec3(tudy-tu, tvdy-tv , 0).length();

			clr = texturas[mat->nro_texture].tex2Dgrad(tu,tv,ddx,ddy);
			//clr = texturas[mat->nro_texture].tex2Dlod(tu,tv,0);
		}
		else
		{
			// color difuso
			clr.x = mat->Diffuse.x;
			clr.y = mat->Diffuse.y;
			clr.z = mat->Diffuse.z;
			clr.w = 1.0;				// alpha = opaco
		}

		vec3 LightDiffuse,LightSpecular;
		ComputeLighting(I.ip,Normal,LightDiffuse,LightSpecular);
		clr.x = clr.x*(saturate(k_la+LightDiffuse.x)) + LightSpecular.x;
		clr.y = clr.y*(saturate(k_la+LightDiffuse.y)) + LightSpecular.y;
		clr.z = clr.z*(saturate(k_la+LightDiffuse.z)) + LightSpecular.z;

		/*
		if(mat->kr>0 && r_deep<5)
		{
			// reflexion
			vec3 r = reflect(d,Normal);
			clr = clr * (1-mat->kr) + shade(I.ip + r*3.0 , r, r_deep+1) * mat->kr;
		}
		*/
	}

	return clr;
}


struct st_traverse_node
{
	float tnear , tfar;
	kd_node *p_nodo;
};


bool CEngine::ray_intersects(vec3 O,vec3 D,ip_data *I)
{
	float R = 10000000;
	float bc_b;
	float bc_g;
	int nro_face = -1;


	// chequeo la interseccion con el bounding box de la escena o 
	float tnear, tfar;
	if(!box_intersection(bb_min, bb_max, O,D,&tnear,&tfar))
		// el rayo no interseca con la escena
		return false;

	// precomputo la inv de direccion del rayo
	float ray_invdir[] = {0,0,0};
	if(fabs(D.x)>0.00001)
		ray_invdir[0]	= 1.f/D.x;
	if(fabs(D.y)>0.00001)
		ray_invdir[1]	= 1.f/D.y;
	if(fabs(D.z)>0.00001)
		ray_invdir[2]	= 1.f/D.z;
	float ray_O[] = {O.x,O.y,O.z};
	float ray_dir[] = {D.x,D.y,D.z};

	// sse triangle - ray intersection
	SSE_O = _mm_setr_ps(O.x, O.y, O.z, 0);
	SSE_D = _mm_setr_ps(D.x, D.y, D.z, 0);

	// lista de caras a chequear
	int aux_F[10000];
	int cant_f;


	// comienzo el traverse con el nodo root = (kd_tree, tnear, tfar)
	kd_node *p_node = kd_tree;
	// pila de pendientes
	int p_stack = 0;
	st_traverse_node S[64];

	while(p_node!=NULL)
	{
		// el rayo atraviesa el nodo p_node entrando por tnear y saliendo por tfar. 


		if(p_node->p_list)
		{
			// nodo hoja: chequeo la interseccion con la lista de caras en dicho nodo
			for(int i=0;i<p_node->cant_f;++i)				
			{
				int n = p_node->p_list[i];
				float b,g,t;
				if(triangle_ray_SSE( n,&t,&b,&g))
				{
					if(t>1 && t<R && t>=tnear-1 && t<=tfar+1)
					{
						// actualizo las coordenadas barycentricas
						bc_b = b;
						bc_g = g;
						R = t;
						nro_face = n;
					}
				}
			}

			// early termination
			if(nro_face!=-1)
			{
				I->ip = O + D*R;
				I->t = R;
				I->bc_b = bc_b;
				I->bc_g = bc_g;
				I->nro_face = nro_face;
				return true;
			}

			// termine de procesar la rama (llegue a un nodo hoja). Si tengo algo pendiente en la pila, lo saco de ahi
			if(p_stack>0)
			{
				p_stack--;
				p_node = S[p_stack].p_nodo;
				tnear = S[p_stack].tnear;
				tfar = S[p_stack].tfar;
			}
			else
				p_node = NULL;			// termino

		}
		else
		{
			// si es un nodo interior: 
			// determino en que orden tengo que chequear los nodos hijos 
			int p = p_node->split_plane;
			float tplane = (p_node->split - ray_O[p])  * ray_invdir[p];

			kd_node *p_near , *p_far;
			if(ray_O[p]<=p_node->split || (ray_O[p] == p_node->split && ray_dir[p] <= 0))
			{
				// proceso primero el Left node y luego el Right node
				p_near = p_node->p_left;
				p_far = p_node->p_right;
			}
			else
			{
				// proceso primero el Right node y luego el Left node
				p_near = p_node->p_right;
				p_far = p_node->p_left;
			}

			// para procesar ambos nodos el tplane tiene que estar entre tnear y tfar
			if(tplane>tfar || tplane<=0)
			{
				// el rayo solo pasa por el primer nodo (el nodo cercano) : avanzo hacia el nodo cercano
				p_node = p_near;		
			}
			else
			if(tplane<tnear)
			{
				// el rayo solo pasa por el segundo nodo (el nodo lejano) : avanzo hacia el nodo lejano
				p_node = p_far;
			}
			else
			{
				// pasa por ambos nodos: 

				// tengo que evaluar el segundo nodo luego del primero, asi que lo pongo en la pila de pendientes
				// el nodo far va desde tplane hasta tfar
				S[p_stack].p_nodo = p_far;
				S[p_stack].tnear = tplane;
				S[p_stack].tfar = tfar;
				p_stack++;

				// a continuacion proceso el nodo  cercano: que va desde tnear, hasta tplane
				p_node = p_near;		
				tfar = tplane;
				// tnear queda como esta
			}
		}
	}

	return false;
}




bool CEngine::shadow_ray_intersects(vec3 O,vec3 D,float rmin,float rmax)
{
	float bc_b;
	float bc_g;
	int nro_face = -1;


	
	float tnear = rmin, tfar =rmax;
	/*// chequeo la interseccion con el bounding box de la escena o 
	if(!box_intersection(bb_min, bb_max, O,D,&tnear,&tfar))
		// el rayo no interseca con la escena
			return false;
	*/

	// precomputo la inv de direccion del rayo
	float ray_invdir[] = {0,0,0};
	if(fabs(D.x)>0.00001)
		ray_invdir[0]	= 1.f/D.x;
	if(fabs(D.y)>0.00001)
		ray_invdir[1]	= 1.f/D.y;
	if(fabs(D.z)>0.00001)
		ray_invdir[2]	= 1.f/D.z;
	float ray_O[] = {O.x,O.y,O.z};
	float ray_dir[] = {D.x,D.y,D.z};

	// sse triangle - ray intersection
	SSE_O = _mm_setr_ps(O.x, O.y, O.z, 0);
	SSE_D = _mm_setr_ps(D.x, D.y, D.z, 0);

	// lista de caras a chequear
	int aux_F[10000];
	int cant_f;

	// comienzo el traverse con el nodo root = (kd_tree, tnear, tfar)
	kd_node *p_node = kd_tree;
	// pila de pendientes
	int p_stack = 0;
	st_traverse_node S[64];

	while(p_node!=NULL)
	{
		// el rayo atraviesa el nodo p_node entrando por tnear y saliendo por tfar. 
		if(p_node->p_list)
		{
			// nodo hoja: chequeo la interseccion con la lista de caras en dicho nodo
			for(int i=0;i<p_node->cant_f;++i)				
			{
				int n = p_node->p_list[i];
				float b,g,t;
				if(triangle_ray_SSE( n,&t,&b,&g))
				{
					if(t>tnear && t<=tfar+1)
					{
						return true;
					}
				}
			}

			// termine de procesar la rama (llegue a un nodo hoja). Si tengo algo pendiente en la pila, lo saco de ahi
			if(p_stack>0)
			{
				p_stack--;
				p_node = S[p_stack].p_nodo;
				tnear = S[p_stack].tnear;
				tfar = S[p_stack].tfar;
			}
			else
				p_node = NULL;			// termino

		}
		else
		{
			// si es un nodo interior: 
			// determino en que orden tengo que chequear los nodos hijos 
			int p = p_node->split_plane;
			float tplane = (p_node->split - ray_O[p])  * ray_invdir[p];

			kd_node *p_near , *p_far;
			if(ray_O[p]<=p_node->split || (ray_O[p] == p_node->split && ray_dir[p] <= 0))
			{
				// proceso primero el Left node y luego el Right node
				p_near = p_node->p_left;
				p_far = p_node->p_right;
			}
			else
			{
				// proceso primero el Right node y luego el Left node
				p_near = p_node->p_right;
				p_far = p_node->p_left;
			}

			// para procesar ambos nodos el tplane tiene que estar entre tnear y tfar
			if(tplane>tfar || tplane<=0)
			{
				// el rayo solo pasa por el primer nodo (el nodo cercano) : avanzo hacia el nodo cercano
				p_node = p_near;		
			}
			else
				if(tplane<tnear)
				{
					// el rayo solo pasa por el segundo nodo (el nodo lejano) : avanzo hacia el nodo lejano
					p_node = p_far;
				}
				else
				{
					// pasa por ambos nodos: 

					// tengo que evaluar el segundo nodo luego del primero, asi que lo pongo en la pila de pendientes
					// el nodo far va desde tplane hasta tfar
					S[p_stack].p_nodo = p_far;
					S[p_stack].tnear = tplane;
					S[p_stack].tfar = tfar;
					p_stack++;

					// a continuacion proceso el nodo  cercano: que va desde tnear, hasta tplane
					p_node = p_near;		
					tfar = tplane;
					// tnear queda como esta
				}
		}
	}

	return false;
}








#define EPSILON 0.000001

// The Möller–Trumbore ray-triangle intersection algorithm, named after its inventors Tomas Möller and Ben Trumbore, 
// is a fast method for calculating the intersection of a ray and a triangle in three dimensions without needing precomputation 
// of the plane equation of the plane containing the triangle.[1] Among other uses, it can be used in computer graphics to 
// implement ray tracing computations involving triangle meshes.[2]
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

int triangle_intersection( vec3 V1,  // Triangle vertices
						vec3 V2,
						  vec3 V3,
						  vec3 O,  //Ray origin
						  vec3 D,  //Ray direction
						  float* out, 
						  float *U,
						  float *V)
{
	vec3 e1, e2;  //Edge1, Edge2
	vec3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Find vectors for two edges sharing V1
	e1 = V2-V1;
	e2 = V3-V1;
	//Begin calculating determinant - also used to calculate u parameter
	P = cross( D, e2);
	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	det = dot(e1, P);
	//NOT CULLING
	if(det > -EPSILON && det < EPSILON) return 0;
		inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = O-V1;

	//Calculate u parameter and test bound
	u = dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if(u < 0.f || u > 1.f) return 0;

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate V parameter and test bound
	v = dot(D, Q) * inv_det;
	//The intersection lies outside of the triangle
	if(v < 0.f || u + v  > 1.f) return 0;

	t = dot(e2, Q) * inv_det;

	if(t > EPSILON) { //ray intersection
		*out = t;
		*U = u;
		*V = v;
		return 1;
	}

	// No hit, no win
	return 0;
}


// version optimizada
int CEngine::triangle_ray( int i,	// nro de face
							vec3 O,  //Ray origin
							vec3 D,  //Ray direction
							  float* out, 
							  float *U,
							  float *V)
{
	vec3 e1 = F[i].e1 , e2=F[i].e2;  //Edge1, Edge2
	vec3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Begin calculating determinant - also used to calculate u parameter
	P = cross( D, e2);
	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	det = dot(e1, P);
	//NOT CULLING
	if(det > -EPSILON && det < EPSILON) return 0;
	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = O-F[i].v[0];

	//Calculate u parameter and test bound
	u = dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if(u < 0.f || u > 1.f) return 0;

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate V parameter and test bound
	v = dot(D, Q) * inv_det;
	//The intersection lies outside of the triangle
	if(v < 0.f || u + v  > 1.f) return 0;

	t = dot(e2, Q) * inv_det;

	if(t > EPSILON) { //ray intersection
		*out = t;
		*U = u;
		*V = v;
		return 1;
	}

	// No hit, no win
	return 0;
}


inline int CEngine::triangle_ray_SSE( int i,float* out, float *U,float *V)
{
	//Begin calculating determinant - also used to calculate u parameter
	// P = cross( D, e2);
	__m128 P = sse_cross( SSE_D, sse_F[i].e2);
	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	// det = dot(e1,P)
	float det = _mm_dp_ps(sse_F[i].e1,P,0x7F).m128_f32[0];
	
	//NOT CULLING
	if(det > -EPSILON && det < EPSILON) return 0;
	float inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	__m128 T = _mm_sub_ps(SSE_O,sse_F[i].v0);

	//Calculate u parameter and test bound
	// u = dot(T, P) * inv_det;
	float u = _mm_dp_ps(T,P,0x7F).m128_f32[0] * inv_det;

	//The intersection lies outside of the triangle
	if(u < 0.f || u > 1.f) return 0;

	//Prepare to test v parameter
	__m128 Q = sse_cross(T, sse_F[i].e1);

	//Calculate V parameter and test bound
	//v = dot(D, Q) * inv_det;
	float v = _mm_dp_ps(SSE_D,Q,0x7F).m128_f32[0] * inv_det;

	//The intersection lies outside of the triangle
	if(v < 0.f || u + v  > 1.f) return 0;

	// t = dot(e2, Q) * inv_det;
	float t = _mm_dp_ps(sse_F[i].e2,Q,0x7F).m128_f32[0] * inv_det;

	if(t > EPSILON) { //ray intersection
		*out = t;
		*U = u;
		*V = v;
		return 1;
	}

	// No hit, no win
	return 0;
}

// se usa para computar el gradiente:  da por supuesto que el rayo intersecta al plano, solo importa devolver las baricentricas
void CEngine::plane_ray( int i,	// nro de face
						  vec3 O,  //Ray origin
						  vec3 D,  //Ray direction
						  float *U,
						  float *V)
{
	vec3 e1 = F[i].e1 , e2=F[i].e2;  //Edge1, Edge2
	vec3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Begin calculating determinant - also used to calculate u parameter
	P = cross( D, e2);
	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	det = dot(e1, P);
	// if(det > -EPSILON && det < EPSILON) return 0;			// no deberia pasar asi que no lo chequeo
	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = O-F[i].v[0];

	//Calculate u parameter and test bound
	u = dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	// if(u < 0.f || u > 1.f) return 0;

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate V parameter and test bound
	v = dot(D, Q) * inv_det;
	//The intersection lies outside of the triangle
	//if(v < 0.f || u + v  > 1.f) return 0;
	t = dot(e2, Q) * inv_det;

	*U = u;
	*V = v;
}


bool box_intersection(vec3 pMin, vec3 pMax,
					  vec3 O,	//Ray origin
					  vec3 D,	//Ray direction
					  float* tn, float *tf,
					  float mint, float maxt)
{

	float t0 = mint, t1 = maxt;
	for (int i = 0; i < 3; ++i) {
		// Update interval for _i_th bounding box slab
		float invRayDir = 1.f / D[i];
		float tNear = (pMin[i] - O[i]) * invRayDir;
		float tFar  = (pMax[i] - O[i]) * invRayDir;

		// Update parametric interval from slab intersection $t$s
		if (tNear > tFar) swap(&tNear, &tFar);
		t0 = tNear > t0 ? tNear : t0;
		t1 = tFar  < t1 ? tFar  : t1;
		if (t0 > t1) return false;
	}
	*tn = t0;
	*tf = t1;
	return true;
}



int box_intersection2(vec3 A, 
					 vec3 B,						  
					 vec3 O,	//Ray origin
					 vec3 D,	//Ray direction
					 float* tn, float *tf)
{
	bool fl = true;
	float t0,t1;
	float tnear = -100000;
	float tfar = 100000;

	// planos X
	if(fabs(D.x)<EPSILON)
	{
		// el rayo es paralelo al eje de las X
		if(O.x<A.x || O.x>B.x)
			fl = false;		// pasa antes o despues del BOX
	}
	else
	{
		// calculo la distancia a los planos X
		t0 = (A.x-O.x)/D.x;
		t1 = (B.x-O.x)/D.x;
		if(t0>t1)
			swap(&t0,&t1);
		if(t0>tnear)	// busco el mayor tnear
			tnear = t0;
		if(t1<tfar)		// busco el menor tfar
			tfar = t1;
		if(tnear>tfar || tfar<0)
			fl = false;	// el rayo no toca la caja
	}

	// planos Y
	if(fl)
	{
		if(fabs(D.y)<EPSILON)
		{
			// el rayo es paralelo al plano
			if(O.y<A.y || O.y>B.y)
				fl = false;		// pasa antes o despues del BOX
		}
		else
		{
			// calculo la distancia a los planos X
			t0 = (A.y-O.y)/D.y;
			t1 = (B.y-O.y)/D.y;
			if(t0>t1)
				swap(&t0,&t1);
			if(t0>tnear)	// busco el mayor tnear
				tnear = t0;
			if(t1<tfar)		// busco el menor tfar
				tfar = t1;
			if(tnear>tfar || tfar<0)
				fl = false;	// el rayo no toca la caja
		}
	}

	// planos Z
	if(fl)
	{
		if(fabs(D.z)<EPSILON)
		{
			// el rayo es paralelo al plano
			if(O.z<A.z || O.z>B.z)
				fl = false;		// pasa antes o despues del BOX
		}
		else
		{
			// calculo la distancia a los planos X
			t0 = (A.z-O.z)/D.z;
			t1 = (B.z-O.z)/D.z;
			if(t0>t1)
				swap(&t0,&t1);
			if(t0>tnear)	// busco el mayor tnear
				tnear = t0;
			if(t1<tfar)		// busco el menor tfar
				tfar = t1;
			if(tnear>tfar || tfar<0)
				fl = false;	// el rayo no toca la caja
		}
	}

	if(fl && tnear>0)
	{	
		//ray intersection
		*tn = tnear;
		*tf = tfar;
		return 1;
	}

	// No hit, no win
	return 0;

}


void CEngine::loadScene()
{
	FILE *fp = fopen("c:/leptonpack/escena.ts" , "rt");
	if(!fp)
		return;

	char buffer[255];
	char fname[MAX_PATH];

	fgets(buffer,sizeof(buffer),fp);
	LF.x = atof(buffer);
	fgets(buffer,sizeof(buffer),fp);
	LF.y = atof(buffer);
	fgets(buffer,sizeof(buffer),fp);
	LF.z = atof(buffer);
	fgets(buffer,sizeof(buffer),fp);
	LA.x = atof(buffer);
	fgets(buffer,sizeof(buffer),fp);
	LA.y = atof(buffer);
	fgets(buffer,sizeof(buffer),fp);
	LA.z = atof(buffer);

	fgets(buffer,sizeof(buffer),fp);
	cant_luces = atoi(buffer);
	for(int i=0;i<cant_luces;++i)
	{
		fgets(buffer,sizeof(buffer),fp);
		luces[i].enabled = atoi(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Direction.x = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Direction.y = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Direction.z = atof(buffer);

		fgets(buffer,sizeof(buffer),fp);
		luces[i].Position.x = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Position.y = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Position.z = atof(buffer);

		fgets(buffer,sizeof(buffer),fp);
		luces[i].Diffuse.x = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Diffuse.y = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Diffuse.z = atof(buffer);

		fgets(buffer,sizeof(buffer),fp);
		luces[i].Power = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Type = atoi(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Phi = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].Theta = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		luces[i].wlight = atof(buffer);

	}

	fgets(buffer,sizeof(buffer),fp);
	cant_mat = atoi(buffer);
	for(int i=0;i<cant_mat;++i)
	{

		memset(&materiales[i],0,sizeof(st_material));
		fgets(fname,sizeof(fname),fp);

		materiales[i].nro_texture = CreateTexture(fname);
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].bmp_k = atoi(buffer);

		fgets(buffer,sizeof(buffer),fp);
		materiales[i].Diffuse.x = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].Diffuse.y = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].Diffuse.z = atof(buffer);

		fgets(buffer,sizeof(buffer),fp);
		materiales[i].k0 = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].k1 = atof(buffer);
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].kr = atof(buffer);
		materiales[i].kr = 0.5;
		fgets(buffer,sizeof(buffer),fp);
		materiales[i].kt = atof(buffer);

	}

	fgets(buffer,sizeof(buffer),fp);
	cant_faces = atoi(buffer);
	F = new face3d[cant_faces];
	float min_x = 100000;
	float min_y = 100000;
	float min_z = 100000;
	float max_x = -100000;
	float max_y = -100000;
	float max_z = -100000;
	for(int i=0;i<cant_faces;++i)
	{
		fgets(buffer,sizeof(buffer),fp);
		F[i].layer = atoi(buffer);
		for(int j=0;j<3;++j)
		{
			// vertex position
			fgets(buffer,sizeof(buffer),fp);
			float x = F[i].v[j].x = atof(buffer);
			fgets(buffer,sizeof(buffer),fp);
			float y = F[i].v[j].y = atof(buffer);
			fgets(buffer,sizeof(buffer),fp);
			float z = F[i].v[j].z = atof(buffer);

			// text coords
			fgets(buffer,sizeof(buffer),fp);
			F[i].tu[j] = atof(buffer);
			fgets(buffer,sizeof(buffer),fp);
			F[i].tv[j] = atof(buffer);

			// vertex normal
			fgets(buffer,sizeof(buffer),fp);
			F[i].n[j].x = atof(buffer);
			fgets(buffer,sizeof(buffer),fp);
			F[i].n[j].y = atof(buffer);
			fgets(buffer,sizeof(buffer),fp);
			F[i].n[j].z = atof(buffer);

			// VERTEX color
			fgets(buffer,sizeof(buffer),fp);
			COLORREF clr = atof(buffer);
			
			F[i].color[j] = vec3((float)GetRValue(clr)/255.0,(float)GetGValue(clr)/255.0,(float)GetBValue(clr)/255.0);

			if(x<min_x)
				min_x = x;
			if(y<min_y)
				min_y = y;
			if(z<min_z)
				min_z = z;
			if(x>max_x)
				max_x = x;
			if(y>max_y)
				max_y = y;
			if(z>max_z)
				max_z = z;
		}
	}

	float rx = (max_x + min_x)/2;
	float ry = (max_y + min_y)/2;
	float rz = (max_z + min_z)/2;

	float dx = (max_x - min_x)/2;
	float dy = (max_y - min_y)/2;
	float dz = (max_z - min_z)/2;

	bb_min = vec3(-dx,-dy,-dz);
	bb_max = vec3(dx,dy,dz);

	fclose(fp);
}




float CEngine::best_split(int eje,kd_node *p_node)
{
	//if(p_node->deep>=10)
		//return (p_node->p_min[eje] + p_node->p_max[eje])/2.0f;

	// uso SAH para determinar el mejor plano donde dividir el nodo
	float icost = 80;			// costo de computar una interseccion
	float itrav = 1;			// costo de atrevesar el nodo

	
	// superficie del nodo (tengo que sumar las 6 caras)
	vec3 dim = p_node->p_max - p_node->p_min;
	float invST = 1.0 / (2.0* (dim.x*dim.y + dim.x*dim.z + dim.y*dim.z));		// inverso de la superficie total

	// costo asociado al nodo
	//float oldCost = icost * p_node->cant_f;

	// pruebo dividir en N partes iguales (maximo 50)
	// y selecciono desde la mitad - dim/4, hasta la mitad + dim/4
	int N = 10;
	float splanes[50];
	int cant_splanes = 0;
	float aux_d = dim[eje]/2.0; 
	float s0 = p_node->p_min[eje] + aux_d/2.0;
	float ds = aux_d / (float)N;
	for(int i=0;i<N;++i)
		splanes[cant_splanes++] = (s0+=ds);


	float minCost = FLT_MAX;
	float best_s = -1;
	for(int n=0;n<cant_splanes;++n)
	{
		float s = splanes[n];
		// verifico que no sea igual o muy similar al anterior
		if(n && fabs(s-splanes[n-1])<1)
			continue;

		// clasifico las primitivas con respecto a s 
		int cantL = 0, cantR = 0;
		for(int i=0;i<p_node->cant_f;++i)
		{
			face3d *face = &F[p_node->p_list[i]];
			float pmin = face->pmin[eje];
			float pmax = face->pmax[eje];
			if(pmax>=s)
				cantR++;
			if(pmin<=s)
				cantL++;
		}

		// computo el costo de dividir el nodo por este plano
		// computo la superficie del hemiplano left
		// eje es el que estoy analizando, eje1 y eje2 son los otros 2 ejes. 
		int eje0 = (eje + 1) % 3, eje1 = (eje + 2) % 3;
		float supL = 2 * (dim[eje0] * dim[eje1] + (s - p_node->p_min[eje]) * (dim[eje0] + dim[eje1]));
		float supR = 2 * (dim[eje0] * dim[eje1] + (p_node->p_max[eje] - s) *(dim[eje0] + dim[eje1]));
		float pL = supL*invST;			// prob de que el rayo atraviese el hemiplano Left
		float pR = supR*invST;			// prob de que el rayo atraviese el hemiplano rigth
		float eb = (cantL==0 || cantR==0) ? 0.5f : 0.f;  // empty node bonus
		float cost = icost *(1.f - eb) * (pL*cantL + pR*cantR);

		if(cost<minCost)
		{
			minCost = cost;
			best_s = s;
		}
	}
	
	return best_s;
}



int CEngine::best_split(kd_node *p_node,float *best_s)
{
	// uso SAH para determinar el mejor plano donde dividir el nodo
	float icost = 80;			// costo de computar una interseccion
	float itrav = 1;			// costo de atrevesar el nodo


	// superficie del nodo (tengo que sumar las 6 caras)
	vec3 dim = p_node->p_max - p_node->p_min;
	float invST = 1.0 / (2.0* (dim.x*dim.y + dim.x*dim.z + dim.y*dim.z));		// inverso de la superficie total

	// costo asociado al nodo
	float oldCost = icost * p_node->cant_f;

	float minCost = FLT_MAX;
	int best_axis = -1;

	// empiezo con el eje que mayor dimension tiene
	int ejes[3];
	if(fabs(dim.x)>=fabs(dim.z) && fabs(dim.x)>=fabs(dim.y))
		ejes[0] = 0 , ejes[1] = 1,  ejes[2] = 2;			// split X
	else
	if(fabs(dim.y)>=fabs(dim.x) && fabs(dim.y)>=fabs(dim.z))
		ejes[0] = 1 , ejes[1] = 0,  ejes[2] = 2;			// split Y
	else
		ejes[0] = 2 , ejes[1] = 0,  ejes[2] = 1;			// split Z


	for(int I=0;I<3;++I)
	{
		int eje = ejes[I];

		// pruebo dividir en N partes iguales (maximo 50)
		// y selecciono desde la mitad - dim/4, hasta la mitad + dim/4
		float splanes[50];
		int cant_splanes = 0;
		int N = 15;
		float aux_d = dim[eje]/2.0; 
		float s0 = p_node->p_min[eje] + aux_d/2.0;
		float ds = aux_d / (float)N;
		for(int i=0;i<N;++i)
			splanes[cant_splanes++] = (s0+=ds);

		for(int n=0;n<cant_splanes;++n)
		{
			float s = splanes[n];

			// clasifico las primitivas con respecto a s 
			int cantL = 0, cantR = 0;
			for(int i=0;i<p_node->cant_f;++i)
			{
				face3d *face = &F[p_node->p_list[i]];
				float pmin = face->pmin[eje];
				float pmax = face->pmax[eje];
				if(pmax>=s)
					cantR++;
				if(pmin<=s)
					cantL++;
			}

			// computo el costo de dividir el nodo por este plano
			// computo la superficie del hemiplano left
			// eje es el que estoy analizando, eje1 y eje2 son los otros 2 ejes. 
			int eje0 = (eje + 1) % 3, eje1 = (eje + 2) % 3;
			float supL = 2 * (dim[eje0] * dim[eje1] + (s - p_node->p_min[eje]) * (dim[eje0] + dim[eje1]));
			float supR = 2 * (dim[eje0] * dim[eje1] + (p_node->p_max[eje] - s) *(dim[eje0] + dim[eje1]));
			float pL = supL*invST;			// prob de que el rayo atraviese el hemiplano Left
			float pR = supR*invST;			// prob de que el rayo atraviese el hemiplano rigth
			float eb = (cantL==0 || cantR==0) ? 0.5f : 0.f;  // empty node bonus
			float cost = icost *(1.f - eb) * (pL*cantL + pR*cantR);

			if(cost<minCost)
			{
				minCost = cost;
				best_axis = eje;
				*best_s = s;
			}
		}
	}

	return minCost<oldCost ? best_axis : -1;
}


/*
// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
int
	intersect3D_RayTriangle( Ray R, Triangle T, Point* I )
{
	Vector    u, v, n;              // triangle vectors
	Vector    dir, w0, w;           // ray vectors
	float     r, a, b;              // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = T.V1 - T.V0;
	v = T.V2 - T.V0;
	n = u * v;              // cross product
	if (n == (Vector)0)             // triangle is degenerate
		return -1;                  // do not deal with this case

	dir = R.P1 - R.P0;              // ray direction vector
	w0 = R.P0 - T.V0;
	a = -dot(n,w0);
	b = dot(n,dir);
	if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
		if (a == 0)                 // ray lies in triangle plane
			return 2;
		else return 0;              // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	if (r < 0.0)                    // ray goes away from triangle
		return 0;                   // => no intersect
	// for a segment, also test if (r > 1.0) => no intersect

	*I = R.P0 + r * dir;            // intersect point of ray and plane

	// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	uu = dot(u,u);
	uv = dot(u,v);
	vv = dot(v,v);
	w = *I - T.V0;
	wu = dot(w,u);
	wv = dot(w,v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)         // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                       // I is in T
}


*/

int CEngine::CreateTexture(const char *fname)
{
	return texturas[cant_texturas].CreateFromFile(fname) ? cant_texturas++ : -1;
}


// helper loader
// [-100.00,-100.00,-0.00]
// -100.00,-100.00,-0.00
bool ld_vec3(FILE *fp, vec3 &v)
{
	char buff[255];
	fgets(buff,sizeof(buff),fp);

	// si el primero es un [ lo salteo
	char *p = buff;
	if(*p=='[')
		p++;
	v.x = atof(p++);

	// busco la , 
	while(*p && *p++!=',');
	if(*p)
		v.y = atof(p++);
	else
		return false;

	// busco la , 
	while(*p && *p++!=',');
	if(*p)
		v.z = atof(p++);
	else
		return false;

	return true;
}

// "c:/leptonpack/texturas/wood6.bmp",2000
bool ld_tx_rec(FILE *fp, char *fname,float &k)
{
	char buff[255];
	fgets(buff,sizeof(buff),fp);

	// busco la "
	char *p = buff;
	while(*p && *p++!='\"');
	if(*p)
	{
		char *q = fname;
		while(*p && *p!='\"')
			*q++ = *p++;
		*q++ = '\0';
	}
	else
		return false;

	// busco la , 
	while(*p && *p++!=',');
	if(*p)
		k = atof(p++);
	else
		return false;

	return true;
}


float CEngine::ComputeShadow(int i,vec3 P)
{

	return 1;
	if(!luces[i].enabled)
		return 1;

	// Point light
	//if(shadow_ray_intersects(SO,SD))
	//	clr.x*=0.1 , clr.y*=0.1, clr.z*=0.1;

	vec3 O = luces[i].Position;
	vec3 D = O-P;
	D.normalize();
	
	// area light
	vec3 V = cross(D,VUP);
	V.normalize();
	vec3 U = cross(V,D);

	return adaptative_sampling(O,P,U,V,25);

}


float CEngine::adaptative_sampling(vec3 O,vec3 P,vec3 &U ,vec3 &V,float dw)
{
	// agrego fuentes luminosas puntuales (virtual point lights) en el punto central y en los 4 extremos
	int cant_s = 0;
	int cant_vpl=0;
	vec3 VPL[100];
	VPL[cant_vpl++] = O;
	VPL[cant_vpl++] = O + V*dw + U*dw;
	VPL[cant_vpl++] = O + V*dw - U*dw;
	VPL[cant_vpl++] = O - V*dw + U*dw;
	VPL[cant_vpl++] = O - V*dw - U*dw;
	// y otros n aleatoreos
	for(int t=0;t<6;++t)
	{
		float r0 = 2*dw*(rand()/(float)RAND_MAX-0.5);
		float r1 = 2*dw*(rand()/(float)RAND_MAX-0.5);
		VPL[cant_vpl++] = O + V*r0 + U*r1;
	}

	for(int t=0;t<cant_vpl;++t)
	{
		vec3 D = VPL[t]- P;
		float tfar = D.length();
		D = D * (1/tfar);
		cant_s += shadow_ray_intersects(P,D,1.0,tfar);
	}

	if(dw<4)
		return 1.0 - (float)cant_s / (float)cant_vpl;
	else
	if(cant_s==0)
		return 1;
	else
	if(cant_s==cant_vpl)
		return 0;

	// llamo en forma recursiva donde mas lo necesito: 
	dw /= 2;
	return	
		adaptative_sampling(O-V*dw-U*dw,P,U ,V,dw)*0.25 +  
		adaptative_sampling(O-V*dw+U*dw,P,U ,V,dw)*0.25 +
		adaptative_sampling(O+V*dw-U*dw,P,U ,V,dw)*0.25 +
		adaptative_sampling(O+V*dw+U*dw,P,U ,V,dw)*0.25 ;

}




// luz puntual, emite luz hacia todas las direcciones
void CEngine::ComputePointLight(int i, vec3 Pos,vec3 N,float &ld, float &ls)
{
	

	float k_shadow = ComputeShadow(i,Pos);
	if(k_shadow<0.001)
		return;			// el punto esta en sombras, no hace falta computar la ecuacion de phong

	// 1- calculo la luz diffusa
	vec3 LD= luces[i].Position - Pos;
	LD.normalize();
	float cf = dot(N,LD);
	ld = saturate(cf)*k_ld*k_shadow;

	// 2- calcula la reflexion specular
	float mat_ks = 0.7;			// todo: llevar a material
	ls = 0;
	if(mat_ks>0)
	{
		vec3 D = Pos-LF;
		D.normalize();
		float ks = saturate(dot(reflect(LD,N), D));
		ks = pow(ks,10);
		ls += ks*mat_ks*k_ls*k_shadow;
	}
}

void CEngine::ComputeSpotLight(int i, vec3 Pos,vec3 N,float &ld, float &ls)
{
	// caso b: spot light, emite luz en una direccion especifica
	ld = 0;
	ls = 0;

	vec3 vLight = luces[i].Position - Pos;
	vLight.normalize();

	float mat_ks = 0.7;			// todo: llevar a material

	float cono = dot( vLight, luces[i].Direction);
	// Verifico si el punto cae sobre el cono de luz
	if( cono > luces[i].Phi)
	{
		// es iluminado por la luz
		float K = 1;
		if( cono <= luces[i].Theta)
			// Fall off
			K = (cono-luces[i].Phi) / (luces[i].Theta-luces[i].Phi);

		float k_shadow = ComputeShadow(i,Pos);
		if(k_shadow<0.001)
			return;			// el punto esta en sombras, no hace falta computar la ecuacion de phong

		K*=k_shadow;

		vec3 LD = vLight*(-1);
		float cf = dot(LD,N);
		ld = saturate(cf)*k_ld*K;

		// 2- calcula la reflexion specular
		if(mat_ks>0)
		{
			vec3 D = Pos-LF;
			D.normalize();
			float ks = saturate(dot(reflect(LD,N), D));
			ks = pow(ks,10);
			ls = ks*mat_ks*k_ls*K;
		}
	}
	
}


void CEngine::ComputeLighting(vec3 Pos,vec3 N,vec3 &Diffuse,vec3 &Specular)
{
	Diffuse = Specular = vec3(0,0,0);
	for(int i=0;i<cant_luces;++i)
	if(luces[i].enabled)
	{

		// computo diffuse y especular 
		float kd,ks;
		//float2 L = g_LightType[i]==2?ComputeSpotLight(i,Pos,N):ComputePointLight(i,Pos,N);
		//ComputeSpotLight(i,Pos,N,kd,ks);
		ComputePointLight(i,Pos,N,kd,ks);

		// intensidad de la fuente luminosa 
		float Intensidad = 1;
		vec3 lightColor = luces[i].Diffuse;

		// al diffuse lo clampeo entre 0 y 1
		Diffuse = Diffuse + saturate(lightColor*(Intensidad*kd));
		// el especular puede pasarse de 1 
		Specular = Specular + lightColor*(Intensidad*ks);
	}
}
