//----------------------------------------------------------------------
// CIndexSprite565.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "CSpriteSurface.h"
#include "CIndexSprite.h"
#include "CIndexSprite565.h"

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// fstream�� save �Ѵ�.    ( file���� 5:6:5�� �����Ѵ�. )
//----------------------------------------------------------------------
bool	
CIndexSprite565::SaveToFile(std::ofstream& file)
{
	// width�� height�� �����Ѵ�.
	file.write((const char*)&m_Width , 2);
	file.write((const char*)&m_Height, 2);

	// NULL�̸� �������� �ʴ´�. ���̸� ����Ǵ� ���̴�.
	if (m_Pixels==NULL || m_Width==0 || m_Height==0)
		return false;
	
	// ���� �� �� ����
	WORD index;	

	int colorCount, transPair, indexCount;

	register int i;
	register int j;

	//--------------------------------
	// 5:6:5
	//--------------------------------
	for (i=0; i<m_Height; i++)
	{
		// �ݺ� ȸ���� 2 byte
		transPair = m_Pixels[i][0];
				
		index	= 1;

		// �� line���� byte���� ��� �����ؾ��Ѵ�.
		for (j=0; j<transPair; j++)
		{			
			// transCount = m_Pixels[i][index];
			index++;	// ���� ��
			indexCount = m_Pixels[i][index++];	// indexPair ��

			index += indexCount;		// index���� ���� ���� �� ��ŭ

			// Normal�� ��
			colorCount = m_Pixels[i][index++];

			// Normal�� ����ŭ ����
			index	+= colorCount;
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
CIndexSprite565::LoadFromFile(std::ifstream& file)
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
	
	m_Pixels = NULL;
	m_Pixels = new WORD* [m_Height];	
	WORD len;

	register int i;

	//--------------------------------
	// 5:6:5
	//--------------------------------
	for (i=0; i<m_Height; i++)
	{
		// byte���� ���� data�� Load�Ѵ�.
		file.read((char*)&len, 2);
		
		m_Pixels[i] = NULL;
//		if(len>0)		//add by viva
			m_Pixels[i] = new WORD [len];		

		file.read((char*)m_Pixels[i], len<<1);
	}	

	m_bInit = true;

	return true;
}