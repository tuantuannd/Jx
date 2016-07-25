#include "KCore.h"
#include "KSubWorld.h"
#include "KSubWorldSet.h"
#include "KSprControl.h"
#include "MyAssert.h"

#ifndef _SERVER

//---------------------------------------------------------------------------
//	功能：	构造函数
//---------------------------------------------------------------------------
KSprControl::KSprControl()
{
	Release();
}

//---------------------------------------------------------------------------
//	功能：	清空，初始化
//---------------------------------------------------------------------------
void	KSprControl::Release()
{
	m_bChange = TRUE;
	m_nTotalFrame = 1;
	m_nCurFrame = 0;
	m_nTotalDir = 1;
	m_nCurDir = 0;
	m_dwTimer = 0;
	m_dwInterval = 1;
	m_nXpos = 0;
	m_nYpos = 0;
	m_nCgXpos = 0;
	m_nCgYpos = 0;
	m_szName[0] = 0;
	m_dwNameID = 0;
}

//---------------------------------------------------------------------------
//	功能：	设定控制对象
//---------------------------------------------------------------------------
void	KSprControl::SetSprFile(char *lpszName, int nTotalFrame, int nTotalDir, int nInterVal)
{
	if (!lpszName)
		return;
	if (!lpszName[0])
	{
		Release();
		return;
	}

	if (strcmp(m_szName, lpszName) == 0)
		return;

	strcpy(m_szName, lpszName);
	m_dwNameID = g_FileName2Id(m_szName);
	m_nTotalDir = nTotalDir;
	if (m_nTotalDir < 1)
		m_nTotalDir = 1;
	if (nTotalFrame < m_nTotalDir)
	{
		m_nTotalFrame = m_nTotalDir;
	}
	else
	{
		m_nTotalFrame = nTotalFrame;
	}
	if (m_nTotalFrame < 1)
		m_nTotalFrame = 1;
	m_dwInterval = nInterVal;
	m_bChange = TRUE;
	this->m_dwTimer = SubWorld[0].m_dwCurrentTime;
}

//---------------------------------------------------------------------------
//	功能：	设定当前帧
//---------------------------------------------------------------------------
void	KSprControl::SetCurFrame(int nCurFrame)
{
	if (nCurFrame < 0 || nCurFrame >= m_nTotalFrame)
		return;
	m_nCurFrame = nCurFrame;
	if (m_nTotalFrame && m_nTotalDir)
		m_nCurDir = nCurFrame / (m_nTotalFrame / m_nTotalDir);
	else
		m_nCurDir = 0;
	m_dwTimer = SubWorld[0].m_dwCurrentTime;
}

//---------------------------------------------------------------------------
//	功能：	设定某方向为第几帧
//---------------------------------------------------------------------------
void	KSprControl::SetDirFrame(int nDir, int nFrame)
{
	if (nDir < 0 || nDir >= m_nTotalDir)
		return;
	m_nCurDir = nDir;
	if (nFrame < 0 || nFrame >= m_nTotalFrame / m_nTotalDir)
		m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nDir;
	else
		m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nDir + nFrame;
	m_dwTimer = SubWorld[0].m_dwCurrentTime;
}

//---------------------------------------------------------------------------
//	功能：	设定某方向为第几帧(方向需从64方向转换到真正的方向)
//---------------------------------------------------------------------------
void	KSprControl::Set64DirFrame(int nDir, int nFrame)
{
	if (nDir < 0 || nDir >= 64)
		return;
	m_nCurDir = (nDir + (32 / m_nTotalDir)) / (64 / m_nTotalDir);
	if (m_nCurDir >= m_nTotalDir)
		m_nCurDir -= m_nTotalDir;
	if (nFrame < 0 || nFrame >= m_nTotalFrame / m_nTotalDir)
		m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nDir;
	else
		m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nDir + nFrame;
	m_dwTimer = SubWorld[0].m_dwCurrentTime;
}

//---------------------------------------------------------------------------
//	功能：	设定当前方向
//---------------------------------------------------------------------------
BOOL	KSprControl::SetCurDir(int nDir)
{
	if (m_nCurDir == nDir)
		return TRUE;
	if (nDir < 0 || nDir >= m_nTotalDir)
		return FALSE;
	m_nCurDir = nDir;
	m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nDir;
	m_dwTimer = SubWorld[0].m_dwCurrentTime;
	return FALSE;
}

//---------------------------------------------------------------------------
//	功能：	设定当前方向(方向需从64方向转换到真正的方向)
//---------------------------------------------------------------------------
BOOL	KSprControl::SetCurDir64(int nDir)
{
	if (nDir < 0 || nDir >= 64)
		return FALSE;

	int nTempDir;

	nTempDir = (nDir + (32 / m_nTotalDir)) / (64 / m_nTotalDir);
	if (nTempDir >= m_nTotalDir)
		nTempDir -= m_nTotalDir;
	if (m_nCurDir == nTempDir)
		return TRUE;
	m_nCurDir = nTempDir;
	m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * nTempDir;
	m_dwTimer = SubWorld[0].m_dwCurrentTime;
	return FALSE;
}

//---------------------------------------------------------------------------
//	功能：	设定重心位置
//---------------------------------------------------------------------------
void	KSprControl::SetCenterPos(int x, int y)
{
	m_nCgXpos = x, m_nCgYpos = y;
}

//---------------------------------------------------------------------------
//	功能：	获得单方向帧数
//---------------------------------------------------------------------------
int		KSprControl::GetOneDirFrames()
{
	return m_nTotalFrame / m_nTotalDir;
}


//---------------------------------------------------------------------------
//	功能：	设定位置
//---------------------------------------------------------------------------
void	KSprControl::SetPos(int x, int y)
{
	m_nXpos = x;
	m_nYpos = y;
}


//---------------------------------------------------------------------------
//	功能：	判断动画是否播放到最后，当前是第 0 帧
//---------------------------------------------------------------------------
BOOL	KSprControl::CheckEnd()
{
	if (m_nCurFrame == (m_nTotalFrame / m_nTotalDir) * (m_nCurDir + 1) - 1)
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
//	功能：	设定当前方向的当前帧为第一帧
//---------------------------------------------------------------------------
void	KSprControl::SetDirStart()
{
	m_nCurFrame = m_nCurDir * (m_nTotalFrame / m_nTotalDir);
	this->m_dwTimer = SubWorld[0].m_dwCurrentTime;
}

//---------------------------------------------------------------------------
//	功能：	设定当前方向的当前帧为最后一帧
//---------------------------------------------------------------------------
void	KSprControl::SetDirEnd()
{
	m_nCurFrame = (m_nCurDir + 1) * (m_nTotalFrame / m_nTotalDir) - 1;
}

//---------------------------------------------------------------------------
//	功能：	获得当前方向第几帧
//---------------------------------------------------------------------------
int		KSprControl::GetCurDirFrameNo()
{
	return m_nCurFrame - m_nCurDir * (m_nTotalFrame / m_nTotalDir);
}

//---------------------------------------------------------------------------
//	功能：	取得当前方向的下一帧
//	参数：bLoop		是否循环播放 TRUE 循环播放 FALSE 播放到最后一帧停止
//	返回值：FALSE 文件名错误或者bLoop==FALSE时播放到最后一帧(可以通知调用者这个动画播放完了)
//			TRUE otherwise
//---------------------------------------------------------------------------
BOOL	KSprControl::GetNextFrame(BOOL bLoop)
{
	if ( !m_szName[0] )
		return FALSE;

	BOOL	bRetVal = TRUE;

	if (m_dwInterval <= 0)
		m_dwInterval = 1;
	
	_ASSERT(SubWorld[0].m_dwCurrentTime >= m_dwTimer);
	if (SubWorld[0].m_dwCurrentTime - m_dwTimer >= m_dwInterval)
	{
		if (bLoop)
		{
			m_dwTimer = SubWorld[0].m_dwCurrentTime;
			m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * m_nCurDir;
			bRetVal = TRUE;
		}
		else
		{
			m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * (m_nCurDir + 1) - 1;
			bRetVal = TRUE;
		}
	}
	else
	{
		m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * m_nCurDir + (m_nTotalFrame / m_nTotalDir) * (SubWorld[0].m_dwCurrentTime - m_dwTimer) / m_dwInterval;
		bRetVal = TRUE;
	}

	return bRetVal;
}

//---------------------------------------------------------------------------
//	功能：	取得当前方向的前一帧
//---------------------------------------------------------------------------
BOOL	KSprControl::GetPrevFrame(BOOL bLoop)
{
	if ( !m_szName[0] )
		return FALSE;
	if (SubWorld[0].m_dwCurrentTime - m_dwTimer >= m_dwInterval)
	{
		m_dwTimer = SubWorld[0].m_dwCurrentTime;
		m_nCurFrame--;
		if (m_nCurFrame < (m_nTotalFrame / m_nTotalDir) * m_nCurDir)
		{
			if (bLoop)
				m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * (m_nCurDir + 1) - 1;
			else
				m_nCurFrame = (m_nTotalFrame / m_nTotalDir) * m_nCurDir;
		}
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------
//	功能：	设定 spr 文件名
//---------------------------------------------------------------------------
void	KSprControl::SetFileName(char *lpszName)
{
	if (!lpszName)
		return;

	if (!lpszName[0])
		Release();
	else
	{
		strcpy(m_szName, lpszName);
		m_bChange = TRUE;
	}
}

//---------------------------------------------------------------------------
//	功能：	设定总帧数
//---------------------------------------------------------------------------
void	KSprControl::SetTotalFrame(int nTotalFrame)
{
	if (nTotalFrame > 0)
		m_nTotalFrame = nTotalFrame;
}

//---------------------------------------------------------------------------
//	功能：	设定总方向数
//---------------------------------------------------------------------------
void	KSprControl::SetTotalDir(int nTotalDir)
{
	if (nTotalDir > 0)
		m_nTotalDir = nTotalDir;
}

//---------------------------------------------------------------------------
//	功能：	设定帧间隔
//---------------------------------------------------------------------------
void	KSprControl::SetInterVal(DWORD dwInterval)
{
	m_dwInterval = dwInterval;
}
/*
//---------------------------------------------------------------------------
//	功能：	绘制
//---------------------------------------------------------------------------
void	KSprControl::DrawAlpha(
							   int nX,			// 屏幕坐标 x
							   int nY,			// 屏幕坐标 y
							   int nExAlpha)	// alpha 度
{
	KCacheNode	*pSprNode = NULL;
	KSprite		*pSprite;

	pSprNode = (KCacheNode *)g_SpriteCache.GetNode(m_szName, (KCacheNode *)pSprNode);
	pSprite = (KSprite*)pSprNode->m_lpData;
	if (pSprite)
		pSprite->DrawAlpha( nX - m_nCgXpos, nY - m_nCgYpos, m_nCurFrame, nExAlpha );
}

//---------------------------------------------------------------------------
//	功能：	绘制边框
//---------------------------------------------------------------------------
void	KSprControl::DrawBorder(
								int nX,			// 屏幕坐标 x
								int nY,			// 屏幕坐标 y
								int nColor)		// 颜色
{
	KCacheNode	*pSprNode = NULL;
	KSprite		*pSprite;

	pSprNode = (KCacheNode *)g_SpriteCache.GetNode(m_szName, (KCacheNode *)pSprNode);
	pSprite = (KSprite*)pSprNode->m_lpData;
	if (pSprite)
		pSprite->DrawBorder( nX - m_nCgXpos, nY - m_nCgYpos, m_nCurFrame, nColor );
}
*/
#endif