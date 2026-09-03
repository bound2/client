//----------------------------------------------------------------------
// MSkillManager.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MSkillManager.h"
#include "MTypeDef.h"
#include "MItem.h"		// the item host carries the millisecond clock the delays run on

//----------------------------------------------------------------------
//
// Global
//
//----------------------------------------------------------------------
// SkillInfoTable
MSkillInfoTable	*					g_pSkillInfoTable = NULL;

// SkillManager
MSkillManager*						g_pSkillManager = NULL;

// SkillAvailable
MSkillSet*							g_pSkillAvailable = NULL;

// 
//----------------------------------------------------------------------
//
//							SKILLINFO_NODE
//
//----------------------------------------------------------------------
SKILLINFO_NODE::SKILLINFO_NODE()
{
	m_Level = 1;

	m_SpriteID = 0;

	m_MP = 0;
		
	m_bPassive = false;
	m_bActive = false;
		
	m_ExpLevel = 0;			// 스킬 레벨
	m_SkillExp = 0;			// 스킬 경험치

	m_LearnLevel = 100;
	m_eSkillRace = RACE_SLAYER;

	m_DelayTime = 0;		// the delay before it can be used again
	m_AvailableTime = 0;	// when it can be used again

	m_bEnable = false;

	// Where it sits in its tree, and where it is drawn: the skill file
	// fills all three, and a domain branches on the step, so they are
	// defined here rather than left to whatever the memory held.
	m_SkillStep = SKILL_STEP_NULL;
	m_X = 0;
	m_Y = 0;

	DomainType = 0;		// 그 기술이 어느 도메인에 속하는가.
	minDamage = 0;		// 최소 데미지 또는 효과치.
	maxDamage = 0;		// 최대 데미지 또는 효과치.
	minDelay = 0;		// 최소 사용 딜레이.
	maxDelay = 0;		// 최대 사용 딜레이.
	minCastTime = 0;	// 최소 캐스팅 타임.
	maxCastTime = 0;	// 최대 캐스팅 타임.		
	minDuration = 0;	// 최소 지속 시간
	maxDuration = 0;	// 최대 지속 시간
	minRange = 1;		// 최소 사정거리
	maxRange = 1;		// 최대 사정거리
	maxExp = 0;			// 그 기술의 100% 경험치. 1 회당 + 1 씩 올라감
	SkillPoint = 0;
	LevelUpPoint = 0;
	Fire= 0;
	Water= 0;
	Earth= 0;
	Wind= 0;
	Sum= 0;
	Wristlet= 0;
	Stone1= 0;
	Stone2= 0;
	Stone3= 0;
	Stone4= 0;
	ElementalDomain= 0;
	CanDelete = 0;
}
//----------------------------------------------------------------------
// Save From File ServerSkillInfo
//----------------------------------------------------------------------
void 
SKILLINFO_NODE::SaveFromFileServerSkillInfo(ofstream &file)
{
	
	file.write((char*)&m_LearnLevel, 4);
	file.write((char*)&DomainType, 4);
	m_Name.SaveToFile( file );
	m_HName.SaveToFile( file );
	file.write((char*)&minDamage, 4);		// 최소 데미지 또는 효과치.
	file.write((char*)&maxDamage, 4);		// 최대 데미지 또는 효과치.
	file.write((char*)&minDelay, 4);			// 최소 사용 딜레이.
	file.write((char*)&maxDelay, 4);			// 최대 사용 딜레이.
	file.write((char*)&minDuration, 4);		// 최소 캐스팅 타임.
	file.write((char*)&maxDuration, 4);		// 최대 캐스팅 타임.
	file.write((char*)&m_MP, 4);					// 마나 소모량.(m_MP)
	file.write((char*)&minRange, 4);			// 최소 사정거리
	file.write((char*)&maxRange, 4);			// 최대 사정거리
	file.write((char*)&maxExp, 4);			// 그 기술의 100% 경험치. 1 회당 + 1 씩 올라감	
	
	if(DomainType == SKILLDOMAIN_OUSTERS)
	{
		file.write((char*)&SkillPoint,		sizeof(int));
		file.write((char*)&LevelUpPoint,		sizeof(int));
		int szSkill= SkillTypeList.size();
		file.write((char*)&szSkill,			sizeof(int));
		SKILLTYPE_LIST::const_iterator iSkillID = SkillTypeList.begin();
		while (iSkillID!=SkillTypeList.end())
 		{
 			int skillType = *iSkillID;
 
 			file.write((const char*)&skillType, sizeof(int));
 
 			iSkillID++;
 		}
// 		for(int i = 0; i < szSkill; i++)
// 		{
// 			int skillType;
// 			file.write((char*)&skillType,		sizeof(int));
// 			SkillTypeList.push_back(skillType);
// 		}
		file.write((char*)&Fire,				sizeof(int));
		file.write((char*)&Water,			sizeof(int));
		file.write((char*)&Earth,			sizeof(int));
		file.write((char*)&Wind,				sizeof(int));
		file.write((char*)&Sum,				sizeof(int));
		file.write((char*)&Wristlet,			sizeof(int));
		file.write((char*)&Stone1,			sizeof(int));
		file.write((char*)&Stone2,			sizeof(int));
		file.write((char*)&Stone3,			sizeof(int));
		file.write((char*)&Stone4,			sizeof(int));
		file.write((char*)&ElementalDomain,	sizeof(int));
		file.write((char*)&CanDelete,		sizeof(BYTE));
	} 
}
//----------------------------------------------------------------------
// Load From File ServerSkillInfo
//----------------------------------------------------------------------
void		
SKILLINFO_NODE::LoadFromFileServerSkillInfo(std::ifstream& file)
{
	int ll;
	MString name;
	MString hname;
	int mp;
	
	file.read((char*)&ll, 4);
	file.read((char*)&DomainType, 4);
	name.LoadFromFile( file );
	hname.LoadFromFile( file );
	file.read((char*)&minDamage, 4);		// 최소 데미지 또는 효과치.
	file.read((char*)&maxDamage, 4);		// 최대 데미지 또는 효과치.
	file.read((char*)&minDelay, 4);			// 최소 사용 딜레이.
	file.read((char*)&maxDelay, 4);			// 최대 사용 딜레이.
	file.read((char*)&minDuration, 4);		// 최소 캐스팅 타임.
	file.read((char*)&maxDuration, 4);		// 최대 캐스팅 타임.
	file.read((char*)&mp, 4);					// 마나 소모량.(m_MP)
	file.read((char*)&minRange, 4);			// 최소 사정거리
	file.read((char*)&maxRange, 4);			// 최대 사정거리
	file.read((char*)&maxExp, 4);			// 그 기술의 100% 경험치. 1 회당 + 1 씩 올라감	
	
	if(DomainType == SKILLDOMAIN_OUSTERS)
	{
		file.read((char*)&SkillPoint,		sizeof(int));
		file.read((char*)&LevelUpPoint,		sizeof(int));
		int szSkill;
		file.read((char*)&szSkill,			sizeof(int));
		SkillTypeList.clear();
		for(int i = 0; i < szSkill; i++)
		{
			int skillType;
			file.read((char*)&skillType,		sizeof(int));
			SkillTypeList.push_back(skillType);
		}
		file.read((char*)&Fire,				sizeof(int));
		file.read((char*)&Water,			sizeof(int));
		file.read((char*)&Earth,			sizeof(int));
		file.read((char*)&Wind,				sizeof(int));
		file.read((char*)&Sum,				sizeof(int));
		file.read((char*)&Wristlet,			sizeof(int));
		file.read((char*)&Stone1,			sizeof(int));
		file.read((char*)&Stone2,			sizeof(int));
		file.read((char*)&Stone3,			sizeof(int));
		file.read((char*)&Stone4,			sizeof(int));
		file.read((char*)&ElementalDomain,	sizeof(int));
		file.read((char*)&CanDelete,		sizeof(BYTE));
	} else
	{
		ElementalDomain = -1;
	}
	if (name != "Empty Skill")
	{
		m_LearnLevel = ll;
		m_Name = name;
		m_HName = hname;
		m_MP = mp;
	}
}

//----------------------------------------------------------------------
// Add NextSkill
//----------------------------------------------------------------------
// 다음에 배울 수 있는 Skill들을 설정한다.
//----------------------------------------------------------------------
bool			
SKILLINFO_NODE::AddNextSkill(ACTIONINFO id)
{
	SKILLID_LIST::iterator iSkill = m_listNextSkill.begin();

	// sort해서 add한다.
	while (iSkill != m_listNextSkill.end())
	{
		// 이미 있으면 추가 불가
		if (*iSkill==id)
		{
			return false;
		}
		// 큰거 앞에..
		else if (*iSkill > id)
		{
			// 앞에 추가한다.
			m_listNextSkill.insert( iSkill, id );

			return true;
		}

		iSkill++;
	}

	m_listNextSkill.push_back( id );

	return true;
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
void		
SKILLINFO_NODE::SaveToFile(std::ofstream& file)
{

	m_Name.SaveToFile( file );							// 기술 이름
	m_HName.SaveToFile( file );
	file.write((const char*)&m_Level, 4);
	file.write((const char*)&m_X, 4);
	file.write((const char*)&m_Y, 4);					// 화면에서의 출력 시작 위치
	file.write((const char*)&m_SpriteID, SIZE_SPRITEID);	// 기술의 Icon Sprite
	file.write((const char*)&m_MP, 4);				// MP소비량
	file.write((const char*)&m_bPassive, 1);		// passive skill인가?
	file.write((const char*)&m_bActive, 1);			// 항상 사용 가능한 skill인가?

	BYTE skillStep = m_SkillStep;
	file.write((const char*)&skillStep, 1);

	// id list 저장
	int idNum = m_listNextSkill.size();
	file.write((const char*)&idNum, 4);
	SKILLID_LIST::const_iterator iSkillID = m_listNextSkill.begin();

	while (iSkillID!=m_listNextSkill.end())
	{
		TYPE_ACTIONINFO id = *iSkillID;

		file.write((const char*)&id, SIZE_ACTIONINFO);

		iSkillID++;
	}
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
void		
SKILLINFO_NODE::LoadFromFile(std::ifstream& file)
{
	m_Name.LoadFromFile( file );					// 기술 이름
	m_HName.LoadFromFile( file );
	file.read((char*)&m_Level, 4);
	file.read((char*)&m_X, 4);
	file.read((char*)&m_Y, 4);						// 화면에서의 출력 시작 위치
	file.read((char*)&m_SpriteID, SIZE_SPRITEID);	// 기술의 Icon Sprite
	file.read((char*)&m_MP, 4);						// MP소비량
	file.read((char*)&m_bPassive, 1);				// passive 스킬?
	file.read((char*)&m_bActive, 1);				// 항상 사용 가능한 skill인가?

	BYTE skillStep;
	file.read((char*)&skillStep, 1);
	m_SkillStep = (SKILL_STEP)skillStep;
	// id list load
	int idNum;
	file.read((char*)&idNum, 4);

	m_listNextSkill.clear();
	for (int i=0; i<idNum; i++)
	{
		TYPE_ACTIONINFO id;

		file.read((char*)&id, SIZE_ACTIONINFO);

		m_listNextSkill.push_back( (ACTIONINFO)id );
	}	

	// 배운 level
	m_ExpLevel = 0;
}	

//----------------------------------------------------------------------
// Set DelayTime ( delay )
//----------------------------------------------------------------------
// 기술 사용후 다시 사용할 수 있는 delay시간 설정
//----------------------------------------------------------------------
void
SKILLINFO_NODE::SetDelayTime(DWORD delay)		
{
	// 3초 이하 기술은 delay가 없는 걸로 표시한다.
	if (delay < 1800)
	{
		delay = 0;
	}			
				
	m_DelayTime = delay;	
}

//----------------------------------------------------------------------
// Is AvailableTime ?
//----------------------------------------------------------------------
// 지금 사용 가능한가?
//----------------------------------------------------------------------
bool
SKILLINFO_NODE::IsAvailableTime() const
{
	// The delays run on the item host's millisecond clock; without one
	// (a test binary) there is no delay.
	const DWORD* pClock = MItem::Clock();

	return pClock==NULL || *pClock >= m_AvailableTime;
}

//----------------------------------------------------------------------
// Get AvailableTimeLeft
//----------------------------------------------------------------------
// 남은 사용 가능 시간
//----------------------------------------------------------------------
DWORD				
SKILLINFO_NODE::GetAvailableTimeLeft() const
{
	const DWORD* pClock = MItem::Clock();

	if (pClock!=NULL)
	{
		int timeGap = (int)m_AvailableTime - (int)*pClock;

		if (timeGap > 0)
		{
			return timeGap;
		}
	}

	return 0;
}

//----------------------------------------------------------------------
// Set AvailableTime
//----------------------------------------------------------------------
// 지금 바로 사용 가능하게 설정한다.
//----------------------------------------------------------------------
void
SKILLINFO_NODE::SetAvailableTime(int delay)
{
	// No delay at all reads as zero rather than as "now".
	const DWORD* pClock = MItem::Clock();

	if(delay == 0)
		m_AvailableTime = 0;
	else
		m_AvailableTime = (pClock!=NULL ? *pClock : 0) + delay;
}

//----------------------------------------------------------------------
// Set Next AvailableTime
//----------------------------------------------------------------------
// 다음 사용 가능한 시간을 결정한다.
//----------------------------------------------------------------------
void
SKILLINFO_NODE::SetNextAvailableTime()
{
	// Available again once the delay has run from now.
	const DWORD* pClock = MItem::Clock();

	m_AvailableTime = (pClock!=NULL ? *pClock : 0) + m_DelayTime;
}

//----------------------------------------------------------------------
// Set Enable
//----------------------------------------------------------------------
void				
SKILLINFO_NODE::SetEnable(bool enable)
{
	m_bEnable = enable;
}

//----------------------------------------------------------------------
// Set Disable
//----------------------------------------------------------------------
/*
void				
SKILLINFO_NODE::SetDisable()
{
	m_bEnable = false;
}
*/

//----------------------------------------------------------------------
//
//							MSkillSet
//
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Add Skill
//----------------------------------------------------------------------
bool			
MSkillSet::AddSkill(ACTIONINFO id, BYTE flag)
{
	SKILLID_MAP::iterator	iSkill;

	iSkill = find( id );
	
	//-----------------------------------------------
	// 아직 없는 Skill이면 추가	
	//-----------------------------------------------
	if (iSkill == end())
	{
		insert(SKILLID_MAP::value_type( id, SKILLID_NODE(id, flag) ));

		return true;
	}
	
	//-----------------------------------------------
	// 이미 있다면 flag만 바꾼다.
	//-----------------------------------------------
	SKILLID_NODE& node = (*iSkill).second;
	
	node.Flag = flag;

	return false;
}

//----------------------------------------------------------------------
// Remove Skill
//----------------------------------------------------------------------
// SkillID 제거
//----------------------------------------------------------------------
bool			
MSkillSet::RemoveSkill(ACTIONINFO id)
{
	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = find(id);
    
	//--------------------------------------------------
	// 그런 id를 가진 Skill이 없는 경우
	//--------------------------------------------------
	if (iSkill == end())
	{
		return false;
	}

	//--------------------------------------------------
	// 찾은 경우 --> 제거	
	//--------------------------------------------------
	// map에서 제거
	erase( iSkill );

	return true;
}

//----------------------------------------------------------------------
// Is Enable Skill?
//----------------------------------------------------------------------
// id의 skill이 사용 가능한가?
//----------------------------------------------------------------------
bool			
MSkillSet::IsEnableSkill(ACTIONINFO id) const
{
	SKILLID_MAP::const_iterator		iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = find(id);
    
	//--------------------------------------------------
	// 그런 id를 가진 Skill이 없는 경우
	//--------------------------------------------------
	if (iSkill == end())
	{
		return false;
	}

	//--------------------------------------------------
	// 찾은 경우 --> Is enable?
	//--------------------------------------------------
			
	return ((*iSkill).second).IsEnable()!=0;	
}

//----------------------------------------------------------------------
// Enable Skill
//----------------------------------------------------------------------
bool			
MSkillSet::EnableSkill(ACTIONINFO id)
{
	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = find(id);
    
	//--------------------------------------------------
	// 그런 id를 가진 Skill이 없는 경우
	//--------------------------------------------------
	if (iSkill == end())
	{
		return false;
	}

	//--------------------------------------------------
	// 찾은 경우 --> enable
	//--------------------------------------------------
	// map에서 제거
	((*iSkill).second).SetEnable();

	return true;
}

//----------------------------------------------------------------------
// Disable Skill 
//----------------------------------------------------------------------
bool			
MSkillSet::DisableSkill(ACTIONINFO id)	
{
	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = find(id);
    
	//--------------------------------------------------
	// 그런 id를 가진 Skill이 없는 경우
	//--------------------------------------------------
	if (iSkill == end())
	{
		return false;
	}

	//--------------------------------------------------
	// 찾은 경우 --> enable
	//--------------------------------------------------
	// map에서 제거
	((*iSkill).second).SetDisable();

	return true;
}

//----------------------------------------------------------------------
//
//							MSkillDomain
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
// constructor / destructor
//
//----------------------------------------------------------------------
MSkillDomain::MSkillDomain()
{
	m_bNewSkill			= false;

	m_MaxLevel			= 0;
	m_MaxLearnedLevel	= -1;
	m_pLearnedSkillID	= NULL;
	m_DomainLevel		= 0;
	m_DomainExpRemain	= 0;

	m_DomainExpTable.Init( 151 );
}

MSkillDomain::~MSkillDomain()
{
	if (m_pLearnedSkillID!=NULL)
	{
		delete [] m_pLearnedSkillID;
	}


	//---------------------------------------------------------
	// Skill Step map 지우기
	//---------------------------------------------------------
	SKILL_STEP_MAP::iterator iList = m_mapSkillStep.begin();

	while (iList != m_mapSkillStep.end())
	{
		SKILL_STEP_LIST* pList = iList->second;

		if (pList != NULL)
		{
			delete pList;
		}
		
		iList ++;
	}

	m_mapSkillStep.clear();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Set Max Level
//----------------------------------------------------------------------
// domain의 최고 level 기술
//----------------------------------------------------------------------
void		
MSkillDomain::SetMaxLevel()
{
	if (m_pLearnedSkillID!=NULL)
	{
		delete [] m_pLearnedSkillID;
	}

	m_pLearnedSkillID = new ACTIONINFO [m_MaxLevel+1];	// 0부터 시작한다.
	m_MaxLearnedLevel = -1;

	for (int i=0; i<=m_MaxLevel; i++)
	{
		m_pLearnedSkillID[i] = MAX_ACTIONINFO;//ACTIONINFO_NULL;
	}
}

//----------------------------------------------------------------------
// Clear
//----------------------------------------------------------------------
void			
MSkillDomain::Clear()
{ 
//	SKILLID_MAP::iterator	iSkill = m_mapSkillID.begin();
//	SKILLID_MAP::iterator	endItr = m_mapSkillID.end();
//
//	while (iSkill!=endItr)
//	{		
//		//-----------------------------------------------
//		// 현재 사용 가능한 Skill에서 제거한다.
//		//-----------------------------------------------
//		(*g_pSkillAvailable).RemoveSkill( (*iSkill).first );
//
//		iSkill++;
//	}
//
//	m_iterator = NULL; 
//	m_mapSkillID.clear();

	ClearSkillList();

	//-----------------------------------------------
	// 배운 level 제거
	//-----------------------------------------------
	if (m_pLearnedSkillID!=NULL)
	{
		delete [] m_pLearnedSkillID;
		
		m_MaxLevel = 0;
		m_MaxLearnedLevel = -1;	
		m_pLearnedSkillID = NULL;
	}
}

void			
MSkillDomain::ClearSkillList()
{ 
	SKILLID_MAP::iterator	iSkill = m_mapSkillID.begin();
	SKILLID_MAP::iterator	endItr = m_mapSkillID.end();

	while (iSkill!=endItr)
	{		
		//-----------------------------------------------
		// 현재 사용 가능한 Skill에서 제거한다.
		//-----------------------------------------------
		(*g_pSkillAvailable).RemoveSkill( (*iSkill).first );

		iSkill++;
	}

	m_mapSkillID.clear();

	if(m_pLearnedSkillID != NULL)
	{
		delete [] m_pLearnedSkillID;
		m_pLearnedSkillID = NULL;
	}
}

//----------------------------------------------------------------------
// AddSkill
//----------------------------------------------------------------------
// id와 그의 하위에 있는 것들을 모두 추가한다.
// 단 id의 skill의 level은 0이어야 한다.
//----------------------------------------------------------------------
bool			
MSkillDomain::SetRootSkill(ACTIONINFO id, bool reset)
{
	//--------------------------------------------------
	// 다 지운다.
	//--------------------------------------------------
	//Clear();
	if(reset)
		m_DomainLevel		= 0;

	//--------------------------------------------------
	// level이 0인 skill이어야 한다.
	//--------------------------------------------------
//	if ((*g_pSkillInfoTable)[id].GetLevel()==0)
	{
		int oldMaxLevel = m_MaxLevel;

		if( reset )
			m_MaxLevel = 0;
		bool bOK = AddSkill( id );		
		
		if (oldMaxLevel > m_MaxLevel && reset)
		{
			m_MaxLevel = oldMaxLevel;
		}

		//--------------------------------------------------
		// 제대로 추가된 경우
		//--------------------------------------------------
		if (bOK)
		{
			//--------------------------------------------------
			// domain의 skill level을 설정해준다.
			//--------------------------------------------------
			SetMaxLevel();
		}
	}

	return false;
}

//----------------------------------------------------------------------
// AddSkill
//----------------------------------------------------------------------
// id와 그의 하위에 있는 것들을 모두 추가한다.
//----------------------------------------------------------------------
bool
MSkillDomain::AddSkill(ACTIONINFO id)
{
	SKILLID_MAP::iterator	iSkill;

	if(SKILL_ABERRATION == id)
		int a= 0;

	iSkill = m_mapSkillID.begin();

	while( iSkill != m_mapSkillID.end() )
	{
		if( iSkill->first == id )
		{
			break;
		}
		iSkill++;
	}
	int skillLevel = (*g_pSkillInfoTable)[id].GetLevel();

	//--------------------------------------------------
	// domain 최고 skill level을 찾는다.
	//--------------------------------------------------
	if (m_MaxLevel < skillLevel)
	{
		m_MaxLevel = skillLevel;
	}

	//-----------------------------------------------
	// 아직 없는 Skill이면 추가	
	//-----------------------------------------------
	if (iSkill == m_mapSkillID.end() )
	{		
		//-----------------------------------------------
		// root level이면 다음에 배울 수 있는 걸로 체크한다.
		//-----------------------------------------------
		if(id == SKILL_ABERRATION)
			int a =0 ;
		if (skillLevel==0)
		{
			m_mapSkillID.insert(SKILLID_MAP::value_type( id, SKILLSTATUS_NEXT ));
		}
		else
		{
			m_mapSkillID.insert(SKILLID_MAP::value_type( id, SKILLSTATUS_OTHER ));
		}	
		
		//-----------------------------------------------
		// 이 skill의 SkillStep에 관한 정보를 설정한다.
		//-----------------------------------------------

		SKILL_STEP skillStep = (*g_pSkillInfoTable)[id].GetSkillStep();
		if(skillStep == SKILL_STEP_ETC && id == SKILL_SOUL_CHAIN)
		{
			if(this == &(*g_pSkillManager)[SKILL_DOMAIN_VAMPIRE])
				AddSkillStep(SKILL_STEP_VAMPIRE_INNATE, id);
			else if( this == &(*g_pSkillManager)[SKILL_DOMAIN_OUSTERS])
				AddSkillStep(SKILL_STEP_OUSTERS_ETC, id);
			else
				AddSkillStep(SKILL_STEP_APPRENTICE, id);
		}
		else
		{
			AddSkillStep(skillStep, id);
		}

		//--------------------------------------------------
		// 다음에 배울 수 있는 것들을 찾아서 추가한다.
		//--------------------------------------------------
		const SKILLINFO_NODE::SKILLID_LIST& listNextSkill = (*g_pSkillInfoTable)[id].GetNextSkillList();

		SKILLINFO_NODE::SKILLID_LIST::const_iterator iNextSkill = listNextSkill.begin();

		while (iNextSkill != listNextSkill.end())
		{
			//--------------------------------------------------
			// ID가 *iNextSkil인 Skill를 찾는다.
			//--------------------------------------------------
			iSkill = m_mapSkillID.find( *iNextSkill );

			//--------------------------------------------------
			// 아직 없는 경우에 추가한다.
			//--------------------------------------------------
			if (iSkill == m_mapSkillID.end())
			{
				AddSkill( *iNextSkill );
			}

			iNextSkill++;
		}

		return true;
	}

	//-----------------------------------------------
	// 이미 있는 Skill이면 false
	//-----------------------------------------------
	return false;
}


//----------------------------------------------------------------------
// Set SkillStatus
//----------------------------------------------------------------------
// skill의 상태를 변경한다.
//----------------------------------------------------------------------
/*
bool		
MSkillDomain::SetSkillStatus(ACTIONINFO id, SKILLSTATUS status)
{
	SKILLID_MAP::iterator	iSkill;

	iSkill = m_mapSkillID.find( id );
	
	//-----------------------------------------------
	// domain에 있는 Skill이면..
	//-----------------------------------------------
	if (iSkill != m_mapSkillID.end())
	{
		m_mapSkillID.insert(SKILLID_MAP::value_type( id, status ));

		switch (status)
		{
			case SKILLSTATUS_LEARNED :		// 배웠다.
				//-----------------------------------------------
				// 현재 사용 가능한 Skill에 추가한다.
				//-----------------------------------------------
				//(*g_pSkillAvailable).AddSkill( id );
				LearnSkill(id);
			break;

			case SKILLSTATUS_NEXT :				// 다음에 배울 수 있다.
			case SKILLSTATUS_OTHER :			// 아직은 배울 수 없다
				//-----------------------------------------------
				// 현재 사용 가능한 Skill에서 제거한다.
				//-----------------------------------------------
				(*g_pSkillAvailable).RemoveSkill( id );
			break;
		}

		return true;
	}

	//-----------------------------------------------
	// 이미 있는 Skill이면 false
	//-----------------------------------------------
	return false;
}
*/

//----------------------------------------------------------------------
// Get SkillStatus
//----------------------------------------------------------------------
// id의 상태는?
//----------------------------------------------------------------------
MSkillDomain::SKILLSTATUS		
MSkillDomain::GetSkillStatus(ACTIONINFO id) const
{
	SKILLID_MAP::const_iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = m_mapSkillID.find(id);

	//--------------------------------------------------
	// 없을 경우 NULL을 return한다.
	//--------------------------------------------------
	if (iSkill == m_mapSkillID.end()) 
	{
		return SKILLSTATUS_NULL;
	}

	//--------------------------------------------------
	// 있으면 그 Skill를 return한다.
	//--------------------------------------------------
	return (*iSkill).second;
}

//----------------------------------------------------------------------
// Remove Skill
//----------------------------------------------------------------------
// mapSkill에서 제거하고 Skill의 pointer를 넘겨준다.
//----------------------------------------------------------------------
/*
bool
MSkillDomain::RemoveSkill(ACTIONINFO id)
{
	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = m_mapSkillID.find(id);
    
	//--------------------------------------------------
	// 그런 id를 가진 Skill이 없는 경우
	//--------------------------------------------------
	if (iSkill == m_mapSkillID.end())
	{
		return false;
	}

	//--------------------------------------------------
	// 찾은 경우 --> 제거	
	//--------------------------------------------------
	// map에서 제거
	m_mapSkillID.erase( iSkill );

	return true;
}
*/

//----------------------------------------------------------------------
// Add NextSkill
//----------------------------------------------------------------------
// 다음에 배울 수 있는 기술들을 체크한다.
//----------------------------------------------------------------------
void
MSkillDomain::AddNextSkill(ACTIONINFO id)
{
	const SKILLINFO_NODE::SKILLID_LIST& listNextSkill = (*g_pSkillInfoTable)[id].GetNextSkillList();

	SKILLINFO_NODE::SKILLID_LIST::const_iterator iNextSkill = listNextSkill.begin();

	while (iNextSkill != listNextSkill.end())
	{
		//--------------------------------------------------
		// ID가 *iNextSkil인 Skill를 찾는다.
		//--------------------------------------------------
		SKILLID_MAP::iterator iSkill = m_mapSkillID.find( *iNextSkill );

		//--------------------------------------------------
		// 있으면 그 Skill의 값을 바꾼다.
		//--------------------------------------------------
		// 물론, 현재 domain에 속해있는 경우에만 가능하고
		// 아직 배우지 않은 것일 경우에만 NEXT로 설정한다.
		//--------------------------------------------------
		if (iSkill != m_mapSkillID.end())
		{
			if ((*iSkill).second==SKILLSTATUS_OTHER)
			{						
				(*iSkill).second = SKILLSTATUS_NEXT;
			}
		}

		iNextSkill++;
	}
}
void
MSkillDomain::AddNextSkillForce(ACTIONINFO id)
{
	if (id==MAX_ACTIONINFO)
	{
		return;
	}
	bool HasChildSkill = false;
//	const SKILLINFO_NODE::SKILLID_LIST& listNextSkill = (*g_pSkillInfoTable)[id].GetNextSkillList();
//
//	SKILLINFO_NODE::SKILLID_LIST::const_iterator iNextSkill = listNextSkill.begin();
//
//	while (iNextSkill != listNextSkill.end())
//	{
//		//--------------------------------------------------
//		// ID가 *iNextSkil인 Skill를 찾는다.
//		//--------------------------------------------------
//		SKILLID_MAP::iterator iSkill = m_mapSkillID.find( *iNextSkill );
//
//		//--------------------------------------------------
//		// 있으면 그 Skill의 값을 바꾼다.
//		//--------------------------------------------------
//		// 물론, 현재 domain에 속해있는 경우에만 가능하고
//		// 아직 배우지 않은 것일 경우에만 NEXT로 설정한다.
//		//--------------------------------------------------
//		if (iSkill != m_mapSkillID.end())
//		{
//			if ((*iSkill).second==SKILLSTATUS_LEARNED)
//			{		
//				HasChildSkill = true; // 자기 밑에 딸린 스킬중에 배운스킬이 없어야 한다.
//			}
//		}
//
//		iNextSkill++;
//	}
	if(!HasChildSkill && (*g_pSkillInfoTable)[id].CanDelete)// 자기밑에 딸린 스킬이 없고 삭제 가능한 스킬이면
	{
		SKILLID_MAP::iterator	iSkill = m_mapSkillID.find( id );
		if (iSkill != m_mapSkillID.end())
		{
			if ((*iSkill).second==SKILLSTATUS_LEARNED)
			{						
				(*iSkill).second = SKILLSTATUS_NEXT;
			//	RemoveNextSkill(id);
			}

		}
	}
}

//----------------------------------------------------------------------
// Remove NextSkill
//----------------------------------------------------------------------
// 다음에 배울 수 있게 표시된 기술들을 제거한다.
//----------------------------------------------------------------------
void
MSkillDomain::RemoveNextSkill(ACTIONINFO id)
{
	if (id==MAX_ACTIONINFO)
	{
		return;
	}

	SKILLID_MAP::iterator iPreviousSkill = m_mapSkillID.find( id );

	// 바로 전에 배웠던 기술의 ID		
	ACTIONINFO previousID = (*iPreviousSkill).first;

	//--------------------------------------------------
	//
	// 다음에 배울 수 있는 것들을 찾아서 설정한다.
	//
	//--------------------------------------------------
	const SKILLINFO_NODE::SKILLID_LIST& listNextSkill = (*g_pSkillInfoTable)[previousID].GetNextSkillList();

	SKILLINFO_NODE::SKILLID_LIST::const_iterator iNextSkill = listNextSkill.begin();

	while (iNextSkill != listNextSkill.end())
	{
		//--------------------------------------------------
		// ID가 *iNextSkil인 Skill를 찾는다.
		//--------------------------------------------------
		SKILLID_MAP::iterator	iSkill = m_mapSkillID.find( *iNextSkill );

		//--------------------------------------------------
		// 있으면 그 Skill의 값을 바꾼다.
		//--------------------------------------------------
		// 물론, 현재 domain에 속해있는 경우에만 가능하고
		// NEXT이면 OTHER로 바꾼다.
		//--------------------------------------------------
		if (iSkill != m_mapSkillID.end())
		{
			if ((*iSkill).second==SKILLSTATUS_NEXT)
			{						
				(*iSkill).second = SKILLSTATUS_OTHER;
			}
		}

		iNextSkill++;
	}
}

//----------------------------------------------------------------------
// Learn Skill
//----------------------------------------------------------------------
// id의 skill을 Learn상태로 바꾼다.
// 다음에 배울 수 있는 skill들을 Next상태로 바꾼다.
//
// 같은 level의 기술은 하나밖에 배울 수 없다.
// 그렇게 되면, 현재 배울 수 있는 기술은 최고의 level인 기술이다. (m_MaxLevel)
//----------------------------------------------------------------------
bool
MSkillDomain::LearnSkill(ACTIONINFO id)
{
	//--------------------------------------------------
	// 새로운 기술을 배울 수 없으면..
	//--------------------------------------------------
	if (!m_bNewSkill)
	{
		return false;
	}


	//--------------------------------------------------
	// 이번에 배울 수 있는 level의 기술이 아니면 return
	//--------------------------------------------------
	if ((*g_pSkillInfoTable)[id].GetLevel()!=m_MaxLearnedLevel+1)
	{
	//	return false;
	}

	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = m_mapSkillID.find(id);

	//--------------------------------------------------
	// 없을 경우 NULL을 return한다.
	//--------------------------------------------------
	if (iSkill == m_mapSkillID.end()) 
	{
		return false;
	}

	//--------------------------------------------------
	// 이미 배운 것이면 return false
	//--------------------------------------------------
	if ((*iSkill).second==SKILLSTATUS_LEARNED)
	{
		return false;
	}

	
	//-----------------------------------------------
	// 현재 사용 가능한 Skill에 추가한다.
	//-----------------------------------------------
	(*g_pSkillAvailable).AddSkill( id );

	//-----------------------------------------------
	// 배운 기술 level 체크
	//-----------------------------------------------
	int skillLevel = (*g_pSkillInfoTable)[id].GetLevel();

	// 현재 level에서 배운 기술 설정
	m_pLearnedSkillID[skillLevel] = id;

	if (skillLevel > m_MaxLearnedLevel)
	{
		m_MaxLearnedLevel = skillLevel;

		//--------------------------------------------------
		//
		// 현재에 배울 수 있게 표시된 것들을 모두 없애준다.
		//
		//--------------------------------------------------
		// 바로 전 level의 하위 level
		//--------------------------------------------------
		//if (m_MaxLearnedLevel > 0)
		//{	
			///RemoveNextSkill( m_pLearnedSkillID[m_MaxLearnedLevel-1] );
		//}

		//--------------------------------------------------
		// 배울 수 있다고 표시된거 모두 제거
		//--------------------------------------------------
		SKILLID_MAP::iterator iSkill2 = m_mapSkillID.begin();

		while (iSkill2 != m_mapSkillID.end())
		{
			if ((*iSkill2).second==SKILLSTATUS_NEXT)
			{						
				(*iSkill2).second = SKILLSTATUS_OTHER;
			}

			iSkill2++;
		}

		//--------------------------------------------------
		// 배웠다고 체크한다.
		//--------------------------------------------------
		(*iSkill).second = SKILLSTATUS_LEARNED;	

		//--------------------------------------------------
		//
		// 다음에 배울 수 있는 것들을 찾아서 설정한다.
		//
		//--------------------------------------------------
		AddNextSkill( id );
	}	
	else
	{
		//--------------------------------------------------
		// 배웠다고 체크한다.
		//--------------------------------------------------
		(*iSkill).second = SKILLSTATUS_LEARNED;	
	}

	m_bNewSkill = false;

	return true;
}

//----------------------------------------------------------------------
// UnLearn Skill
//----------------------------------------------------------------------
// id의 skill을 안 배운 상태로 바꾼다.
//
// 제거된 것들 중에서 최고 level의 기술의
// 다음에 배울 수 있는 skill들을 Next상태로 바꾼다.
//
//----------------------------------------------------------------------
bool
MSkillDomain::UnLearnSkill(ACTIONINFO id)
{
	//--------------------------------------------------
	// 현재 최고 level의 기술만 제거할 수 있다.
	//--------------------------------------------------
	if ((*g_pSkillInfoTable)[id].GetLevel()!=m_MaxLearnedLevel)
	{
		return false;
	}

	SKILLID_MAP::iterator	iSkill;

	//--------------------------------------------------
	// ID가 id인 Skill를 찾는다.
	//--------------------------------------------------
	iSkill = m_mapSkillID.find(id);

	//--------------------------------------------------
	// 없을 경우 NULL을 return한다.
	//--------------------------------------------------
	if (iSkill == m_mapSkillID.end()) 
	{
		return false;
	}

	//--------------------------------------------------
	//
	// Skill의 값을 UnLearned로 바꾼다.
	//
	//--------------------------------------------------
	// 배운 것이면 아니면 return false
	if ((*iSkill).second!=SKILLSTATUS_LEARNED)
	{
		return false;
	}
	
	//--------------------------------------------------
	// 제거할려는 기술의 다음 기술들을 
	// 못 배우는 걸로 체크한다.
	//--------------------------------------------------
	RemoveNextSkill( m_pLearnedSkillID[m_MaxLearnedLevel] );

	//--------------------------------------------------
	// 기술 level 없애주기
	//--------------------------------------------------
	m_pLearnedSkillID[m_MaxLearnedLevel] = MAX_ACTIONINFO;//ACTIONINFO_NULL;
	m_MaxLearnedLevel--;
	

	(*iSkill).second = SKILLSTATUS_OTHER;	// 실제로는 NEXTSKILL이지만..머..

	//-----------------------------------------------
	// 현재 사용 가능한 Skill에서 제거한다.
	//-----------------------------------------------
	(*g_pSkillAvailable).RemoveSkill( id );
	
	//--------------------------------------------------
	//
	// 다음에 배울 수 있는 것들을 찾아서 설정한다.
	//
	//--------------------------------------------------
	if (m_MaxLearnedLevel>=0)
	{
		AddNextSkill( m_pLearnedSkillID[m_MaxLearnedLevel] );
	}


	return true;
}


//----------------------------------------------------------------------
// Is Exist SkillStep
//----------------------------------------------------------------------
BOOL
MSkillDomain::IsExistSkillStep(SKILL_STEP ss) const
{
	SKILL_STEP_MAP::const_iterator iList = m_mapSkillStep.find( ss );

	if (iList == m_mapSkillStep.end())
	{
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------
// Get SkillStep List
//----------------------------------------------------------------------
const MSkillDomain::SKILL_STEP_LIST*	
MSkillDomain::GetSkillStepList(SKILL_STEP ss) const
{
	SKILL_STEP_MAP::const_iterator iList = m_mapSkillStep.find( ss );

	if (iList == m_mapSkillStep.end())
	{
		return NULL;
	}

	return iList->second;
}

//----------------------------------------------------------------------
// Add SkillStep List
//----------------------------------------------------------------------
void
MSkillDomain::AddSkillStep(SKILL_STEP ss, ACTIONINFO ai)
{
	SKILL_STEP_LIST* pList;
	SKILL_STEP_MAP::const_iterator iList = m_mapSkillStep.find( ss );

	if (iList == m_mapSkillStep.end())
	{
		// 없으면 new해서 추가한다.
		pList = new SKILL_STEP_LIST;
	}
	else
	{
		pList = iList->second;
	}

	// list에 ai추가
	SKILL_STEP_LIST list = *pList;
	bool bExist = false;
	for(int i=0;i<list.size();i++)
	{
		if( list[i] == ai )
		{			
			bExist = true;
			break;
		}
	}
	
	if(bExist == false)
//		pList->push_back( ai );	
	{
		
		if( pList->empty() )
		{
			pList->push_back( ai );
		} else
		{
			bool	bAdded = false;
			SKILL_STEP_LIST::iterator itr = pList->begin();
			SKILL_STEP_LIST::iterator endItr = pList->end();	
			int		LearnLevel = (*g_pSkillInfoTable)[ai].GetLearnLevel();
			while( itr != endItr )
			{
				int currentPositionLevel = (*g_pSkillInfoTable)[*itr].GetLearnLevel();
				
				if( currentPositionLevel > LearnLevel )
				{
					pList->insert( itr, ai );
					bAdded = true;
					break;
				}
				
				itr++;
			}
			if(bAdded == false )
			{
				pList->push_back( ai );
			}
		}
	}

	// (다시) 설정한다.
	m_mapSkillStep[ss] = pList;
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
// Skill ID를 File에 저장한다.
//----------------------------------------------------------------------
void		
MSkillDomain::SaveToFile(std::ofstream& file)
{
	SKILLID_MAP::iterator	iSkill = m_mapSkillID.begin();

	// size저장
	int size = m_mapSkillID.size();
	file.write((const char*)&size, 4);

	// 각 id저장
	while (iSkill != m_mapSkillID.end())
	{
		WORD id = (*iSkill).first;
		BYTE status = (*iSkill).second;

		file.write((const char*)&id, 2);
		file.write((const char*)&status, 1);

		iSkill++;
	}    
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
// Skill ID를 File에서 읽어온다.
//----------------------------------------------------------------------
void		
MSkillDomain::LoadFromFile(std::ifstream& file)
{
	Clear();
	//m_mapSkillID.clear();
	
	// size읽어오기
	int size;
	file.read((char*)&size, 4);

	// 읽어와서 저장
	WORD id;
	BYTE status;
	for (int i=0; i<size; i++)
	{		
		file.read((char*)&id, 2);
		file.read((char*)&status, 1);

		m_mapSkillID.insert(SKILLID_MAP::value_type( 
										(enum ACTIONINFO)id, 
										(enum SKILLSTATUS)status ));
	} 
}

//----------------------------------------------------------------------
// LoadFromFileServerDomainInfo
//----------------------------------------------------------------------
void		
MSkillDomain::LoadFromFileServerDomainInfo(std::ifstream& file)
{	
	int level;

	file.read((char*)&level, 4);

	// level에 맞춰서 loading..
	m_DomainExpTable[level].LoadFromFile( file );	
}

bool		
MSkillDomain::IsAvailableDeleteSkill(ACTIONINFO id)
{	
	const SKILLINFO_NODE::SKILLID_LIST& listNextSkill = (*g_pSkillInfoTable)[id].GetNextSkillList();

	SKILLINFO_NODE::SKILLID_LIST::const_iterator iNextSkill = listNextSkill.begin();

	while (iNextSkill != listNextSkill.end())
	{
		//--------------------------------------------------
		// ID가 *iNextSkil인 Skill를 찾는다.
		//--------------------------------------------------
		int TempSkillID = *iNextSkill;
		if(false == (*g_pSkillInfoTable)[*iNextSkill].CanDelete)
			return false;
		SKILLID_MAP::iterator iSkill = m_mapSkillID.find( *iNextSkill );

		if (iSkill != m_mapSkillID.end())
		{
			if ((*iSkill).second==SKILLSTATUS_LEARNED)
			{						
				return false;
			}
		}

		iNextSkill++;
	}
	return true;
}

//----------------------------------------------------------------------
// Get ExpInfo
//----------------------------------------------------------------------
const ExpInfo&	
MSkillDomain::GetExpInfo(int level) const
{
	return m_DomainExpTable[level];
}

//----------------------------------------------------------------------
//
// MSkillManager
//
//----------------------------------------------------------------------
MSkillManager::MSkillManager()
{
}

MSkillManager::~MSkillManager()
{
}

//----------------------------------------------------------------------
// Init
//----------------------------------------------------------------------
void
MSkillManager::Init()
{
	//--------------------------------------------------
	//
	// Skill Tree 초기화
	//
	//--------------------------------------------------
	CTypeTable<MSkillDomain>::Init( MAX_SKILLDOMAIN );
	//--------------------------------------------------
	// 기본 기술로부터 skill tree를 초기화한다.
	//--------------------------------------------------

	m_pTypeInfo[SKILLDOMAIN_BLADE].SetRootSkill( SKILL_SINGLE_BLOW );
	m_pTypeInfo[SKILLDOMAIN_SWORD].SetRootSkill( SKILL_DOUBLE_IMPACT );
	m_pTypeInfo[SKILLDOMAIN_GUN].SetRootSkill( SKILL_FAST_RELOAD );
	m_pTypeInfo[SKILLDOMAIN_ENCHANT].SetRootSkill( MAGIC_CREATE_HOLY_WATER );
	m_pTypeInfo[SKILLDOMAIN_HEAL].SetRootSkill( MAGIC_CURE_LIGHT_WOUNDS );
	m_pTypeInfo[SKILLDOMAIN_VAMPIRE].SetRootSkill( MAGIC_HIDE );
	m_pTypeInfo[SKILLDOMAIN_OUSTERS].SetRootSkill( SKILL_FLOURISH );
	m_pTypeInfo[SKILLDOMAIN_ETC].SetRootSkill( SKILL_SOUL_CHAIN );

	// The per-domain experience the server ships is read from a file
	// the executable opens, through LoadFromFileServerDomainInfo below
	// (InitSkillTree in GameInit.cpp).
}

void
MSkillManager::InitSkillList()
{
	for(int i = SKILLDOMAIN_BLADE; i < MAX_SKILLDOMAIN; i++ )
		m_pTypeInfo[i].ClearSkillList();

	m_pTypeInfo[SKILLDOMAIN_BLADE].SetRootSkill( SKILL_SINGLE_BLOW , false);
	m_pTypeInfo[SKILLDOMAIN_SWORD].SetRootSkill( SKILL_DOUBLE_IMPACT , false);
	m_pTypeInfo[SKILLDOMAIN_GUN].SetRootSkill( SKILL_FAST_RELOAD , false);
	m_pTypeInfo[SKILLDOMAIN_ENCHANT].SetRootSkill( MAGIC_CREATE_HOLY_WATER , false);
	m_pTypeInfo[SKILLDOMAIN_HEAL].SetRootSkill( MAGIC_CURE_LIGHT_WOUNDS , false);
	m_pTypeInfo[SKILLDOMAIN_VAMPIRE].SetRootSkill( MAGIC_HIDE , false);
	m_pTypeInfo[SKILLDOMAIN_OUSTERS].SetRootSkill( SKILL_FLOURISH , false);
	m_pTypeInfo[SKILLDOMAIN_ETC].SetRootSkill( SKILL_SOUL_CHAIN , false);
}



//----------------------------------------------------------------------
// LoadFromFileServerSkillInfo
//----------------------------------------------------------------------
void		
MSkillManager::LoadFromFileServerDomainInfo(std::ifstream& file)
{
	int num, domain;

	file.read((char*)&num, 4);

	// The count and each domain are the file's. m_pTypeInfo is indexed
	// raw here, past even the typed table's own bound, so a domain the
	// file names outside the table would write through a wild pointer;
	// and once a row is refused the stream is no longer where the next
	// row begins, so reading stops.
	for (int i=0; i<num; i++)
	{
		if (!file.read((char*)&domain, 4))
		{
			return;
		}

		if (domain < 0 || domain >= GetSize())
		{
			return;
		}

		m_pTypeInfo[domain].LoadFromFileServerDomainInfo( file );
	}
}
