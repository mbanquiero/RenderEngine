#include "stdafx.h"
#include "Texture.h"

static const float inv_255 = 1.0/255.0;

CTexture::CTexture()
{
	cant_mipmap = 0;

}


CTexture::~CTexture()
{
	for(int i=0;i<cant_mipmap;++i)
	if(mipmap[i].p_data)
		delete []mipmap[i].p_data;
}


bool CTexture::CreateFromFile(const char *file_name)
{
	// fuerzo extension jpg
	strcpy(fname,file_name);
	extension(fname,"jpg");

	CImage image;
	if(!SUCCEEDED(image.Load(fname)))
	{
		// pruebo png
		extension(fname,"png");
		if(!SUCCEEDED(image.Load(fname)))
				return false;
	}

	int iwidth = image.GetWidth();
	int iheight = image.GetHeight();
	// fuerzo a multipos de 2 
	width = mipmap[0].width = upper_power2(iwidth);
	height = mipmap[0].height = upper_power2(iheight);
	mipmap[0].p_data = new st_rgba[width*height];

	BYTE* byteptr = (BYTE*)image.GetBits();
	int pitch = image.GetPitch(); 
	int bpp = image.GetBPP()/8;

	int t = 0;
	float du = (float)iwidth / (float)width;
	float dv = (float)iheight / (float)height;
	for(int i=0;i<height;++i)
		for(int j=0;j<width;++j)
		{
			float tu = j*du;
			float tv = i*dv;

			// int part
			int u = tu;
			int v = tv;
			// frac part
			float ru = tu-u;
			float rv = tv-v;

			int u1 = min(u+1,iwidth-1);
			int v1 = min(v+1,iheight-1);

			float b00 = *(byteptr+pitch*v+bpp*u);
			float g00 = *(byteptr+pitch*v+bpp*u+1);
			float r00 = *(byteptr+pitch*v+bpp*u+2); 
			float b10 = *(byteptr+pitch*v+bpp*u1);
			float g10 = *(byteptr+pitch*v+bpp*u1+1);
			float r10 = *(byteptr+pitch*v+bpp*u1+2); 
			float b01 = *(byteptr+pitch*v1+bpp*u);
			float g01 = *(byteptr+pitch*v1+bpp*u+1);
			float r01 = *(byteptr+pitch*v1+bpp*u+2); 
			float b11 = *(byteptr+pitch*v1+bpp*u1);
			float g11 = *(byteptr+pitch*v1+bpp*u1+1);
			float r11 = *(byteptr+pitch*v1+bpp*u1+2); 


			/*
			COLORREF c00 = image.GetPixel(u,v);
			COLORREF c10 = image.GetPixel(min(u+1,iwidth-1),v);
			COLORREF c01 = image.GetPixel(u,min(v+1,iheight-1));
			COLORREF c11 = image.GetPixel(min(u+1,iwidth-1),min(v+1,iheight-1));
			
			float r00 = GetRValue(c00);
			float g00 = GetGValue(c00);
			float b00 = GetBValue(c00);
			float r10 = GetRValue(c10);
			float g10 = GetGValue(c10);
			float b10 = GetBValue(c10);
			float r01 = GetRValue(c01);
			float g01 = GetGValue(c01);
			float b01 = GetBValue(c01);
			float r11 = GetRValue(c11);
			float g11 = GetGValue(c11);
			float b11 = GetBValue(c11);
			*/


			float w00 = (1-ru)*(1-rv);
			float w01 = (1-ru)*rv;
			float w10 = ru*(1-rv);
			float w11 = ru*rv;

			// nearest point sampler
			//w00 = 1;
			//w10 = w01 = w11 = 0;

			mipmap[0].p_data[t].r = r00 * w00 +  r01 * w01 + r10 *w10 + r11* w11;
			mipmap[0].p_data[t].g = g00 * w00 +  g01 * w01 + g10 *w10 + g11* w11;
			mipmap[0].p_data[t].b = b00 * w00 +  b01 * w01 + b10 *w10 + b11* w11;
			mipmap[0].p_data[t].a = 255;
			

			++t;

		}


	// Creo la cadena de mipmaps
	cant_mipmap = 1;
	int W = width / 2;
	int H = height / 2;
	while(W>=1 && H>=1)
	{
		// creo un nuevo mipmap
		mipmap[cant_mipmap].width = W;
		mipmap[cant_mipmap].height = H;
		mipmap[cant_mipmap].p_data = new st_rgba[W*H];

		int t = 0;
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
			{
				int pos = 4*i*W + 2*j;
				st_rgba *p = mipmap[cant_mipmap-1].p_data;
				st_rgba c00 = *(p + pos);
				st_rgba c10 = *(p + pos+1);
				st_rgba c01 = *(p + pos+2*W);
				st_rgba c11 = *(p + pos+2*W+1);

				mipmap[cant_mipmap].p_data[t].r = c00.r * 0.25 +  c01.r * 0.25 + c10.r * 0.25 + c11.r * 0.25;
				mipmap[cant_mipmap].p_data[t].g = c00.g * 0.25 +  c01.g * 0.25 + c10.g * 0.25 + c11.g * 0.25;
				mipmap[cant_mipmap].p_data[t].b = c00.b * 0.25 +  c01.b * 0.25 + c10.b * 0.25 + c11.b * 0.25;
				mipmap[cant_mipmap].p_data[t].a = c00.a * 0.25 +  c01.a * 0.25 + c10.a * 0.25 + c11.a * 0.25;
				++t;
			}

		// paso al siguiente mimmap
		++cant_mipmap;
		W /= 2;
		H /= 2;


	}

	return true;
}

vec4 CTexture::tex2Dgrad(float tu, float tv,float ddx,float ddy)
{
	float lod = log2(max(ddx*width,max(ddy*height,1.0f)));
	int l = (int)lod;

	float delta = lod - l;
	return tex2Dlod(tu,tv,l)*(1.0-delta) + tex2Dlod(tu,tv,l+1) * delta;
	//return tex2Dlod(tu,tv,l);
	
}

#define clamp(x,a,b) (x)<(a)?(a): (x) > (b) ? (b) : (x)

vec4 CTexture::tex2Dlod(float tu, float tv,int level)
{
	// TODO: cual sera el maximo numero (que no de overflow de float)
	// evito numeros negativos
	tu+=100;
	tv+=100;

	if(level>=cant_mipmap)
		level = cant_mipmap-1;

	// bilinear sampling
	int W = mipmap[level].width;
	int H = mipmap[level].height;
	int stride = W;

	float fu = (W) * tu;
	int u = fu;
	int u1 = u+1;
	float ru = fu - u;

	// mirror
	/*
	u%=2*W;
	if(u>=W)u=2*W-1-u;
	u1 %=2*W;
	if(u1>=W)u1=2*W-1-u1;
	*/
	
	// wrapped
	u%=W;
	u1%=W;

	float fv = (H) * tv;
	int v = fv;
	int v1 = v+1;
	float rv = fv - v;

	/*
	v%=2*H;
	if(v>=H) v=2*H-1-v;
	v1 %=2*H;
	if(v1>=H)v1=2*H-1-v1;
	*/
	v%=H;
	v1%=H;


	st_rgba *p = mipmap[level].p_data;
	st_rgba c00 = *(p + v*stride + u);
	st_rgba c10 = *(p + v*stride + u1);
	st_rgba c01 = *(p + v1*stride + u);
	st_rgba c11 = *(p + v1*stride + u1);

	float w00 = (1-ru)*(1-rv);
	float w01 = (1-ru)*rv;
	float w10 = ru*(1-rv);
	float w11 = ru*rv;

	// nearest point sampling
	//w00 =1;
	//w01=w10=w11=0;

	float r = (float)c00.r * inv_255 * w00 +  (float)c01.r * inv_255 * w01 + (float)c10.r * inv_255 * w10 + (float)c11.r * inv_255 * w11;
	float g = (float)c00.g * inv_255 * w00 +  (float)c01.g * inv_255 * w01 + (float)c10.g * inv_255 * w10 + (float)c11.g * inv_255 * w11;
	float b = (float)c00.b * inv_255 * w00 +  (float)c01.b * inv_255 * w01 + (float)c10.b * inv_255 * w10 + (float)c11.b * inv_255 * w11;
	float a = (float)c00.a * inv_255 * w00 +  (float)c01.a * inv_255 * w01 + (float)c10.a * inv_255 * w10 + (float)c11.a * inv_255 * w11;

	return vec4(r,g,b,a);

}


/*

// constant values that will be needed
static const __m128 CONST_1111 = _mm_set1_ps(1);
static const __m128 CONST_256 = _mm_set1_ps(256);

inline __m128 CalcWeights(float x, float y)
{
	__m128 ssx = _mm_set_ss(x);
	__m128 ssy = _mm_set_ss(y);
	__m128 psXY = _mm_unpacklo_ps(ssx, ssy);      // 0 0 y x

	//__m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
	__m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
	__m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor); // = frac(psXY)

	__m128 psXYfrac1 = _mm_sub_ps(CONST_1111, psXYfrac); // ? ? (1-y) (1-x)
	__m128 w_x = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
	w_x = _mm_movelh_ps(w_x, w_x);      // x (1-x) x (1-x)
	__m128 w_y = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

	// complete weight vector
	return _mm_mul_ps(w_x, w_y);
}

vec4 CTexture::tex2dSSE(float tu, float tv)
{
	float x = tu*width;
	float y = tv*height;

	const st_rgba * p0 = p_data+ (int)x + (int)y * width; // pointer to first pixel

	// Load the data (2 pixels in one load)
	__m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0 * width]); 
	__m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[1 * width]); 

	__m128 weight = CalcWeights(x, y);

	// convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA (AoS to SoA)
	__m128i p1234 = _mm_unpacklo_epi8(p12, p34);
	__m128i p34xx = _mm_unpackhi_epi64(p1234, _mm_setzero_si128());
	__m128i p1234_8bit = _mm_unpacklo_epi8(p1234, p34xx);

	// extend to 16bit 
	__m128i pRG = _mm_unpacklo_epi8(p1234_8bit, _mm_setzero_si128());
	__m128i pBA = _mm_unpackhi_epi8(p1234_8bit, _mm_setzero_si128());

	// convert weights to integer
	weight = _mm_mul_ps(weight, CONST_256); 
	__m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
	weighti = _mm_packs_epi32(weighti, weighti); // 32->2x16bit

	//outRG = [w1*R1 + w2*R2 | w3*R3 + w4*R4 | w1*G1 + w2*G2 | w3*G3 + w4*G4]
	__m128i outRG = _mm_madd_epi16(pRG, weighti);
	//outBA = [w1*B1 + w2*B2 | w3*B3 + w4*B4 | w1*A1 + w2*A2 | w3*A3 + w4*A4]
	__m128i outBA = _mm_madd_epi16(pBA, weighti);

	// horizontal add that will produce the output values (in 32bit)
	__m128i out = _mm_hadd_epi32(outRG, outBA);
	out = _mm_srli_epi32(out, 8); // divide by 256

	// convert 32bit->8bit
	out = _mm_packus_epi32(out, _mm_setzero_si128());
	out = _mm_packus_epi16(out, _mm_setzero_si128());

	// return
	return _mm_cvtsi128_si32(out);
}
*/


//test
void CTexture::Draw(CDC *pDC,int l, int x,int y,int dx,int dy,float k)
{

	vec4 tx = tex2Dlod(5,5, 0);
	tx = tex2Dlod(5,5, 7);

	int fil,col=x;
	for(float i=-2;i<=2;i+=0.01f)
	{
		fil=y; 
		for(float j=-2;j<=2;j+=0.01f)
		{
			vec4 tx = tex2Dlod(i,j, l);
			COLORREF clr = RGB(255 * tx.x , 255 * tx.y , 255 * tx.z);
			pDC->SetPixel(col,fil++,clr);
		}
		col++;
	}
	
	/*
	int fil,col=x;
	for(float i=-1.5;i<=1.5;i+=0.01f)
	{
		fil=y; 
		for(float j=-1.5;j<=1.5;j+=0.01f)
		{
			vec4 tx = tex2Dlod(i,j, l);
			COLORREF clr = RGB(255 * tx.x , 255 * tx.y , 255 * tx.z);
			pDC->SetPixel(col,fil++,clr);
		}
		col++;
	}
	*/
}

void CTexture::DrawSurface(CDC *pDC, int l,int x,int y,int k)
{
	//pDC->FillSolidRect(0,0,2000,2000,RGB(255,0,255));
	int dx = mipmap[l].width;
	int dy = mipmap[l].height;
	for(int i=0;i<dx;++i)
		for(int j=0;j<dy;++j)
		{
			st_rgba clr = mipmap[l].p_data[i*dx + j];
			pDC->FillSolidRect(x+i*k+1,y+j*k+1,k-2,k-2,RGB(clr.r,clr.g,clr.b));
		}
}



// helper

void extension(char *file,char *ext)
{
	// Le saco la extension que tenia antes
	// si es que tiene extension

	// busco de atras para adelante, hasta encontrar el punto o el separador de path
	int i = strlen(file) - 1;
	char c = 0;
	while( i>=0 && (c=file[i])!='\\' && c!='.' && c!='\0')
		--i;			// sigo buscando (hacia adelante)

	if(c=='.')
	{
		// si lo primero que encontro, recorriendo de atras para adelante es un punto, 
		// es poque tenia extension, y lo tengo que cortar ahi
		file[i] = '\0';
	}

	if(ext && ext[0])
	{
		// Ahora no tiene mas extension, le agrego la nueva
		strcat(file,".");
		strcat(file,ext);
	}

}
