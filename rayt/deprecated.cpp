


bool CEngine::interseccion(vec3 O,vec3 D,ip_data *I)
{
	float R = 10000000;
	vec3 MD = D*(-1);
	float bc_b;
	float bc_g;
	int nro_face = -1;

	SSE_O = _mm_setr_ps(O.x, O.y, O.z, 0);
	SSE_D = _mm_setr_ps(D.x, D.y, D.z, 0);

	ray_O = O;
	ray_D = D;


	// Recorro todos los objetos, y determino la interseccion
	// entre el rayo de luz y la superficie del objeto

	for(int i=0;i<cant_faces;++i)
	{
		vec3 A = F[i].v[0];
		vec3 B = F[i].v[1];
		vec3 C = F[i].v[2];
		float b,g,t;
		// version SSE 
		if(triangle_ray_SSE(i,&t,&b,&g))
		//if(triangle_ray(i,O,D,&t,&b,&g))
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
			if(ray_intersects(LF,D,&I))
				pDC->SetPixel(x,y,RGB(255*I.bc_b, 255*I.bc_g, 0));
			else
				pDC->SetPixel(x,y,RGB(0,0,0));

		}
	}
}

// debug kdtree
	// debug
	if(0)
	{
		ox = 550; 
		oy = 300;
		ex = 1000 / (bb_max.x-bb_min.x) * 0.7;
		ey = 600 / (bb_max.y-bb_min.y)* 0.7;

		CPen pen,pen2,*penOld;
		pen.CreatePen(PS_SOLID,3,RGB(100,100,0));
		pen2.CreatePen(PS_SOLID,1,RGB(0,0,0));

		penOld = pDC->SelectObject(&pen);
		pDC->FillSolidRect(0,0,2000,2000,RGB(240,240,240));
		pDC->SetTextColor(RGB(255,255,255));
		char saux[255];
		sprintf(saux,"CantF = %d",cant_faces);
		pDC->TextOut(0,0,saux,strlen(saux));
		debugKDTree(pDC,kd_tree,20,20);


		pDC->SelectObject(&pen2);
		for(int i=0;i<cant_faces;++i)
		{
			pDC->MoveTo(ox + F[i].v[0].x * ex ,oy + F[i].v[0].y * ey);
			pDC->LineTo(ox + F[i].v[1].x * ex ,oy + F[i].v[1].y * ey);
			pDC->LineTo(ox + F[i].v[2].x * ex ,oy + F[i].v[2].y * ey);
			pDC->LineTo(ox + F[i].v[0].x * ex ,oy + F[i].v[0].y * ey);
		}
		pDC->SelectObject(penOld);

		return;
	}
void CEngine::loadMesh()
{
	FILE *fp = fopen("c:/leptonpack/escena.dat" , "rt");
	if(!fp)
		return;
	char buffer[255];

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
	
	/*
	for(int i=0;i<cant_faces;++i)
	{
		for(int j=0;j<3;++j)
		{
			F[i].v[j].x -= rx;
			F[i].v[j].y -= ry;
			F[i].v[j].z -= rz;
		}
	}
	*/

	fclose(fp);
}

/*
bool CEngine::interseccionKDTreeAnt(vec3 O,vec3 D,ip_data *I)
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
	return false;
}
*/	


/*
	int rs = 5;
	float tot_s = (2*rs+1)*(2*rs+1);
	for(int sx=-rs;sx<=rs;++sx)
		for(int sy=-rs;sy<=rs;++sy)
		{
			float lx = sx*rand()/(float)RAND_MAX*100;
			float ly = sy*rand()/(float)RAND_MAX*100;
			vec3 FL = luces[i].Position + V*lx + U*ly;
			vec3 aux_D = FL - P;
			aux_D.normalize();
			if(shadow_ray_intersects(P,aux_D))
				++cant_s;
		}
	*/

