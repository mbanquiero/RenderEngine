#include "StdAfx.h"
#include "RenderEngine.h"
#include <math.h>

#pragma comment ( lib, "OpenGL32.lib" )
#pragma comment ( lib, "glew32.lib" )


CRenderEngine::CRenderEngine() 
{
	game_status = 0;
	fps = 0;
	viewDir= vec3(1,0,0);
	U = vec3(0,1,0);
	V = vec3(0,0,1);

	voxel_step0 = 5.0;
	timer_catch = 0;
	voxel_step = 0.5;
	voxel_step0 = 75.0;
	vel_tras = 20;
	lookFrom = vec3(350,30,120);
}


CRenderEngine::~CRenderEngine(void)
{
}


bool CRenderEngine::Initialize( HDC hContext_i)
{
	m_hDC = hContext_i;
    //Setting up the dialog to support the OpenGL.
    PIXELFORMATDESCRIPTOR stPixelFormatDescriptor;
    memset( &stPixelFormatDescriptor, 0, sizeof( PIXELFORMATDESCRIPTOR ));
    stPixelFormatDescriptor.nSize = sizeof( PIXELFORMATDESCRIPTOR );
    stPixelFormatDescriptor.nVersion = 1;
    stPixelFormatDescriptor.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW ;
    stPixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    stPixelFormatDescriptor.cColorBits = 24;
    stPixelFormatDescriptor.cDepthBits = 32;
    stPixelFormatDescriptor.cStencilBits = 8;
    stPixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE ;
    int nPixelFormat = ChoosePixelFormat( hContext_i, &stPixelFormatDescriptor ); //Collect the pixel format.

    if( nPixelFormat == 0 )
    {
        AfxMessageBox( _T( "Error while Choosing Pixel format" ));
        return false;
    }
    
	//Set the pixel format 
    if( !SetPixelFormat( hContext_i, nPixelFormat, &stPixelFormatDescriptor ))
    {
        AfxMessageBox( _T( "Error while setting pixel format" ));
        return false;
    }

    //Create a device context.
    m_hglContext = wglCreateContext( hContext_i );
    if( !m_hglContext )
    {
        AfxMessageBox( _T( "Rendering Context Creation Failed" ));
        return false;
    }
    //Make the created device context as the current device context.
    BOOL bResult = wglMakeCurrent( hContext_i, m_hglContext );
    if( !bResult )
    {
        AfxMessageBox( _T( "wglMakeCurrent Failed" ));
        return false;
    }

    glClearColor( 0.0f,0.0f, 0.0f, 0.0f );

	// inicializa las texturas 3d, para eso uso el wrapper de opengl glew
    glewInit();
	if(GL_TRUE != glewGetExtension("GL_EXT_texture3D"))
    {
        AfxMessageBox( _T( "3D texture is not supported !" ));
        return false;
    }

	// pongo los shaders
	setShaders();

	// inicio el sistema de fonts simples
	initFonts();

	if( !tex.CreateFromFile( "media/mri-head.raw", 256, 256,256))
	{
		AfxMessageBox( _T( "Failed to read the data" ));
	}
	/*
	if( !tex.CreateFromTest( 1, 256, 256,256))
	{
		AfxMessageBox( _T( "Failed to read the data" ));
	}*/



	GLint dims[4] = {0};
	glGetIntegerv(GL_VIEWPORT, dims);
	fbWidth = dims[2];
	fbHeight = dims[3];

    return true;
}

void CRenderEngine::initFonts()
{
	CDC *pDC =CDC::FromHandle(m_hDC);
	CFont hfont,*hfontOld;
	hfont.CreateFont(32, 0, 0,0, FW_NORMAL,0, 0, 0, 0, 0, 0, 0, 0, "Proxy 1");
	hfontOld = pDC->SelectObject(&hfont);

	for(int i=0;i<255;++i)
	{

		char s[1];
		s[0] = i;
		pDC->SetBkMode(TRANSPARENT);
		pDC->BeginPath();
		pDC->TextOut(0,0,s,1);
		pDC->EndPath();
		pDC->FlattenPath();

		int nNumPoints = pDC->GetPath((LPPOINT)NULL, (LPBYTE)NULL, 0);
		if (nNumPoints > 0)
		{
			glyphs[i].lpPt = new POINT[nNumPoints];
			glyphs[i].lpB = new BYTE[nNumPoints];
			glyphs[i].nNumPoints = pDC->GetPath(glyphs[i].lpPt, glyphs[i].lpB, nNumPoints);
		}

	}
	 pDC->SelectObject(hfontOld);

}


 void CRenderEngine::Resize( int nWidth_i, int nHeight_i )
 {
     GLdouble AspectRatio = ( GLdouble )(nWidth_i) / ( GLdouble )(nHeight_i ); 
     glViewport( 0, 0, nWidth_i, nHeight_i );
     glMatrixMode( GL_PROJECTION );
     glLoadIdentity();

     if( nWidth_i <= nHeight_i )
     {
         glOrtho( -1, 1, -( 1 / AspectRatio ) ,1 / AspectRatio, 2.0f*-1, 2.0f*1 );
     }
     else
     {
         glOrtho( -1 * AspectRatio, 1 * AspectRatio, -1, 1, 2.0f*-1, 2.0f*1 );
     }

     glMatrixMode( GL_MODELVIEW );
     glLoadIdentity();
 }

 void CRenderEngine::Render()
 {
	 glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );

	 if(tex.id==0)
	 {
		 renderText(10,510,"sin archivo cargado");
	 }
	 else
	 {
		 RayCasting();
		 //TextureVR();
	 }

	 SwapBuffers( m_hDC);


 }
 
 // VERSION CON RAYCASTING
 void CRenderEngine::RayCasting()
 {
	float fov = M_PI/4.0;
	float DX = fbWidth;
	float DY = fbHeight;
	float k = 2*tan(fov/2);
	vec3 Dy = U * (k*DY/DX);
	vec3 Dx = V * k;
	// direccion de cada rayo
	// D = N + Dy*y + Dx*x;
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glEnable(GL_TEXTURE_3D);

	glUseProgramObjectARB(shader_prog);

	viewDir = vec3(0,0,0) - lookFrom;
	viewDir.normalize();
	glUniform3f (	glGetUniformLocation(shader_prog, "iLookFrom") ,lookFrom.x,lookFrom.y,lookFrom.z);  
	glUniform3f (	glGetUniformLocation(shader_prog, "iViewDir") ,viewDir.x,viewDir.y,viewDir.z);  
	glUniform3f (	glGetUniformLocation(shader_prog, "iDx") ,Dx.x,Dx.y,Dx.z);  
	glUniform3f (	glGetUniformLocation(shader_prog, "iDy") ,Dy.x,Dy.y,Dy.z);  
	glUniform1f (	glGetUniformLocation(shader_prog, "voxel_step") ,voxel_step);  
	glUniform1f (	glGetUniformLocation(shader_prog, "voxel_step0") ,voxel_step0);  
	glUniform1i (	glGetUniformLocation(shader_prog, "game_status") ,game_status);  
	glUniform1f (	glGetUniformLocation(shader_prog, "time") ,time);  


	glActiveTexture(GL_TEXTURE);
	glBindTexture( GL_TEXTURE_3D,  tex.id);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);

	float dx = 0.0;
	float dy = 0.0;
	glTexCoord2f(-1, -1);  
	glVertex3f(-1,-1,0);

	glTexCoord2f(1, -1);  
	glVertex3f(1-dx,-1,0);

	glTexCoord2f(1, 1);  
	glVertex3f(1-dx,1-dy,0);

	glTexCoord2f(-1, 1);  
	glVertex3f(-1,1-dy,0);


	glEnd();
	glUseProgramObjectARB(0);
	glLoadIdentity();
	glDisable(GL_TEXTURE_3D);
	 
 }
 

 

 void CRenderEngine::TextureVR()
{
	glDisable(GL_DEPTH_TEST);

	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_GREATER, 0.05f );

	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();

	// escalo y roto con respecto al centro del cubo 

	glTranslatef( 0.5f, 0.5f, 0.5f);
	glScaled( 2,2,2);
	//mat4 transform = mat4::RotateX(0.0) * mat4::RotateY(-0.5) * mat4::RotateZ(-0.5);
	mat4 transform = mat4::fromBase(viewDir , U,V);
	glMultMatrixd( (const double *)transform.m);
	glTranslatef( -0.5f,-0.5f,-0.5f);

	glEnable(GL_TEXTURE_3D);

	glUseProgramObjectARB(shader_prog2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_3D,  tex.id);

	GLint texLoc = glGetUniformLocation(shader_prog2, "s_texture0");
	glUniform1i(texLoc, 0);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);


	int slice = 100;
	int t = 0;
	for ( float fIndx = -1.0f; fIndx <= 1.0f; fIndx+=0.01f )
	{
		glBegin(GL_QUADS);
		float TexIndex = fIndx;
		float s = (1-fIndx)/4.0 * 0.1;
		float d = 0;

		glTexCoord3f(0, 0, ((float)TexIndex+1.0f)/2.0f);  
		glVertex3f(-1+s+d,-1+s+d,TexIndex);

		glTexCoord3f(1, 0, ((float)TexIndex+1.0f)/2.0f);  
		glVertex3f(1-s,-1+s+d,TexIndex);

		glTexCoord3f(1, 1, ((float)TexIndex+1.0f)/2.0f);  
		glVertex3f(1-s,1-s,TexIndex);

		glTexCoord3f(0, 1, ((float)TexIndex+1.0f)/2.0f);  
		glVertex3f(-1+s+d,1-s,TexIndex);

		glEnd();
	}


	glUseProgramObjectARB(0);
	glLoadIdentity();
	glDisable(GL_TEXTURE_3D);

	/*char saux[40];
	sprintf(saux,"fps = %.1f",fps);
	renderText(10,10,saux);*/
}

void CRenderEngine::Release()
{
	//TODO:
	// falta liberar todo....	
}


CTexture::CTexture() 
{
	dx = dy = dz = 0;
	id = 0;
}


CTexture::~CTexture(void)
{
	if( 0 != id)
	{
		glDeleteTextures( 1, (GLuint*)&id);
	}
}

bool CTexture::CreateFromFile(LPCTSTR lpDataFile_i, int nWidth_i, int nHeight_i, int nSlices_i )
{
	CFile Medfile;
	if( !Medfile.Open(lpDataFile_i ,CFile::modeRead ))
	{
		return false;
	}

	dx= nWidth_i;
	dy= nHeight_i;
	dz= nSlices_i;



	int size = dx*dy*dz;
	BYTE * chBuffer = new BYTE[2*size];
	if( !chBuffer)
	{
		return false;
	}
	char* pRGBABuffer = new char[size*4];
	if( !pRGBABuffer)
	{
		return false;
	}
	memset(chBuffer,0, size);
	Medfile.Read(chBuffer, size);

	for( int nIndx = 0; nIndx < size; ++nIndx )
	{
		pRGBABuffer[nIndx*4] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+1] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+2] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+3] = chBuffer[nIndx];
	}

	if( 0 != id)
	{
		glDeleteTextures( 1, (GLuint*)&id);
	}
	glGenTextures(1,(GLuint*)&id);

	glBindTexture( GL_TEXTURE_3D, id );
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	/*
	bool rep = true;		// repito texturas ? 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	*/

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	//GL_UNSIGNED_SHORT_4_4_4_4
	//GL_UNSIGNED_BYTE
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA4, dx, dy, dz, 0,GL_RGBA,GL_UNSIGNED_BYTE , pRGBABuffer );
	glBindTexture( GL_TEXTURE_3D, 0 );

	delete[] chBuffer;
	delete[] pRGBABuffer;
	return true;
}

void CTexture::Box(BYTE *buff, int x0,int y0,int z0,int x1,int y1,int z1)
{

	for(int x=x0;x<x1;++x)
		for(int y=y0;y<y1;++y)
			for(int z=z0;z<z1;++z)
			{
				buff[z*dx*dy + y*dx + x] = 255;
			}
}

bool CTexture::CreateFromTest(int n,int nWidth_i, int nHeight_i, int nSlices_i )
{
	dx= nWidth_i;
	dy= nHeight_i;
	dz= nSlices_i;

	int size = dx*dy*dz;
	BYTE * chBuffer = new BYTE[size];
	if( !chBuffer)
	{
		return false;
	}
	char* pRGBABuffer = new char[size*4];
	if( !pRGBABuffer)
	{
		return false;
	}
	memset(chBuffer,0, size);


	// creo una caja
	if(n==0)
	{
		Box(chBuffer,0,0,0,dx,dy,10);
		Box(chBuffer,0,0,dz-10,dx,dy,dz);
	}
	else
	{
		Box(chBuffer,dx/2-10,dy/2-10,dz/2-10,dx/2+10,dy/2+10,dz/2+10);
	}
	for( int nIndx = 0; nIndx < size; ++nIndx )
	{
		pRGBABuffer[nIndx*4] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+1] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+2] = chBuffer[nIndx];
		pRGBABuffer[nIndx*4+3] = chBuffer[nIndx];
	}


	/*
	for( int nIndx = 0; nIndx < size; ++nIndx )
	{
		char r =255*(float)rand()/(float)RAND_MAX;
		pRGBABuffer[nIndx*4] = r;
		pRGBABuffer[nIndx*4+1] = r;
		pRGBABuffer[nIndx*4+2] = r;
		pRGBABuffer[nIndx*4+3] = r;
	}
	*/

	if( 0 != id)
	{
		glDeleteTextures( 1, (GLuint*)&id);
	}
	glGenTextures(1,(GLuint*)&id);

	glBindTexture( GL_TEXTURE_3D, id );
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	/*
	bool rep = false;		// repito texturas ? 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	*/

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, dx, dy, dz, 0,GL_RGBA, GL_UNSIGNED_BYTE, pRGBABuffer );
	glBindTexture( GL_TEXTURE_3D, 0 );

	delete[] chBuffer;
	delete[] pRGBABuffer;
	return true;
}

void CRenderEngine::renderText(int px, int py,char *text) 
{
	glLineWidth(2.5); 
	glColor3f(1.0, 0.0, 0.0);

	float K = 0.0016;
	float x0 = 2*px/1000.0-1;
	float y0 = 1- 2*py/700.0;
	int len = strlen(text);
	float max_x = -999;
	for(int t=0;t<len;++t)
	{
		int j = text[t];
		if(j==32)
		{
			x0+=24*K;
			continue;;
		}
		
		st_glyph *G = &glyphs[j];

		float ant_x, ant_y;
		for (int i= 0; i< G->nNumPoints; i++)
		{
			// ajusto posicion , origen y escala (en teoria orgien = 0, y escala = 1)
			float x = x0 + G->lpPt[i].x * K;
			float y = y0 - G->lpPt[i].y * K;

			if(x>max_x)
				max_x = x;

			switch(G->lpB[i])
			{

			case PT_MOVETO:
				ant_x = x;
				ant_y = y;
				break;

			case PT_LINETO | PT_CLOSEFIGURE:
			case PT_LINETO:
				glBegin(GL_LINES);
				glVertex3f(ant_x, ant_y, 0.0);
				glVertex3f(x, y, 0.0);
				glEnd();
				ant_x = x;
				ant_y = y;
				break;
			}
		}

		x0 = max_x + 10*K;
	}
	glEnd();
}




void CRenderEngine::loadShaders(char *vs,char *fs,GLhandleARB *vs_main,GLhandleARB *fs_main,GLhandleARB *shader_prog)
{

	*vs_main = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	*fs_main = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	
	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(*vs_main, 1, &vv,NULL);
	glShaderSourceARB(*fs_main, 1, &ff,NULL);


	glCompileShaderARB(*vs_main);
	glCompileShaderARB(*fs_main);

	GLint status;
	glGetObjectParameterivARB(*vs_main, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(!status)
	{
		GLint maxLength = 0;
		glGetShaderiv(*vs_main, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar *errorLog = new GLchar [maxLength];
		glGetShaderInfoLog(*vs_main, maxLength, &maxLength, errorLog);
		AfxMessageBox(errorLog);
		glDeleteShader(*vs_main); 
		delete errorLog;
		exit(0);
	}

	glGetObjectParameterivARB(*fs_main, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(!status)
	{
		GLint maxLength = 0;
		glGetShaderiv(*fs_main, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar *errorLog = new GLchar [maxLength];
		glGetShaderInfoLog(*fs_main, maxLength, &maxLength, errorLog);
		AfxMessageBox(errorLog);
		glDeleteShader(*fs_main); 
		delete errorLog;
		exit(0);
	}

	*shader_prog = glCreateProgramObjectARB();

	glAttachObjectARB(*shader_prog,*vs_main);
	glAttachObjectARB(*shader_prog,*fs_main);

	glLinkProgramARB(*shader_prog);

}


void CRenderEngine::setShaders() 
{
	char *vs,*fs;

	// shaders ray casting
	vs = textFileRead("shaders/ray_casting.vs");
	fs = textFileRead("shaders/ray_casting.fs");
	loadShaders(vs,fs,&vs_main , &fs_main , &shader_prog);
	free(vs);
	free(fs);

	// shaders texture volumen
	vs = textFileRead("shaders/texture_vr.vs");
	fs = textFileRead("shaders/texture_vr.fs");
	loadShaders(vs,fs,&vs2_main , &fs2_main , &shader_prog2);
	free(vs);
	free(fs);

}



char *textFileRead(char *fn) 
{


	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}
