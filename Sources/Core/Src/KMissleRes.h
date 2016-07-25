/*******************************************************************************
// FileName			:	KMissleRes.h
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-7-8 16:21:51
// FileDescription	:	
该类主要的功能是
一、维护子弹的资源，包括图像、声音等资源的加载、删除
二、维护子弹的换帧、跳帧，同步问题
三、处理子弹的图像显示、声音播放等问题
// Revision Count	:	
*******************************************************************************/

#ifndef __KMISSLERES_H__
#define __KMISSLERES_H__
#include "KEngine.h"
#include "KCore.h"
#include "SkillDef.h"
#ifndef _SERVER

#include "../../Represent/iRepresent/KRepresentUnit.h"

class KShadowNode :public KNode
{
public:
	int X;
	int Y;
	int Frame;
	int Alpha;
};

typedef struct
{
	char	AnimFileName[100];							// 图像spr 文件名
	int		nTotalFrame;
	int		nInterval;
	int		nDir;
	char	SndFileName[100];							// 声音wav 文件名
} TMissleRes;


#ifndef TOOLVERSION
class KMissleRes
#else
class CORE_API KMissleRes
#endif
{
public:
	KMissleRes(	);
	~KMissleRes();
	KList		m_SkillSpecialList;		//	子弹附加效果列表

	BOOL		m_bHaveEnd;
	BOOL		m_bLoopAnim;											//是否循环播放动画
	TMissleRes	m_MissleRes[MAX_MISSLE_STATUS * 2];							//几种状态下的资源情况	
	KCacheNode  * m_pSndNode ;
	int			m_nLastSndIndex;
	int			m_nMissleId;
	KList		m_ShadowList;
	int			m_nMaxShadowNum;		//	最大保留的残影量
	BOOL		m_bNeedShadow;
	int			m_nLastShadowLifeTime;
	int			m_nLifePerFrame;		//	每帧播放时间
	
	BOOL		m_bSubLoop;
	int			m_nSubStart;			//	子循环的起始帧
	int			m_nSubStop;			//	子循环的结束帧
	KRUImage	m_RUImage[MAX_MISSLE_STATUS];			//	图
	int			m_SceneID;
#ifdef TOOLVERSION
	KCacheNode * m_pSprNode;
#endif
public:
	BOOL		Init();
	void		Remove();
	void		Clear();
	void		LoadResource(int nStatus, char * MissleImage, char * MissleSound);
	int			Draw(int nStatus, int nX, int nY , int nZ, int nDir,  int nAllTime, int nCurLifeTime);
	void		PlaySound(int nStatus, int nX, int nY, int nLoop);
	int			GetSndVolume(int nVol);
	void		StopSound();
	void		NextFrame();
	BOOL		SpecialMovieIsAllEnd()
	{
		
		if (m_SkillSpecialList.GetHead())
		{
			return FALSE;
		}
		
		return TRUE;
	};
};
#endif
#endif //__KMISSLERES_H__