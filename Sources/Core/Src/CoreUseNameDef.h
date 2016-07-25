
#ifndef COREUSENAMEDEF_H
#define COREUSENAMEDEF_H

#define		MAX_PLAYER_IN_ACCOUNT			3

#define		SETTING_PATH					"\\settings"

//---------------------------- npc res 相关 ------------------------------
#define		RES_INI_FILE_PATH				"\\settings\\npcres"
#define		RES_NPC_STYLE_PATH				"\\spr\\npcres\\style"
#define		RES_SOUND_FILE_PATH				"sound"

#define		NPC_RES_KIND_FILE_NAME			"\\settings\\npcres\\人物类型.txt"
#define		NPC_NORMAL_RES_FILE				"\\settings\\npcres\\普通npc资源.txt"
#define		NPC_NORMAL_SPRINFO_FILE			"\\settings\\npcres\\普通npc资源信息.txt"
#define		STATE_MAGIC_TABLE_NAME			"\\settings\\npcres\\状态图形对照表.txt"
#define		PLAYER_RES_SHADOW_FILE			"\\settings\\npcres\\主角动作阴影对应表.txt"
#define		NPC_RES_SHADOW_FILE				"普通npc动作阴影对应表.txt"
#define		PLAYER_SOUND_FILE				"\\settings\\npcres\\主角动作声音表.txt"
#define		NPC_SOUND_FILE					"\\settings\\npcres\\npc动作声音表.txt"

#define		NPC_ACTION_NAME					"npc动作表.txt"
#define		ACTION_FILE_NAME				"动作编号表.txt"

#define		PLAYER_MENU_STATE_RES_FILE		"\\settings\\npcres\\界面状态与图形对照表.txt"
#define		PLAYER_INSTANT_SPECIAL_FILE		"\\settings\\npcres\\瞬间特效.txt"

#define		SPR_INFO_NAME					"信息"
#define		KIND_NAME_SECT					"人物类型"
#define		KIND_NAME_SPECIAL				"特殊npc"
#define		KIND_NAME_NORMAL				"普通npc"
#define		KIND_FILE_SECT1					"部件说明文件名"
#define		KIND_FILE_SECT2					"武器行为关联表1"
#define		KIND_FILE_SECT3					"武器行为关联表2"
#define		KIND_FILE_SECT4					"动作贴图顺序表"
#define		KIND_FILE_SECT5					"资源文件路经"

//--------------------------- player 门派相关 ------------------------------
// 注：下面这个设定文件必须按 金 金 木 木 水 水 火 火 土 土 的顺序排列
#define		FACTION_FILE					"\\settings\\faction\\门派设定.ini"

//--------------------------- player PK相关 ------------------------------
#define		defPK_PUNISH_FILE				"\\settings\\npc\\player\\PKPunish.txt"

//--------------------------- player 帮会相关 ------------------------------
#define		defPLAYER_TONG_PARAM_FILE		"\\settings\\tong\\TongSet.ini"


//----------------------------- 聊天相关 ------------------------------
#define		CHAT_PATH						"\\chat"
#define		CHAT_TEAM_INFO_FILE_NAME		"Team.cht"
#define		CHAT_CHANNEL_INFO_FILE_NAME		"Channel.cht"

#define		CHAT_CHANNEL_NAME_ALL			"所有玩家"
#define		CHAT_CHANNEL_NAME_SCREEN		"附近玩家"
#define		CHAT_CHANNEL_NAME_SINGLE		"好友"
#define		CHAT_CHANNEL_NAME_TEAM			"队友"
#define		CHAT_CHANNEL_NAME_FACTION		"同门"
#define		CHAT_CHANNEL_NAME_TONG			"帮众"
#define		CHAT_CHANNEL_NAME_SCREENSINGLE	"陌生人"
#define		CHAT_CHANNEL_NAME_SYSTEM		"系统"

#define		CHAT_TAKE_CHANNEL_NAME_TEAM			"队伍频道"
#define		CHAT_TAKE_CHANNEL_NAME_FACTION		"门派频道"
#define		CHAT_TAKE_CHANNEL_NAME_TONG			"帮会频道"
#define		CHAT_TAKE_CHANNEL_NAME_SCREENSINGLE	"私聊频道"

//---------------------------- player 数值相关 ------------------------------
#define		PLAYER_LEVEL_EXP_FILE			"\\settings\\npc\\player\\level_exp.txt"
#define		PLAYER_LEVEL_ADD_FILE			"\\settings\\npc\\player\\level_add.txt"
#define		PLAYER_LEVEL_LEAD_EXP_FILE		"\\settings\\npc\\player\\level_lead_exp.txt"
#define		BASE_ATTRIBUTE_FILE_NAME		"\\settings\\npc\\player\\NewPlayerBaseAttribute.ini"
#define		PLAYER_PK_RATE_FILE				"\\settings\\npc\\PKRate.ini"
#define		PLAYER_BASE_VALUE				"\\settings\\npc\\player\\BaseValue.ini"

// 00男金 01女金 02男木 03女木 04男水 05女水 06男火 07女火 08男土 09女土
#define		NEW_PLAYER_INI_FILE_NAME		"\\settings\\npc\\player\\NewPlayerIni%02d.ini"

//---------------------------- player 交易相关 ------------------------------
#define		BUYSELL_FILE					"\\settings\\buysell.txt"
#define		GOODS_FILE						"\\settings\\goods.txt"

//----------------------------- 物品相关 ------------------------------
#define		CHANGERES_MELEE_FILE			"\\settings\\item\\MeleeRes.txt"
#define		CHANGERES_RANGE_FILE			"\\settings\\item\\RangeRes.txt"
#define		CHANGERES_ARMOR_FILE			"\\settings\\item\\ArmorRes.txt"
#define		CHANGERES_HELM_FILE				"\\settings\\item\\HelmRes.txt"
#define		CHANGERES_HORSE_FILE			"\\settings\\item\\HorseRes.txt"
#define		ITEM_ABRADE_FILE				"\\settings\\item\\AbradeRate.ini"

//--------------- npc skill missles 设定文件，用于生成模板 -------------
#define		SKILL_SETTING_FILE				"\\settings\\Skills.txt"
#define		MISSLES_SETTING_FILE			"\\settings\\Missles.txt"
#define		NPC_SETTING_FILE				"\\settings\\NpcS.txt"
#define		NPC_GOLD_TEMPLATE_FILE			"\\settings\\npc\\NpcGoldTemplate.txt"

//---------------------------- object 相关 ------------------------------
#define		OBJ_DATA_FILE_NAME		"\\settings\\obj\\ObjData.txt"
#define		MONEY_OBJ_FILE_NAME		"\\settings\\obj\\MoneyObj.txt"
#define		OBJ_NAME_COLOR_FILE		"\\settings\\obj\\ObjNameColor.ini"

//---------------------------- 小地图相关 -------------------------------
#define		defLITTLE_MAP_SET_FILE	"\\Ui\\Default\\小地图颜色.ini"

//----------------------------- 声音相关 --------------------------------
#define		defINSTANT_SOUND_FILE	"\\settings\\SoundList.txt"
#define		defMUSIC_SET_FILE		"\\settings\\music\\MusicSet.txt"
#define		defMUSIC_FIGHT_SET_FILE	"\\settings\\music\\MusicFightSet.ini"

//------------------------------------------------------------------------
#define NPC_LEVELSCRIPT_FILENAME		"\\script\\npclevelscript\\npclevelscript.lua"
#define NPC_TEMPLATE_BINFILEPATH		"\\settings"
#define NPC_TEMPLATE_BINFILE			"NpcTemplate.Bin"
#define WEAPON_PHYSICSSKILLFILE			"\\settings\\武器物理攻击对照表.txt"				
#define WEAPON_PARTICULARTYPE			"详细类别"
#define	WEAPON_DETAILTYPE				"具体类别"
#define	WEAPON_SKILLID					"对应物理技能编号"
//-----------------------------------------------------------------------
#define	WORLD_WAYPOINT_TABFILE			"\\settings\\WayPoint.txt"
#define WORLD_STATION_TABFILE			"\\settings\\Station.txt"
#define WORLD_STATIONPRICE_TABFILE		"\\settings\\StationPrice.txt"
#define WORLD_WAYPOINTPRICE_TABFILE		"\\settings\\WayPointPrice.txt"
#define WORLD_DOCK_TABFILE				"\\settings\\Wharf.txt"
#define WORLD_DOCKPRICE_TABFILE			"\\settings\\WharfPrice.txt"
#define STRINGRESOURSE_TABFILE			"\\settings\\StringResource.txt"
#define PLAYER_RANK_SETTING_TABFILE		"\\settings\\RankSetting.txt"	
#define QUESTITEM_TABFILE				"\\settings\\item\\questkey.txt"

//---------------------------------任务--------------------------
#define TASK_MISSION_SETTING_TABFILE	"\\settings\\task\\missions.txt"



//---------------------------- 消息相关 ------------------------------
// 特别注意：以下字符串长度不能超过32字节，包括 %d %s 等接收具体内容以后的长度
#define		MSG_GET_EXP						"您获得%d点经验值。"
#define		MSG_DEC_EXP						"您损失了%d点经验值。"
#define		MSG_LEVEL_UP					"您的等级提升至%d。"
#define		MSG_LEADER_LEVEL_UP				"您的统率等级提升至%d！"
#define		MSG_GET_ATTRIBUTE_POINT			"您获得%d点潜能！"
#define		MSG_GET_SKILL_POINT				"您获得%d个技能点！"
#define		MSG_GET_ATTRIBUTE_SKILL_POINT	"您获得%d点潜能和%d个技能点！"

#define		MSG_TEAM_AUTO_REFUSE_INVITE		"自动拒绝他人对您的组队邀请。"
#define		MSG_TEAM_NOT_AUTO_REFUSE_INVITE	"显示他人对您的组队邀请。"
#define		MSG_TEAM_SEND_INVITE			"您向%s发出组队邀请！"
#define		MSG_TEAM_GET_INVITE				"%s邀请您组队！"
#define		MSG_TEAM_REFUSE_INVITE			"%s拒绝了您的组队邀请！"
#define		MSG_TEAM_CREATE					"您创建了一支队伍。"
#define		MSG_TEAM_CREATE_FAIL			"队伍创建失败。"
#define		MSG_TEAM_CANNOT_CREATE			"您现在不能组队！"
#define		MSG_TEAM_TARGET_CANNOT_ADD_TEAM	"对方现在不能组队！"
#define		MSG_TEAM_OPEN					"您的队伍现在允许接收新队员。"
#define		MSG_TEAM_CLOSE					"您的队伍现在不允许接收新队员。"
#define		MSG_TEAM_ADD_MEMBER				"%s成为您的队友了。"
#define		MSG_TEAM_SELF_ADD				"您加入%s的队伍。"
#define		MSG_TEAM_DISMISS_CAPTAIN		"您解散了自己的队伍！"
#define		MSG_TEAM_DISMISS_MEMBER			"%s解散了队伍！"
#define		MSG_TEAM_KICK_ONE				"%s被开除出队伍！"
#define		MSG_TEAM_BE_KICKEN				"您被开除出队伍！"
#define		MSG_TEAM_APPLY_ADD				"%s申请加入队伍！"
#define		MSG_TEAM_APPLY_ADD_SELF_MSG		"您申请加入%s的队伍！"
#define		MSG_TEAM_LEAVE					"%s离开队伍。"
#define		MSG_TEAM_LEAVE_SELF_MSG			"您离开%s的队伍。"
#define		MSG_TEAM_CHANGE_CAPTAIN_FAIL1	"队长任命失败！"
#define		MSG_TEAM_CHANGE_CAPTAIN_FAIL2	"%s统帅力不够！"
#define		MSG_TEAM_CHANGE_CAPTAIN_FAIL3	"您的队伍不能移交给新手！"
#define		MSG_TEAM_CHANGE_CAPTAIN			"%s被任命为队长！"
#define		MSG_TEAM_CHANGE_CAPTAIN_SELF	"您被%s任命为队长！"

#define		MSG_CHAT_APPLY_ADD_FRIEND		"您向%s申请好友。"
#define		MSG_CHAT_FRIEND_HAD_IN			"%s已被加为好友。"
#define		MSG_CHAT_GET_FRIEND_APPLY		"%s申请加为好友！"
#define		MSG_CHAT_REFUSE_FRIEND			"%s拒绝好友申请！"
#define		MSG_CHAT_ADD_FRIEND_FAIL		"添加%s为好友失败！"
#define		MSG_CHAT_ADD_FRIEND_SUCCESS		"您与%s成为好友！"
#define		MSG_CHAT_CREATE_TEAM_FAIL1		"好友分组创建失败！"
#define		MSG_CHAT_CREATE_TEAM_FAIL2		"已存在同名组！"
#define		MSG_CHAT_CREATE_TEAM_FAIL3		"已达最大分组数！"
#define		MSG_CHAT_RENAME_TEAM_FAIL		"组名修改失败！"
#define		MSG_CHAT_DELETE_TEAM_FAIL1		"组删除失败！"
#define		MSG_CHAT_DELETE_TEAM_FAIL2		"这个组不允许删除！"
#define		MSG_CHAT_FRIEND_ONLINE			"%s上线了！"
#define		MSG_CHAT_FRIEND_OFFLINE			"%s下线了！"
#define		MSG_CHAT_DELETE_FRIEND			"您与%s解除了好友关系！"
#define		MSG_CHAT_DELETED_FRIEND			"%s与您解除了好友关系！"
#define		MSG_CHAT_MSG_FROM_FRIEND		"%s发来消息！"
#define		MSG_CHAT_FRIEND_NOT_NEAR		"%s不在附近！"
#define		MSG_CHAT_FRIEND_NOT_ONLINE		"%s不在线！"
#define		MSG_CHAT_TAR_REFUSE_SINGLE_TALK	"%s没有订阅私聊频道！"

#define		MSG_SHOP_NO_ROOM				"背包空间不足！"
#define		MSG_SHOP_NO_MONEY				"金钱不足！"

#define		MSG_NPC_NO_MANA					"内力不足！"
#define		MSG_NPC_NO_STAMINA				"体力不足！"
#define		MSG_NPC_NO_LIFE					"生命不足！"
#define		MSG_NPC_DEATH					"%s气绝身亡！！"

#define		MSG_OBJ_CANNOT_PICKUP			"您不能拾取别人的物品！"
#define		MSG_MONEY_CANNOT_PICKUP			"您不能拾取别人的钱！"
#define		MSG_OBJ_TOO_FAR					"物品太远，无法拾取！"
#define		MSG_DEC_MONEY					"您损失了%d两银子！"
#define		MSG_EARN_MONEY					"您获得了%d两银子！"
#define		MSG_DEATH_LOSE_ITEM				"您丢失了物品%s！"
#define		MSG_ADD_ITEM					"您获得了%s！"
#define		MSG_ITEM_SAME_DETAIL_IN_IMMEDIATE	"快捷栏已经有同类型的物品了！"
#define		MSG_CAN_NOT_VIEW_ITEM			"现在不能察看他人装备！"
#define		MSG_ITEM_DAMAGED				"您装备的%s长期磨损，已经彻底损坏！"
#define		MSG_ITEM_NEARLY_DAMAGED			"您装备的%s已经接近损坏，请及时修理！"

#define		MSG_TRADE_STATE_OPEN			"您现在可以交易。"
#define		MSG_TRADE_STATE_CLOSE			"您现在不能交易。"
#define		MSG_TRADE_SELF_LOCK				"您交易物品已经锁定。"
#define		MSG_TRADE_SELF_UNLOCK			"您交易物品解除锁定。"
#define		MSG_TRADE_DEST_LOCK				"%s交易物品已经锁定。"
#define		MSG_TRADE_DEST_UNLOCK			"%s交易物品解除锁定。"
#define		MSG_TRADE_SUCCESS				"您与%s交易成功。"
#define		MSG_TRADE_FAIL					"您与%s交易失败。"
#define		MSG_TRADE_SELF_ROOM_FULL		"您的背包空间不足！"
#define		MSG_TRADE_DEST_ROOM_FULL		"%s的背包空间不足！"
#define		MSG_TRADE_SEND_APPLY			"您向%s申请交易！"
#define		MSG_TRADE_GET_APPLY				"%s申请与您交易！"
#define		MSG_TRADE_REFUSE_APPLY			"%s拒绝与您交易！"
#define		MSG_TRADE_TASK_ITEM				"任务物品不能交易！"

#define		MSG_PK_NORMAL_FLAG_OPEN			"PK开关打开！"
#define		MSG_PK_NORMAL_FLAG_CLOSE		"PK开关关闭！"
#define		MSG_PK_VALUE					"您目前的PK值是%d！"
#define		MSG_PK_ERROR_1					"您现在处于非战斗状态！"
#define		MSG_PK_ERROR_2					"新手不能仇杀！"
#define		MSG_PK_ERROR_3					"您正与他人切磋！"
#define		MSG_PK_ERROR_4					"您正与他人仇杀！"
#define		MSG_PK_ERROR_5					"您不能仇杀新手！"
#define		MSG_PK_ERROR_6					"对方正处于非战斗状态！"
#define		MSG_PK_ERROR_7					"对方正与他人切磋！"
#define		MSG_PK_ENMITY_SUCCESS_1			"您与%s建立仇杀关系！"
#define		MSG_PK_ENMITY_SUCCESS_2			"10秒钟后仇杀开始执行！"
#define		MSG_PK_ENMITY_CLOSE				"您的仇杀关系结束！"
#define		MSG_PK_ENMITY_OPEN				"仇杀现在开始执行！"

#define		MSG_TONG_CREATE_ERROR01			"帮会名过长！"
#define		MSG_TONG_CREATE_ERROR02			"帮会阵营错误！"
#define		MSG_TONG_CREATE_ERROR03			"帮会成员不能另外建立帮会！"
#define		MSG_TONG_CREATE_ERROR04			"杀手才能建立帮会！"
#define		MSG_TONG_CREATE_ERROR05			"建立帮会的等级不够！"
#define		MSG_TONG_CREATE_ERROR06			"建立帮会的统帅等级不够！"
#define		MSG_TONG_CREATE_ERROR07			"建立帮会需要 %d 金钱！"
#define		MSG_TONG_CREATE_ERROR08			"组队中，不能建立帮会！"
#define		MSG_TONG_CREATE_ERROR09			"帮会名错误！"
#define		MSG_TONG_CREATE_ERROR10			"建立帮会需要的金钱不够！"
#define		MSG_TONG_CREATE_ERROR11			"帮会名不合法或与别的帮会重名！"
#define		MSG_TONG_APPLY_CREATE			"您申请创建帮会！"
#define		MSG_TONG_CREATE_SUCCESS			"帮会创建成功！"
#define		MSG_TONG_APPLY_ADD				"您申请加入帮会！"
#define		MSG_TONG_APPLY_ADD_ERROR1		"帮会成员不能另外加入帮会！"
#define		MSG_TONG_APPLY_ADD_ERROR2		"杀手才能加入帮会！"
#define		MSG_TONG_APPLY_ADD_ERROR3		"组队中，不能加入帮会！"
#define		MSG_TONG_REFUSE_ADD				"%s拒绝您加入帮会！"
#define		MSG_TONG_ADD_SUCCESS			"您加入帮会！"
#define		MSG_TONG_CANNOT_LEAVE1			"帮主不能离开帮会！"
#define		MSG_TONG_CANNOT_LEAVE2			"长老不能离开帮会！"
#define		MSG_TONG_BE_KICKED				"您被踢出帮会！"
#define		MSG_TONG_LEAVE_SUCCESS			"您主动离开帮会成功！"
#define		MSG_TONG_LEAVE_FAIL				"您主动离开帮会失败！"
#define		MSG_TONG_CHANGE_AS_MASTER		"您被任命为帮主！"
#define		MSG_TONG_CHANGE_AS_MEMBER		"您现在成为普通帮众！"


#define		MESSAGE_SYSTEM_ANNOUCE_HEAD		"公告"

enum enumMSG_ID
{
	enumMSG_ID_NONE = 0,
	enumMSG_ID_TEAM_KICK_One,
	enumMSG_ID_TEAM_DISMISS,
	enumMSG_ID_TEAM_LEAVE,
	enumMSG_ID_TEAM_REFUSE_INVITE,
	enumMSG_ID_TEAM_SELF_ADD,
	enumMSG_ID_TEAM_CHANGE_CAPTAIN_FAIL,
	enumMSG_ID_TEAM_CHANGE_CAPTAIN_FAIL2,
	enumMSG_ID_OBJ_CANNOT_PICKUP,
	enumMSG_ID_OBJ_TOO_FAR,
	enumMSG_ID_DEC_MONEY,
	enumMSG_ID_TRADE_SELF_ROOM_FULL,
	enumMSG_ID_TRADE_DEST_ROOM_FULL,
	enumMSG_ID_TRADE_REFUSE_APPLY,
	enumMSG_ID_TRADE_TASK_ITEM,
	enumMSG_ID_GET_ITEM,
	enumMSG_ID_ITEM_DAMAGED,
	enumMSG_ID_MONEY_CANNOT_PICKUP,
	enumMSG_ID_CANNOT_ADD_TEAM,
	enumMSG_ID_TARGET_CANNOT_ADD_TEAM,
	enumMSG_ID_PK_ERROR_1,
	enumMSG_ID_PK_ERROR_2,
	enumMSG_ID_PK_ERROR_3,
	enumMSG_ID_PK_ERROR_4,
	enumMSG_ID_PK_ERROR_5,
	enumMSG_ID_PK_ERROR_6,
	enumMSG_ID_PK_ERROR_7,
	enumMSG_ID_DEATH_LOSE_ITEM,
	enumMSG_ID_TONG_REFUSE_ADD,
	enumMSG_ID_TONG_BE_KICK,
	enumMSG_ID_TONG_LEAVE_SUCCESS,
	enumMSG_ID_TONG_LEAVE_FAIL,
	enumMSG_ID_TONG_CHANGE_AS_MASTER,
	enumMSG_ID_TONG_CHANGE_AS_MEMBER,
	enumMSG_ID_NUM,
};

//---------------------------- 鼠标指针相关 ------------------------------
#define		MOUSE_CURSOR_NORMAL				0
#define		MOUSE_CURSOR_FIGHT				1
#define		MOUSE_CURSOR_DIALOG				2
#define		MOUSE_CURSOR_PICK				3
#define		MOUSE_CURSOR_USE				8
#endif
