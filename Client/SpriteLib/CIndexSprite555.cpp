//----------------------------------------------------------------------
// CIndexSprite555.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "CSpriteSurface.h"
#include "CIndexSprite.h"
#include "CIndexSprite555.h"

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// fstream�� save �Ѵ�.    ( file���� 5:6:5�� �����Ѵ�. )
//----------------------------------------------------------------------
// Normal���� ���ؼ��� 5:6:5�� �ٲ��ָ� �ȴ�.
//----------------------------------------------------------------------
bool	
CIndexSprite555::SaveToFile(std::ofstream& file)
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
	register int k;

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
			indexCount = m_Pixels[i][index++];	// index�� ��

			index += indexCount;	// index���� ���� ���� �� ��ŭ

			// Normal�� ��
			colorCount = m_Pixels[i][index++];

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:5:5�� 5:6:5�� �ٲ㼭 �����ϰ� �ٽ� 5:5:5�� �ٲ��ش�.
			for (k=0; k<colorCount; k++)								
			{
				m_Pixels[i][index] = CDirectDraw::Convert555to565(m_Pixels[i][index]);
				index++;
			}
		}

		// byte���� ���� data�� �����Ѵ�.
		file.write((const char*)&index, 2);
		file.write((const char*)m_Pixels[i], index<<1);

		//-------------------------------------------------------
		// m_Pixels[i]�� �� ���� Save�ߴ�.
		// ���� �ٿ���.. Normal color�� �ش��ϴ� �κ���
		// �ٽ� 5:6:5���� 5:5:5�� �ٲ���� �Ѵ�.
		//-------------------------------------------------------

		// �ݺ� ȸ���� 2 byte
		transPair = m_Pixels[i][0];
				
		WORD	index	= 1;

		for (j=0; j<transPair; j++)
		{			
			// transCount = m_Pixels[i][index];
			index++;	// ���� ��
			indexCount = m_Pixels[i][index++];	// index����

			index += indexCount;	// index���� ���� ���� �� ��ŭ

			// Normal�� ��
			colorCount = m_Pixels[i][index++];

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:6:5�� 5:5:5�� �ٲ۴�.
			for (k=0; k<colorCount; k++)								
			{
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index++;
			}
		}
	}


	return true;
}

//----------------------------------------------------------------------
// fstream���� load�Ѵ�.
//----------------------------------------------------------------------
bool	
CIndexSprite555::LoadFromFile(std::ifstream& file)
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
	register int j;
	register int k;
	//--------------------------------
	// 5:6:5
	//--------------------------------
	for (i=0; i<m_Height; i++)
	{
		// byte���� ���� data�� Load�Ѵ�.
		file.read((char*)&len, 2);
		
		m_Pixels[i] = NULL;
		m_Pixels[i] = new WORD [len];		

		file.read((char*)m_Pixels[i], len<<1);

		
		//-------------------------------------------------------
		// m_Pixels[i]�� �� ���� Load�ߴ�.
		// ���� �ٿ���.. Normal color�� �ش��ϴ� �κ���
		// 5:6:5���� 5:5:5�� �ٲ���� �Ѵ�.
		//-------------------------------------------------------

		int transPair, colorCount, indexCount;

		// �ݺ� ȸ���� 2 byte
		transPair = m_Pixels[i][0];
				
		WORD	index	= 1;

		for (j=0; j<transPair; j++)
		{			
			// transCount = m_Pixels[i][index];
			index++;	// ���� ��
			indexCount = m_Pixels[i][index++];	// indexPair ��

			index += indexCount;	// index���� ���� ���� �� ��ŭ

			// Normal�� ��
			colorCount = m_Pixels[i][index++];

			// m_Pixels[i][index] ~ m_Pixels[i][index+colorCount-1]
			// 5:6:5�� 5:5:5�� �ٲ۴�.
			for (k=0; k<colorCount; k++)								
			{
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index++;
			}
		}
	}	
	
	m_bInit = true;

	return true;
}