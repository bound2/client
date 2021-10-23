//----------------------------------------------------------------------
// CFilter.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "CFilter.h"
//----------------------------------------------------------------------
//
// contructor/destructor
//
//----------------------------------------------------------------------
CFilter::CFilter()
{
	m_Width		= 0;
	m_Height	= 0;
	m_ppFilter	= NULL;
}

CFilter::~CFilter()
{
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
CFilter::Init(WORD width, WORD height)
{
	if (width==0 || height==0)
		return;

	// �ϴ� ����
	Release();

	m_Width = width;
	m_Height = height;
	
	// linear�� memory����
	m_ppFilter = new BYTE* [m_Height];

	for (int i=0; i<m_Height; i++)
	{
		m_ppFilter[i] = new BYTE [m_Width];
	}
}

//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void
CFilter::Release()
{
	if (m_ppFilter != NULL)
	{
		for (int i=0; i<m_Height; i++)
			delete [] m_ppFilter[i];

		delete [] m_ppFilter;
		m_ppFilter = NULL;

		m_Width = 0;
		m_Height = 0;
	}
}

//----------------------------------------------------------------------
// Set Filter
//----------------------------------------------------------------------
// pSurface�� �� ���� �о �׿� ���� Filter���� ������ش�.
// Filter�� �� ���� ���ؼ� �ϳ��� ���ε�,
// �� ���� (R,G,B)�� �̷���� �����Ƿ�...
//
// �Ϲ������� Filter�� ���� (R,G,B)�� ������ R==G==B�̶�� �����ϰ�..
// �� �߿��� B�� �����ϵ��� �Ѵ�. �׳�.. *_*;
// 
//----------------------------------------------------------------------
void		
CFilter::SetFilter(WORD* pSurface, WORD pitch, WORD width, WORD height)
{
	//------------------------------------------------------------
	// ���� �����ִ� �޸𸮿� �뷮�� �ٸ� ��� memory���
	//------------------------------------------------------------
	if (width!=m_Width || height!=m_Height)
		Init(width, height);

	
	register int i;
	register int j;
			

	//------------------------------------------------------------
	// pSurface�� �� ���� ���� ������ �о m_pFilter�� ����
	//------------------------------------------------------------
	WORD*	pSurfaceTemp;
	BYTE*	pFilter;
	for (i=0; i<height; i++)
	{
		pSurfaceTemp = pSurface;

		pFilter = m_ppFilter[i];

		j = width;
		do
		{
			*pFilter = (BYTE)(*pSurfaceTemp & 0x1F);	// Blue��

			pFilter++;
			pSurfaceTemp++;
			
		} while (--j);

		// ���� ��
		pSurface = (WORD*)((BYTE*)pSurface + pitch);
	}
}

//----------------------------------------------------------------------
// �ٸ� Filter�� �����Ѵ�.
//----------------------------------------------------------------------
void		
CFilter::SetFilter(const CFilter& filter)
{
	// size�� 0�̸� return
	if (filter.m_Width==0 || filter.m_Height==0)
		return;

	// memory ���
	Init( filter.m_Width, filter.m_Height );

	// byte��
	int width2 = m_Width << 1;

	for (int i=0; i<m_Height; i++)
	{
		memcpy((void*)m_ppFilter[i], (void*)filter.m_ppFilter[i], width2);		
	}
}

//----------------------------------------------------------------------
// �ٸ� Filter�� ��Ӱ� �ؼ�.. �����Ѵ�.
//----------------------------------------------------------------------
void		
CFilter::SetFilterDarkness(const CFilter& filter, BYTE DarkBits)
{
	// size�� 0�̸� return
	if (filter.m_Width==0 || filter.m_Height==0)
		return;

	// memory ���
	Init( filter.m_Width, filter.m_Height );

	register int i;
	register int j;

	// �� filter������ DarkBits��ŭ ��Ӱ� �Ѵ�.
	for (i=0; i<m_Height; i++)
	{
		for (j=0; j<m_Width; j++)
		{
			m_ppFilter[i][j] = (filter.m_ppFilter[i][j] >> DarkBits);
		}
	}
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
bool		
CFilter::SaveToFile(std::ofstream& file)
{
	// Size ����
	file.write((const char*)&m_Width, 2);
	file.write((const char*)&m_Height, 2);

	// ����� ������ ������ return
	if (m_Width==0 || m_Height==0 || m_ppFilter==NULL)
		return false;

	// Filter ������ �����Ѵ�.
	for (int i=0; i<m_Height; i++)
	{
		file.write((const char*)m_ppFilter[i], m_Width);
	}
	return true;
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
bool		
CFilter::LoadFromFile(std::ifstream& file)
{
	// Size Load
	file.read((char*)&m_Width, 2);
	file.read((char*)&m_Height, 2);

	// size�� 0�̸� return
	if (m_Width==0 || m_Height==0)
		return false;

	// memory ���
	Init( m_Width, m_Height );	

	// Filter ������ �о�´�.
	for (int i=0; i<m_Height; i++)
	{
		file.read((char*)m_ppFilter[i], m_Width);
	}
	return true;
}

//----------------------------------------------------------------------
// Blt Filter
//----------------------------------------------------------------------
// (x,y)�� filter�� ����Ѵ�.
//----------------------------------------------------------------------
void		
CFilter::BltFilter(int x, int y, const CFilter& filter)
{
	BYTE	*pDest;			// ����ҷ��� ��ġ
	BYTE	*pSource;		// ����ҷ��� ��ġ

	int startY, endY;						// y����
	int sourceStartX, destStartX, width;	// x����
	
	register int sourceIndex;
	register int destIndex;

	//-------------------------------------------------
	// Y��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (y > m_Height)
	{
		return;	
	}
	// 0���� ���� ���
	if (y < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (y+filter.m_Height <= 0)
		{
			return;
		}

		// ���� clip
		startY = -y;

		// �Ʒ��� clip�� �Ǵ� ���
		if (y+filter.m_Height > m_Height)
		{
			endY = startY + m_Height;
		}
		else
		{
			endY = filter.m_Height;
		}

		destIndex = 0;
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (y+filter.m_Height > m_Height)
	{
		startY = 0;
		endY = m_Height - y;
		destIndex = y;
	}	
	// y clip�� ���� ���
	else 
	{
		startY = 0;
		endY = filter.m_Height;
		destIndex = y;
	}

	//-------------------------------------------------
	// X��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (x > m_Width)
	{
		return;	
	}
	// 0���� ���� ���
	if (x < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (x+filter.m_Width <= 0)
		{
			return;
		}

		// ������ Clip�� �ʿ��� ���
		if (x+filter.m_Width > m_Width)
		{
			width = m_Width;
		}
		else
		{
			width = filter.m_Width + x;
		}

		// ���� clip
		sourceStartX = -x;
		destStartX = 0;		
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (x+filter.m_Width > m_Width)
	{
		sourceStartX = 0;
		destStartX = x;
		width = m_Width - x;		
	}	
	// X clip�� ���� ���
	else 
	{
		sourceStartX = 0;		
		destStartX = x;
		width = filter.m_Width;
	}


	//-------------------------------------------------
	// startY���� endY������ source�� dest��...
	//-------------------------------------------------
	for (sourceIndex=startY; sourceIndex<endY; destIndex++, sourceIndex++)
	{
		pSource = filter.m_ppFilter[sourceIndex] + sourceStartX;
		pDest	= m_ppFilter[destIndex] + destStartX;

		// width��ŭ pDest�� pSource�� blt�Ѵ�.
		memcpy(pDest, pSource, width);		
	}
}

//----------------------------------------------------------------------
// Blt Filter Add (31����)
//----------------------------------------------------------------------
// (x,y)�� filter�� Lighten���� ����Ѵ�.
//----------------------------------------------------------------------
void		
CFilter::BltFilterAdd(int x, int y, const CFilter& filter)
{
	BYTE	*pDest;			// ����ҷ��� ��ġ
	BYTE	*pSource;		// ����ҷ��� ��ġ

	int startY, endY;						// y����
	int sourceStartX, destStartX, width;	// x����
	
	register int sourceIndex;
	register int destIndex;

	//-------------------------------------------------
	// Y��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (y > m_Height)
	{
		return;	
	}
	// 0���� ���� ���
	if (y < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (y+filter.m_Height <= 0)
		{
			return;
		}

		// ���� clip
		startY = -y;

		// �Ʒ��� clip�� �Ǵ� ���
		if (y+filter.m_Height > m_Height)
		{
			endY = startY + m_Height;
		}
		else
		{
			endY = filter.m_Height;
		}

		destIndex = 0;
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (y+filter.m_Height > m_Height)
	{
		startY = 0;
		endY = m_Height - y;
		destIndex = y;
	}	
	// y clip�� ���� ���
	else 
	{
		startY = 0;
		endY = filter.m_Height;
		destIndex = y;
	}

	//-------------------------------------------------
	// X��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (x > m_Width)
	{
		return;	
	}
	// 0���� ���� ���
	if (x < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (x+filter.m_Width <= 0)
		{
			return;
		}

		// ������ Clip�� �ʿ��� ���
		if (x+filter.m_Width > m_Width)
		{
			width = m_Width;
		}
		else
		{
			width = filter.m_Width + x;
		}

		// ���� clip
		sourceStartX = -x;
		destStartX = 0;		
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (x+filter.m_Width > m_Width)
	{
		sourceStartX = 0;
		destStartX = x;
		width = m_Width - x;		
	}	
	// X clip�� ���� ���
	else 
	{
		sourceStartX = 0;		
		destStartX = x;
		width = filter.m_Width;
	}


	//int d, s, t;
	int t;
	//-------------------------------------------------
	// startY���� endY������ source�� dest��...
	//-------------------------------------------------
	for (sourceIndex=startY; sourceIndex<endY; destIndex++, sourceIndex++)
	{
		pSource = filter.m_ppFilter[sourceIndex] + sourceStartX;
		pDest	= m_ppFilter[destIndex] + destStartX;

		//------------------------------------------------------------	
		// Lighten : min(source, dest)
		// filter���� 0�� �������� �ȴ�. 0xFF�� �ְ�...
		//------------------------------------------------------------
		// width��ŭ pDest�� pSource�� blt�Ѵ�.
		//memcpyLighten( pDest, pSource, width );		
		register int i = width;
		
		if (i>0)
		{
			do
			{
				///*
				//s = *pSource;
				//d = *pDest;	
				
				t = *pDest + *pSource;
				if (t > 31)
				{
					*pDest = 31;
				}
				else
				{
					*pDest = t;
				}		
					
				pDest++;
				pSource++;		
			} while(--i);
		}
	}

}

//----------------------------------------------------------------------
// Blt Filter Subtract
//----------------------------------------------------------------------
// (x,y)�� filter�� Lighten���� ����Ѵ�.
//----------------------------------------------------------------------
void		
CFilter::BltFilterSub(int x, int y, const CFilter& filter)
{
	BYTE	*pDest;			// ����ҷ��� ��ġ
	BYTE	*pSource;		// ����ҷ��� ��ġ

	int startY, endY;						// y����
	int sourceStartX, destStartX, width;	// x����
	
	register int sourceIndex;
	register int destIndex;

	//-------------------------------------------------
	// Y��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (y > m_Height)
	{
		return;	
	}
	// 0���� ���� ���
	if (y < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (y+filter.m_Height <= 0)
		{
			return;
		}

		// ���� clip
		startY = -y;

		// �Ʒ��� clip�� �Ǵ� ���
		if (y+filter.m_Height > m_Height)
		{
			endY = startY + m_Height;
		}
		else
		{
			endY = filter.m_Height;
		}

		destIndex = 0;
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (y+filter.m_Height > m_Height)
	{
		startY = 0;
		endY = m_Height - y;
		destIndex = y;
	}	
	// y clip�� ���� ���
	else 
	{
		startY = 0;
		endY = filter.m_Height;
		destIndex = y;
	}

	//-------------------------------------------------
	// X��� ���� ����
	//-------------------------------------------------
	// ȭ���� �Ѿ�� ���
	if (x > m_Width)
	{
		return;	
	}
	// 0���� ���� ���
	if (x < 0)
	{		
		// �������ٵ� ȭ�鿡 ��Ÿ���� �ʴ� ���
		if (x+filter.m_Width <= 0)
		{
			return;
		}

		// ������ Clip�� �ʿ��� ���
		if (x+filter.m_Width > m_Width)
		{
			width = m_Width;
		}
		else
		{
			width = filter.m_Width + x;
		}

		// ���� clip
		sourceStartX = -x;
		destStartX = 0;		
	}
	// 0�̻�
	// �Ʒ��� clip
	else if (x+filter.m_Width > m_Width)
	{
		sourceStartX = 0;
		destStartX = x;
		width = m_Width - x;		
	}	
	// X clip�� ���� ���
	else 
	{
		sourceStartX = 0;		
		destStartX = x;
		width = filter.m_Width;
	}


	//int d, s, t;
	int t, d, s;
	//-------------------------------------------------
	// startY���� endY������ source�� dest��...
	//-------------------------------------------------
	for (sourceIndex=startY; sourceIndex<endY; destIndex++, sourceIndex++)
	{
		pSource = filter.m_ppFilter[sourceIndex] + sourceStartX;
		pDest	= m_ppFilter[destIndex] + destStartX;

		//------------------------------------------------------------	
		// Lighten : min(source, dest)
		// filter���� 0�� �������� �ȴ�. 0xFF�� �ְ�...
		//------------------------------------------------------------
		// width��ŭ pDest�� pSource�� blt�Ѵ�.
		//memcpyLighten( pDest, pSource, width );		
		register int i = width;
		
		if (i>0)
		{
			do
			{
				///*
				//s = *pSource;
				//d = *pDest;	
				d = *pDest;
				s = *pSource;
				
				t = d - s;
				if (t < 0)
				{
					*pDest = 0;
				}
				else
				{
					*pDest = (BYTE)t;
				}

				/*
				// s�� ���
                if (d > s)
                {
                        t = s - ((15 - d)>>1);
                        if (t < 0) t = 0;
                        *pDest = t;
                }
				// d�� ���
                else
                {
                        t = d - ((15 - s)>>1);
                        if (t < 0) t = 0;
                        *pDest = t;
                }
				*/

				//*pDest = min( *pDest, *pSource );				
					
				pDest++;
				pSource++;		
			} while(--i);
		}
	}

}

//----------------------------------------------------------------------
// Blt 4444
//----------------------------------------------------------------------
// Texture������ ����� �� �ֵ��� Filter�� pSurface�� ����Ѵ�.
// Filter�� Alpha���� �ǹ��ϰ� �ȴ�.
//
// (-_-;;) Surface�� Filter�� Width�� Height�� ���ٰ� ����.
// �ٸ� ���� ... ������ �� ����.. ����.. 
//----------------------------------------------------------------------
void
CFilter::Blt4444(WORD* pSurface, WORD pitch)
{
	register int i;
	register int j;

	BYTE* pFilter;
	WORD* pSurfaceTemp;

	for (i=0; i<m_Height; i++)
	{
		pSurfaceTemp = pSurface;
		pFilter = m_ppFilter[i];

		j = m_Width;
		do 
		{
			// ������ RGB���� �츱 ���..
			//*pSurfaceTemp = (*pFilter << 12) | (*pSurfaceTemp & 0x0FFF);

			*pSurfaceTemp = (*pFilter << 12);
			pSurfaceTemp ++;
			pFilter ++;
		} while (--j);

		pSurface = (WORD*)((BYTE*)pSurface + pitch);
	}
}

//----------------------------------------------------------------------
// Blt 4444
//----------------------------------------------------------------------
// Texture������ ����� �� �ֵ��� Filter�� pSurface�� ����Ѵ�.
// Filter�� Alpha���� �ǹ��ϰ� �ȴ�.
//
// (-_-;;) Surface�� Filter�� Width�� Height�� ���ٰ� ����.
// �ٸ� ���� ... ������ �� ����.. ����.. 
//----------------------------------------------------------------------
void
CFilter::Blt4444Color(WORD* pSurface, WORD pitch, WORD color)
{
	// R:G:B�� �츰��.
	color &= 0x0FFF;

	register int i;
	register int j;

	BYTE* pFilter;
	WORD* pSurfaceTemp;

	for (i=0; i<m_Height; i++)
	{
		pSurfaceTemp = pSurface;
		pFilter = m_ppFilter[i];

		j = m_Width;
		do 
		{
			// ������ RGB���� �츱 ���..
			*pSurfaceTemp = (*pFilter << 12) | color;

			//*pSurfaceTemp = (*pFilter << 12);
			pSurfaceTemp ++;
			pFilter ++;
		} while (--j);

		pSurface = (WORD*)((BYTE*)pSurface + pitch);
	}
}