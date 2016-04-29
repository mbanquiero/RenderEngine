#include "StdAfx.h"
#include "RenderEngine.h"

#pragma comment ( lib, "OpenGL32.lib" )
#pragma comment ( lib, "glew32.lib" )


CRenderEngine::CRenderEngine() 
{
	dist = 0.0f;
	escale = 1.0f;
	an_x = an_y = an_z = 0;
	fps = 0;
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

	
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0.05f );

		glEnable(GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();

		// escalo y roto con respecto al centro del cubo 
		glTranslatef( 0.5f, 0.5f, 0.5f );
		glScaled( escale, 1.0f*(float)tex.dx/(float)tex.dy*escale, (float)tex.dx/(float)tex.dz*escale);
		mat4 transform = mat4::RotateX(an_x) * mat4::RotateY(an_y) * mat4::RotateZ(an_z);
		glMultMatrixd( (const double *)transform.m);
		glTranslatef( -0.5f,-0.5f, dist-0.5f);

		glEnable(GL_TEXTURE_3D);
		glBindTexture( GL_TEXTURE_3D,  tex.id);
		glUseProgramObjectARB(shader_prog);

		for ( float fIndx = -1.0f; fIndx <= 1.0f; fIndx+=0.01f )
		{
			glBegin(GL_QUADS);
			float TexIndex = fIndx;
			float s = (1-fIndx)/4.0;

			glTexCoord3f(0.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  
			glVertex3f(-1+s,-1+s,TexIndex);

			glTexCoord3f(1.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  
			glVertex3f(1-s,-1+s,TexIndex);

			glTexCoord3f(1.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  
			glVertex3f(1-s,1-s,TexIndex);

			glTexCoord3f(0.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  
			glVertex3f(-1+s,1-s,TexIndex);

			glEnd();
		 }

		glUseProgramObjectARB(0);
		glLoadIdentity();
		glDisable(GL_TEXTURE_3D);
	}


	char saux[40];
	sprintf(saux,"fps = %.1f",fps);
	renderText(10,10,saux);

	SwapBuffers( m_hDC);
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
	bool rep = false;		// repito texturas ? 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, rep?GL_MIRRORED_REPEAT:GL_CLAMP_TO_BORDER);
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

	float K = 0.002;
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


void CRenderEngine::setShaders() 
{

	char *vs,*fs;

	vs_main = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fs_main = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

	vs = textFileRead("shaders/main.vs");
	fs = textFileRead("shaders/main.fs");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(vs_main, 1, &vv,NULL);
	glShaderSourceARB(fs_main, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShaderARB(vs_main);
	glCompileShaderARB(fs_main);

	GLint status;
	glGetObjectParameterivARB(vs_main, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(!status)
	{
		GLint maxLength = 0;
		glGetShaderiv(vs_main, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar *errorLog = new GLchar [maxLength];
		glGetShaderInfoLog(vs_main, maxLength, &maxLength, errorLog);
		AfxMessageBox(errorLog);
		glDeleteShader(vs_main); 
		delete errorLog;
		exit(0);
	}

	glGetObjectParameterivARB(fs_main, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(!status)
	{
		GLint maxLength = 0;
		glGetShaderiv(fs_main, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar *errorLog = new GLchar [maxLength];
		glGetShaderInfoLog(fs_main, maxLength, &maxLength, errorLog);
		AfxMessageBox(errorLog);
		glDeleteShader(fs_main); 
		delete errorLog;
		exit(0);
	}

	shader_prog = glCreateProgramObjectARB();

	glAttachObjectARB(shader_prog,vs_main);
	glAttachObjectARB(shader_prog,fs_main);

	glLinkProgramARB(shader_prog);

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
