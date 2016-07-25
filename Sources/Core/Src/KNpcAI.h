#ifndef	KNpcAIH
#define	KNpcAIH

#include "KMath.h"

class	KNpcAI
{
public:
	int				m_nIndex;
	BOOL			m_bActivate;
private:
public:
	KNpcAI();	
	void			Activate(int nIndex);
	void			Enable(){ m_bActivate = TRUE; }
	void			Disable(){ m_bActivate = FALSE; }	
private:	
	int				GetNearestNpc(int nRelation);
	int				GetNpcNumber(int nRelation);
	void			ProcessPlayer();

/*	void			ProcessAIType1();
	void			ProcessAIType2();
	void			ProcessAIType3();
	void			ProcessAIType4();
	void			ProcessAIType5();
	void			ProcessAIType6();
	void			ProcessAIType7();
	void			ProcessAIType8();
	void			ProcessAIType9();
	void			ProcessAIType10();*/
	void			ProcessAIType01();		// 普通主动类1
	void			ProcessAIType02();		// 普通主动类2
	void			ProcessAIType03();		// 普通主动类3
	void			ProcessAIType04();		// 普通被动类1
	void			ProcessAIType05();		// 普通被动类2
	void			ProcessAIType06();		// 普通被动类3
	void			TriggerObjectTrap();
	void			TriggerMapTrap();
	void			FollowAttack(int nIdx);
	BOOL			InEyeshot(int nIdx);
	void			CommonAction();
	BOOL			KeepActiveRange();
	void			KeepAttackRange(int nEnemy, int nRange);
	void			Flee(int nIdx);
#ifndef _SERVER
	void			FollowPeople(int nIdx);
	void			FollowObject(int nIdx);
#endif
	friend class KNpc;
#ifndef _SERVER
	// 装饰性质NPC运动函数系列 
	// flying add these on Jun.4.2003
	// 所有装饰性NPC运动处理入口，由Activate(int)内部调用
	int				ProcessShowNpc();
	int  			ShowNpcType11();
	int				ShowNpcType12();
	int				ShowNpcType13();
	int				ShowNpcType14();
	int				ShowNpcType15();
	int				ShowNpcType16();
	int				ShowNpcType17();
	int             GetNpcMoveOffset(int nDir, int nDistance, int *pnX, int *pnY);

	// 判断是否超出范围
	//BOOL KeepActiveShowRange();
	// 16/17 AiMode NPC的逃逸动作
	int				DoShowFlee(int nIdx);

	// 判断是否这个帧内可以给该NPC下指令
	BOOL			CanShowNpc();
#endif
	// flying add the function to get nearest player.
	int			IsPlayerCome();
};

#if !defined _SERVER
inline int KNpcAI::GetNpcMoveOffset(int nDir, int nDistance, int *pnX, int *pnY)
{
    _ASSERT(pnX);
    _ASSERT(pnY);

    *pnX = -nDistance * g_DirSin(nDir, 64);
    *pnY = -nDistance * g_DirCos(nDir, 64);

    return true;
}

inline BOOL KNpcAI::CanShowNpc()
{
	BOOL bResult = TRUE;
	if (Npc[m_nIndex].m_AiParam[5] < Npc[m_nIndex].m_AiParam[4])
		bResult = FALSE;
	Npc[m_nIndex].m_AiParam[5]++;
	return bResult;
}
#endif

extern KNpcAI NpcAI;
#endif
