//----------------------------------------------------------------------
// CSprite555.cpp
//----------------------------------------------------------------------
#include "client_PCH.h"
#include "CSpriteSurface.h"
#include "CSprite.h"
#include "CSprite555.h"

//extern BYTE	LOADING_STATUS_NONE;
//extern BYTE	LOADING_STATUS_NOW;
//extern BYTE	LOADING_STATUS_LOADING;

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// fstream�� save �Ѵ�.    ( file���� 5:6:5�� �����Ѵ�. )
//----------------------------------------------------------------------
bool
CSprite555::SaveToFile(std::ofstream& file)
{
	// width�� height�� �����Ѵ�.
	file.write((const char*)&m_Width , 2);
	file.write((const char*)&m_Height, 2);

	// NULL�̸� �������� �ʴ´�. ���̸� ����Ǵ� ���̴�.
	if (m_Pixels==NULL || m_Width==0 || m_Height==0)
		return false;
	
	// ���� �� �� ����
	WORD index;	

	register int i;
	register int j;
	register int k;

	//--------------------------------
	// 5:5:5
	//--------------------------------
	for (i=0; i<m_Height; i++)
	{
		// �ݺ� ȸ���� 2 byte
		int	count = m_Pixels[i][0], 
				colorCount;
		index	= 1;

		// �� line���� byte���� ��� �����ؾ��Ѵ�.
		for (j=0; j<count; j++)
		{
			//transCount = m_Pixels[i][index];
			colorCount = m_Pixels[i][index+1];				

			index+=2;	// �� count ��ŭ

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:5:5�� 5:6:5�� �ٲ㼭 �����ϰ� �ٽ� 5:5:5�� �ٲ��ش�.
			for (k=0; k<colorCount; k++)								
			{
				m_Pixels[i][index] = CDirectDraw::Convert555to565(m_Pixels[i][index]);
				index++;
			}

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}

		// byte���� ���� data�� �����Ѵ�.
		file.write((const char*)&index, 2);			
		file.write((const char*)m_Pixels[i], index<<1);			


		// �ٽ� 5:5:5�� �ٲ��ش�.						
		index	= 1;
			
		for (j=0; j<count; j++)
		{
			//transCount = m_Pixels[i][index];
			colorCount = m_Pixels[i][index+1];				

			index+=2;	// �� count ��ŭ

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:5:5�� 5:6:5�� �ٲ㼭 �����ϰ� �ٽ� 5:5:5�� �ٲ��ش�.
			for (k=0; k<colorCount; k++)								
			{					
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index++;
			}

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}
	}

	return true;
}

//----------------------------------------------------------------------
// fstream���� load�Ѵ�.
//----------------------------------------------------------------------
bool	
CSprite555::LoadFromFile(std::ifstream& file)
{
	// �̰Ÿ� �ϳ��� ����� �ϴµ�..
	if (m_bLoading) 
	{	
		return false;
	}
	m_bLoading = true;

	// F --> T
	/*
	static BYTE LoadingStatus = LOADING_STATUS_NONE;
	BYTE*	pCheck = &LoadingStatus;
	
	// Loading�ϰ� ���� ���� ������ ���
	// Loading�ҷ��� ���·� �����.
	InterlockedCompareExchange( 
		(PVOID *)&pCheck,  // pointer to the destination pointer
		(PVOID)&LOADING_STATUS_NOW,      // the exchange value
		(PVOID)&LOADING_STATUS_NONE		// the value to compare
	);
 
	// ���� loading�Ұ��� �ƴϸ� return
	if (LoadingStatus!=LOADING_STATUS_NOW)
	{
		return false;
	}	

	LoadingStatus = LOADING_STATUS_LOADING;
	*/
	// loadind���̶�� ǥ��
	m_bLoading = true;
	

	

	// �̹� �����ִ� memory�� release�Ѵ�.
	Release();

	// width�� height�� �����Ѵ�.
	file.read((char*)&m_Width , 2);
	file.read((char*)&m_Height, 2);	

	// ���̰� 0�̸� �� Load�Ұ� ������..
	if (m_Width==0 || m_Height==0) 
	{	
		m_bInit = true;

		m_bLoading = false;

		return true;
	}

	//---------------------------------
	// for OLD version of CSprite
	//---------------------------------
	//BOOL dummy;
	//file.read((char*)&dummy, 1);	
	//---------------------------------
	
	m_Pixels = new WORD* [m_Height];

	WORD len;

	//--------------------------------
	// 5:5:5
	//--------------------------------
	// 5:6:5�� ����Ȱ� �о��� ������ 5:6:5�� 5:5:5�� �ٲ���� �Ѵ�.	
	WORD index;
	int	count, colorCount;

	register int i;
	register int j;
	register int k;

	for (i=0; i<m_Height; i++)
	{			
		// byte���� ���� data�� Load�Ѵ�.
		file.read((char*)&len, 2);
		m_Pixels[i] = new WORD [len];
		file.read((char*)m_Pixels[i], len<<1);

		count = m_Pixels[i][0];			
		index = 1;

		for (j=0; j<count; j++)
		{
			//transCount = m_Pixels[i][index];
			colorCount = m_Pixels[i][index+1];				

			index+=2;	// �� count ��ŭ

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:5:5�� 5:6:5�� �ٲ㼭 �����ϰ� �ٽ� 5:5:5�� �ٲ��ش�.
			for (k=0; k<colorCount; k++)								
			{					
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index++;
			}

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}
	}

	m_bInit = true;

	m_bLoading = false;
//	LoadingStatus = LOADING_STATUS_NONE;

	return true;
}


//----------------------------------------------------------------------
// fstream���� load�Ѵ�.
//----------------------------------------------------------------------
/*
bool	
CSprite555::LoadFromFileToBuffer(ifstream& file)
{
	// width�� height�� �����Ѵ�.
	file.read((char*)&s_Width , 2);
	file.read((char*)&s_Height, 2);	

	// ���̰� 0�̸� �� Load�Ұ� ������..
	if (s_Width==0 || s_Height==0) 
		return false;

	//---------------------------------
	// for OLD version of CSprite
	//---------------------------------
	//BOOL dummy;
	//file.read((char*)&dummy, 1);	
	//---------------------------------
	
	//--------------------------------
	// 5:5:5
	//--------------------------------
	// 5:6:5�� ����Ȱ� �о��� ������ 5:6:5�� 5:5:5�� �ٲ���� �Ѵ�.	
	WORD	count, index, colorCount;

	for (int i=0; i<s_Height; i++)
	{			
		// byte���� ���� data�� Load�Ѵ�.
		file.read((char*)&s_BufferLen[i], 2);
		
		file.read((char*)s_Buffer[i], s_BufferLen[i]<<1);

		// converter to 5:5:5
		count = s_Buffer[i][0];			
		index = 1;

		for (int j=0; j<count; j++)
		{
			//transCount = s_Buffer[i][index];
			colorCount = s_Buffer[i][index+1];				

			index+=2;	// �� count ��ŭ

			// s_Buffer[i][index] ~ s_Buffer[i][index+colorCount-1]
			// 5:5:5�� 5:6:5�� �ٲ㼭 �����ϰ� �ٽ� 5:5:5�� �ٲ��ش�.
			for (int j=0; j<colorCount; j++)								
			{					
				s_Buffer[i][index] = CDirectDraw::Convert565to555(s_Buffer[i][index]);
				index++;
			}

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}
	}


	return true;
}
*/

