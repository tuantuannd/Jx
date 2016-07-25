
#pragma once

#ifndef _SERVER

#define		SPR_X_OFF	160
#define		SPR_Y_OFF	190

class KSprControl
{
public:
	int			m_bChange;							// 是否发生变化
	int			m_nTotalFrame;						// 总帧数
	int			m_nCurFrame;						// 当前帧
	int			m_nTotalDir;						// 总方向数
	int			m_nCurDir;							// 当前方向
	DWORD		m_dwTimer;							// 时间计数器
	DWORD		m_dwInterval;						// 帧间隔（动画速度）
	int			m_nXpos;							// 坐标 x
	int			m_nYpos;							// 坐标 y
	int			m_nCgXpos;							// 重心坐标 x
	int			m_nCgYpos;							// 重心坐标 y
	char		m_szName[80];						// 文件名
	DWORD		m_dwNameID;							// 文件名id

public:
	KSprControl();									// 构造函数
	void		Release();							// 清空
	void		SetFileName(char *lpszName);		// 设定 spr 文件名
	void		SetTotalFrame(int nTotalFrame);		// 设定总帧数
	void		SetCurFrame(int nCurFrame);			// 设定当前帧
	void		SetTotalDir(int nTotalDir);			// 设定总方向数
	BOOL		SetCurDir(int nDir);				// 设定当前方向
	void		SetInterVal(DWORD dwInterval);		// 设定帧间隔
	void		SetPos(int x, int y);				// 设定坐标
	void		SetCenterPos(int x, int y);			// 设定重心位置
	void		SetDirFrame(int nDir, int nFrame);	// 设定某方向为第几帧
	void		Set64DirFrame(int nDir, int nFrame);// 设定某方向为第几帧(方向需从64方向转换到真正的方向)
	BOOL		SetCurDir64(int nDir);				// 设定当前方向(方向需从64方向转换到真正的方向)
	int			GetOneDirFrames();					// 获得单方向帧数

	void		SetSprFile(							// 获得一个 spr 文件的各种信息
		char *lpszName,
		int nTotalFrame,
		int nTotalDir,
		int nInterVal);

	BOOL		GetNextFrame(BOOL bLoop = TRUE);	// 自动向后换帧
	BOOL		GetPrevFrame(BOOL bLoop = TRUE);	// 自动向前换帧
	BOOL		CheckExist();						// 判断文件是否存在
	BOOL		CheckEnd();							// 判断动画是否播放到最后
	void		SetDirStart();						// 设定当前方向的当前帧为第一帧
	void		SetDirEnd();						// 设定当前方向的当前帧为最后一帧
	int			GetCurDirFrameNo();					// 获得当前方向第几帧

	// 绘制
	void		DrawAlpha(
						int nX,						// 屏幕坐标 x
						int nY,						// 屏幕坐标 y
						int nExAlpha = 32);			// alpha 度
	// 绘制边框
	void		DrawBorder(
						int nX,						// 屏幕坐标 x
						int nY,						// 屏幕坐标 y
						int nColor);				// 颜色
};

//---------------------------------------------------------------------------
//	功能：	判断控制对象是否存在
//---------------------------------------------------------------------------
inline BOOL KSprControl::CheckExist()
{
	if ((m_nTotalFrame > 0) && m_szName[0])
		return TRUE;
	
    return FALSE;
}


#endif