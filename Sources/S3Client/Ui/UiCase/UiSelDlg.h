// -------------------------------------------------------------------------
//	文件名		：	UiSelDlg.h
//	创建者		：	彭建波
//	创建时间	：	2002-9-10 11:25:36
//	功能描述	：	选择对话框	
//
// -------------------------------------------------------------------------
#ifndef __UiSelDlg_H__
#define __UiSelDlg_H__

#include "../Elem/WndLabeledButton.h"
#include "../../../core/src/GameDataDef.h"

#define KUISEL_MAX_OPTION	4
class KUiSelDlg : protected KWndImage
{
public:
	int		Initialize();						//初始化
	void	Show(KUiQuestionAndAnswer* pContent);//显示窗口
	void	LoadScheme(const char* pScheme);	//载入界面方案

private:
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	void	OnClickButton(KWndWindow* pWnd);	//响应点击按钮

private:
	KWndRichLabeledButton m_OptionBtn[KUISEL_MAX_OPTION];//选项按钮
	KWndText256	m_IlluminateTxt;						//说明文字
};

extern KUiSelDlg	g_UiSelDlg;

#endif // __UiSelDlg_H__