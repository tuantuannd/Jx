/*******************************************************************************
File        : UiTrembleItem.h
Creator     : Fyt(Fan Zhanpeng)
create data : 09-08-2003(mm-dd-yyyy)
Description : 打造武器的界面
********************************************************************************/

#if !defined(AFX_UITREMBLEITEM_H__69079BE2_10C3_4AA0_A3C2_190285964E36__INCLUDED_)
#define AFX_UITREMBLEITEM_H__69079BE2_10C3_4AA0_A3C2_190285964E36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../elem/wndobjcontainer.h"
#include "../elem/wndbutton.h"
#include "../elem/wndimage.h"

class KCanGetNumImage : public KWndImage
{
public:
	int GetMaxFrame();
	int GetCurrentFrame();
};

class KUiTrembleItem : public KWndImage
{
public:
	KUiTrembleItem();
	virtual ~KUiTrembleItem();

	static        KUiTrembleItem* OpenWindow();      //打开窗口
	static        KUiTrembleItem* GetIfVisible();    //如果窗口正被显示，则返回实例指针
	static void   CloseWindow(bool bDestory = TRUE); //关闭窗口，同时可以选则是否删除对象实例
	static void   LoadScheme(const char* pScheme);   //载入界面方案

private:
	static        KUiTrembleItem *ms_pSelf;

private:
	void          Initialize();
	virtual int   WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	virtual void  Breathe();

private:
	void          ApplyAssemble();                   //向Core发送打造请求和数据
	void          ResultReturn();                    //打造结果返回
	int           PlayEffect();                      //播放特效
	void          UpdateResult();                    //把结果更新到界面上

private:
	enum THIS_INTERFACE_STATUS
	{
		STATUS_WAITING_MATERIALS,
		STATUS_BEGIN_TREMBLE,
		STATUS_TREMBLING,
		STATUS_CHANGING_ITEM,
		STATUS_FINISH,
	};

private:
	KWndObjectBox m_GemSpirit;                       //灵气宝石插槽
	KWndObjectBox m_GemWater;                        //水清澈宝石插槽
	KWndObjectBox m_GemEarth;                        //土混沌宝石插槽
	KWndObjectBox m_GemLevel;                        //等级宝石插槽
	KWndObjectBox m_GemFire;                         //火跃动宝石插槽
	KWndObjectBox m_GemGold;                         //金华丽宝石插槽
	KWndObjectBox m_GemWood;                         //木朴实宝石插槽
	KWndObjectBox m_Item;                            //洗练的物品

	KWndButton    m_Confirm;                         //合成按钮
	KWndButton    m_Cancel;                          //关闭按钮

	KCanGetNumImage
		          m_TrembleEffect;                   //合成中的特效

private:
	int           m_nStatus;                         //当前界面所处的状态
};

#endif // !defined(AFX_UITREMBLEITEM_H__69079BE2_10C3_4AA0_A3C2_190285964E36__INCLUDED_)
