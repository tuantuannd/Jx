//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNpcResNode.h
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------

#pragma once

#ifndef _SERVER
#include	"KNode.h"

// 男主角 女主角 （拼接npc） 普通npc
enum
{
	NPC_RES_SPECIAL = 0,
	NPC_RES_NORMAL,
	MAX_NPC_RES_KIND,
};

#define	MAX_BODY_PART			4		// 整个npc分为几个大部分
#define	MAX_BODY_PART_SECT		4		// 每个大部分分为几个小部分
#define	MAX_PART				(MAX_BODY_PART*MAX_BODY_PART_SECT)
#define MAX_NPC_IMAGE_NUM		(1 + MAX_PART + 6 + 1 + 1)			// 部件加阴影加状态加特殊加menustate

#define	MAX_ACTION				128
#define MAX_RES_STATE			128

// 处理动作的种类名称
class CActionName
{
private:
	char	m_szName[MAX_ACTION][FILE_NAME_LENGTH];	// 动作名称
	int		m_nMaxAction;				// 最大允许动作种类
	int		m_nCurActionNo;				// 动作种类

public:
	CActionName();
	// 获取动作种类、名称等信息
	BOOL	Init(char *lpszFileName);
	// 由动作名称得到动作编号
	int		GetActionNo(char *lpszName);
	// 得到动作种类数
	int		GetActionCount();
	// 由动作编号得到动作名称
	BOOL	GetActionName(int nNo, char *lpszName, int nSize);

};

class CShadowFileName
{
private:
	struct	SShadowFileInfo
	{
		char	m_szName[FILE_NAME_LENGTH];
		int		m_nTotalFrame;
		int		m_nTotalDir;
		int		m_nInterval;
		int		m_nCgX;
		int		m_nCgY;
	}	*m_psInfo;
	int		m_nInfoLength;

public:
	CShadowFileName();
	~CShadowFileName();
	// 初始化内存分配
	void		Init(int nActionCount);
	// 得到某个动做的阴影文件名及其相关文件信息
	BOOL		GetFile(int nActionNo, int *pnFrame, int *pnDir, int *pnInterval, int *pnCgX, int *pnCgY, char *lpszName);
friend class KNpcResNode;
};

enum StateMagicType
{
	STATE_MAGIC_HEAD = 0,
	STATE_MAGIC_BODY,
	STATE_MAGIC_FOOT,
	STATE_MAGIC_TYPE_NUM,
};

// 状态与随身光效对应表
class CStateMagicTable
{
private:
	int			m_nType[MAX_RES_STATE];			// 类型：头顶、脚底、身上
	int			m_nPlayType[MAX_RES_STATE];		// 播放类型：循环播放
	int			m_nBackStart[MAX_RES_STATE];	// 身上类型npc背后开始帧
	int			m_nBackEnd[MAX_RES_STATE];		// 身上类型npc背后结束帧
	int			m_nTotalFrame[MAX_RES_STATE];	// spr 文件总帧数
	int			m_nTotalDir[MAX_RES_STATE];		// spr 文件总方向数
	int			m_nInterVal[MAX_RES_STATE];// spr 文件帧间隔
	char		m_szName[MAX_RES_STATE][80];	// spr 文件名
public:
	CStateMagicTable();							// 构造函数
	BOOL		Init();							// 初始化
	// 功能:	得到某种状态的光影效果的信息
	void		GetInfo(int nNo, char *lpszGetName, int *pnType, int *pnPlayType, int *pnBackStart, int *pnBackEnd, int *pnTotalFrame, int *pnTotalDir, int *pnInterVal);
};

typedef struct SPR_INFO
{
	int		nTotalFrames;
	int		nTotalDirs;
	int		nInterval;
	char	szFileName[80];
} CSPR_INFO;
// 某部件的所有装备在各种动作下的图像文件名
class CRESINFO
{
public:
	int			m_nActionKind;				// 动作种类数
	int			m_nEquipKind;				// 装备种类数
	CSPR_INFO	*m_cSprInfo;				// 某部件所有资源文件的相关信息
public:
	CRESINFO();
	~CRESINFO();
	// 清除数据
	void		AutoDelete();
	// 设定动作种类数，装备种类数，分配表格内存缓冲区
	BOOL		AutoNew(int nWidth, int nHeight);
	// 根据动作编号、装备编号得到资源文件名
	BOOL		GetName(int nActionNo, int nEquipNo, char *lpszDefault, char *lpszGetName, int nStrLen);
	int			GetInterval(int nActionNo, int nEquipNo, int nDefault);
	int			GetTotalFrames(int nActionNo, int nEquipNo, int nDefault);
	int			GetTotalDirs(int nActionNo, int nEquipNo, int nDefault);
};


// 装备、状态与动作对照表，KNpcResNode 中有两个，一个用于没骑马的，一个用于骑马的
class CEquipStyleTable
{
private:
	int			m_nStyleKind;		// 状态种类数
	int			m_nEquipKind;		// 装备种类数
	int			*m_lpnActionNo;		// 表格数据，大小为 m_nStyleKind * m_nEquipKind
public:
	CEquipStyleTable();
	~CEquipStyleTable();
	// 清除数据
	void		AutoDelete();
	// 设定表格长宽，初始化表格缓冲区
	BOOL		AutoNew(int nWidth, int nHeight);
	// 设定表格中某一项的值
	void		SetValue(int nXpos, int nYpos, int nValue);
	// 得到表格中某一项的值
	int			GetValue(int nXpos, int nYpos);
};


// 用于排序表
typedef struct SACTTABLEOFF
{
	int		nActOff;			// 此动作在顺序对应表中信息的位置
	int		nLineNum;			// 此动作在对应表中的相应信息有多少行
	BOOL	bUseDefault;		// 此动作各个方向的顺序信息是否使用默认方式
	SACTTABLEOFF() { Clear(); };
	void	Clear() { nActOff = 0; nLineNum = 0; bUseDefault = TRUE; };
} SActTableOff;

// 对排序表的处理
class CSortTable
{
private:
	SActTableOff	m_sActTableOff[MAX_ACTION];	// 每个动作在表格中的查找方式
	int				*m_lpnSortTable;			// 排序表，大小为：m_nSortTableSize * (m_nPartNum + 1)
	int				m_nSortTableSize;			// 表格有多少行
	int				m_nPartNum;					// 有多少种部件，表格每一行第一个单元表示第几桢，
												// 后面跟着 m_nPartNum 个单元，存储每个部件的编号
	
public:
	CSortTable();
	~CSortTable();
	char			m_sSortTableFileName[100]; //排序表的文件名，用于编缉器
	// 从排序文件中把排序信息读取出来，需要传入的信息有：排序文件名、各种动作的名称等信息、部件的数量
	BOOL			GetTable(char *lpszFileName, CActionName *cActionName, int nPartNum);
	BOOL			GetTable(KIniFile *pIni, CActionName *cActionName, int nPartNum);
	// 从表格中得到一组排序信息
	BOOL			GetSort(int nActNo, int nDir, int nFrameNo, int *lpnTable, int nTableLen);
private:
	// 清除数据
	void			Release();
	// 从字符串中把一系列数字取出来
	void			SortStrToNum(char *lpszStr, int *lpnTable, int nTableSize);
};
// 注：表格前面 MAX_SORT_DIR 行是所有动作在默认方式下的排序方式，当动作指明使用默认
//     方式而且不在特殊桢的情况下使用这些信息；跟着是所有动作的特殊排序信息，每一个
//     动作一个单元，每个单元分为两部分：第一部分是所有 MAX_SORT_DIR 个方向的排序信
//     息，如果此动作指明不用默认方式，则这个部分存在并且占 MAX_SORT_DIR 行；第二部
//     分是特殊桢的排序方式，特殊桢的数量由动作指明（当存在第一部分时为 nLineNum - 
//     MAX_SORT_DIR，当不存在第一部分时为 nLineNum），如果没有特殊桢，则这部分不存在

#ifndef TOOLVERSION
class KNpcResNode : public KNode
#else
class CORE_API KNpcResNode : public KNode
#endif
{
private:
	int					m_nNpcKind;				// 男主角 女主角 普通npc
	int					m_nPartNum;
	char				m_szResPath[80];		// 图形文件(spr)存放的路径
	CRESINFO			m_cResInfo[MAX_PART];	// 某部件的所有资源文件信息（包括所有资源文件名）
	CEquipStyleTable	m_NoHorseTable;
	CEquipStyleTable	m_OnHorseTable;
	char				m_szSoundName[MAX_ACTION][80];
	
public:
	CSortTable			m_cSortTable;

	struct SECTINFO
	{
		int				nFlag;					// 此部件是否存在
		char			szSectName[32];			// 此部件的部件名
		char			szSectResName[80];		// 此部件对应的资源说明文件名
		char			szSectSprInfoName[80];	// 此部件对应的资源信息说明文件
		SECTINFO() { Clear(); };
		void	Clear() { nFlag = 0; szSectName[0] = 0; szSectResName[0] = 0; szSectSprInfoName[0] = 0; };

	}		 			m_nSectInfo[MAX_PART];	// 部件信息

	CShadowFileName		m_cShadowInfo;			// 对应所有动作的阴影文件信息

	char				m_szNpcName[80];		// 玩家类型名

public:
	KNpcResNode();
	~KNpcResNode();
	//	初始化，载入所有相应数据
	BOOL			Init(char *lpszNpcName, CActionName *cActionName, CActionName *cNpcAction);
	// 判断资源类型(非拼接类型还是拼接类型)
	int				GetNpcKind() { return m_nNpcKind; };
	// 判断某个部件是否存在
	BOOL			CheckPartExist(int nPartNo);
	// 获得某个部件的某个装备在某个动作下的 spr 文件名
	BOOL			GetFileName(int nPartNo, int nActionNo, int nEquipNo, char *lpszDefault, char *lpszGetName, int nStrLen);
	// 获得某个部件的某个装备在某个动作下的 spr 文件的帧间隔
	int				GetInterval(int nPartNo, int nActionNo, int nEquipNo, int nDefault);
	// 获得某个部件的某个装备在某个动作下的 spr 文件的总帧数
	int				GetTotalFrames(int nPartNo, int nActionNo, int nEquipNo, int nDefault);
	// 获得某个部件的某个装备在某个动作下的 spr 文件的总方向数
	int				GetTotalDirs(int nPartNo, int nActionNo, int nEquipNo, int nDefault);
	// 获得某武器某状态下的动作编号
	int				GetActNo(int nDoing, int nEquipNo, BOOL bRideHorse);
	// 从表格中得到一组排序信息
	BOOL			GetSort(int nActNo, int nDir, int nFrameNo, int *lpnTable, int nTableLen);

	// 把路经和文件名合成在一起成为带路径的文件名
	void			ComposePathAndName(char *lpszGet, char *lpszPath, char *lpszName);
	// 获得某个动作对应的音效文件名
	void			GetActionSoundName(int nAction, char *lpszSoundName);

	static void		GetShadowName(char *lpszShadow, char *lpszSprName);
};
#endif