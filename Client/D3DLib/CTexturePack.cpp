//----------------------------------------------------------------------
// CTexturePack.cpp
//----------------------------------------------------------------------

#include "CTexturePack.h"
#include "CDirect3D.h"
#include <stdio.h>
//#include <fstream>
extern HWND g_hWnd;

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CTexturePack::CTexturePack()
{
	m_nTextures = 0;
	m_pTextures = NULL;
}

CTexturePack::~CTexturePack()
{
	// array�� �޸𸮿��� �����Ѵ�.
	Release();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Init
//----------------------------------------------------------------------
void
CTexturePack::Init(int count)
{
	// ������ ���� ��� 
	if (count==0) 
		return;

	// �ϴ� ����
	Release();

	// �޸� ���
	m_nTextures = count;
	m_pTextures = new CSpriteSurface [m_nTextures];
}


//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void
CTexturePack::Release()
{
	if (m_pTextures != NULL)
	{
		// ��� CTexture�� �����.
		delete [] m_pTextures;

		m_pTextures = NULL;		
		m_nTextures = 0;
	}
}

//----------------------------------------------------------------------
// Init
//----------------------------------------------------------------------
// AlphaSpritePack�� ������ TexturePack�� �����Ѵ�.
//
// PixelFormat�� 4:4:4:4��.. �ϴ� �����ϱ�� �Ѵ�.
// AlphaSpritePack��ü�� Alpha���� �����Ƿ�... ����~�ϴٰ� ������.
//----------------------------------------------------------------------
bool
CTexturePack::Init( CAlphaSpritePack& ASPK )
{
	//--------------------------------------------
	// 4:4:4:4�� �´� PixelFormatã��
	//--------------------------------------------
	/*
	DDPIXELFORMAT	PixelFormat4444;
	if (!CDirect3D::FindBestPixelFormat(4,4,4,4, &PixelFormat4444))
	{
		//InitFail("Initialize Failed! - Can't Find PixelFormat");
		return false;
	}
	*/

	// ������ TextureSurface�� ũ��
	int width, height;


	/*
	DDSCAPS2 ddsCaps2;
	DWORD    dwTotal;   
	DWORD    dwFree;     
	*/

	//---------------------------------------------
	// memory���
	//---------------------------------------------
	Init( ASPK.GetSize() );
	
	//---------------------------------------------
	// ASPK ---> TextureSurface �� ��ȯ
	//---------------------------------------------
	for (int sur=0; sur<m_nTextures; sur++)
	{		
		//---------------------------------------------------
		// Video Memory ������ ����
		//---------------------------------------------------
		/*
		ZeroMemory(&ddsCaps2, sizeof(ddsCaps2)); 
		ddsCaps2.dwCaps = DDSCAPS_TEXTURE; 

		HRESULT hr = CDirectDraw::GetDD()->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);

		if (FAILED(hr))
			return false;

		char str[80];
		sprintf(str, "[%d/%d] mem=%ld/%ld", sur+1, m_nTextures, dwFree, dwTotal);
		MessageBox( g_hWnd, str, NULL, MB_OK);
		*/

		width = ASPK[sur].GetWidth();
		height = ASPK[sur].GetHeight();
		CDirect3D::GetTextureSize(width, height);

		// ũ�� ����
		/*
		char str[80];
		sprintf(str, "[%d/%d] mem=%d/%d", sur+1, m_nTextures, width, height);
		MessageBox( g_hWnd, str, NULL, MB_OK);
		*/
	

		//---------------------------------------------------
		// TextureSurface ����
		//---------------------------------------------------		
		m_pTextures[sur].InitTextureSurface(width, height, 0, CDirect3D::GetPixelFormat4444());
		POINT point;
		point.x = 0;
		point.y = 0;
		m_pTextures[sur].Lock();

		int i;

		//---------------------------------------------------
		// Texture Surface �ʱ�ȭ
		//---------------------------------------------------
		WORD *pSurface = (WORD*)m_pTextures[sur].GetSurfacePointer();
				//,	*pSurfaceTemp;
		long pitch	= m_pTextures[sur].GetSurfacePitch();

		DWORD width2 = width << 1;	// *2 
		for (i=0; i<height; i++)
		{
			/*
			WORD* pSurfaceTemp = pSurface;
			for (int j=0; j<width; j++)
			{
				*pSurfaceTemp = 0x8000;
				pSurfaceTemp++;
			}
			*/
			memset(pSurface, 0, width2);
			pSurface = (WORD*)((BYTE*)pSurface + pitch);
		}

		// AlphaSprite���
		m_pTextures[sur].BltAlphaSprite4444(&point, &ASPK[ sur ]);
		
		m_pTextures[sur].Unlock();

		// PixelFormat�� ���� Colorkey�� �ٲ� �� �ִ�. T_T;;
		//m_pTextures[sur].SetTransparency( 0x0000 );
	}

	return true;
}

//----------------------------------------------------------------------
// InitPart
//----------------------------------------------------------------------
// Shadow SpritePack�� �Ϻθ�... Texture�� ����
//----------------------------------------------------------------------
bool			
CTexturePack::InitPart( CShadowSpritePack& SSPK, TYPE_SPRITEID firstShadowSpriteID, TYPE_SPRITEID lastShadowSpriteID )
{
	//--------------------------------------------
	// TextureSurface�� Maxũ�⿡ ���� ����
	//--------------------------------------------
	// Get the device caps
    D3DDEVICEDESC7 ddDesc;
    if( FAILED( CDirect3D::GetDevice()->GetCaps( &ddDesc) ) )
        return false;

	// Limit max texture sizes, if the driver can't handle large textures
    int maxWidth  = (ddDesc.dwMaxTextureWidth)? ddDesc.dwMaxTextureWidth : 256;
    int maxHeight = (ddDesc.dwMaxTextureHeight)? ddDesc.dwMaxTextureHeight : 256;  

	// ������ TextureSurface�� ũ��
	int width, height;


	/*
	DDSCAPS2 ddsCaps2;
	DWORD    dwTotal;   
	DWORD    dwFree;     
	*/
	
	//---------------------------------------------
	// SSPK ---> TextureSurface �� ��ȯ
	//---------------------------------------------
	for (int sur=firstShadowSpriteID; sur<lastShadowSpriteID; sur++)
	{		
		//---------------------------------------------------
		// Video Memory ������ ����
		//---------------------------------------------------
		/*
		ZeroMemory(&ddsCaps2, sizeof(ddsCaps2)); 
		ddsCaps2.dwCaps = DDSCAPS_TEXTURE; 

		HRESULT hr = CDirectDraw::GetDD()->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);

		if (FAILED(hr))
			return false;

		char str[80];
		sprintf(str, "[%d/%d] mem=%ld/%ld", sur+1, m_nTextures, dwFree, dwTotal);
		MessageBox( g_hWnd, str, NULL, MB_OK);
		*/

		//---------------------------------------------------------------
		// �ϵ��� �����ϴ� TextureSurface�� ũ�⸦ �����Ѵ�.
		//---------------------------------------------------------------
		// Adjust width and height to be powers of 2, if the device requires it
		width	= SSPK[sur].GetWidth();
		height	= SSPK[sur].GetHeight();
		if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
		{
			for( width=1;  SSPK[sur].GetWidth()>width;   width<<=1 );
			for( height=1; SSPK[sur].GetHeight()>height; height<<=1 );
		}

		width  = min( width,  maxWidth );
		height = min( height, maxHeight );

		// Make the texture square, if the driver requires it
		if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
		{
			if( width > height ) 
				height = width;
			else	
				width  = height;
		}

		// ũ�� ����
		/*
		char str[80];
		sprintf(str, "[%d/%d] mem=%d/%d", sur+1, m_nTextures, width, height);
		MessageBox( g_hWnd, str, NULL, MB_OK);
		*/
	

		//---------------------------------------------------
		// TextureSurface ����
		//---------------------------------------------------		
		m_pTextures[sur].InitTextureSurface(width, height, 0, CDirect3D::GetPixelFormat4444());
		POINT point;
		point.x = 0;
		point.y = 0;
		m_pTextures[sur].Lock();

		int i;

		//---------------------------------------------------
		// Texture Surface �ʱ�ȭ
		//---------------------------------------------------
		WORD *pSurface = (WORD*)m_pTextures[sur].GetSurfacePointer();
				//,	*pSurfaceTemp;
		long pitch	= m_pTextures[sur].GetSurfacePitch();

		DWORD width2 = width << 1;	// *2 
		for (i=0; i<height; i++)
		{
			/*
			WORD* pSurfaceTemp = pSurface;
			for (int j=0; j<width; j++)
			{
				*pSurfaceTemp = 0x8000;
				pSurfaceTemp++;
			}
			*/
			memset(pSurface, 0, width2);
			pSurface = (WORD*)((BYTE*)pSurface + pitch);
		}

		// ShadowSprite���
		m_pTextures[sur].BltShadowSprite4444(&point, &SSPK[ sur ], 0x8000);
		
		m_pTextures[sur].Unlock();

		// PixelFormat�� ���� Colorkey�� �ٲ� �� �ִ�. T_T;;
		//m_pTextures[sur].SetTransparency( 0x0000 );
	}

	return true;
}