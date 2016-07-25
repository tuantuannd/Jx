
#pragma once

#ifndef _SERVER

#include "KWavSound.h"
#include "KList.h"
#include "KNpcResNode.h"
#include "KNpcResList.h"
#include "KSprControl.h"
#include "../../Represent/iRepresent/KRepresentUnit.h"

class KCacheNode;

class KStateSpr
{

public:
	int				m_nID;						// 在表格文件中的位置（从 1 开始，0为空）
	int				m_nType;					// 类型：头顶、脚底、身上
	int				m_nPlayType;				// 播放类型
	int				m_nBackStart;				// 身上类型 背后开始帧
	int				m_nBackEnd;					// 身上类型 背后结束帧
	KSprControl		m_SprContrul;				// spr 控制
public:
	KStateSpr();
	void			Release();
};

#define		MAX_BLUR_FRAME		7
#define		START_BLUR_ALPHA	128
#define		BLUR_ALPHA_CHANGE	16

class KNpcBlur
{
public:
	int				m_nActive;							// 当前残影处理是否处于激活状态
	int				m_nCurNo;							// 当前帧指针
	DWORD			m_dwTimer;							// 时间计数器
	DWORD			m_dwInterval;						// 多少帧取一次残影
	int				m_nMapXpos[MAX_BLUR_FRAME];			// 对应的地图坐标 x
	int				m_nMapYpos[MAX_BLUR_FRAME];			// 对应的地图坐标 y
	int				m_nMapZpos[MAX_BLUR_FRAME];			// 对应的地图坐标 z
	unsigned int	m_SceneIDNpcIdx[MAX_BLUR_FRAME];
	unsigned int	m_SceneID[MAX_BLUR_FRAME];			// 
	KRUImage		m_Blur[MAX_BLUR_FRAME][MAX_PART];	// 残影绘制列表
public:
	KNpcBlur();
	~KNpcBlur();
	BOOL			Init();
	void			Remove();
	void			SetNextNo();
	void			SetMapPos(int x, int y, int z, int nNpcIdx);
	void			ChangeAlpha();
	void			ClearCurNo();
	void			SetFile(int nNo, char *lpszFileName, int nSprID, int nFrameNo, int nXpos, int nYpos, int nZpos);
	void			Draw(int nIdx);
	BOOL			NowGetBlur();
	void			AddObj();
	void			RemoveObj();
};

#ifdef TOOLVERSION
class CORE_API  KNpcRes
#else
class KNpcRes
#endif
{

	enum
	{
		SHADOW_BEGIN	= 0,
		STATE_BEGIN		= 1,
		PART_BEGIN		= 1 + 6,
		SPEC_BEGIN		= 1 + MAX_PART + 6,
		MENUSTATE_BEGIN = 1 + MAX_PART + 6 + 1,
	};
private:
	int				m_nDoing;							// Npc的动作
	int				m_nAction;							// Npc的实际动作（与武器、骑马有关）
	int				m_nNpcKind;							// 特殊 普通
	int				m_nXpos;							// 坐标 x
	int				m_nYpos;							// 坐标 y
	int				m_nZpos;							// 坐标 z
	unsigned int 	m_SceneID_NPCIdx;                   // 在场景中的ID 对应的NPCidx
	unsigned int	m_SceneID;							// 在场景中的ID
	int				m_nHelmType;						// 当前头部类型
	int				m_nArmorType;						// 当前身体类型
	int				m_nWeaponType;						// 当前武器类型
	int				m_nHorseType;						// 当前马匹类型
	BOOL			m_bRideHorse;						// 当前是否骑马
	int				m_nBlurState;
	char			m_szSoundName[80];					// 当前音效文件名
	KCacheNode		*m_pSoundNode;						// 声效指针
	KWavSound		*m_pWave;							// 声效wav指针
public:
	enum
	{
		adjustcolor_physics = 0,		// 物理伤害
			adjustcolor_poison, 
			adjustcolor_freeze,			// 火焰伤害
			adjustcolor_burn,			// 冰冻伤害
			adjustcolor_confuse,			// 闪电伤害
			adjustcolor_stun,			// 毒素伤害
	};
	KSprControl		m_cNpcImage[MAX_PART];				// 所有动作的所有spr文件名
	KSprControl		m_cNpcShadow;						// npc阴影
	KStateSpr		m_cStateSpr[6];						// 状态特效，0 1 为头顶 2 3 为脚底 4 5 为身上
	KSprControl		m_cSpecialSpr;						// 特殊的只播放一遍的随身spr文件
	unsigned int	m_ulAdjustColorId;

	KSprControl		m_cMenuStateSpr;
	int				m_nMenuState;
	int				m_nBackMenuState;
	int				m_nSleepState;
	//char			m_szSentence[MAX_SENTENCE_LENGTH];
	//char			m_szBackSentence[MAX_SENTENCE_LENGTH];
	
	int				m_nSortTable[MAX_PART];				// 排序表

	KRUImage		m_cDrawFile[MAX_NPC_IMAGE_NUM];// 绘制列表 身体部件 + 阴影 + 魔法状态 + 特殊动画 + 头顶状态
	KNpcBlur		m_cNpcBlur;							// npc 残影

	KNpcResNode		*m_pcResNode;						// npc 资源

private:
	// 由一个图像资源文件名得到他的阴影图像文件名
	void			GetSoundName();						// 获得当前动作的音效文件名
	void			PlaySound(int nX, int nY);			// 播放当前动作的音效
	void			SetMenuStateSpr(int nMenuState);					// set menu state spr
public:
	KNpcRes();
	~KNpcRes();
	BOOL			Init(char *lpszNpcName, KNpcResList *pNpcResList);	// 初始化
	void			Remove(int nNpcIdx);								// 清除
	void			Draw(int nNpcIdx, int nDir, int nAllFrame, int nCurFrame, BOOL bInMenu = FALSE);		// 绘制
	void			DrawBorder();
	int				DrawMenuState(int nHeightOffset);
	BOOL			SetHelm(int nHelmType);								// 设定头盔类型
	BOOL			SetArmor(int nArmorType);							// 设定盔甲类型
	BOOL			SetWeapon(int nWeaponType);							// 设定武器类型
	BOOL			SetHorse(int nHorseType);							// 设定马匹类型
	BOOL			SetAction(int nDoing);								// 设定动作类型
	int				GetHelm(){return m_nHelmType;};
	int				GetArmor(){return m_nArmorType;	};
	int				GetWeapon(){return m_nWeaponType;};
	BOOL			SetRideHorse(BOOL bRideHorse);						// 设定是否骑马
	void			SetPos(int nNpcIdx, int x, int y, int z = 0, BOOL bFocus = FALSE, BOOL bMenu = FALSE);// 设定 npc 位置
	void			SetState(KList *pNpcStateList, KNpcResList *pNpcResList);	// 设定状态特效
	void			SetSpecialSpr(char *lpszSprName);					// 设定特殊的只播放一遍的随身spr文件
	void			SetBlur(BOOL bBlur);								// 残影打开关闭
	void			SetAdjustColorId(unsigned long ulColorId){m_ulAdjustColorId = ulColorId;};			// 设置偏色情况，如果为0表示不偏色.
	int				GetAction(){return m_nAction;};
	void			SetMenuState(int nState, char *lpszSentence = NULL, int nSentenceLength = 0);	// 设定头顶状态
	int				GetMenuState();						// 获得头顶状态
	void			SetSleepState(BOOL bFlag);			// 设定睡眠状态
	BOOL			GetSleepState();						// 获得睡眠状态
	void			StopSound();
	int				GetSndVolume(int nVol);
	static void		GetShadowName(char *lpszShadow, char *lpszSprName);
	int				GetNormalNpcStandDir(int nFrame);	// 动画帧数转换成逻辑方向(0 - 63)
};
#endif
