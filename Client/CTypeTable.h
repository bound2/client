//----------------------------------------------------------------------
// CTypeTable.h
//----------------------------------------------------------------------
//행행 326220963
#ifndef	__CTYPETABLE_H__
#define	__CTYPETABLE_H__

//#include "DebugInfo.h"
//#define	 new DEBUG_NEW
#include <fstream>

//----------------------------------------------------------------------
//
// Info에 대한 정보 Table
//
//----------------------------------------------------------------------
template <class Type>
class CTypeTable {
	public :
		CTypeTable();
		~CTypeTable();

		//-------------------------------------------------------
		// Init / Release
		//-------------------------------------------------------
		void			Init(int size);
		void			Release();

		//-------------------------------------------------------
		// Size
		//-------------------------------------------------------
		int				GetSize() const		{ return m_Size; }

		//-------------------------------------------------------
		// Debug/Internal access
		//-------------------------------------------------------
		Type*			GetInternalPointer() const { return m_pTypeInfo; }
		
		//-------------------------------------------------------
		// Reference
		//
		// m_Size is whatever the data file the table was loaded
		// from declared, while the ids indexing it come from
		// packets and from compile-time enums a shorter file does
		// not cover, so the range test has to hold in every
		// configuration. It used to be #ifdef _DEBUG - which MSVC
		// defines for Debug and not for Release - so the only
		// build without the check was the one that ships.
		//
		// Out of range yields a default-constructed Type, exactly
		// what the Debug build has always returned. Note that a
		// default MString holds a NULL string, so a caller that
		// copies out of the result still needs its own test.
		//-------------------------------------------------------
		const Type&	operator [] (int type) const {
			if (m_pTypeInfo == NULL || type < 0 || type >= m_Size) {
				static Type dummy;
				return dummy;
			}
			return m_pTypeInfo[type];
		}
		Type&	operator [] (int type) {
			if (m_pTypeInfo == NULL || type < 0 || type >= m_Size) {
				static Type dummy;
				return dummy;
			}
			return m_pTypeInfo[type];
		}
		Type&	Get(int type) {
			if (m_pTypeInfo == NULL || type < 0 || type >= m_Size) {
				static Type dummy;
				return dummy;
			}
			return m_pTypeInfo[type];
		}


		//-------------------------------------------------------
		// File I/O
		//-------------------------------------------------------
		void			SaveToFile(std::ofstream& file);
		void			LoadFromFile(std::ifstream& file);
		void			SaveToFile(const char *filename);
		void			LoadFromFile(const char *filename);
		bool			LoadFromFile_NickNameString(std::ifstream& file);
	protected :
		// Entry counts at or below this are taken on trust; see
		// IsEntryCountSane.
		enum { MAX_UNMEASURED_ENTRIES = 65536 };

		//-------------------------------------------------------
		// Is an entry count read out of a file plausible?
		//
		// Every entry costs at least one byte on disk, so a count
		// larger than what is left of the file cannot describe
		// real entries however small Type is. Fails open: when the
		// stream will not say how much is left, only the sign is
		// judged, and a negative count is always rejected because
		// new Type[negative] is undefined.
		//-------------------------------------------------------
		static bool	IsEntryCountSane(std::ifstream& file, int count)
		{
			if (count < 0)
				return false;

			// No shipped table comes near this, and a count below it
			// cannot ask for an allocation worth refusing. Nested
			// tables run this once per outer entry, so the ordinary
			// case must not pay for the seek below.
			if (count <= MAX_UNMEASURED_ENTRIES)
				return true;

			if (!file.good())
				return false;

			std::streamoff	cur = file.tellg();

			if (cur < 0)
				return true;

			file.seekg(0, std::ios::end);

			std::streamoff	end = file.tellg();

			// put the stream back exactly where it was
			file.clear();
			file.seekg(cur, std::ios::beg);

			if (end < cur)
				return true;

			return (std::streamoff)count <= end - cur;
		}

		int			m_Size;					// number of Types held
		Type*		m_pTypeInfo;			// the Type information

};


//----------------------------------------------------------------------
//
//    constructor/destructor
//
//----------------------------------------------------------------------
template <class Type>
CTypeTable<Type>::CTypeTable()
{
	m_pTypeInfo	= NULL;
	m_Size		= 0;
}

template <class Type>
CTypeTable<Type>::~CTypeTable()
{
	Release();
}

//----------------------------------------------------------------------
//
//  member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Init
//----------------------------------------------------------------------
template <class Type>
void
CTypeTable<Type>::Init(int size)
{
	// nothing to hold; a negative count would make new Type[] undefined
	if (size<=0)
		return;

	// release what is held first
	Release();

	// m_Size is only published once the array exists, so a throwing
	// allocation cannot leave a size behind with no table under it
	Type*	pTypeInfo = new Type [size];

	m_pTypeInfo	= pTypeInfo;
	m_Size		= size;
}


//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
template <class Type>
void
CTypeTable<Type>::Release()
{
	if (m_pTypeInfo != NULL)
	{
		// 모든 CSprite를 지운다.
		delete [] m_pTypeInfo;
		m_pTypeInfo = NULL;
		
		m_Size = 0;
	}
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
template <class Type>
void			
CTypeTable<Type>::SaveToFile(std::ofstream& file)
{
	// size 저장
	file.write((const char*)&m_Size, 4);

	// 아무 것도 없는 경우
	if (m_pTypeInfo==NULL)
		return;

	// 각각의 정보 저장
	for (int i=0; i<m_Size; i++)
	{
		if (i==557)//石头返回效果
		{
			i=i;
		}
		m_pTypeInfo[i].SaveToFile(file);
	}
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
template <class Type>
void			
CTypeTable<Type>::LoadFromFile(std::ifstream& file)
{
	int numSize=0;

	// read the size
	file.read((char*)&numSize, 4);

	// the count is whatever the file says, and Init allocates from it
	if (!IsEntryCountSane(file, numSize))
		return;

	// reallocate when the size differs from what is currently held
	if (m_Size != numSize)
	{
		// release the memory
		Release();

		// allocate
		Init( numSize );
	}

	// read each entry from the file
	for (int i=0; i<m_Size; i++)
	{
		if (i==700)
		{
			i=i;
		}
 		m_pTypeInfo[i].LoadFromFile( file );
	}
}

template <class Type>
void
CTypeTable<Type>::LoadFromFile(const char* lpszFilename)
{
	std::ifstream file(lpszFilename, std::ios::binary );
	if(file.is_open())
	{
		LoadFromFile(file);
		file.close();
	}
}

template <class Type>
void
CTypeTable<Type>::SaveToFile(const char* lpszFilename)
{
	std::ofstream file(lpszFilename, std::ios::binary);
	SaveToFile(file);
	file.close();
}
// 2004, 6, 18 sobeit add start - nick name - 파일 구조가 쩜 틀려서 전용으로 만듬..^^:
//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
template <class Type>
bool			
CTypeTable<Type>::LoadFromFile_NickNameString(std::ifstream& file)
{
	int numSize=0;
	WORD wIndex;
	// read the size
	file.read((char*)&numSize, 4);

	// the count is whatever the file says, and Init allocates from it
	if (!IsEntryCountSane(file, numSize))
		return false;

	// reallocate when the size differs from what is currently held
	if (m_Size != numSize)
	{
		// release the memory
		Release();

		// allocate
		Init( numSize );
	}


	for (int i=0; i<m_Size; i++)
	{
		file.read((char*)&wIndex, 2);
		if(wIndex>=m_Size)
			return false;
 		m_pTypeInfo[wIndex].LoadFromFile( file );
	}
	return true;
}
// 2004, 6, 18 sobeit add start - nick name - 파일 구조가 쩜 틀려서 전용으로 만듬..^^:
#endif