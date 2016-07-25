/*****************************************************************************************
//	Core需要被外引用的物品的类属定义
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-17
------------------------------------------------------------------------------------------
*****************************************************************************************/

#ifndef COREOBJGENREDEF_H
#define COREOBJGENREDEF_H

//低十六位为粗分类型
#define	CGOG_NOTHING			 0	//啥都不是
#define	CGOG_MONEY				 1	//钱！！！
#define	CGOG_PLAYER_FACE		 2	//玩家头像
#define	CGOG_OBJECT				 3	//地面物品
#define	CGOG_SKILL				 4	//技能
#define	CGOG_ITEM				 5	//物品
#define	CGOG_NPC				 6	//NPC
#define	CGOG_MENU_NPC			 7	//界面上的NPC
#define	CGOG_NPC_BLUR			 8	//人物残影
#define	CGOG_MISSLE				 9	//子弹资源
#define	CGOG_EQUIP				 10 // 没用了
#define	CGOG_NPCSELLITEM		 11	// Npc买卖的东西

//高16位为类型细分数值，有的对象没有细分类型
//类型细分的取值规定都为非0
#define	CGOG_MAKE_DETAIL(_base, _detail)		(((_detail) << 16) + (_base))
//--装备细分--
#define	CGOG_EQUIP_CAP			CGOG_MAKE_DETAIL(CGOG_EQUIP, 1)	//装备-帽子
#define	CGOG_EQUIP_WEAPON		CGOG_MAKE_DETAIL(CGOG_EQUIP, 2)	//装备-武器
#define	CGOG_EQUIP_NECKLACE		CGOG_MAKE_DETAIL(CGOG_EQUIP, 3)	//装备-项链
#define	CGOG_EQUIP_BANGLE		CGOG_MAKE_DETAIL(CGOG_EQUIP, 4)	//装备-手镯
#define	CGOG_EQUIP_CLOTHING		CGOG_MAKE_DETAIL(CGOG_EQUIP, 5)	//装备-衣服
#define	CGOG_EQUIP_CAESTUS		CGOG_MAKE_DETAIL(CGOG_EQUIP, 6)	//装备-腰带
#define	CGOG_EQUIP_RING			CGOG_MAKE_DETAIL(CGOG_EQUIP, 7)	//装备-戒指
#define	CGOG_EQUIP_WAISTDECOR	CGOG_MAKE_DETAIL(CGOG_EQUIP, 8)	//装备-腰缀
#define	CGOG_EQUIP_SHOE			CGOG_MAKE_DETAIL(CGOG_EQUIP, 9)	//装备-鞋子
//--技能细分--
#define	CGOG_SKILL_LIVE			CGOG_MAKE_DETAIL(CGOG_SKILL, 1)	//生活技能
#define	CGOG_SKILL_FIGHT		CGOG_MAKE_DETAIL(CGOG_SKILL, 2)	//战斗技能
#define	CGOG_SKILL_CUSTOM		CGOG_MAKE_DETAIL(CGOG_SKILL, 3)	//自创武功
#define CGOG_SKILL_SHORTCUT		CGOG_MAKE_DETAIL(CGOG_SKILL, 4)	//快捷武功
//--物品细分--
#define	CGOG_ITEM_CORDIAL		CGOG_MAKE_DETAIL(CGOG_ITEM, 1)	//物品-滋补药品
//#define	CGOG_ITEM_OTHER			CGOG_MAKE_DETAIL(CGOG_ITEM, 2)	//物品-其它
//--人物残影细分--
#define	CGOG_NPC_BLUR_DETAIL(x)	CGOG_MAKE_DETAIL(CGOG_NPC_BLUR, x)
#endif
