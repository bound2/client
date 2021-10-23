#ifndef __CTYPEPACKVECTOR_H__
#define __CTYPEPACKVECTOR_H__

#include <windows.h>
#include <fstream>
#include <vector>

template <class Type>
class CTypePackVector
{
public:
	CTypePackVector()	{}
	~CTypePackVector()	{ Release(); }
	
	//--------------------------------------------------------
	// Init/Release
	//--------------------------------------------------------
	void	Release()	{ for(int i = 0; i < GetSize(); i++)delete m_vData[i]; m_vData.clear(); }

	//--------------------------------------------------------
	// Add/Remove
	//--------------------------------------------------------
	void	AddData(Type* data, int index = -1)			{ if(index == -1)m_vData.push_back(data); else m_vData.insert(&m_vData[index], data); }
	void	RemoveData(WORD dataID)		{ delete m_vData[dataID]; m_vData.erase(&m_vData[dataID]); }

	//--------------------------------------------------------
	// Size
	//--------------------------------------------------------
	DWORD		GetSize() const { return m_vData.size(); }
	
	//--------------------------------------------------------
	// operator
	//--------------------------------------------------------
	Type*		operator [] (WORD n)	{ return m_vData[n]; }
	Type*		GetData(WORD n)			{ return m_vData[n]; }
	
	//--------------------------------------------------------
	// file I/O
	//--------------------------------------------------------
	bool LoadFromFile(std::ifstream &file);
	bool SaveToFile(std::ofstream &dataFile, std::ofstream &indexFile);
	
	bool LoadFromFile(LPCTSTR lpszFilename);
	bool SaveToFile(LPCTSTR lpszFilename);
	
protected:
	std::vector<Type *>			m_vData;
};

template <class Type>
bool CTypePackVector<Type>::LoadFromFile(LPCTSTR lpszFilename)
{
	std::ifstream file(lpszFilename, ios::binary);
	bool re = LoadFromFile(file);
	file.close();

	return re;
}

template <class Type>
bool CTypePackVector<Type>::SaveToFile(LPCTSTR lpszFilename)
{
	char szIndexFilename[512];
	sprintf(szIndexFilename, "%si", lpszFilename);

	std::ofstream dataFile(lpszFilename, std::ios::binary);
	std::ofstream indexFile(szIndexFilename, std::ios::binary);

	bool re = SaveToFile(dataFile, indexFile);

	dataFile.close();
	indexFile.close();

	return re;
}


template <class Type>
bool CTypePackVector<Type>::LoadFromFile(std::ifstream &file)
{
	Release();

	WORD m_Size;
	file.read((char *)&m_Size, 2);
	
	register int i;

	for(i = 0; i < m_Size; i++)
	{
		Type *data = new Type;
		m_vData.push_back(data);
		data->LoadFromFile(file);
	}
	
	return true;
}

template <class Type>
bool CTypePackVector<Type>::SaveToFile(std::ofstream &dataFile, std::ofstream &indexFile)
{
	//--------------------------------------------------
	// index file�� �����ϱ� ���� ����
	//--------------------------------------------------
//	long*	pIndex = new long [m_Size];
	std::vector<DWORD> vIndex;

	//--------------------------------------------------
	// Size ����
	//--------------------------------------------------
	WORD m_Size = GetSize();
	dataFile.write((const char *)&m_Size, 2); 
	indexFile.write((const char *)&m_Size, 2); 
	WORD realSize = m_Size;
	DWORD index = 0;

	for(int i = 0; i < m_Size; i++)
	{
		index = dataFile.tellp();
		if(m_vData[i]->SaveToFile(dataFile) == false)
		{
			realSize--;
		}
		else
		{
			vIndex.push_back(index);
		}
	}
	
	if(realSize != m_Size)
	{
		char szTemp[512];
		sprintf(szTemp, "real size : %d size : %d", realSize, m_Size);
		MessageBox(NULL, szTemp, "CTypePackVector", MB_OK);

		dataFile.seekp(0);
		dataFile.write((const char *)&realSize, 2);
		indexFile.seekp(0);
		indexFile.write((const char *)&realSize, 2);
	}

	//--------------------------------------------------
	// index ����
	//--------------------------------------------------
	for (int i=0; i<vIndex.size(); i++)
	{
		indexFile.write((const char*)&vIndex[i], 4);
	}
	
	return true;
}


#endif