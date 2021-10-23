//----------------------------------------------------------------------
// CAlphaSprite565.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "CAlphaSprite.h"
#include "CAlphaSprite565.h"

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// fstream�� save �Ѵ�.    ( file���� 5:6:5�� �����Ѵ�. )
//----------------------------------------------------------------------
bool	
CAlphaSprite565::SaveToFile(std::ofstream& file)
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

	//--------------------------------
	// 5:6:5
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

			index += (colorCount<<1);	// ����� �ƴѰ͸�ŭ +				
		}

		// byte���� ���� data�� �����Ѵ�.
		file.write((const char*)&index, 2);
		file.write((const char*)m_Pixels[i], index<<1);
	}


	return true;
}

//----------------------------------------------------------------------
// fstream���� load�Ѵ�.
//----------------------------------------------------------------------
bool	
CAlphaSprite565::LoadFromFile(std::ifstream& file)
{
	// �̹� �����ִ� memory�� release�Ѵ�.
	Release();

	// width�� height�� Load�Ѵ�.
	file.read((char*)&m_Width , 2);
	file.read((char*)&m_Height, 2);	

	// ���̰� 0�̸� �� Load�Ұ� ������..
	if (m_Width==0 || m_Height==0) 
	{	
		m_bInit = true;

		return true;
	}

	//---------------------------------
	// for OLD version of CAlphaSprite
	//---------------------------------
	//BOOL dummy;
	//file.read((char*)&dummy, 1);	
	//---------------------------------
	
	m_Pixels = NULL;
	m_Pixels = new WORD* [m_Height];	
	//m_Pixels = (WORD**)malloc(sizeof(WORD*)*m_Height);

	WORD len;


	//--------------------------------
	// 5:6:5
	//--------------------------------
	for (int i=0; i<m_Height; i++)
	{
		// byte���� ���� data�� Load�Ѵ�.
		file.read((char*)&len, 2);
		
		m_Pixels[i] = NULL;
		m_Pixels[i] = new WORD [len];		
		//m_Pixels[i] = (WORD*)malloc(sizeof(WORD)*len);		

		file.read((char*)m_Pixels[i], len<<1);
	}	
	
	m_bInit = true;

	return true;
}

