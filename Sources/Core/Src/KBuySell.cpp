#include "KEngine.h"
#include "KCore.h"
#include "KTabFile.h"
#include "KPlayer.h"
#include "KNpc.h"
#include "KItemGenerator.h"
#include "KSubWorldSet.h"
#include "KItemSet.h"
#include "KBuySell.h"
#ifndef _STANDALONE
#include "CoreShell.h"
#include "crtdbg.h"
#endif
#include "CoreUseNameDef.h"
#ifdef _SERVER
//#include "../MultiServer/Heaven/Interface/iServer.h"
#endif

#define	SHOP_BOX_WIDTH		6
#define	SHOP_BOX_HEIGHT		10

KBuySell	BuySell;

KBuySell::KBuySell()
{
#ifndef _SERVER
	m_pShopRoom = NULL;
#endif
	m_Item = NULL;
	m_SellItem = NULL;
	m_Width = 0;
	m_Height = 0;
	m_MaxItem = 0;
}

KBuySell::~KBuySell()
{
#ifndef _SERVER
	if (m_pShopRoom)
	{
		delete m_pShopRoom;
		m_pShopRoom = NULL;
	}
#endif
	if (m_Item)
	{
		delete [] m_Item;
		m_Item = NULL;
	}
	if (m_SellItem)
	{
		for (int i = 0; i < m_Height; i++)
		{
			if (m_SellItem[i])
			{
				delete m_SellItem[i];
				m_SellItem[i] = NULL;
			}
		}
		delete m_SellItem;
		m_SellItem = NULL;
	}
	m_Width = 0;
	m_Height = 0;
	m_MaxItem = 0;
}

BOOL KBuySell::Init()
{
	KTabFile		GoodsFile;
	KTabFile		BuySellFile;

//	g_SetFilePath("\\");
	if (!BuySellFile.Load(BUYSELL_FILE) || !GoodsFile.Load(GOODS_FILE))
		return FALSE;

	int nHeight = GoodsFile.GetHeight() - 1;
	int nWidth = GoodsFile.GetWidth() - 1;
	if (nWidth == 0 || nHeight == 0)
		return FALSE;

	m_Item = (KItem *)new KItem[nHeight];
	if (!m_Item)
		return FALSE;

	ItemGenerate ItemGenerator;
	for (int k = 0; k < nHeight; k++)
	{
		GoodsFile.GetInteger(k + 2, 1, -1, &ItemGenerator.nGenre);
		GoodsFile.GetInteger(k + 2, 2, -1, &ItemGenerator.nDetailType);
		GoodsFile.GetInteger(k + 2, 3, -1, &ItemGenerator.nParticularType);
		GoodsFile.GetInteger(k + 2, 4, -1, &ItemGenerator.nSeriesReq);
		GoodsFile.GetInteger(k + 2, 5, -1, &ItemGenerator.nLevel);
		switch(ItemGenerator.nGenre)
		{
		case item_equip:
			ItemGen.Gen_Equipment(ItemGenerator.nDetailType,
				ItemGenerator.nParticularType,
				ItemGenerator.nSeriesReq,
				ItemGenerator.nLevel,
				NULL,
				0,
				g_SubWorldSet.GetGameVersion(),
				&m_Item[k]);
			break;
		case item_medicine:
			ItemGen.Gen_Medicine(ItemGenerator.nDetailType, 
				ItemGenerator.nLevel, 
				g_SubWorldSet.GetGameVersion(),
				&m_Item[k]);
			break;
		case item_townportal:
			ItemGen.Gen_TownPortal(&m_Item[k]);
			break;
		default:
			break;
		}
		m_MaxItem++;
	}

	m_Height = BuySellFile.GetHeight() - 1;
	m_Width  = BuySellFile.GetWidth();
	
	if (m_Width == 0 || m_Height == 0)
		return FALSE;

	m_SellItem = (int **)new int*[m_Height];
	if (!m_SellItem)
		return FALSE;

	for (int i = 0; i < m_Height; i++)
	{
		m_SellItem[i] = NULL;
		m_SellItem[i] = (int *)new int[m_Width];
		if (!m_SellItem[i])
			return FALSE;
		for (int j = 0; j < m_Width; j++)
		{
			BuySellFile.GetInteger(i + 2, j + 1, -1, &m_SellItem[i][j]);
			if (m_SellItem[i][j] == -1)
			{
				continue;
			}
			_ASSERT(m_SellItem[i][j] > 0);		// 策划是从1开始的
			if (m_SellItem[i][j] > 0)
				m_SellItem[i][j] -= 1;			// 为了策划从1开始填表
		}
	}

#ifndef _SERVER
	if (!m_pShopRoom)
	{
		m_pShopRoom = new KInventory;
		m_pShopRoom->Init(SHOP_BOX_WIDTH, SHOP_BOX_HEIGHT);
	}
#endif
	return TRUE;
}

KItem* KBuySell::GetItem(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_MaxItem || !m_Item)
		return NULL;

	return &m_Item[nIndex];
}

int KBuySell::GetItemIndex(int nShop, int nIndex)
{
	if (!m_SellItem || nShop < 0 || nShop >= m_Height || nIndex < 0 || nIndex >= m_Width)
		return -1;

	if (!m_SellItem[nShop])
		return -1;

	return m_SellItem[nShop][nIndex];
}

#ifdef _SERVER
BOOL KBuySell::Buy(int nPlayerIdx, int nBuy, int nBuyIdx, int nPlace, int nX, int nY)
{
	KASSERT(nPlayerIdx >= 0 && nPlayerIdx < MAX_PLAYER);

	if (nBuy != Player[nPlayerIdx].m_BuyInfo.m_nBuyIdx)
	{
		g_DebugLog("BuySell: %s buy idx error!", Npc[Player[nPlayerIdx].m_nIndex].Name);
		return FALSE;
	}

	if (nBuyIdx > m_Width)
		return FALSE;

	if (m_SellItem[nBuy][nBuyIdx] < 0 || m_SellItem[nBuy][nBuyIdx] >= m_MaxItem)
		return FALSE;

	int nIdx = m_SellItem[nBuy][nBuyIdx];
	if (Player[nPlayerIdx].m_ItemList.GetEquipmentMoney() < m_Item[nIdx].GetPrice())
		return FALSE;

	int nItemIdx = ItemSet.Add(&m_Item[nIdx]);
	if (!nItemIdx)
		return FALSE;

	Player[nPlayerIdx].Pay(m_Item[nIdx].GetPrice());
	Player[nPlayerIdx].m_ItemList.Add(nItemIdx, nPlace, nX, nY);
	return TRUE;
}

/*******************************************************************************
参数 nIdx 指游戏里Item数组的编号
*******************************************************************************/
BOOL KBuySell::Sell(int nPlayerIdx, int nBuy, int nIdx)
{
	KASSERT(nPlayerIdx >= 0 && nPlayerIdx < MAX_PLAYER);
	KASSERT(nIdx >= 0 && nIdx < MAX_ITEM);

	if (Item[nIdx].GetGenre() == item_task)
		return FALSE;

	if (nBuy != Player[nPlayerIdx].m_BuyInfo.m_nBuyIdx)
	{
		g_DebugLog("BuySell: %s buy idx error!", Npc[Player[nPlayerIdx].m_nIndex].Name);
		return FALSE;
	}
	int nMoney = Item[nIdx].GetPrice();
	nMoney /= BUY_SELL_SCALE;
	if (nMoney <= 0)
		nMoney = 1;
	Player[nPlayerIdx].Earn(nMoney);
	Player[nPlayerIdx].m_ItemList.Remove(nIdx);
	ItemSet.Remove(nIdx);
	return TRUE;
}
#endif

#ifndef _SERVER
void KBuySell::PaintItem(int nIdx, int nX, int nY)
{
	int nShop = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nBuyIdx;
	if (nShop < 0 || nShop >= m_Height)
		return;
	int nItemIdx = GetItemIndex(nShop, nIdx);

	int x = nX;
	int y = nY;

	KItem* pItem = GetItem(nItemIdx);

	if (pItem)
	{
		pItem->Paint(x, y);
	}
}

void KBuySell::OpenSale(int nShop)
{
	if (nShop < 0 || nShop >= m_Height)
		return;
	Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nBuyIdx = nShop;
	CoreDataChanged(GDCNI_NPC_TRADE, NULL, TRUE);
}
#endif

#ifdef _SERVER
void KBuySell::OpenSale(int nPlayerIdx, int nShop)
{
	if (nPlayerIdx <= 0 || nPlayerIdx > MAX_PLAYER)
	{
		return;
	}

	Player[nPlayerIdx].m_BuyInfo.m_nBuyIdx = nShop;
	Player[nPlayerIdx].m_BuyInfo.m_SubWorldID = Npc[Player[nPlayerIdx].m_nIndex].m_SubWorldIndex;
	Npc[Player[nPlayerIdx].m_nIndex].GetMpsPos(
		&Player[nPlayerIdx].m_BuyInfo.m_nMpsX,
		&Player[nPlayerIdx].m_BuyInfo.m_nMpsY);

	SALE_BOX_SYNC saleSync;
	saleSync.ProtocolType = s2c_opensalebox;
	saleSync.nShopIndex = nShop;
	g_pServer->PackDataToClient(Player[nPlayerIdx].m_nNetConnectIdx, &saleSync, sizeof(SALE_BOX_SYNC));
}
#endif
