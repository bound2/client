//---------------------------------------------------------------------------------
// ExpInfo.cpp
//---------------------------------------------------------------------------------
#include "Client_PCH.h"
#include "ExpInfo.h"

//---------------------------------------------------------------------------------
//
//					StatusInfo
//
//---------------------------------------------------------------------------------
void				
ExpInfo::LoadFromFile(std::ifstream& file)
{
	file.read((char*)&GoalExp, 4);
	file.read((char*)&AccumExp, 4);
}
