// -------------------------------------------------------------------------
//	文件名		：	UiNewPlayer2.h
//	创建者		：	彭建波
//	创建时间	：	2002-9-10 14:25:21
//	功能描述	：	角色新建界面－2
// -------------------------------------------------------------------------
#ifndef __UINEWPLAYER2_H__
#define __UINEWPLAYER2_H__

#include "../Elem/WndButton.h"
#include "../Elem/WndText.h"
#include "../Elem/WndShowAnimate.h"
#include "../../Login/Character/SelSavedCharacter.h"

class KUiNewPlayer2 : protected KWndShowAnimate
{
public:
	static KUiNewPlayer2* OpenWindow(const char* pName, int bMale);//打开窗口，返回唯一的一个类对象实例
	static void			CloseWindow(bool bDestroy);		//关闭窗口，同时可以选则是否删除对象实例

private:
	KUiNewPlayer2();
	~KUiNewPlayer2();
	void	LoadScheme(const char* pScheme);//载入界面方案
	void	Initialize();					//初始化
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	void	OnClickButton(KWndWindow* pWnd);//响应点击按钮
	void	UpdateProperty();				//更新属性说明
	void	OnOk();							//完成
	void	OnCancel();						//取消
	void	Breathe();
private:
	static KUiNewPlayer2* m_pSelf;
	
private:
	KWndText256	m_PropertyShow;				// 属性说明
	KWndButton	m_Gold;						// 金
	KWndButton	m_Wood;						// 木
	KWndButton	m_Water;					// 水
	KWndButton	m_Fire;						// 火
	KWndButton	m_Earth;					// 土
	KWndButton	m_OK;						// 确定
	KWndButton  m_Cancel;					// 取消
	
	KNewCharacterInfo	m_Info;
	bool		m_bCreating;
};

#endif // __UINEWPLAYER2_H__