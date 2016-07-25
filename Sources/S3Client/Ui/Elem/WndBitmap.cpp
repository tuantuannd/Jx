/*****************************************************************************************
//	界面窗口体系结构--位图窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-7-22
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "WndBitmap.h"
#include "KJpgFile.h"

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KWndBitmap::KWndBitmap()
{
	m_bBitmapLoaded = FALSE;
}

//--------------------------------------------------------------------------
//	功能：析构函数
//--------------------------------------------------------------------------
KWndBitmap::~KWndBitmap()
{
	m_Bitmap.Free();
}

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
BOOL KWndBitmap::Init(KIniFile* pIniFile, const char* pSection)
{
	m_bBitmapLoaded = FALSE;
	if (KWndWindow::Init(pIniFile, pSection))
	{
		KJpgFile	ImgJpg;
		char		Buffer[80];
		pIniFile->GetString(pSection, "Image", "", Buffer, sizeof(Buffer));
		m_bBitmapLoaded = ImgJpg.Load(Buffer, &m_Bitmap);
		return TRUE;
	}
	return FALSE;
}

#ifdef _DEBUG
#include "../../../../../Utility/Headers/iFont.h"
#endif
//--------------------------------------------------------------------------
//	功能：绘制窗口
//--------------------------------------------------------------------------
void KWndBitmap::PaintWindow()
{
	//to be replace
//	if (m_bBitmapLoaded && g_pCanvas)
//	{
//		g_pCanvas->DrawBitmap16(m_nAbsoluteLeft, m_nAbsoluteTop,
//			m_Bitmap.GetWidth(), m_Bitmap.GetHeight(), m_Bitmap.GetBitmap());
//	}
#ifdef _DEBUG
//	else
	{
		KWndWindow::PaintWindow();
		
		if (l_pFont16)
		{
			l_pFont16->TextOut(m_Caption, KF_ZERO_END, m_nAbsoluteLeft + 3, m_nAbsoluteTop + 3);
			l_pFont16->TextOut(m_Caption, KF_ZERO_END, m_nAbsoluteLeft + 2, m_nAbsoluteTop + 2, 0xFFFFFFFF);
		}
	}
#endif
}