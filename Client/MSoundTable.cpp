//----------------------------------------------------------------------
// MSoundTable.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MSoundTable.h"

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------
SOUND_TABLE*		g_pSoundTable = NULL;


//----------------------------------------------------------------------
// Save
//----------------------------------------------------------------------
void			
SOUNDTABLE_INFO::SaveToFile(class ofstream& file)
{
	Filename.SaveToFile( file );
}

//----------------------------------------------------------------------
// Load
//----------------------------------------------------------------------
void			
SOUNDTABLE_INFO::LoadFromFile(class ifstream& file)
{
	Filename.LoadFromFile( file );
}