#ifndef KPLAYERDEF_H
#define KPLAYERDEF_H

//数据库加载列表数据时每帧加载数量
#define DBLOADPERTIME_ITEM		10
#define DBLOADPERTIME_SKILL		10
#define DBLOADPERTIME_FRIEND	10
#define	DBLOADPERTIME_TASK		10



#define SCRIPT_PLAYERID		"PlayerId"		//脚本中的玩家Id全局变量名
#define SCRIPT_PLAYERINDEX	"PlayerIndex"	//脚本中的玩家的编号全局变量名
#define SCRIPT_OBJINDEX		"ObjIndex"		//触发脚本的物件的INDEX
#define SCRIPT_SUBWORLDINDEX		"SubWorld"		//脚本中当前世界的Index的全局变量名
#define SCRIPT_SUBWORLDID	"SubWorldID"	//脚本中当前世界的ID的全局变量名	

#ifdef _SERVER
#define	MAX_PLAYER		1200
#else
#define	MAX_PLAYER		2	// Index 0 reserved
#endif

#define	CLIENT_PLAYER_INDEX	1

#define		MAX_TEAM_APPLY_LIST		32		// 每个队伍的最大当前申请加入人数

//---------------------------------------------------------------------------
#define	QUEST_BEFORE	0x0000
#define	QUEST_START		0x0001
#define	QUEST_FINISHED	0xffff
//---------------------------------------------------------------------------
#ifndef _SERVER
enum MOUSE_BUTTON
{
	button_left  = 0,
	button_right,
};
#endif

enum SCRIPTACTION  
{
	SCRIPTACTION_UISHOW,			//显示某个UI界面
	SCRIPTACTION_EXESCRIPT,			//执行脚本
};
/*
enum UISEL_DATATYPE
{
	UISEL_nInfo_sOption,		//主信息以数字标识，选项为字符串
	UISEL_nInfo_sOption,		//主信息以数字标识，选项为数字标识	
	
	UISEL_sInfo_nOption,		
	UISEL_sInfo_sOption,
};
*/
enum
{
	ATTRIBUTE_STRENGTH = 0,
	ATTRIBUTE_DEXTERITY,
	ATTRIBUTE_VITALITY,
	ATTRIBUTE_ENGERGY,
};

enum	// 队伍创建失败原因
{
	Team_Create_Error_InTeam = 0,	// 已经在队伍中
	Team_Create_Error_SameName,		// 同名错误
	Team_Create_Error_Name,			// 队伍名字非法
	Team_Create_Error_TeamFull,		// 队伍数量太多，容纳不下
	Team_Create_Error_CannotCreate,	// 当前处于不能组队状态
};

enum TEAM_FIGURE
{
	TEAM_CAPTAIN,				// 队伍成员身份 队长
	TEAM_MEMBER,				// 队伍成员身份 队员
};

enum SYNC_STEP
{
	STEP_BASE_INFO = 0,
	STEP_FIGHT_SKILL_LIST,
	STEP_LIFE_SKILL_LIST,
	STEP_TASK_LIST,
	STEP_ITEM_LIST,
//	STEP_FRIEND_LIST,
	STEP_SYNC_END
};

enum SAVE_STATUS
{
	SAVE_IDLE = 0,
	SAVE_REQUEST,
	SAVE_DOING,
};
#endif
