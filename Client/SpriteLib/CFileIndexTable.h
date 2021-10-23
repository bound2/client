//----------------------------------------------------------------------
// CFileIndexTable.h
//----------------------------------------------------------------------
//
// Index File�� Load�Ѵ�.
//
//----------------------------------------------------------------------

#ifndef	__CFILEINDEXTABLE_H__
#define	__CFILEINDEXTABLE_H__

#include <Windows.h>
#include <fstream>

class CFileIndexTable {
	public :
		CFileIndexTable();
		~CFileIndexTable();		

		//--------------------------------------------------------
		// file I/O		
		//--------------------------------------------------------		
		bool			LoadFromFile(std::ifstream& indexFile);		

		WORD				GetSize()					{ return m_Size; }

		//--------------------------------------------------------
		// operator
		//--------------------------------------------------------
		const long&		operator [] (WORD n) { return m_pIndex[n]; }

		//--------------------------------------------------------
		// Release
		//--------------------------------------------------------
		void			Release();


	protected :
		//--------------------------------------------------------
		// Init/Release
		//--------------------------------------------------------
		void			Init(WORD count);		
		

	protected :
		WORD			m_Size;				// ����
		long*			m_pIndex;			// File position
};

#endif
