/***************************************************************************
//	界面--装备界面
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue), 吕桂华
//	CreateTime:	2002-9-2
----------------------------------------------------------------------------
	装备界面
		选项卡：装备一、装备二
		总共20个装备空位,每个选项卡上有10个
		按钮：一个关闭按钮
			通过选项卡设定两套装备，游戏中可以通过快捷键自由切换两套装备。共
		有10个装备摆放位置，武器2×4，衣服2×3，头部2×2，腰带2×1，手镯1×1，
		鞋子2×2，戒指1×1，腰坠1×2，项链1×1。
*****************************************************************************/
#pragma once

#include "../elem/wndbutton.h"
#include "../elem/wndpage.h"
#include "../Elem/WndText.h"
#include "../elem/WndObjContainer.h"

#define _ITEM_COUNT 10

struct KUiObjAtRegion;

// -------------------------------------------------------------------------
// ---> 装备属性页
class KUiEquipPage : public KWndPage
{
public:
	void	Initialize();						//初始化
	void	LoadScheme(const char* pScheme);	//载入界面方案
	void	UpdateEquip(KUiObjAtRegion* pEquip, int bAdd);	//装备变化更新
	void	UpdateData();									//更新数据
	void	Clear();
private:
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void	OnEquiptChanged(ITEM_PICKDROP_PLACE* pPickPos, ITEM_PICKDROP_PLACE* pDropPos);//响应界面操作引起装备的改变

private:
	// 窗口元素
	KWndObjectBox	m_EquipBox[_ITEM_COUNT];
};

// ---> 马装备属性页
class KUiHorsePage : public KWndPage
{
public:
	void	Initialize();						//初始化
	void	LoadScheme(const char* pScheme);	//载入界面方案
	void	UpdateEquip(KUiObjAtRegion* pEquip, int bAdd);	//装备变化更新
	void	Clear();
	void	UpdateRideStatus(int bRiding);	//更新骑马状态
private:
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	int		OnClickItem(unsigned int uMsg, unsigned int uParam, int nParam);//响应点击按钮
	void	OnEquiptChanged(ITEM_PICKDROP_PLACE* pPickPos, ITEM_PICKDROP_PLACE* pDropPos);//响应界面操作引起装备的改变
private:
	KWndObjectBox	m_HorseBox;
	KWndButton		m_UpDownHorseBtn;
	KWndText256		m_HorseDesc;
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ---> 装备窗口
class KUiEquip : public KWndPageSet
{
public:
	static KUiEquip* OpenWindow();					//打开窗口，返回唯一的一个类对象实例
	static KUiEquip* GetIfVisible();				//如果窗口正被显示，则返回实例指针
	static void		 CloseWindow(bool bDestroy);	//关闭窗口，同时可以选则是否删除对象实例
	static void		LoadScheme(const char* pScheme);//载入界面方案
	static void		UpdateRideStatus(int bRiding);	//更新骑马状态
	void			UpdateEquip(KUiObjAtRegion* pEquipt, int bAdd);//装备变化更新
private:
	KUiEquip() {}
	~KUiEquip() {}
	void	Initialize();							//初始化	
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
private:
	static KUiEquip*	m_pSelf;
private:
	KUiEquipPage	m_EquipPage;	// 装备1属性页
	KUiHorsePage	m_HorsePage;	// 装备2属性页
	KWndButton		m_Equip1Btn;	// 切换到装备1
	KWndButton		m_Equip2Btn;	// 切换到装备2
	KWndButton		m_Close;		// 关闭按钮
};