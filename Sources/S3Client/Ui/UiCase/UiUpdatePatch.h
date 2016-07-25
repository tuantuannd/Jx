/*****************************************************************************************
//	界面--程序更新窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-12
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "../Elem/WndImage.h"
#include "../Elem/WndEdit.h"
#include "../Elem/WndButton.h"
#include "../Elem/WndList.h"
#include "../Elem/WndScrollBar.h"
#include "../Elem/WndMessageListBox.h"
#include "../Elem/WndImagePart.h"
#include "../Elem/WndShowAnimate.h"

//PatchShell Inclued
#include "../../../SUpdate/SUpdateShell.h"


//====================================
//	界面--程序更新界面
//====================================
class KUiUpdataPatch : protected KWndShowAnimate
{
public:
	static KUiUpdataPatch* OpenWindow();	//打开窗口，返回唯一的一个类对象实例
	static void			   CloseWindow();	//关闭窗口

private:
	KUiUpdataPatch();
	~KUiUpdataPatch();
	int		Initialize();					//初始化
	void	Breathe();
	void	LoadScheme(const char* pScheme);//载入界面方案
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);	//窗口函数

	void	OnFinished(bool bRestart);
	void	SetCloseParam(const char* pInfo, bool bQuit, bool bOk);
private:
	static KUiUpdataPatch*	m_pSelf;
private:
	KWndMessageListBox	m_MsgList;
	KWndScrollBar		m_MsgScroll;
	KWndImagePart		m_Progress;
	KWndButton			m_OkBtn;
	KWndButton			m_CancelBtn;
	KWndText256			m_StatusText;
	int					m_bQuit;
	HMODULE				m_PatchShell;
	fnSwordUpdateGetResult	m_fnSwordUpdateGetResult;
};
