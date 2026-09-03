//-----------------------------------------------------------------------------
// MPriceManager.cpp
//-----------------------------------------------------------------------------
#include "Client_PCH.h"
#pragma warning(disable:4786)

#include "MPriceManager.h"
#include "MItem.h"
#include "MItemOptionTable.h"
#include "UserInformation.h"
#include "MTimeItemManager.h"

#define CHARGE_PRICE		5000

//-----------------------------------------------------------------------------
// Global
//-----------------------------------------------------------------------------
MPriceManager*		g_pPriceManager = NULL;
const MPriceHost*	MPriceManager::s_pHost = NULL;

//-----------------------------------------------------------------------------
// The host's answers, and what they are without a host: no race, no
// level, no stats, no discount, no tax.
//-----------------------------------------------------------------------------
namespace {

int		HostRace()				{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL ? p->Race() : -1; }
int		HostLevel()				{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL ? p->Level() : 0; }
int		HostStatSum()			{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL ? p->StatSum() : 0; }
int		HostBasicStatSum()		{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL ? p->BasicStatSum() : 0; }
bool	HostPotionHalfPrice()	{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL && p->IsPotionHalfPrice(); }
bool	HostGambleHalfPrice()	{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL && p->IsGambleHalfPrice(); }
int		HostShopTaxPercent()	{ const MPriceHost* p = MPriceManager::GetHost(); return p!=NULL ? p->ShopTaxPercent() : 100; }

} // namespace

//-----------------------------------------------------------------------------
//
// contructor / destructor
// 
//-----------------------------------------------------------------------------
MPriceManager::MPriceManager()
{
	m_MarketCondBuy		= 25;		// NPC가 살때(25)
	m_MarketCondSell	= 100;		// NPC가 팔때(100)
	m_EventFixPrice		= 0;
}

MPriceManager::~MPriceManager()
{
}

//-----------------------------------------------------------------------------
//
// member functions
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Get Item Price
//-----------------------------------------------------------------------------
int
MPriceManager::GetItemPrice(MItem* pItem, TRADE_TYPE type, bool bMysterious)
{
	if (pItem==NULL)
	{
		return 0;
	}
	if(!pItem->IsIdentified())
		return GetMysteriousPrice(pItem);

	__int64	finalPrice;


	// 2004, 08, 02, sobeit add start
	if(pItem->GetItemClass() == ITEM_CLASS_MOON_CARD && pItem->GetItemType() == 4)
		return m_EventFixPrice;
	// 2004, 08, 02, sobeit add end				
	//-------------------------------------------------------
	// 살때, 팔때..
	//-------------------------------------------------------
	int nRatio;

	switch (type)
	{
		//-------------------------------------------------------
		// 상점에서 살때
		//-------------------------------------------------------
		case NPC_TO_PC :
			if (pItem->GetItemClass()==ITEM_CLASS_SKULL)
			{
				// 해골은 파는 경우밖에 없다.
				nRatio = m_MarketCondBuy;
			}
			else
			{
				nRatio = m_MarketCondSell;
			}
		break;

		//-------------------------------------------------------
		// 상점에 팔 때
		//-------------------------------------------------------
		case PC_TO_NPC :
			nRatio = m_MarketCondBuy;
		break;

		//-------------------------------------------------------
		// 은도금
		//-------------------------------------------------------
		case SILVERING :
		{
			ITEM_CLASS itemClass = pItem->GetItemClass();

			if (itemClass==ITEM_CLASS_BLADE
				|| itemClass==ITEM_CLASS_SWORD
				|| itemClass==ITEM_CLASS_CROSS
				|| itemClass==ITEM_CLASS_MACE)
			{
				double    maxSilver  = pItem->GetSilverMax();
				double    curSilver  = pItem->GetSilver();
				double    finalPrice = 0;

				// 은도금 꽉차있는상태..
				if (maxSilver==curSilver)
				{
					return 0;
				}

				// 땜빵으로 집어넣은 은의 가격이다.
				finalPrice = maxSilver; 

				return (int)finalPrice;
			}
			else
			{
				return 0;
			}
		}
		break;

		//-------------------------------------------------------
		// 수리
		//-------------------------------------------------------
		case REPAIR :
			// vampire 포탈은 수리 안된다.
			if (pItem->GetItemClass()==ITEM_CLASS_VAMPIRE_PORTAL_ITEM 
				// 2004, 8, 17, sobeit add start - 시간제 아이템 착용시 전체 수리가 안되던 버그 수정 때문에 추가.
				|| g_pTimeItemManager->IsExist( pItem->GetID() )
				// 2004, 8, 17, sobeit add end

				// 2004, 10, 21, sobeit add start = 블러드 바이블 수리 할필요 없음.
				|| pItem->GetItemClass() == ITEM_CLASS_BLOOD_BIBLE_SIGN
				// 2004, 10, 21, sobeit add end
				)
			{
				return 0;	
			}

			// 수리할때는 원래 가격의 1/10
			nRatio = 10;

			// 음.. - -; 
//			bMysterious = false;
		break;
	}

	//-------------------------------------------------------
	// Charge Item인 경우
	//-------------------------------------------------------
	if (pItem->IsChargeItem())
	{		
		int curCharge = pItem->GetNumber();
		int maxCharge = pItem->GetMaxNumber();
		
		int ChargePrice = CHARGE_PRICE;
		
		if( pItem->GetItemClass() == ITEM_CLASS_OUSTERS_SUMMON_ITEM )
			ChargePrice = 1000;

		if (type==REPAIR)
		{
			// 부족한 charge를 채워준다.
			int charge = maxCharge - curCharge;

			return charge * ChargePrice;
		}
		
		int itemPrice = pItem->GetPrice();

		int finalPrice = itemPrice + curCharge * ChargePrice;
			
		finalPrice = finalPrice * nRatio / 100;

		return finalPrice;
	}	

	//-------------------------------------------------------
	//
	//						Skull
	//
	//-------------------------------------------------------
	/*
	if (pItem->GetItemClass()==ITEM_CLASS_SKULL)
	{
		if (type==REPAIR)
		{
			return 0;
		}

		//finalPrice = pItem->GetCurrentDurability();
	}
	*/
	//-------------------------------------------------------
	//
	//					일반적인 Item들..
	//
	//-------------------------------------------------------
	//else
	{
		//-------------------------------------------------------
		// Item의 정보를 읽는다.
		//-------------------------------------------------------
		// 원래 물건 값과 최대 내구치를 알아낸다.
		int		itemDur = pItem->GetMaxDurability();
		long	curDurability = pItem->GetCurrentDurability();
		
		//--------------------------------------------------
		// 수리할때는 maxDur==curDur이면 가격이 0이다.
		// maxDurability가 0이하이면 수리할 필요가 없는 item이다.
		//--------------------------------------------------
		if (type==REPAIR)
		{
			if (itemDur<0 || itemDur==curDurability)
			{
				return 0;
			}
		}
		

		if (itemDur<0)
		{
			itemDur = 0;
		}

		long	originalPrice = pItem->GetPrice();
		long	maxDurability = itemDur;
			
		finalPrice = originalPrice;
		
		//-------------------------------------------------------
		// Gamble은 비싸다
		//-------------------------------------------------------
//		if (bMysterious)
//		{
//			// 할인율에 따라서 물건값을 낮춘다.
//			finalPrice = finalPrice*nRatio / 100;
//
//			// MYSTERIOUS 아이템은 무조건 5배다. 카카
//			finalPrice = finalPrice * 5;
//		}
//		else
		{
			// 옵션 여부에 따라서 물건값을 더한다.
			if (!pItem->IsEmptyItemOptionList())
			{
				int priceMult = pItem->GetItemOptionPriceMultiplier();//(*g_pItemOptionTable)[pItem->GetItemOption()].PriceMultiplier;
				finalPrice = finalPrice * priceMult / 100;
			}
			
			// 내구성이 얼마나 떨어졌는가에 따라 물건값을 낮춘다.
			float damaged;
			if (maxDurability==0)
			{
				damaged = 1;
			}
			else
			{
				damaged = (float)curDurability / (float)maxDurability;
			}

			//--------------------------------------------------
			// 수리할때는 damage 받은 만큼 비싸진다.
			//--------------------------------------------------
			if (type==REPAIR)
			{
				damaged = 1.0f - damaged;
			}
			
			finalPrice = finalPrice * damaged;
			
			// 할인율에 따라서 물건값을 낮춘다.
			finalPrice = finalPrice * nRatio / 100;
		}	
	}

	
	//-------------------------------------------------------
	// potion인 경우
	//-------------------------------------------------------
	// A weak slayer pays 70% for the two basic potions.
	if (pItem->GetItemClass()==ITEM_CLASS_POTION)
	{
		if ((pItem->GetItemType()==0 || pItem->GetItemType()==5)
			&& HostRace()==RACE_SLAYER
			&& HostStatSum() <= 40)
		{
			finalPrice = finalPrice * 70 / 100;
		}
	}

	// The consumables are half price under the premium event or the
	// NEMA blood bible.
	if ((pItem->GetItemClass() == ITEM_CLASS_POTION ||
		 pItem->GetItemClass() == ITEM_CLASS_SERUM ||
		 pItem->GetItemClass() == ITEM_CLASS_LARVA ||
		 pItem->GetItemClass() == ITEM_CLASS_PUPA ||
		 pItem->GetItemClass() == ITEM_CLASS_COMPOS_MEI)
		&& HostPotionHalfPrice())
	{
		finalPrice /= 2;
	}

	// The tax-change event scales what the shop charges.
	if (type == NPC_TO_PC)
	{
		finalPrice = finalPrice * HostShopTaxPercent() / 100;
	}

	// Nothing is free.
	if (finalPrice==0)
	{
		return 1;
	}

	// A skull is worth half to a vampire and three quarters to an Ousters.
	if (pItem->GetItemClass()==ITEM_CLASS_SKULL)
	{
		if (HostRace()==RACE_VAMPIRE)
		{
			finalPrice >>= 1;
		}
		else if (HostRace()==RACE_OUSTERS)
		{
			finalPrice = finalPrice * 75 / 100;
		}
	}

		
	// Then the head-price rate the server sent.
	if(pItem->GetItemClass() == ITEM_CLASS_SKULL)
	{
		finalPrice    = finalPrice * g_pUserInformation->HeadPrice / 100;
	}


	return (int)finalPrice;
}

//-----------------------------------------------------------------------------
// Get ItemPrice
//-----------------------------------------------------------------------------
// 뭔가 이상한 경우는..
// price.type = -1;
// price.number = 0;
//-----------------------------------------------------------------------------
void		
MPriceManager::GetItemPrice(MItem* pItem, STAR_ITEM_PRICE& price)
{
	if (pItem==NULL)
	{
		price.type = -1;
		price.number = 0;
		return;
	}

	// 옵션에 따라서 별의 type을 결정한다.
	switch (pItem->GetItemOptionPart())
    {
		case ITEMOPTION_TABLE::PART_DAMAGE		: price.type = 0; break;	// 검정
        case ITEMOPTION_TABLE::PART_STR			: price.type = 1; break;	// 빨강
        case ITEMOPTION_TABLE::PART_INT			: price.type = 2; break;	// 파랑
        case ITEMOPTION_TABLE::PART_DEX			: price.type = 3; break;	// 초록
        case ITEMOPTION_TABLE::PART_ATTACK_SPEED	: price.type = 4; break;	// 하늘

        default: 
			price.type = -1;
			price.number = 0;
		return;
	}

	// type에 따라서 별의 개수를 결정한다.
	price.number = (pItem->GetItemType() - 1) * 20;
}


//-----------------------------------------------------------------------------
// Get Mysterious Price
//-----------------------------------------------------------------------------
// The gamble: an unidentified item's price follows its class and the
// character buying it.
//-----------------------------------------------------------------------------
int MPriceManager::GetMysteriousPrice(MItem *pItem) const
{
	// A slayer pays by basic stats, everyone else by level: one to
	// twenty times the class's average price.
	int multiplier = 1;

	if (HostRace()==RACE_SLAYER)
	{
		int CSUM = HostBasicStatSum();

		if(CSUM > 0)
			multiplier = CSUM / 15;
	}
	else
	{
		multiplier = HostLevel() / 5;
	}

	multiplier = max(1, multiplier);

	int avr = (*g_pItemTable)[pItem->GetItemClass()].GetAveragePrice();

	int final_price = avr * multiplier;

	// Half under the JAVE blood bible, then the shop tax.
	if (HostGambleHalfPrice())
	{
		final_price /= 2;
	}

	final_price = final_price * HostShopTaxPercent() / 100;

	return final_price;
}

