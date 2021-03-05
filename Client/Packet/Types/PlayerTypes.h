//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerTypes.h
// Written By  : Reiot
// Description : 
//////////////////////////////////////////////////////////////////////////////

#ifndef __PLAYER_TYPES_H__
#define __PLAYER_TYPES_H__

#include "SystemTypes.h"

//////////////////////////////////////////////////////////////////////////////
// 플레이어 생성 및 변경 관련 에러 ID
//////////////////////////////////////////////////////////////////////////////
enum ErrorID {
	INVALID_ID_PASSWORD,
	ALREADY_CONNECTED,
	ALREADY_REGISTER_ID,
	ALREADY_REGISTER_SSN,
	EMPTY_ID,
	SMALL_ID_LENGTH,
	EMPTY_PASSWORD,
	SMALL_PASSWORD_LENGTH,
	EMPTY_NAME,
	EMPTY_SSN,
	INVALID_SSN,
	NOT_FOUND_PLAYER,
	NOT_FOUND_ID,
	NOT_PAY_ACCOUNT,
	NOT_ALLOW_ACCOUNT,
	ETC_ERROR,
	IP_DENYED,
	CHILDGUARD_DENYED,
	CANNOT_AUTHORIZE_BILLING,   // 빌링 정보를 찾을 수 없습니다.
    CANNOT_CREATE_PC_BILLING,    // 유료 사용자가 아니라서 캐릭터를 못 만듭니다.
	KEY_EXPIRED,                // 키 유효기간이 지났다.
    NOT_FOUND_KEY,               // 키가 없다.
	// add by Coffee
	CHECK_VERSION_ERROR,

};


//////////////////////////////////////////////////////////////////////////////
// 플레이어 아이디
//////////////////////////////////////////////////////////////////////////////
const uint minIDLength = 4;
const uint maxIDLength = 10;


//////////////////////////////////////////////////////////////////////////////
// 플레이어 암호
//////////////////////////////////////////////////////////////////////////////
const uint minPasswordLength = 6;
const uint maxPasswordLength = 10;


//////////////////////////////////////////////////////////////////////////////
// 플레이어 이름
//////////////////////////////////////////////////////////////////////////////
const uint maxNameLength = 20;


//////////////////////////////////////////////////////////////////////////////
// 주민등록번호 ( '-' 를 포함해야 한다. )
//////////////////////////////////////////////////////////////////////////////
const uint maxSSNLength = 20;


//////////////////////////////////////////////////////////////////////////////
// 집전화
//////////////////////////////////////////////////////////////////////////////
const uint maxTelephoneLength = 15;


//////////////////////////////////////////////////////////////////////////////
// 휴대폰
//////////////////////////////////////////////////////////////////////////////
const uint maxCellularLength = 15;


//////////////////////////////////////////////////////////////////////////////
// 우편번호
//////////////////////////////////////////////////////////////////////////////
const uint maxZipCodeLength = 7;


//////////////////////////////////////////////////////////////////////////////
// 집주소
//////////////////////////////////////////////////////////////////////////////
const uint maxAddressLength = 100;


//////////////////////////////////////////////////////////////////////////////
// 전자메일
//////////////////////////////////////////////////////////////////////////////
const uint maxEmailLength = 50;


//////////////////////////////////////////////////////////////////////////////
// 홈페이지
//////////////////////////////////////////////////////////////////////////////
const uint maxHomepageLength = 50;


//////////////////////////////////////////////////////////////////////////////
// 자기소개글
//////////////////////////////////////////////////////////////////////////////
const uint maxProfileLength = 200;


//////////////////////////////////////////////////////////////////////////////
// 국적
//////////////////////////////////////////////////////////////////////////////
typedef BYTE Nation_t;
const uint szNation = szBYTE;

enum Nation
{   
	KOREA,
	USA,
	JAPAN
};

const std::string Nation2String [] =
{   
	"KOREA",
	"USA",
	"JAPAN"
};


//////////////////////////////////////////////////////////////////////////////
// 패널티 타입
//////////////////////////////////////////////////////////////////////////////
enum PenaltyType 
{
	PENALTY_TYPE_KICKED,				// 나가라
	PENALTY_TYPE_MUTE,					// 닥쳐라
	PENALTY_TYPE_FREEZING,				// 멈춰라
	PENALTY_TYPE_MAX
};


#endif
