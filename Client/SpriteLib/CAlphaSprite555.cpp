//----------------------------------------------------------------------
// CAlphaSprite555.cpp
//----------------------------------------------------------------------
#include "CSpriteSurface.h"
#include "CAlphaSprite.h"
#include "CAlphaSprite555.h"

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// fstream�� save �Ѵ�.    ( file���� 5:6:5�� �����Ѵ�. )
//----------------------------------------------------------------------
bool
CAlphaSprite555::SaveToFile(std::ofstream& file)
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

			// Alpha�� skip
			index++;
			for (k=0; k<colorCount; k++)								
			{
				m_Pixels[i][index] = CDirectDraw::Convert555to565(m_Pixels[i][index]);

				// Alpha,Color, Alpha,Color, .... ���̱� ������
				// Color������ Color�� �ٲٱ� ���ؼ�..
				index+=2;
			}
			index--;

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
			index++;
			for (k=0; k<colorCount; k++)								
			{					
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index+=2;
			}
			index--;

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}
	}

	return true;
}

//----------------------------------------------------------------------
// fstream���� load�Ѵ�.
//----------------------------------------------------------------------
bool	
CAlphaSprite555::LoadFromFile(std::ifstream& file)
{
	// �̹� �����ִ� memory�� release�Ѵ�.
	Release();

	// width�� height�� �����Ѵ�.
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
	
	m_Pixels = new WORD* [m_Height];

	WORD len;

	//--------------------------------
	// 5:5:5
	//--------------------------------
	// 5:6:5�� ����Ȱ� �о��� ������ 5:6:5�� 5:5:5�� �ٲ���� �Ѵ�.	
	int	count, index, colorCount;

	register int i;
	register int j;

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
			index++;	// Alpha�� skip
			for (int j=0; j<colorCount; j++)								
			{					
				m_Pixels[i][index] = CDirectDraw::Convert565to555(m_Pixels[i][index]);
				index+=2;
			}
			index--;

			//index += colorCount;	// ����� �ƴѰ͸�ŭ +				
		}
	}

	m_bInit = true;

	return true;
}


