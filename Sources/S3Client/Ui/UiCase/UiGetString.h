// -------------------------------------------------------------------------
//	文件名		：	UiGetString.h
//	创建者		：	Wooy(Wu yue)
//	创建时间	：	2003-3-18
//	功能描述	：	获得字串界面
// -------------------------------------------------------------------------
#ifndef __UiGetString_H__
#define __UiGetString_H__

#include "../Elem/WndButton.h"
#include "../Elem/WndEdit.h"


class KUiGetString : protected KWndImage
{
public:
	//----界面面板统一的接口函数----
	static KUiGetString*	OpenWindow(const char* pszTitle,
				const char* pszInitString,				
				KWndWindow* pRequester, unsigned int uParam,
				int nMinLen = 4, int nMaxLen = 16);	//打开窗口，返回唯一的一个类对象实例
	static KUiGetString*	GetIfVisible();
	static void			LoadScheme(const char* pScheme);	//载入界面方案
	static void			CloseWindow(bool bDestroy);		//关闭窗口
private:
	KUiGetString();
	~KUiGetString() {}
	void	Initialize();								//初始化
	void	Show();
	void	Hide();
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	void	OnCancel();
	void	OnOk();
private:
	static KUiGetString*	m_pSelf;
	KWndText32			m_Title;
	short				m_nMinLen, m_nMaxLen;
	KWndEdit32			m_StringEdit;
	KWndButton			m_OkBtn;
	KWndButton			m_CancelBtn;
	KWndWindow*			m_pRequester;
	unsigned int		m_uRequesterParam;
};


#endif // __UiGetString_H__