
/*****************************************************************************************
//	外界访问Core接口方法定义
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-12
------------------------------------------------------------------------------------------
	外界（如界面系统）通过此接口从Core获取游戏世界数据。

	包含伍个接口函数CoreOperationRequest、CoreGetGameData、CoreDrawGameObj、CoreProcessInput
与CoreSetCallDataChangedNofify。
	CoreOperationRequest用于发送对游戏的操作请求。参数uOper为操作的id，其值为梅举类型
GAMEOPERATION_INDEX的取值之一。参数uParam以及nParam的具体含义依据uOper的取值情况而定。
如果成功发送操作请求，函数返回非0值，否则返回0值。这些请求都要求Core立即接受，Core的
客户不保证通过此函数发送的数据在函数调用之后依然有效。
	获知游戏数据有两种方式，一种是调用接口函数CoreGetGameData主动获取，另外一种是注册
通知函数，当游戏数据变更的时候，被注册的通知函数就会被调用，不一定在调用通知函数的同时
传递改变的游戏数据。并且两种方式所处里的数据项范围并不相同。
    接口函数CoreGetGameData参数uDataId表示获取游戏数据的数据项内容索引，其值为梅举类型
GAMEDATA_INDEX的取值之一。参数uParam、nParam以及函数返回值的具体含义依据uDataId的取值
情况而定。
    注册通知函数的接口方法为CoreSetCallDataChangedNofify。参数pNotifyFunc为通知函数的
指针。返回值为非0值表示注册成功，否则表示失败。传入参数pNotifyFunc的值入为0，则已经注
册的通知函数将被取消。通过通知函数通知发生游戏数据改变的时候，不一定同时通过通知函数
传送数据改变。设计原则上是如果改变的数据内容少，可以方便简单地通过通知函数的参数传递的，
则随通知函数传递；否则只是发送通知而已，并不传送改变的游戏的

	CoreDrawGameObj用于绘制单个游戏对象。参数uObjGenre指出对象的类属，uId指出对象的id，
x、y指出绘制范围的左上角坐标，Width、Heightn指出了绘制范围的大小，Param用于额外的参数传
递，其含义将依赖于具体要绘制的对象类型。
*****************************************************************************************/

#pragma once

//=========================================================
// Core外部客户向core获取游戏数据的数据项内容索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
//如果特别指明返回值含义，则成功获取数据返回1，未成功返回0。
enum GAMEDATA_INDEX
{
	GDI_GAME_OBJ_DESC = 1,		//游戏对象描述说明文本串
	//uParam = (KUiObjAtContRegion*) 描述游戏对象的结构数据的指针，其中KUiObjAtContRegion::Region的数据固定为0，无意义。
	//nParam = (KGameObjDesc*) 所指的结构空间用于保存取得的描述说明。
	
	GDI_GAME_OBJ_DESC_INCLUDE_TRADEINFO,	//游戏对象描述说明文本串(包含交易相关信息)
	//参数含义同GDI_GAME_OBJ_DESC

	GDI_GAME_OBJ_DESC_INCLUDE_REPAIRINFO,	//游戏对象描述说明文本串(包含修理相关信息)
	//参数含义同GDI_GAME_OBJ_DESC
	
	GDI_GAME_OBJ_LIGHT_PROP,	//对象的光源属性数据
	//uParam = (KUiGameObject*) 描述游戏对象的结构数据的指针
	//nParam = to be def

	GDI_PLAYER_BASE_INFO,		//主角的一些不易变的数据
	//uParam = (KUiPlayerBaseInfo*)pInfo

	GDI_PLAYER_RT_INFO,			//主角的一些易变的数据
	//uParam = (KUiPlayerRuntimeInfo*)pInfo

	GDI_PLAYER_RT_ATTRIBUTE,	//主角的一些易变的属性数据
	//uParam = (KUiPlayerAttribute*)pInfo

	GDI_PLAYER_IMMED_ITEMSKILL,	//主角的立即使用物品与武功
	//uParam = (KUiPlayerImmedItemSkill*)pInfo

	GDI_PLAYER_HOLD_MONEY,		//主角随身携带的钱
	//nRet = 主角随身携带的钱

	GDI_PLAYER_IS_MALE,			//主角是否男性
	//nRet = (int)(bool)bMale	是否男性

	// from now on, flying add this item, get the information whether
	// a player can ride a horse.
	GDI_PLAYER_CAN_RIDE,		//主角是否可以骑马马
	//nRet = (int)bCanRide		是否可以
	//0 - 不可以骑马哦
	//1 - 可以哦

	GDI_ITEM_TAKEN_WITH,		//主角随身携带的物品等
	//uParam = (KUiObjAtRegion*) pInfo -> KUiObjAtRegion结构数组的指针，KUiObjAtRegion
	//				结构用于存储物品的数据及其放置区域位置信息。
	//nParam = pInfo数组中包含KUiObjAtRegion结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pInfo数组中的前多少个KUiObjAtRegion
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiObjAtRegion结构的数组
	//			才够存储全部的随身携带的物品信息。

	GDI_ITEM_IN_STORE_BOX,		//储物箱里的物品
	//参数及返回值含义同GDI_ITEM_TAKEN_WITH的

	GDI_EQUIPMENT,				//主角装备物品
	//uParam = (KUiObjAtRegion*)pInfo -> 包含11个元素的KUiObjAtRegion结构数组指针，
	//				KUiObjAtRegion结构用于存储装备的数据和放置位置信息。
	//			KUiObjAtRegion::Region::h = 0
	//			KUiObjAtRegion::Region::v 表示属于哪个位置的装备,其值为梅举类型
	//			UI_EQUIPMENT_POSITION的取值之一。请参看UI_EQUIPMENT_POSITION的注释。
	//Return =  其值表示pInfo数组中的前多少个KUiObjAtRegion结构被填充了有效的数据。

	GDI_TRADE_NPC_ITEM,			//npc列出来交易的物品
	//uParam = (KUiObjAtContRegion*) pInfo -> KUiObjAtContRegion结构数组的指针，KUiObjAtContRegion
	//				结构用于存储物品的数据及其放置区域位置信息。
	//				其中KUiObjAtContRegion::nContainer值表示第几页的物品
	//nParam = pInfo数组中包含KUiObjAtContRegion结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pInfo数组中的前多少个KUiObjAtContRegion
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiObjAtContRegion结构的数组
	//			才够存储全部的npc列出来交易的物品信息。

	GDI_TRADE_ITEM_PRICE,		//交易物品的价格
	//uParam = (KUiObjAtContRegion*) pItemInfo -> 用于指出是哪处的哪个物品
	//nParam = (KUiItemBuySelInfo*) pPriceInfo -> 用于接收物品名称交易价格等信息
	//Return = (int)(bool) 可否交易

	GDI_REPAIR_ITEM_PRICE,		//修理物品的价格
	//uParam = (KUiObjAtContRegion*) pItemInfo -> 用于指出是哪处的哪个物品
	//nParam = (KUiItemBuySelInfo*) pPriceInfo -> 用于接收物品名称修理费用等信息
	//Return = (int)(bool) 可否修理

	GDI_TRADE_OPER_DATA,		//交易操作相关的数据
	//uParam = (UI_TRADE_OPER_DATA)eOper 具体含义见UI_TRADE_OPER_DATA
	//nParam 具体应用与含义由uParam的取值状况决定,见UI_TRADE_OPER_DATA的说明
	//Return 具体含义由uParam的取值状况决定,见UI_TRADE_OPER_DATA的说明

	GDI_LIVE_SKILL_BASE,		//主角的生活技能数据
	//uParam = (KUiPlayerLiveSkillBase*) pInfo -> 主角的生活技能数据

	GDI_LIVE_SKILLS,			//主角掌握的各项生活技能
	//uParam = (KUiSkillData*) pSkills -> 包含10个KUiSkillData的数组用于存储各项生活技能的的数据。

	GDI_FIGHT_SKILL_POINT,		//剩余战斗技能点数
	//Return = 剩余战斗技能点数

	GDI_FIGHT_SKILLS,			//主角掌握的各项战斗技能
	//uParam = (KUiSkillData*) pSkills -> 包含50个KUiSkillData的数组用于存储各项战斗的数据。

	GDI_CUSTOM_SKILLS,			//主角的自创武功
	//uParam = (KUiSkillData*) pSkills -> 包含5个KUiSkillData的数组用于存储各项自创武功的数据。

	GDI_LEFT_ENABLE_SKILLS,		//可以设置在左键的(物理攻击)技能
	//uParam = (KUiSkillData*) pSkills -> 包含65个KUiSkillData的数组用于存储各技能的数据。
	//								KUiSkillData::nLevel用来表述技能显示在第几行
	//Return = 返回有效数据的Skills的数目

	GDI_RIGHT_ENABLE_SKILLS,	//可设置在右键的技能
	//uParam = (KUiSkillData*) pSkills -> 包含65个KUiSkillData的数组用于存储各技能的数据。
	//								KUiSkillData::nLevel用来表述技能显示在第几行
	//Return = 返回有效数据的Skills的数目

	GDI_NEARBY_PLAYER_LIST,		//获取周围玩家的列表
	//uParam = (KUiPlayerItem*)pList -> 人员信息列表
	//			KUiPlayerItem::nData = 0
	//nParam = pList数组中包含KUiPlayerItem结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pList数组中的前多少个KUiPlayerItem
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiPlayerItem结构的数组
	//			才够存储全部人员信息。

	GDI_NEARBY_IDLE_PLAYER_LIST,//获取周围孤单可受邀请的玩家的列表
	//参数含义同GDI_NEARBY_PLAYER_LIST

	GDI_NEARBY_NOT_FRIEND_LIST,//获取周围非好友的玩家列表
	//参数含义同GDI_NEARBY_PLAYER_LIST

	GDI_PLAYER_LEADERSHIP,		//主角统帅能力相关的数据
	//uParam = (KUiPlayerLeaderShip*) -> 主角统帅能力相关的数据结构指针

	GDI_ITEM_IN_ENVIRO_PROP,	//获得物品在某个环境位置的属性状态
	//uParam = (KUiGameObject*)pObj（当nParam==0时）物品的信息
	//uParam = (KUiObjAtContRegion*)pObj（当nParam!=0时）物品的信息
	//			此时KUiObjAtContRegion::Region的数据固定为0，无意义。
	//nParam = (int)(bool)bJustTry  是否只是尝试放置
	//Return = (ITEM_IN_ENVIRO_PROP)eProp 物品的属性状态
	
	GDI_CHAT_SEND_CHANNEL_LIST,		//玩家可以发送消息的聊天频道的列表
	//uParam = (KUiChatChannel*) pList -> KUiChatChannel结构数组的指针
	//nParam = pList数组中包含KUiChatChannel结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pInfo数组中的前多少个KUiChatChannel
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiChatChannel结构的数组
	//			才够存储全部的指定频道信息。

	GDI_CHAT_RECEIVE_CHANNEL_LIST,	//玩家可以订阅的消息频道的列表
	//参数及返回值含义同GDI_CHAT_SEND_CHANNEL_LIST

	GDI_CHAT_CURRENT_SEND_CHANNEL,	//获取当前发送消息的频道的信息
	//uParam = (KUiChatChannel*) pChannelInfo 指向用于填充频道的信息的结构空间

	GDI_CHAT_GROUP_INFO,			//聊天的好友分组信息
	//从nParam给定的索引开始查找第一个有效的分组，返回该分组的信息与分组索引。
	//uParam = (KUiChatGroupInfo*) pGroupInfo 分组信息
	//nParam = nIndex 欲获取的分组的索引
	//Return = 实际返回数据的分组的索引，如果未获得则返回-1

	GDI_CHAT_FRIENDS_IN_AGROUP,		//聊天一个好友分组中好友的信息
	//uParam = (KUiPlayerItem*)pList -> 人员信息列表
	//			KUiPlayerItem::nData = (CHAT_STATUS)eFriendStatus 好友的当前状态
	//nParam = 要获取列表的好友分组的索引
	//Return = 其值表示pList数组中的前多少个KUiPlayerItem结构被填充了有效的数据.

	GDI_PK_SETTING,					//获取pk设置
	//Return = (int)(bool)bEnable	是否允许pk

	GDI_SHOW_PLAYERS_NAME,			//获取显示各玩家人名
	//Return = (int)(bool)bShow	是否显示
	GDI_SHOW_PLAYERS_LIFE,			//获取显示各玩家生命
	//Return = (int)(bool)bShow	是否显示
	GDI_SHOW_PLAYERS_MANA,			//获取显示各玩家内力
	//Return = (int)(bool)bShow	是否显示
	
	GDI_PARADE_EQUIPMENT,				//看玩家装备物品,消息含义同GDI_EQUIPMENT
	
	GDI_IMMEDIATEITEM_NUM,				//取得快捷物品个数
	//uParam = nIndex（0，1，2）
};

//=========================================================
// Core外部客户向core获取游戏数据的数据项内容索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
enum GAMEDATA_CHANGED_NOTIFY_INDEX
{
	GDCNI_HOLD_OBJECT = 1,	//拿起了某个对象
	//uParam = (KUiGameObject*)pObject -> 对象信息，如果取值为0，表示放弃手上拿的东西

	GDCNI_PLAYER_BASE_INFO,	//主角的一些不易变的数据, see also GDI_PLAYER_BASE_INFO
	GDCNI_PLAYER_RT_ATTRIBUTE,	//主角的一些易变的属性数据, see also GDI_PLAYER_RT_ATTRIBUTE
	GDCNI_PLAYER_IMMED_ITEMSKILL,//主角的立即使用物品与武功
	//uParam = (KUiGameObject*)pInfo 物品或者武功的数据
	//nParam = 哪个位置的物品或者武功 当nParam >=0 时候 nParam表示第nParam个立即物品
	//			当nParam = -1 时，表示为左手武功，-2时为右手武功

	GDCNI_PLAYER_BRIEF_PROP,//主角身份地位等一些关键属性项的变化
	//uParam = (PLAYER_BRIEF_PROP)eProp 变化内容项，取值为枚举PLAYER_BRIEF_PROP的值之一。
	//nParam = 含义依据uParam的具体含义而定

	GDCNI_OBJECT_CHANGED,		//某处物品的新增或者减少
	//uParam = (KUiObjAtContRegion*)pInfo -> 物品数据及其放置区域位置信息
	//          当pInfo->eContainer == UOC_EQUIPTMENT时
	//				KUiObjAtRegion::Region::h 表示属于第几套装备
	//				KUiObjAtRegion::Region::v 表示属于哪个位置的装备,其值为枚举类型
	//				UI_EQUIPMENT_POSITION的取值之一。请参看UI_EQUIPMENT_POSITION的注释。
	//nParam = bAdd -> 0值表示减少这个物品，非0值表示增加这个物品
	
	GDCNI_CONTAINER_OBJECT_CHANGED,	//整个面板里的东西全部更新
	//uParam = (UIOBJECT_CONTAINER)eContainer;	//面板位置

	GDCNI_LIVE_SKILL_BASE,		//生活技能数值变化

	GDCNI_FIGHT_SKILL_POINT,	//剩余战斗技能点数变化
	//nParam = 新的剩余战斗技能点数

	GDCNI_SKILL_CHANGE,			//新掌握了一个技能/技能升級
	//uParam = (KUiSkillData*)pSkill -> 新掌握的技能
	//nParam = 新技能在同類中的排序位置

	GDCNI_PLAYER_LEADERSHIP,	//主角统帅能力相关的数据发生变化
	
	GDCNI_TEAM,					//队伍信息发生变化（队伍解散、玩家脱离这个队伍，队长权利转给别人等，关注别人的队伍等）
	//uParam = (KUiPlayerTeam*)pTeam -> 队伍信息,可以为NULL,表示玩家脱离了队伍

	GDCNI_TEAM_NEARBY_LIST,		//附近队伍的列表
	//uParam = (KUiTeamItem*)pList 存队伍信息的数组
	//nParam = nCount 队伍的数目

	GDCNI_TRADE_START,			//与玩家交易开始
	//uParam = (KUiPlayerItem*) pPlayer对方的信息

	GDCNI_TRADE_DESIRE_ITEM,	//对方增减想交易的物品
	//uParam = (KUiObjAtRegion*) pObject -> 物品信息，其中坐标信息为在交易界面中的坐标
	//nParam = bAdd -> 0值表示减少，1值表示增加
	//Remark : 如果物品是金钱的话，则KUiObjAtRegion::Obj::uId表示把金钱额调整为这个值，且nParam无意义。

	GDCNI_TRADE_OPER_DATA,		//交易操作相关的(状态)数据发生变化
	//uParam = (const char*) pInfoText 提示的文字，如"对方解除锁定"等

	GDCNI_TRADE_END,			//交易结束
	//nParam = (int)(bool)bTraded	是否进行了交易

	GDCNI_NPC_TRADE,			//与npc交易开始
	//nParam = (bool)bStart	是否交易为交易开始，如果取值非真表示交易（关闭）结束

	GDCNI_NPC_TRADE_ITEM,		//npc交易物品的变化
	//通过调用iCoreShell::GetGameData,使参数uDataId为GDI_TRADE_NPC_ITEM，来获取新的物品信息。

	GDCNI_GAME_START,			//进入游戏世界

	GDCNI_GAME_QUIT,			//离开了游戏世界

	GDCNI_QUESTION_CHOOSE,		//问题选择
	//uParam = (KUiQuestionAndAnswer*)pQuestionAndAnswer

	GDCNI_SPEAK_WORDS,			//npc说话内容
	//uParam = (KUiInformationParam*) pWordDataList 指向KUiInformationParam数组
	//nParam = pWordDataList包含KUiInformationParam元素的数目

	GDCNI_INFORMATION,			//新的信息来到（弹出专门的消息框）
	//uParam = (KUiInformationParam*)pInformation	//消息内容

	GDCNI_MSG_ARRIVAL,			//新（聊天内容窗口）消息来到
	//uParam = (cons char*)pMsgBuff 消息内容缓冲区
	//nParam = (KUiMsgParam*)pMsgParam 消息参数

	GDCNI_SEND_CHAT_CHANNEL,	//当前发送消息的频道改变了

	GDCNI_CHAT_GROUP,			//聊天好友分组发生变化

	GDCNI_CHAT_FRIEND,			//聊天好友发生变化
	//nParam = nGroupIndex 发生好友变化的分组的索引

	GDCNI_CHAT_FRIEND_STATUS,	//聊天好友状态发生变化
	//uParam = (KUiPlayerItem*)pFriend 发生状态变化的好友
	//			KUiPlayerItem::nData = (CHAT_STATUS)eFriendStatus 好友的当前状态
	//nParam = nGroupIndex 好友所属于的组的索引
	
	GDCNI_CHAT_MESSAGE,			//好有发来的讯息
	//uParam = (KUiChatMessage*)pMessage 消息数据
	//nParam = (KUiPlayerItem*)pFriend   发来讯息的好友

	GDCNI_SYSTEM_MESSAGE,		//系统消息
	//uParam = (KSystemMessage*)pMsg 系统消息的内容
	//nParam = (void*)pParamBuf 指向一个参数缓冲区，缓冲内数据含义依据pMsg的内容而定，
	//			参看KSystemMessage的注释说明，缓冲区的大小由pMsg->byParamSize给出。

	GDCNI_NEWS_MESSAGE,			//新闻消息
	//uParam = (KNewsMessage*)pMsg 新闻消息内容
	//nParam = (SYSTEMTIME*)pTime  新闻消息的时间参数，具体含义依赖于新闻消息的类型而定

	GDCNI_SWITCH_CURSOR,		//切换鼠标指针图形
	//nParam = \Ui\???\鼠标.ini中给出的图形索引数值

	GDCNI_OPEN_STORE_BOX,		//打开储物箱
	
	GDCNI_SWITCHING_SCENEPLACE,	//地图切换
	//nParam = (int)(bool)bStart 为非0值表示开始地图切换，为0值表示结束地图切换

	GDCNI_MISSION_RECORD,		//要求纪录下此任务提示信息
	//uParam = (KMissionRecord*) pRecord 纪录内容

	GDCNI_PK_SETTING,			//pk允许状况发生了变化
	//nParam = (int)(bool)bEnable 是否允许pk

	GDCNI_VIEW_PLAYERITEM,			//被申请玩家装备的数据已到
	//uParam = (KUiPlayerItem*)		//玩家的 player data

	GDCNII_RANK_INDEX_LIST_ARRIVE,	//排名项id列表到了
	//uParam = uCount 排名项id的数目
	//nParam = (KRankIndex*)pIndexList 排名项id描述结构的数组

	GDCNII_RANK_INFORMATION_ARRIVE,	//排名项的内容到了
	//uParam = ((unsigned short)usCount) | ((unsigned short)usIndexId << 16)
	//			usCount    此次传来的内容项的数目
	//			usIndexId  排名项id，表示传回的是这个id的排名项的内容
	//nParam = (KRankMessage *) 列表内容

	//====帮派相关====
	GDCNI_TONG_INFO,				//某人所在的帮派的信息
									//对应于GTOI_REQUEST_PLAYER_TONG（查询某人是哪帮会）
	//uParam = (KUiPlayerRelationWithOther*) 那个（某）人,以及此人与此帮派的关系
					//KUiPlayerRelationWithOther::nParam 是否开着的招人开关
	//nParam = (KTongInfo*) pToneInfo	//帮派的信息

	GDCNI_TONG_MEMBER_LIST,			//某个帮派中的成员的列表
	//uParam = (KUiGameObjectWithName*) pTong 用于描述是哪个帮派
							//KUiGameObjectWithName::szName 帮派名称
							//KUiGameObjectWithName::nData  pMemberList所给帮派成员的类型，参看TONG_MEMBER_FIGURE
							//KUiGameObjectWithName::nParam pMemberList所给帮派成员列表包含成员的起始的索引
							//KUiGameObjectWithName::uParam pMemberList所给帮派成员列表包含成员的数目
	//nParam = (KTongMemberItem*) pMemberList

	GDCNI_TONG_ACTION_RESULT,       //帮会操作返回的结果
	//uParam = (KUiGameObjectWithName*) pInfo 返回的数据
	                        //KUiGameObjectWithName::szName 对象玩家的名字
							//KUiGameObjectWithName::nData  操作的种类，参考TONG_ACTION_TYPE
							//KUiGameObjectWithName::nParam   V
							//KUiGameObjectWithName::uParam   V
							//KUiGameObjectWithName::szString 这几个涵义随不同操作不同
	// nParam : 成功 1 失败 0

	GDCNI_OPEN_TONG_CREATE_SHEET,   //通知界面打开帮会创建界面
	//uParam = ~0 打开    0 关闭
};

enum GAMEDEBUGCONTROL
{
	DEBUG_SHOWINFO = 1,
	DEBUG_SHOWOBSTACLE,
};

//=========================================================
// Core外部客户对core的操作请求的索引定义
//=========================================================
enum GAMEOPERATION_INDEX
{
	GOI_EXIT_GAME = 1,		//离开游戏
	//uParam = bIpSpotExit

	GOI_SWITCH_OBJECT_QUERY,		//交换
	//uParam = (KUiObjAtContRegion*)pObject1 -> 拿起的物品操作前的信息
	//如果无拿起的东西，则uParam = 0
	//nParam = (KUiObjAtContRegion*)pObject2 -> 放下的物品操作后的信息
	//如果无放下的东西，则nParam = 0
	//nRet = bSwitchable -> 是否可交换

	GOI_SWITCH_OBJECT,		//交换
	//uParam = (KUiObjAtContRegion*)pObject1 -> 拿起的物品操作前的信息
	//nParam = (KUiObjAtContRegion*)pObject2 -> 放下的物品操作后的信息
	//nRet = bSwitched -> 是否交换了

	GOI_REJECT_OBJECT,		//丢弃物品
	//uParam = (KUiObjAtContRegion*)pObject -> 欲丢弃的物品

	GOI_MONEY_INOUT_STORE_BOX,	//从StoreBox存取钱
	//uParam = (unsigned int)bIn 为非0值时表示存入，否则表示取出
	//nParam = 钱的数额

	GOI_PLAYER_ACTION,		//玩家执行/取消某个动作
	//uParam = (PLAYER_ACTION_LIST)eAction 动作标识

	GOI_PLAYER_RENASCENCE,		//玩家重生
	//nParam = (int)(bool)bBackTown 是否回城

	GOI_INFORMATION_CONFIRM_NOTIFY,	//消息获得确认的通知

	GOI_QUESTION_CHOOSE,	//问题选择答案
	//nParma = nAnswerIndex

	GOI_USE_ITEM,			//使用物品
	//uParam = (KUiObjAtRegion*)pInfo -> 物品的数据以及物品原来摆放的位置
	//nParam = 物品使用前放置的位置，取值为枚举类型UIOBJECT_CONTAINER。

	GOI_WEAR_EQUIP,			//穿上装备
	//uParam = (KUiObjAtRegion*)pInfo -> 装备的数据和放置位置信息
	//			KUiObjAtRegion::Region::h 表示属于第几套装备
	//			KUiObjAtRegion::Region::v 表示属于哪个位置的装备,其值为梅举类型
	//			UI_EQUIPMENT_POSITION的取值之一。请参看UI_EQUIPMENT_POSITION的注释。

	GOI_USE_SKILL,			//施展武功/技能
	//uParam = (KUiGameObject*)pInfo -> 技能数据
	//nParam = (目前只传0值。)鼠标指针当前坐标（绝对坐标），横坐标在低16位，纵坐标在高16位。(像素点坐标)

	GOI_SET_IMMDIA_SKILL,	//设置立即技能
	//uParam = (KUiGameObject*)pSKill, 技能信息
	//nParam = 立即位置，0表示为左键技能，1表示为右键技能

	GOI_TONE_UP_SKILL,		//增强一种技能，，一次加一点
	//uParam = 技能类属
	//nParam = (uint)技能id

	GOI_TONE_UP_ATTRIBUTE,	//增强一些属性的值，一次加一点
	//uParam = 表示要增强的是哪个属性，取值为UI_PLAYER_ATTRIBUTE的梅举值之一

	//============（与其它玩家）交易相关================
	GOI_TRADE_INVITE_RESPONSE,	//答应/拒绝交易请求
	//uParam = (KUiPlayerItem*)pRequestPlayer 发出请求的玩家
	//nParam = (int)(bool)bAccept 是否接受请求

	GOI_TRADE_DESIRE_ITEM,		//增减一个欲卖出的物品
	//uParam = (KUiObjAtRegion*) pObject -> 物品信息，其中坐标信息为在交易界面中的坐标
	//nParam = bAdd -> 0值表示减少，1值表示增加
	//Remark : 如果物品是金钱的话，则KUiObjAtRegion::Obj::uId表示把金钱额调整为这个值，且nParam无意义。

	GOI_TRADE_WILLING,			//有无交易意向
	//uParam = (const char*)pszTradMsg 关于交易消息一句话，当bWilling为true时有效
	//nParam = (int)(bool)bWilling 是否期待交易(叫卖)

	GOI_TRADE_LOCK,				//锁定交易
	//nParam = (int)(bool)bLock 是否锁定

	GOI_TRADE,					//交易
	//nParam = (int)(bool)bTrading
	
	GOI_TRADE_CANCEL,			//交易取消

	//============（与npc）交易相关================
	GOI_TRADE_NPC_BUY,			//向npc买物品
	//uParam = (KUiGameObject*)pObj -> 物品信息

	GOI_TRADE_NPC_SELL,			//卖物品给npc
	//uParam = (KUiObjAtContRegion*)pObj -> 物品信息

	GOI_TRADE_NPC_REPAIR,		//修理物品
	//uParam = (KUiObjAtContRegion*) pObj -> 想要的物品的信息

	GOI_TRADE_NPC_CLOSE,		//结束交易

	GOI_DROP_ITEM_QUERY,		//查询是否可以丢某个东西到游戏窗口
	//uParam = (KUiGameObject*)pObject -> 物品信息
	//nParam = 被拖动东西的当前坐标（绝对坐标），横坐标在低16位，纵坐标在高16位。(像素点坐标)
	//Return = 是否可以放下
	
//	GOI_DROP_ITEM,				//放置物品到游戏窗口
	//参数含义同GOI_DROP_ITEM_QUERY参数含义相同
	//Return = 是否东西被放下了

	GOI_SEND_MSG,				//发送消息
	//uParam = (const char*)pMsgBuff 消息内容缓冲区
	//nParam = (KUiMsgParam*)pMsgParam 消息参数
	
	//============聊天相关================
	GOI_SET_SEND_CHAT_CHANNEL,	//设置当前发送消息的频道
	//uParam = (KUiChatChannel*) pChannelInfo 要设置的频道的信息
	
	GOI_SET_SEND_WHISPER_CHANNEL,//设置当前发送消息的频道为与某人单聊
	//uParam = (KUiPlayerItem*) pFriend

	GOI_SET_RECEIVE_CHAT_CHANNEL,//设置订阅/取消消息频道
	//uParam = (KUiChatChannel*) pChannelInfo 要设置的频道的信息
	//nParam = (int)(bool) bEnable 是否订阅

	GOI_CHAT_GROUP_NEW,			//新建聊天好友组
	//uParam = (const char*) pGroupName
	//Return = (int)(bool) bSuccessed

	GOI_CHAT_GROUP_RENAME,		//重命名聊天好友组
	//uParam = (const char*) pGroupName
	//nParam = nIndex 组索引
	//Return = (int)(bool) bSuccessed

	GOI_CHAT_GROUP_DELETE,		//删除聊天好友组
	//nParam = nIndex 组索引
	//Return = (int)(bool) bSuccessed

	GOI_CHAT_FRIEND_ADD,		//新添加聊天好友
	//uParam = (KUiPlayerItem*)pFriend
	//			KUiPlayerItem::nData = 0

	GOI_CHAT_FRIEND_DELETE,		//删除聊天好友
	//uParam = (KUiPlayerItem*)pFriend
	//			KUiPlayerItem::nData = 0
	//nParam = nGroupIndex 好友所在的组的索引
	//Return = (int)(bool) bSuccessed

	GOI_CHAT_FRIEND_MOVE,		//好友移到新组
	//uParam = (KUiPlayerItem*)pFriend
	//			KUiPlayerItem::nData = 0
	//nParam = nGroupIndex 好友所到的新组的索引
	//Return = (int)(bool) bSuccessed

	GOI_CHAT_SET_STATUS,		//切换聊天状态
	//uParam = (CHAT_STATUS)eStatus 新的聊天状态

	GOI_CHAT_WORDS_TO_FRIEND,	//发给好友一条讯息
	//uParam = (KUiChatMessage*)pMessage 消息数据
	//nParam = (KUiPlayerItem*)pFriend   发来讯息的好友

	GOI_CHAT_FRIEND_INVITE,		//对别人要加自己为好友的回复
	//uParam = (KUiPlayerItem*)pRequestPlayer 发出请求的玩家
	//nParam = (int)(bool)bAccept 是否接受请求

	GOI_OPTION_SETTING,			//选项设置
	//uParam = (OPTIONS_LIST)eOptionItem 要设置的选项
	//nParam = (int)nValue 设置的值，其含义依赖于eOptionItem的含义
	//					参看OPTIONS_LIST各值的注释

	GOI_PLAY_SOUND,				//播放声音
	//uParam = (const char*)pszFileName

	GOI_PK_SETTING,				//设置PK
	//nParam = (int)(bool)bEnable	是否允许pk

	GOI_REVENGE_SOMEONE,		//仇杀某人
	//uParam = (KUiPlayerItem*) pTarget	仇杀目标

	GOI_SHOW_PLAYERS_NAME,		//显示各玩家人名
	//nParam = (int)(bool)bShow	是否显示
	GOI_SHOW_PLAYERS_LIFE,		//显示各玩家生命
	//nParam = (int)(bool)bShow	是否显示
	GOI_SHOW_PLAYERS_MANA,		//显示各玩家内力
	//nParam = (int)(bool)bShow	是否显示

	GOI_GAMESPACE_DISCONNECTED,	//游戏世界断开连接了
	
	GOI_VIEW_PLAYERITEM,		//申请看玩家装备
	//uParam = dwNpcID	玩家的m_dwID
	GOI_VIEW_PLAYERITEM_END,	//看玩家装备结束

	GOI_FOLLOW_SOMEONE,			//跟随某人
	//uParam = (KUiPlayerItem*) pTarget	跟随目标

	GOI_QUERY_RANK_INFORMATION,  //获取数据请求
	//uParam = usIndexId 排名项的id
};

//=========================================================
// Core外部客户对core的场景地图相关的操作请求的索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
//如果特别指明返回值含义，则成功获取数据返回1，未成功返回0。
enum GAME_SCENE_MAP_OPERATION_INDEX
{
	GSMOI_SCENE_TIME_INFO,			//当前主角所处的地域时间环境
	//uParam = (KUiSceneTimeInfo*)pInfo

	GSMOI_SCENE_MAP_INFO,				//当前主角所处的场景的地图信息
	//uParam = (KSceneMapInfo*) pInfo 用于获取信息的结构缓冲区的指针
	//Return = (int)(bool)bHaveMap 返回值表示当前场景是否有小地图。如果返回0值时， pInfo内返回的值无意义

	GSMOI_IS_SCENE_MAP_SHOWING,	//设置场景的小地图是否显示的状态
	//uParam = uShowElem,		//显示哪些内容，取值为SCENE_PLACE_MAP_ELEM枚举的一个或多个的组合。
				//SCENE_PLACE_MAP_ELEM在GameDataDef.h中定义
				//浏览小地图与其它一些项是互斥的
	//nParam = 低16位表示显示的宽度，高16位表示显示的高度（单位：像素点）

	GSMOI_PAINT_SCENE_MAP,		//绘制场景的小地图
	//uParam = (int)h 表示绘制起始点在屏幕上横坐标坐标（单位：像素点）
	//nParam = (int)v 表示绘制起始点在屏幕上纵坐标坐标（单位：像素点）

	GSMOI_SCENE_MAP_FOCUS_OFFSET,//设置小地图的焦点（/中心）
	//uParam = (int)nOffsetH	设置小地图焦点的水平坐标（单位：场景坐标）
	//nParam = (int)nOffsetV	设置小地图焦点的垂值坐标（单位：场景坐标）

	GSMOI_SCENE_FOLLOW_WITH_MAP,	//设置场景是否随着地图的移动而移动
	//nParam = (int)nbEnable 场景是否随着地图的移动而移动
};

//=========================================================
// Core外部客户对core的帮会相关的操作请求的索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
//如果特别指明返回值含义，则成功获取数据返回1，未成功返回0。
enum GAME_TONG_OPERATION_INDEX
{
	GTOI_TONG_CREATE,			//创建帮会
	//uParam = (const char*) pszTongName 帮会的名字
	//nParam = (NPCCAMP)enFaction 帮会阵营

	GTOI_TONG_IS_RECRUIT,		//查询某人的招人开关
	//uParam = (KUiPlayerItme*) 要查谁
	//Return = (int)(bool)		是否开着的招人开关

	GTOI_TONG_RECRUIT,          //招人开关
	//uParam = (int)(bool)bRecruit 是否愿意招人

	GTOI_TONG_ACTION,           //对帮内成员做的动作，或自己与帮会的关系的改变
	//uParam = (KTongOperationParam*) pOperParam 动作时的参数
	//nParam = (KTongMemberItem*) pMember 指出了操作（帮会成员）对象，

	GTOI_TONG_JOIN_REPLY,       //对申请加入的答复
	//uPAram = (KUiPlayerItem *) pTarget   申请方
	//nParam : !=0同意     ==0拒绝

	GTOI_REQUEST_PLAYER_TONG,	//查询某人是哪帮会
	//uParam = (KUiPlayerItem*) 要查谁
	//nParam = (int)(bool)bReturnTongDetail 是否要返回那个帮会的信息

	GTOI_REQUEST_TONG_DATA,     //要求某个帮会的各种资料
	//uParam = (KUiGameObjectWithName*)pTong 要查询的帮会
			//KUiGameObjectWithName::szName 帮会的名字
			//KUiGameObjectWithName::nData 资料的种类，值取自枚举TONG_MEMBER_FIGURE
			//			列表的种类是enumTONG_FIGURE_MASTER的话代表要求的是帮会的资讯。
			//KUiGameObjectWithName::nParam 开始的索引
};

//=========================================================
// Core外部客户对core的组队相关的操作请求的索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
//如果特别指明返回值含义，则成功获取数据返回1，未成功返回0。
enum GAME_TEAM_OPERATION_INDEX
{
	//----获取数据----
	TEAM_OI_GD_INFO,				//主角所在的队伍信息
	//uParam = (KUiPlayerTeam*)pTeam -> 队伍信息
	//Return = bInTeam, 如果为非0值表示主角在队伍中，pTeam结构是否被填充信息。
	//					如果为0值表示主角不在队伍中，pTeam结构未被填充有效信息。

	TEAM_OI_GD_MEMBER_LIST,		//获取主角所在队伍成员列表
	//uParam = (KUiPlayerItem*)pList -> 人员信息列表
	//			KUiPlayerItem::nData = (int)(bool)bCaptain 是否是队长
	//nParam = pList数组中包含KUiPlayerItem结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pList数组中的前多少个KUiPlayerItem
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiPlayerItem结构的数组
	//			才够存储全部的成员信息。

	TEAM_OI_GD_REFUSE_INVITE_STATUS,//获取拒绝邀请的状态
	//Return = (int)(bool)bEnableRefuse 为真值表示拒绝状态生效，否则表示不拒绝。

	//----操作请求----
	TEAM_OI_COLLECT_NEARBY_LIST,//获取周围队伍的列表

	TEAM_OI_APPLY,				//申请加入她人队伍
	//uParam = (KUiTeamItem*)	要申请加入的队伍的信息

	TEAM_OI_CREATE,				//新组队伍

	TEAM_OI_APPOINT,			//任命队长，只有队长调用才有效果
	//uParam = (KUiPlayerItem*)pPlayer -> 新队长的信息
	//			KUiPlayerItem::nData = 0

	TEAM_OI_INVITE,			//邀请别人加入队伍，只有队长调用才有效果
	//uParam = (KUiPlayerItem*)pPlayer -> 要邀请的人的信息
	//			KUiPlayerItem::nData = 0

	TEAM_OI_KICK,				//踢除队里的一个队员，只有队长调用才有效果
	//uParam = (KUiPlayerItem*)pPlayer -> 要踢除的队员的信息
	//			KUiPlayerItem::nData = 0

	TEAM_OI_LEAVE,				//离开队伍

	TEAM_OI_CLOSE,				//关闭组队，只有队长调用才有效果
	//nParam = (int)(bool)bClose 为真值表示处于关闭状态，否则表示不处于关闭状态

	TEAM_OI_REFUSE_INVITE,		//拒绝别人邀请自己加入队伍
	//nParam = (int)(bool)bEnableRefuse 为真值表示拒绝状态生效，否则表示不拒绝。

	TEAM_OI_APPLY_RESPONSE,			//是否批准他人加入队伍，只有队长调用才有效果
	//uParam = (KUiPlayerItem*)pPlayer -> 要欲批准的人信息
	//			KUiPlayerItem::nData = 0
	//nParam = (int)(bool)bApprove -> 是否批准了

	TEAM_OI_INVITE_RESPONSE,	//对组队邀请的回复
	//uParam = (KUiPlayerItem*)pTeamLeader 发出组队邀请的队长
	//nParam = (int)(bool)bAccept 是否接受邀请

};


//-------游戏世界数据改变的通知函数原型---------
struct IClientCallback
{
	virtual void CoreDataChanged(unsigned int uDataId, unsigned int uParam, int nParam) = 0;
	virtual void ChannelMessageArrival(DWORD nChannelID, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength, bool bSucc) = 0;
	virtual void MSNMessageArrival(char* szSourceName, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength, bool bSucc) = 0;
	virtual void NotifyChannelID(char* ChannelName, DWORD channelid, BYTE cost) = 0;
	virtual void FriendInvite(char* roleName) = 0;
	virtual void AddFriend(char* roleName, BYTE answer) = 0;
	virtual void FriendStatus(char* roleName, BYTE state) = 0;
	virtual void FriendInfo(char* roleName, char* unitName, char* groupname, BYTE state) = 0;
	virtual void AddPeople(char* unitName, char* roleName) = 0;
};

struct _declspec (novtable) iCoreShell
{
	virtual	int	 GetProtocolSize(BYTE byProtocol) = 0;
	virtual int	 Debug(unsigned int uDataId, unsigned int uParam, int nParam) = 0;
	//向游戏发送操作
	virtual int	 OperationRequest(unsigned int uOper, unsigned int uParam, int nParam) = 0;
	//发送输入设备的输入操作消息
	virtual void ProcessInput(unsigned int uMsg, unsigned int uParam, int nParam) = 0;
	//找x, y所指的Player
	virtual int FindSelectNPC(int x, int y, int nRelation, bool bSelect, void* pReturn, int& nKind) = 0;
	//找Name指定的Player
	virtual int FindSpecialNPC(char* Name, void* pReturn, int& nKind) = 0;
	//找x, y所指的Obj
	virtual int FindSelectObject(int x, int y, bool bSelect, int& nObjectIdx, int& nKind) = 0;
	virtual int ChatSpecialPlayer(void* pPlayer, const char* pMsgBuff, unsigned short nMsgLength) = 0;
	virtual void ApplyAddTeam(void* pPlayer) = 0;
	virtual void TradeApplyStart(void* pPlayer) = 0;
	virtual int UseSkill(int x, int y, int nSkillID) = 0;
	virtual int LockSomeoneUseSkill(int nTargetIndex, int nSkillID) = 0;
	virtual int LockSomeoneAction(int nTargetIndex) = 0;
	virtual int LockObjectAction(int nTargetIndex) = 0;
	virtual void GotoWhere(int x, int y, int mode) = 0;	//mode 0 is auto, 1 is walk, 2 is run
	virtual void Goto(int nDir, int mode) = 0;	//nDir 0~63, mode 0 is auto, 1 is walk, 2 is run
	virtual void Turn(int nDir) = 0;	//nDir 0 is left, 1 is right, 2 is back
	virtual int ThrowAwayItem() = 0;
	virtual int GetNPCRelation(int nIndex) = 0;

	//===========对游戏世界的分类操作==========
	//与地图相关的操作, uOper的取值来自 GAME_SCENE_MAP_OPERATION_INDEX
	virtual int	SceneMapOperation(unsigned int uOper, unsigned int uParam, int nParam) = 0;
	//与帮会相关的操作, uOper的取值来自 GAME_TONG_OPERATION_INDEX
	virtual int	TongOperation(unsigned int uOper, unsigned int uParam, int nParam) = 0;
	//与组队相关的操作，uOper的取值来自 GAME_TEAM_OPERATION_INDEX
	virtual int TeamOperation(unsigned int uOper, unsigned int uParam, int nParam) = 0;

	//从游戏世界获取数据
	virtual int	 GetGameData(unsigned int uDataId, unsigned int uParam, int nParam) = 0;

	//绘制游戏对象
	virtual void DrawGameObj(unsigned int uObjGenre, unsigned int uId, int x, int y, int Width, int Height, int nParam) = 0;
	//绘制游戏世界
	virtual void DrawGameSpace() = 0;

	virtual DWORD GetPing() = 0;
	//virtual void SendPing() = 0;
	//设置游戏世界数据改变的通知函数
	virtual int	 SetCallDataChangedNofify(IClientCallback* pNotifyFunc) = 0;
	//接受与分派处理网络消息
	virtual void NetMsgCallbackFunc(void* pMsgData) = 0;
	//设置绘图接口实例的指针
	virtual void SetRepresentShell(struct iRepresentShell* pRepresent) = 0;
	virtual void SetMusicInterface(void* pMusicInterface) = 0;
	virtual void SetRepresentAreaSize(int nWidth, int nHeight) = 0;
	//日常活动，core如果要寿终正寝则返回0，否则返回非0值
	virtual int  Breathe() = 0;
	//释放接口对象
	virtual void Release() = 0;
	virtual void SetClient(LPVOID pClient) = 0;
	
	virtual void SendNewDataToServer(void* pData, int nLength) = 0;
};

#ifndef CORE_EXPORTS

	//获取iCoreShell接口实例的指针
	extern "C" iCoreShell* CoreGetShell();

#else

	//对外发送游戏世界数据发生改变的通知
	void	CoreDataChanged(unsigned int uDataId, unsigned int uParam, int nParam);

#endif
