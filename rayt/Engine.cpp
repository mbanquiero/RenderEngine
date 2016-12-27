#include "stdafx.h"
#include "Engine.h"

// Agregar esto para debug 
//#define DEBUG_NEW new(__FILE__, __LINE__)
//#define new DEBUG_NEW

CEngine::CEngine()
{
	cant_faces = 0;
	F = NULL;
	p_root = NULL;


	VUP = vec3(0,0,1);				// look up vector
	LF = vec3(-3000,3000,2000);
	LA = vec3(0,0,0);
	fov = 0.9;
	W = 800;
	H = 600;


	clr_layer[0] = RGB(255,0,0);
	clr_layer[1] = RGB(0,255,0);
	clr_layer[2] = RGB(0,0,255);
	clr_layer[3] = RGB(255,0,255);

	clr_layer[4] = RGB(255,100,0);
	clr_layer[5] = RGB(0,255,100);
	clr_layer[6] = RGB(100,100,255);
	clr_layer[7] = RGB(255,100,255);

}


CEngine::~CEngine()
{
	SAFE_DELETE(F);
	if(p_root!=NULL)
		deleteOctreeNode(p_root);
	if(kd_tree!=NULL)
		deleteKDTreeNode(kd_tree);

}

void CEngine::initFromTest()
{

	loadMesh();
	//quadTest();

	
	/*
	F = new face3d[100000];
	for(int k=-3;k<3;++k)
		for(int i=-3;i<3;++i)
		for(int j=-3;j<3;++j)
		createBox(vec3(i*300,j*300,k*300) , vec3(100,100,100));
	*/
	

	precalc();


	// createOctree();
	createKDTree();
}

void CEngine::quadTest()
{
	cant_faces = 2;
	F = new face3d[cant_faces];

	F[0].v[0] = vec3(0,0,0);
	F[0].v[1] = vec3(0,0,1000);
	F[0].v[2] = vec3(0,1000,0);

	F[1].v[0] = vec3(0,0,1000);
	F[1].v[1] = vec3(0,1000,1000);
	F[1].v[2] = vec3(0,1000,0);
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
	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dx + dy;
	F[cant_faces].v[2] = pos + dx;
	cant_faces++;

	// Cara de arriba
	F[cant_faces].v[0] = pos + dz;
	F[cant_faces].v[1] = pos + dy + dz;
	F[cant_faces].v[2] = pos + dx + dy + dz;
	cant_faces++;

	F[cant_faces].v[0] = pos + dz;
	F[cant_faces].v[1] = pos + dx + dy + dz;
	F[cant_faces].v[2] = pos + dx+ dz;
	cant_faces++;

	// Cara de izquierda
	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dz;
	F[cant_faces].v[2] = pos + dx + dz;
	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dx + dz;
	F[cant_faces].v[2] = pos + dx;
	cant_faces++;

	// Cara de derecha
	F[cant_faces].v[0] = pos + dy;
	F[cant_faces].v[1] = pos + dz + dy;
	F[cant_faces].v[2] = pos + dx + dz + dy; 
	cant_faces++;

	F[cant_faces].v[0] = pos + dy;
	F[cant_faces].v[1] = pos + dx + dz + dy;
	F[cant_faces].v[2] = pos + dx + dy;
	cant_faces++;

	// Cara de atras
	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dz;
	F[cant_faces].v[2] = pos + dy + dz;
	cant_faces++;

	F[cant_faces].v[0] = pos;
	F[cant_faces].v[1] = pos + dy + dz;
	F[cant_faces].v[2] = pos + dy;
	cant_faces++;

	// Cara de adelante
	F[cant_faces].v[0] = pos + dx;
	F[cant_faces].v[1] = pos + dz + dx;
	F[cant_faces].v[2] = pos + dy + dz + dx;
	cant_faces++;

	F[cant_faces].v[0] = pos + dx;
	F[cant_faces].v[1] = pos + dy + dz + dx;
	F[cant_faces].v[2] = pos + dy + dx;
	cant_faces++;

}


// precalculos x face
void CEngine::precalc()
{
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
	}
}



void CEngine::createOctree()
{
	//vec3 size = vec3(8000,8000,5000);
	p_root = createOctreeNode(bb_min,bb_max-bb_min, 0);
	int cant = countOctreeNodes(p_root);


	for(int i=0;i<cant_faces;++i)
	{
		// pila de busqueda
		int p_stack = 0;
		st_node *S[10000];
		S[p_stack++] = p_root;
		while(p_stack)
		{
			// saco el nodo de la pila
			st_node *p_node = S[--p_stack];
			// chequeo la interseccion
			if(box_overlap(F[i].pmin , F[i].pmax , p_node->p_min , p_node->p_max))
			{
				if(p_node->deep==max_deep)
				{
					// node leaf: le agrego esta cara
					if(p_node->p_list==NULL)
					{
						// lazzy alloc
						p_node->p_list = new int[MAX_FACE_X_NODE];
						p_node->cant_f = 0;
					}
					if(p_node->cant_f<MAX_FACE_X_NODE-1)
						p_node->p_list[p_node->cant_f++] = i;
				}
				else
				{
					// la cara puede estar en cualquiera de las hijas
					for(int j=0;j<8;++j)
						if(p_node->p[j]!=NULL)
							S[p_stack++] = p_node->p[j];
				}
			}
		}
		// paso a la siguiente cara
	}
}

st_node *CEngine::createOctreeNodeOld(vec3 pmin, vec3 dim,int deep)
{
	st_node *p_node = new st_node;
	memset(p_node , 0 , sizeof(st_node));
	p_node->deep = deep;

	/*
	if(deep<MAX_DEEP)
	{
		p_node->p_min = pmin;
		p_node->p_max = pmin + dim;

		dim = dim *0.5;
		vec3 dx = vec3(dim.x , 0, 0);
		vec3 dy = vec3(0 , dim.y, 0);
		vec3 dz = vec3(0 , 0, dim.z );
		p_node->p[0] = createOctreeNode(pmin , dim, deep+1);
		p_node->p[1] = createOctreeNode(pmin+dx , dim, deep+1);
		p_node->p[2] = createOctreeNode(pmin+dy , dim, deep+1);
		p_node->p[3] = createOctreeNode(pmin+dx+dy , dim, deep+1);

		p_node->p[4] = createOctreeNode(pmin+dz , dim, deep+1);
		p_node->p[5] = createOctreeNode(pmin+dx+dz , dim, deep+1);
		p_node->p[6] = createOctreeNode(pmin+dy+dz , dim, deep+1);
		p_node->p[7] = createOctreeNode(pmin+dx+dy+dz , dim, deep+1);
	}
	else
	{
		// leaf = p_node->deep == MAX_DEEP
		dim = dim *0.25;
		p_node->p_min = pmin+dim;
		p_node->p_max = pmin+dim+dim;
	}
	*/

	p_node->p_min = pmin;
	p_node->p_max = pmin + dim;
	bool leave = true;
	int cant_f = 0;

	if(deep<max_deep)
	{
		for(int i=0;i<cant_faces && cant_f < MAX_FACE_X_NODE;++i)
		{
			if(box_overlap(F[i].pmin , F[i].pmax , p_node->p_min , p_node->p_max))
				cant_f++;

		}
		leave = cant_f < MAX_FACE_X_NODE ? true : false;
	}
	else
	{
		for(int i=0;i<cant_faces;++i)
		{
			if(box_overlap(F[i].pmin , F[i].pmax , p_node->p_min , p_node->p_max))
				cant_f++;
		}
	}

	if(leave)
	{
		// node leaf
		p_node->p_list = new int[cant_f];
		p_node->cant_f = cant_f;
		cant_f = 0;

		for(int i=0;i<cant_faces;++i)
			if(box_overlap(F[i].pmin , F[i].pmax , p_node->p_min , p_node->p_max))
				p_node->p_list[cant_f++] = i;
	}
	else
	{
		// demasiadas caras en el nodo, tengo que dividirlo
		p_node->p_min = pmin;
		p_node->p_max = pmin + dim;

		dim = dim *0.5;
		vec3 dx = vec3(dim.x , 0, 0);
		vec3 dy = vec3(0 , dim.y, 0);
		vec3 dz = vec3(0 , 0, dim.z );
		p_node->p[0] = createOctreeNode(pmin , dim, deep+1);
		p_node->p[1] = createOctreeNode(pmin+dx , dim, deep+1);
		p_node->p[2] = createOctreeNode(pmin+dy , dim, deep+1);
		p_node->p[3] = createOctreeNode(pmin+dx+dy , dim, deep+1);

		p_node->p[4] = createOctreeNode(pmin+dz , dim, deep+1);
		p_node->p[5] = createOctreeNode(pmin+dx+dz , dim, deep+1);
		p_node->p[6] = createOctreeNode(pmin+dy+dz , dim, deep+1);
		p_node->p[7] = createOctreeNode(pmin+dx+dy+dz , dim, deep+1);
	}

	return p_node;

}



st_node *CEngine::createOctreeNode(vec3 pmin, vec3 dim,int deep)
{
	st_node *p_node = new st_node;
	memset(p_node , 0 , sizeof(st_node));
	p_node->deep = deep;
	p_node->p_min = pmin;
	p_node->p_max = pmin + dim;

	if(deep<max_deep)
	{
		dim = dim *0.5;
		vec3 dx = vec3(dim.x , 0, 0);
		vec3 dy = vec3(0 , dim.y, 0);
		vec3 dz = vec3(0 , 0, dim.z );
		p_node->p[0] = createOctreeNode(pmin , dim, deep+1);
		p_node->p[1] = createOctreeNode(pmin+dx , dim, deep+1);
		p_node->p[2] = createOctreeNode(pmin+dy , dim, deep+1);
		p_node->p[3] = createOctreeNode(pmin+dx+dy , dim, deep+1);

		p_node->p[4] = createOctreeNode(pmin+dz , dim, deep+1);
		p_node->p[5] = createOctreeNode(pmin+dx+dz , dim, deep+1);
		p_node->p[6] = createOctreeNode(pmin+dy+dz , dim, deep+1);
		p_node->p[7] = createOctreeNode(pmin+dx+dy+dz , dim, deep+1);
	}
	else
	{
		// nodo leave
		p_node->cant_f = 0;
	}

	return p_node;

}


// octree
void CEngine::deleteOctreeNode(st_node *p_node)
{
	for(int i=0;i<8;++i)
		if(p_node->p[i]!=NULL)
			deleteOctreeNode(p_node->p[i]);

	SAFE_DELETE(p_node->p_list);
	SAFE_DELETE(p_node);

}

int CEngine::countOctreeNodes(st_node *p_node)
{
	int cant = 1;
	for(int i=0;i<8;++i)
		if(p_node->p[i]!=NULL)
			cant += countOctreeNodes(p_node->p[i]);
	return cant;
}



void CEngine::createKDTree()
{
	max_deep = (8 + 1.3f * log(cant_faces));

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

	if(cant_f<MAX_FACE_X_NODE || deep>=max_deep)
	{
		// creo un nodo leaf
		p_node->cant_f = cant_f;
		p_node->p_list = p_list;
	}
	else
	{
		// divido el nodo en 2:
		vec3 dim = pmax - pmin;
		vec3 Lmin , Lmax , Rmin, Rmax;
		if(fabs(dim.z)>=fabs(dim.x) && fabs(dim.z)>=fabs(dim.y))
		{
			// splite Z
			float Z = (pmin.z + pmax.z) / 2.0;
			p_node->split = Z;
			p_node->split_plane = 2;
			Lmin = pmin;
			Lmax = vec3(pmax.x , pmax.y , Z);
			Rmin = vec3(pmin.x , pmin.y , Z);
			Rmax = pmax;
		}
		else
		if(fabs(dim.x)>=fabs(dim.z) && fabs(dim.x)>=fabs(dim.y))
		{
			// splite X
			float X = (pmin.x + pmax.x) / 2.0;
			p_node->split = X;
			p_node->split_plane = 0;

			Lmin = pmin;
			Lmax = vec3(X , pmax.y , pmax.z);
			Rmin = vec3(X , pmin.y , pmin.z);
			Rmax = pmax;

		}
		else
		{
			// splite Y
			float Y = (pmin.y + pmax.y) / 2.0;
			p_node->split = Y;
			p_node->split_plane = 1;
			Lmin = pmin;
			Lmax = vec3(pmax.x, Y , pmax.z);
			Rmin = vec3(pmin.x, Y , pmin.z);
			Rmax = pmax;
		}

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

		// libero la memoria original 
		if(p_list)
			delete []p_list;

		// creo 2 nodos
		p_node->p_left = createKDTreeNode(Lmin,Lmax,deep+1,cant_L,list_L);
		p_node->p_right = createKDTreeNode(Rmin,Rmax,deep+1,cant_R,list_R);

		// hago el nodo interior
		p_node->cant_f = cant_f;
		//p_node->cant_f = 0;

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
	int ox = 550; 
	int oy = 350;
	float ex = 1000 / (bb_max.x-bb_min.x);
	float ey = 600 / (bb_max.y-bb_min.y);

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


void CEngine::renderKDTree(CDC *pDC)
{

	// debug
	if(0)
	{
		pDC->FillSolidRect(0,0,2000,2000,RGB(240,240,240));
		pDC->SetTextColor(RGB(255,255,255));
		char saux[255];
		sprintf(saux,"CantF = %d",cant_faces);
		pDC->TextOut(0,0,saux,strlen(saux));
		debugKDTree(pDC,kd_tree,20,20);

		int ox = 550; 
		int oy = 350;
		float ex = 1000 / (bb_max.x-bb_min.x);
		float ey = 600 / (bb_max.y-bb_min.y);
		for(int i=0;i<cant_faces;++i)
		{
			pDC->MoveTo(ox + F[i].v[0].x * ex ,oy + F[i].v[0].y * ey);
			pDC->LineTo(ox + F[i].v[1].x * ex ,oy + F[i].v[1].y * ey);
			pDC->LineTo(ox + F[i].v[2].x * ex ,oy + F[i].v[2].y * ey);
			pDC->LineTo(ox + F[i].v[0].x * ex ,oy + F[i].v[0].y * ey);
		}

		return;
	}

	pDC->FillSolidRect(0,0,W,H,RGB(255,0,255));

	// camara simple
	N = (LA-LF);						// viewing direction
	N.normalize();
	V = cross(N,VUP);
	V.normalize();
	U = cross(V,N);
	double k = 2*tan(fov/2)/H;
	Dy = U*k;
	Dx = V*k;

	int x,y;
	for(x=0;x<W;++x)
	{
		for(y=0;y<H;++y)
		{
			//x = W/2;
			//y = H/2;
			// Direccion of each pixel
			vec3 D = N + Dy*(0.5*(H-2*y)) + Dx*(0.5*(2*x-W));
			D.normalize();

			// Ecuacion del rayo
			// Origen del rayo LF
			// Direccion del rayo D
			// r = LF + k*D
			ip_data I;
			if(interseccionKDTree(LF,D,&I))
			{
				pDC->SetPixel(x,y,RGB(255*I.bc_b, 255*I.bc_g, 0));
				/*
				int layer = F[I.nro_face].layer;
				float tu = F[I.nro_face].tu[0]*I.bc_b + F[I.nro_face].tu[1]*I.bc_g + F[I.nro_face].tu[2]*(1-I.bc_g-I.bc_b);
				float tv = F[I.nro_face].tv[0]*I.bc_b + F[I.nro_face].tv[1]*I.bc_g + F[I.nro_face].tv[2]*(1-I.bc_g-I.bc_b);
				BYTE R = GetRValue(clr_layer[layer % 8]) * tu;
				BYTE G = GetGValue(clr_layer[layer % 8]) * tv;
				BYTE B = GetBValue(clr_layer[layer % 8]);
				pDC->SetPixel(x,y,RGB(R,G,B));
				*/
			}
			else
				pDC->SetPixel(x,y,RGB(0,0,0));

		}
	}
}

bool CEngine::interseccionKDTree(vec3 O,vec3 D,ip_data *I)
{
	float R = 10000000;
	vec3 MD = D*(-1);
	float bc_b;
	float bc_g;
	int nro_face = -1;

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


	// lista de caras a chequear
	int aux_F[10000];
	int cant_f;

	// pila de busqueda
	int p_stack = 0;
	kd_node *S[100];
	S[p_stack++] = kd_tree;
	while(p_stack)
	{
		// saco el nodo de la pila
		kd_node *p_node = S[--p_stack];

		// chequeo la interseccion
		float tnear, tfar;
		if(box_intersection(p_node->p_min , p_node->p_max , O,D,&tnear,&tfar))
		{
			// el rayo atraviesa el nodo 
			if(p_node->p_list)
			{
				// nodo hoja: chequeo la interseccion con la lista de caras en dicho nodo
				for(int i=0;i<p_node->cant_f;++i)				
				{
					int n = p_node->p_list[i];
					vec3 A = F[n].v[0];
					vec3 B = F[n].v[1];
					vec3 C = F[n].v[2];
					float b,g,t;
					if(triangle_intersection( A,B,C,O,D,&t,&b,&g))
					{
						if(t>=tnear-1 && t<=tfar+1 && t<R)
						{
							// actualizo las coordenadas barycentricas
							bc_b = b;
							bc_g = g;
							R = t;
							nro_face = n;
						}
						else
						{
							int bp = 1;
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

			}
			else
			{
				// el rayo entra por tnear y sale por tfar
				// determino en que orden tengo que chequear los nodos
				int p = p_node->split_plane;
				float tplane = (p_node->split - ray_O[p])  * ray_invdir[p];
				// para procesar ambos nodos el tplane tiene que estar entre tnear y tfar
				bool pNear = true , pFar = true;
				if(tplane>tfar)
					pFar = false;		// no proceso el nodo mas lejano al origen del rayo 
				else
				if(tplane<tnear)
					pNear = false;		// no proceso el nodo mas cercano

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


				if(p_far!=NULL && pFar)
					S[p_stack++] = p_far;
				if(p_near!=NULL && pNear)
					S[p_stack++] = p_near;


			}
		}
	}


/*
	if(nro_face!=-1)
	{
		I->ip = O + D*R;
		I->t = R;
		I->bc_b = bc_b;
		I->bc_g = bc_g;
		I->nro_face = nro_face;
	}

	return nro_face!=-1?true:false;
	*/

	return false;
}




void CEngine::render(CDC *pDC)
{
	pDC->FillSolidRect(0,0,W,H,RGB(255,0,255));
	// camara simple
	N = (LA-LF);						// viewing direction
	N.normalize();
	V = cross(N,VUP);
	V.normalize();
	U = cross(V,N);
	double k = 2*tan(fov/2)/H;
	Dy = U*k;
	Dx = V*k;

	int x,y;
	for(x=0;x<W;++x)
	{
		for(y=0;y<H;++y)
		{
			//x = W/2;
			//y = H/2;
			// Direccion of each pixel
			vec3 D = N + Dy*(0.5*(H-2*y)) + Dx*(0.5*(2*x-W));
			D.normalize();

			// Ecuacion del rayo
			// Origen del rayo LF
			// Direccion del rayo D
			// r = LF + k*D
			ip_data I;
			if(interseccion(LF,D,&I))
				pDC->SetPixel(x,y,RGB(255*I.bc_b, 255*I.bc_g, 0));
			else
				pDC->SetPixel(x,y,RGB(0,0,0));

		}
	}
}


bool CEngine::interseccion(vec3 O,vec3 D,ip_data *I)
{
	float R = 10000000;
	vec3 MD = D*(-1);
	float bc_b;
	float bc_g;
	int nro_face = -1;

	// Recorro todos los objetos, y determino la interseccion
	// entre el rayo de luz y la superficie del objeto

	for(int i=0;i<cant_faces;++i)
	{
		vec3 A = F[i].v[0];
		vec3 B = F[i].v[1];
		vec3 C = F[i].v[2];
		float b,g,t;
		if(triangle_intersection( A,B,C,O,D,&t,&b,&g))
		{
			if(t>0 && t<R)
			{
				// actualizo las coordenadas barycentricas
				bc_b = b;
				bc_g = g;
				R = t;
				nro_face = i;
			}
		}

	}

	if(nro_face!=-1)
	{
		I->ip = O + D*R;
		I->t = R;
		I->bc_b = bc_b;
		I->bc_g = bc_g;
		I->nro_face = nro_face;
	}

	return nro_face!=-1?true:false;
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


// triangle - ray intersccion usando determinantes
	/*
	mat3 M(B-A,C-A,MD);
	float det = M.det();
	if(det!=0)
	{
		vec3 A0 = p0-A;
		float b,g,t;
		t = M.det(A0,3)/det;
		if(t>0 && t<R)
		{
			b = M.det(A0,1)/det;
			g = M.det(A0,2)/det;
			if(b>=0 && g>=0 && b+g<=1)
			{
				// actualizo las coordenadas barycentricas
				bc_b = b;
				bc_g = g;
				R = t;
				nro_face = i;
			}
		}
	}
	*/



void CEngine::renderOctree(CDC *pDC)
{
	pDC->FillSolidRect(0,0,W,H,RGB(255,0,255));
	// camara simple
	N = (LA-LF);						// viewing direction
	N.normalize();
	V = cross(N,VUP);
	V.normalize();
	U = cross(V,N);
	double k = 2*tan(fov/2)/H;
	Dy = U*k;
	Dx = V*k;

	int x,y;
	for(x=0;x<W;++x)
	{
		for(y=0;y<H;++y)
		{
			//x = W/2;
			//y = H/2;
			// Direccion of each pixel
			vec3 D = N + Dy*(0.5*(H-2*y)) + Dx*(0.5*(2*x-W));
			D.normalize();

			// Ecuacion del rayo
			// Origen del rayo LF
			// Direccion del rayo D
			// r = LF + k*D
			ip_data I;
			if(interseccionB(LF,D,&I))
				pDC->SetPixel(x,y,RGB(255*I.bc_b, 255*I.bc_g, 0));
			else
				pDC->SetPixel(x,y,RGB(0,0,0));

		}
	}
}


bool CEngine::interseccionB(vec3 O,vec3 D,ip_data *I)
{
	float R = 10000000;
	vec3 MD = D*(-1);
	float bc_b;
	float bc_g;
	int nro_face = -1;

	// lista de caras a chequear
	int aux_F[10000];
	int cant_f;

	// pila de busqueda
	int p_stack = 0;
	st_node *S[100];
	S[p_stack++] = p_root;
	while(p_stack)
	{
		// saco el nodo de la pila
		st_node *p_node = S[--p_stack];

		// chequeo la interseccion
		float tnear, tfar;
		if(box_intersection(p_node->p_min , p_node->p_max , O,D,&tnear,&tfar))
		{
			if(p_node->cant_f)
			{
				// nodo hoja: chequeo la interseccion con la lista de caras en dicho nodo
				for(int i=0;i<p_node->cant_f;++i)				
				{
					int n = p_node->p_list[i];
					vec3 A = F[n].v[0];
					vec3 B = F[n].v[1];
					vec3 C = F[n].v[2];
					float b,g,t;
					if(triangle_intersection( A,B,C,O,D,&t,&b,&g))
					{
						if(t>0 && t<R)
						{
							// actualizo las coordenadas barycentricas
							bc_b = b;
							bc_g = g;
							R = t;
							nro_face = n;
						}
					}
				}
			}
			else
			{
				for(int i=0;i<8;++i)
					if(p_node->p[i]!=NULL)
						S[p_stack++] = p_node->p[i];
			}
		}
	}


	if(nro_face!=-1)
	{
		I->ip = O + D*R;
		I->t = R;
		I->bc_b = bc_b;
		I->bc_g = bc_g;
		I->nro_face = nro_face;
	}

	return nro_face!=-1?true:false;
}


int box_intersection(vec3 A, 
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


void CEngine::loadMesh()
{
	FILE *fp = fopen("c:/leptonpack/escena.dat" , "rt");
	if(!fp)
		return;
	char buffer[255];
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
	
	for(int i=0;i<cant_faces;++i)
	{
		for(int j=0;j<3;++j)
		{
			F[i].v[j].x -= rx;
			F[i].v[j].y -= ry;
			F[i].v[j].z -= rz;
		}
	}

	fclose(fp);
}
