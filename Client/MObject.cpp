//----------------------------------------------------------------------
// MObject.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MObject.h"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------
MObject::MObject()	
{ 
	m_ObjectType	= TYPE_OBJECT; 
	m_ID			= OBJECTID_NULL;
	m_X	 			= SECTORPOSITION_NULL;
	m_Y				= SECTORPOSITION_NULL;
	ClearScreenRect();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// AddScreenRect and SetScreenRect are in MObjectScreen.cpp (executable):
// they shift by the draw interpolation state the game update owns.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Save to File
//----------------------------------------------------------------------
void	
MObject::SaveToFile(ofstream& file)
{
	file.write((const char*)&m_ObjectType, 1);
	file.write((const char*)&m_ID, SIZE_OBJECTID);
	file.write((const char*)&m_X, SIZE_SECTORPOSITION);
	file.write((const char*)&m_Y, SIZE_SECTORPOSITION);
}

//----------------------------------------------------------------------
// Load from File
//----------------------------------------------------------------------
void	
MObject::LoadFromFile(ifstream& file)
{
	file.read((char*)&m_ObjectType, 1);
	file.read((char*)&m_ID, SIZE_OBJECTID);	
	file.read((char*)&m_X, SIZE_SECTORPOSITION);
	file.read((char*)&m_Y, SIZE_SECTORPOSITION);
}

