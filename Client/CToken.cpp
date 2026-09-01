//------------------------------------------------------------------------
// CToken.cpp
//------------------------------------------------------------------------
#include "Client_PCH.h"
#include "CToken.h"


//------------------------------------------------------------------------
//
// constructor/destructor
//
//------------------------------------------------------------------------
CToken::CToken(const char* str)
{
   m_pString = NULL;
   m_pCurrent = NULL;

   SetString(str);
}

CToken::~CToken()
{
   Release();
}


//------------------------------------------------------------------------
//
// member functions
//
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Free the buffer.
//
// Both pointers have to be cleared, not just the one being freed. Release()
// is called by ~CToken and again at the top of SetString(), and m_pCurrent
// points into the same allocation - so leaving either set makes the second
// Release() a double free and makes any GetToken() in between write ('\0'
// over the delimiter) into memory that is already back on the heap.
//------------------------------------------------------------------------
void
CToken::Release()
{
   delete [] m_pString;

   m_pString  = NULL;
   m_pCurrent = NULL;
}

//------------------------------------------------------------------------
// string을 다시 설정한다.
//------------------------------------------------------------------------
void
CToken::SetString(const char *str)
{
	Release();
	  
   if (str!=NULL)
   {
      // 메모리 잡기
      m_pString = new char [strlen(str)+1];

      strcpy(m_pString, str);

      m_pCurrent = m_pString;
   }
}

//------------------------------------------------------------------------
// delimiter에 의해서 현재의 token string을 구한다.
//------------------------------------------------------------------------
const char*
CToken::GetToken(const char* delimiter)
{
	if (m_pCurrent==NULL)
	{
		return NULL;
	}

	SkipSpace();

   char* pTemp = m_pCurrent;

   // delimiter가 최초로 나타나는 pointer를 구한다.
   char* pFound = strpbrk(m_pCurrent, delimiter);

   // last token
   if (pFound==NULL)
   {
      m_pCurrent = NULL;
   }
   // else
   else
   {
      *pFound = '\0';

      m_pCurrent = pFound+1;
   }

   return pTemp;
}

//------------------------------------------------------------------------
// 현재 위치부터 끝까지의 string을 넘겨준다.
//------------------------------------------------------------------------
const char*
CToken::GetEnd()
{
   SkipSpace();

   char* pTemp = m_pCurrent;

   m_pCurrent = NULL;

   return pTemp;
}

//------------------------------------------------------------------------
// 공백이 여러개 있을때 무시한다.
//------------------------------------------------------------------------
void
CToken::SkipSpace()
{
	while (m_pCurrent!=NULL && *m_pCurrent!='\0' && *m_pCurrent == ' ')
    {
        m_pCurrent += 1;
    }
}

