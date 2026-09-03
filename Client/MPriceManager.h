//-----------------------------------------------------------------------------
// MPriceManager.h
//-----------------------------------------------------------------------------

#ifndef __MPRICEMANAGER_H__
#define __MPRICEMANAGER_H__

class MItem;

// A price shown as stars: number stars of the given type.
struct STAR_ITEM_PRICE {
	int type;
	int number;
};

//-----------------------------------------------------------------------------
// MPriceHost - what the price manager needs from the executable
// (docs/RESTRUCTURING.md task 4.2). The prices compile into gamemodel,
// which cannot see the player, the event manager or the skill set, so
// the executable installs these once at start-up (GameInit.cpp); a test
// binary installs its own, or none - and without one a price carries
// no player, event or skill adjustment.
//-----------------------------------------------------------------------------
struct MPriceHost {
	int		(*Race)();					// RACE_SLAYER, RACE_VAMPIRE or RACE_OUSTERS (RaceType.h)
	int		(*Level)();
	int		(*StatSum)();				// STR + DEX + INT as worn
	int		(*BasicStatSum)();			// STR + DEX + INT before gear and affects
	bool	(*IsPotionHalfPrice)();		// the premium half-price event, or the NEMA blood bible
	bool	(*IsGambleHalfPrice)();		// the JAVE blood bible
	int		(*ShopTaxPercent)();		// the tax-change event's percentage; 100 without one
};

class MPriceManager {
	public :
		enum TRADE_TYPE
		{
			NPC_TO_PC,		// the player buys, at m_MarketCondSell
			PC_TO_NPC,		// the player sells, at m_MarketCondBuy
			REPAIR,			// the player repairs
			SILVERING,		// the player has the item silver-coated
		};

	public :
		MPriceManager(); 	
		~MPriceManager();

		//-------------------------------------------------------		
		// Get Item Price
		//-------------------------------------------------------		
		int			GetItemPrice(MItem* pItem, TRADE_TYPE type, bool bMysterious=false);
		void		GetItemPrice(MItem* pItem, STAR_ITEM_PRICE& price);
		int			GetMysteriousPrice(MItem* pItem) const;

		//-------------------------------------------------------
		// The market conditions, buy and sell as the NPC sees them
		//-------------------------------------------------------
		void		SetMarketCondBuy(int buy)			{ m_MarketCondBuy = buy; }
		void		SetMarketCondSell(int sell)			{ m_MarketCondSell = sell; }

		int			GetMarketCondBuy() const			{ return m_MarketCondBuy; }
		int			GetMarketCondSell() const			{ return m_MarketCondSell; }

		void		SetEventItemPrice(int Price)		{ m_EventFixPrice = Price; }

		static void					SetHost(const MPriceHost* pHost)	{ s_pHost = pHost; }
		static const MPriceHost*	GetHost()							{ return s_pHost; }

	protected :

		// The market conditions, as the NPC sees them
		int					m_MarketCondBuy;		// when the NPC buys (25)
		int					m_MarketCondSell;		// when the NPC sells (100)
		int					m_EventFixPrice;		// the event item price the server sets

		static const MPriceHost*	s_pHost;
};

extern MPriceManager*		g_pPriceManager;

#endif

