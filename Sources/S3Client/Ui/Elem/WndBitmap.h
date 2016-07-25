/*****************************************************************************************
//	界面窗口体系结构--位图窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-7-22
------------------------------------------------------------------------------------------
	显示(Jpg)位图的窗口。
*****************************************************************************************/
#pragma once
#include "WndWindow.h"
#include "KBitmap16.h"

class KWndBitmap : public KWndWindow
{
private:
	KBitmap16		m_Bitmap;			//位图资源
	BOOL			m_bBitmapLoaded;	//是否有位图被载入
public:
	KWndBitmap();
	virtual			~KWndBitmap();
	virtual BOOL	Init(KIniFile* pIniFile, const char* pSection);//初始化
	virtual void	PaintWindow();							//绘制窗口	
};