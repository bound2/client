//----------------------------------------------------------------------
// MStringMap.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MString.h"
#include "MStringMap.h"

//----------------------------------------------------------------------
// 
// constructor / destructor
//
//----------------------------------------------------------------------
MStringMap::MStringMap()
{
}

MStringMap::~MStringMap()
{
	Release();
}

//----------------------------------------------------------------------
// 
// member functions
//
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void				
MStringMap::Release()
{
	STRING_MAP::iterator iString = begin();

	while (iString != end())
	{
		MString* pKey	= iString->first;
		MString* pValue = iString->second;
		
		//--------------------------------------------------------
		// value를 key값으로 쓰고 있는 경우 
		// --> 하나만 지우면 된다.
		//--------------------------------------------------------
		if (pKey==pValue)
		{
			if (pKey!=NULL)
			{
				delete pKey;
			}
		}
		//--------------------------------------------------------
		// key와 value를 다 지운다.
		//--------------------------------------------------------
		else
		{
			if (pKey!=NULL)
			{
				delete pKey;
			}

			if (pValue!=NULL)
			{
				delete pValue;
			}
		}

		iString++;
	}

	// 다 지우기~
	clear();
}

//----------------------------------------------------------------------
// Add
//----------------------------------------------------------------------
// value가 NULL인 경우는 key를 value로 취급한다.(value를 key로 취급 - -;)
// 새로운 key이면 추가하고 아니면 value를 바꾼다.
//----------------------------------------------------------------------
bool
MStringMap::Add(const char* pKey, const char* pValue)
{
	if (pKey==NULL)
	{
		return false;
	}

	MString* pKeyString = new MString(pKey);
	MString* pValueString;

	//---------------------------------------------------
	// value가 없다면 key를 같이 사용한다.
	//---------------------------------------------------
	if (pValue==NULL)
	{
		pValueString = pKeyString;
	}
	//---------------------------------------------------
	// value가 있는 경우
	//---------------------------------------------------
	else
	{
		pValueString = new MString(pValue);
	}

	STRING_MAP::iterator iString = find( pKeyString );
	
	//---------------------------------------------------
	// key추가
	//---------------------------------------------------	
	if (iString == end())
	{			
		//---------------------------------------------------
		// (key, value)를 추가한다.
		//---------------------------------------------------
		insert( STRING_MAP::value_type( pKeyString, pValueString ) );
	}
	//---------------------------------------------------
	// 이미 key가 있다면
	//---------------------------------------------------
	else
	{
		if (iString->first == iString->second)
		{
			// value가 key로 사용되는 경우이면 지우면 안된다.			
		}
		else
		{			
			delete iString->second;			
		}

		//---------------------------------------------------
		// key값이 value와 같은 경우
		//---------------------------------------------------
		if (*iString->first == *pValueString)
		{
			iString->second = iString->first;

			//---------------------------------------------------
			// 필요없는거 제거
			//---------------------------------------------------
			if (pKeyString==pValueString)
			{
				delete pKeyString;
			}
			else
			{
				delete pKeyString;
				delete pValueString;
			}
		}
		//---------------------------------------------------
		// 다른 경우.. 새로운 value를 대입한다.
		//---------------------------------------------------
		else
		{
			//---------------------------------------------------
			// 새로운 value 대입
			//---------------------------------------------------		
			iString->second = pValueString;

			//---------------------------------------------------
			// 필요없는 key는 제거
			//---------------------------------------------------
			if (pKeyString!=pValueString)
			{
				delete pKeyString;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------
// Remove
//----------------------------------------------------------------------
bool				
MStringMap::Remove(const char* pKey)
{
	MString keyString(pKey);

	STRING_MAP::iterator iString = find( &keyString );
	
	//---------------------------------------------------
	// key가 있으면
	//---------------------------------------------------	
	if (iString != end())
	{			
		//---------------------------------------------------
		// 제거한다.
		//---------------------------------------------------
		if (iString->first == iString->second)
		{
			// value가 key로 사용되는 경우이면 하나만 지우면 된다.
			delete iString->first;
		}
		else
		{
			delete iString->first;
			delete iString->second;			
		}

		//---------------------------------------------------
		// node제거
		//---------------------------------------------------
		erase( iString );

		return true;
	}

	return false;
}

//----------------------------------------------------------------------
// Get
//----------------------------------------------------------------------
const MString*			
MStringMap::Get(const char* pKey) const
{
	MString keyString(pKey);
	
	STRING_MAP::const_iterator iString = find( &keyString );
	
	//---------------------------------------------------
	// key가 없으면
	//---------------------------------------------------	
	if (iString == end())
	{
		return NULL;
	}

	//---------------------------------------------------	
	// value를 넘겨준다.
	//---------------------------------------------------	
	return iString->second;
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
void				
MStringMap::SaveToFile(std::ofstream& file)
{
	int num = size();

	//-----------------------------------------------------
	// size저장
	//-----------------------------------------------------
	file.write((const char*)&num, 4);

	STRING_MAP::iterator iString = begin();

	//-----------------------------------------------------
	// string들 저장
	//-----------------------------------------------------
	bool bSame;
	while (iString != end())
	{
		MString* pKeyString		= iString->first;
		MString* pValueString	= iString->second;

		bSame = (pKeyString==pValueString)? true : false;		

		//---------------------------------------------------
		// key와 value가 같은지 아닌지 체크하기 
		//---------------------------------------------------
		file.write((const char*)&bSame, 1);

		//-----------------------------------------------------
		// 같으면 하나만 저장한다.
		//-----------------------------------------------------
		if (bSame)
		{
			pKeyString->SaveToFile( file );
		}
		//---------------------------------------------------
		// 다르면 둘 다 저장
		//---------------------------------------------------
		else
		{
			pKeyString->SaveToFile( file );
			pValueString->SaveToFile( file );
		}

		iString++;
	}
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
void				
MStringMap::LoadFromFile(std::ifstream& file)
{	
	int num;

	//---------------------------------------------------
	// size 읽기
	//---------------------------------------------------
	file.read((char*)&num, 4);

	//---------------------------------------------------
	// 각각의 string을 읽는다.
	//---------------------------------------------------
	bool bSame;
	for (int i=0; i<num; i++)
	{
		//---------------------------------------------------
		// key와 value가 같은지 아닌지 체크하기 
		//---------------------------------------------------
		file.read((char*)&bSame, 1);

		MString* pKeyString		= new MString;
		MString* pValueString;

		pKeyString->LoadFromFile( file );
		
		//---------------------------------------------------
		// key와 value가 같은 값인 경우..
		//---------------------------------------------------
		if (bSame)
		{
			pValueString = pKeyString;
		}
		//---------------------------------------------------
		// 다른 값이면 하나 더 loading..
		//---------------------------------------------------
		else
		{
			MString* pValueString	= new MString;

			pValueString->LoadFromFile( file );			
		}

		//---------------------------------------------------
		// map에 추가한다.
		//---------------------------------------------------
		insert( STRING_MAP::value_type( pKeyString, pValueString ) );
	}
}