//----------------------------------------------------------------------
// ZoneFileHeader.h
//----------------------------------------------------------------------

#ifndef	__ZONEFILEHEADER_H__
#define	__ZONEFILEHEADER_H__

#include <Windows.h>
#include <fstream>
#include "MString.h"
#include "MZoneDef.h"

//----------------------------------------------------------------------
// FILEINFO_ZONE_HEADER
//----------------------------------------------------------------------
class FILEINFO_ZONE_HEADER {
public:
	enum ZONE_TYPE
	{
		NORMAL,
		SLAYER_GUILD,
		RESERVED_SLAYER_GUILD,
		NPC_VAMPIRE_LAIR,
		PC_VAMPIRE_LAIR,
		NPC_HOME,
		NPC_SHOP,
		RANDOMAP
	};

public :
	MString		ZoneVersion;	// ���� ������Ʈ�� ��¥
	WORD		ZoneID;			//	�� ���̵�
	WORD		ZoneGroupID;	//	�� �׷� ���̵�
	MString		ZoneName;		// �� �̸�


	BYTE		ZoneType;		//	���� Ÿ��
	BYTE		ZoneLevel;		//	���� ������ ������ ����. 1-10 ����.
	MString		Description;	//	���� ���� ����

public :
	FILEINFO_ZONE_HEADER()
	{
		ZoneVersion = MAP_VERSION_2000_05_10;
	}

	void		SaveToFile(std::ofstream& file);
	void		LoadFromFile(std::ifstream& file);

};

#endif

