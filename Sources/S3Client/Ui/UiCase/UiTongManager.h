/*******************************************************************************
File        : UiTongManager.h
Creator     : Fyt(Fan Zhanpeng)
create data : 08-29-2003(mm-dd-yyyy)
Description : 管理帮会的界面
********************************************************************************/

#if !defined(AFX_UITONGMANAGER_H__13BA213D_11EC_4F24_BF98_F51C3F414D6D__INCLUDED_)
#define AFX_UITONGMANAGER_H__13BA213D_11EC_4F24_BF98_F51C3F414D6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../elem/wndlabeledbutton.h"
#include "../elem/wndscrollbar.h"
#include "../elem/wndimage.h"
#include "../elem/wndlist2.h"

#include "../../Engine/Src/LinkStruct.h"
#include "../../Core/Src/GameDataDef.h"


struct KPilgarlicItem : public KTongMemberItem
{
	int nHashID;
	KPilgarlicItem *next;
    KPilgarlicItem *previous;
};


class KUiTongManager : KWndImage
{
public:
	KUiTongManager();
	virtual ~KUiTongManager();

	static        KUiTongManager* OpenWindow();      //打开窗口
	static        KUiTongManager* OpenWindow(        //打开窗口，同时安排界面布局
		                                     char* pszPlayerName);
	static        KUiTongManager* GetIfVisible();    //如果窗口正被显示，则返回实例指针
	static void   CloseWindow(bool bDestory = TRUE); //关闭窗口，同时可以选则是否删除对象实例
	static void   LoadScheme(const char* pScheme);   //载入界面方案

	                                                 //接受新的数据
	static void   NewDataArrive(KUiGameObjectWithName *ParamInfo, KTongMemberItem *pIncome);

	                                                 //接受帮会信息
	static void   TongInfoArrive(KUiPlayerRelationWithOther* Relation, KTongInfo *pTongInfo);
	static int    RemoveTempFile();                  //删除临时文件

	static void   ResponseResult(                    //对结果的响应入口
		                         KUiGameObjectWithName *pResult, int nbIsSucceed);

	
public:
	virtual int   WndProc(unsigned int uMsg, unsigned int uParam, int nParam);
	void          ArrangeComposition(                //根据传入的玩家名字，安排界面布局
		                             char* pszPlayerName);


private:
	void          Initialize();

	void          PopupConfirmWindow(                //弹出确认窗口
		                        const char* pszInfo, unsigned int uHandleID);

	static void   RequestData(int nType, int nIndex);//向Core发出数据请求
	static void   SaveNewData(                       //把新到的列表信息存到临时文件
		                        KUiGameObjectWithName *ParamInfo, KTongMemberItem *pIncome);

	static void   SaveWholeData(int nType);          //把指定类型的列表数据整个存储起来

	static int    IsPlayerExist(                     //查询特定名字的玩家是否已经存在
		                        KPilgarlicItem *MeToCmp, KLinkStruct<KPilgarlicItem> *pData);

	KLinkStruct<KPilgarlicItem>*                     //查询特定名字的玩家的数据位置
		          FindPlayer(char *pszName);

	KLinkStruct<KPilgarlicItem>*                     //查找玩家处理函数
		          HandleFind(KLinkStruct<KPilgarlicItem> *pData, char *pszName);

	void          LoadData(int nType);               //载入资料
	int           LoadDataHandler(                   //载入资料的处理函数
		                        KLinkStruct<KPilgarlicItem> *pData,
							    const char* szSection);

	void          UpdateBtnTheme(                    //更新按钮配置方案
		                        int nType, BOOL IsDissable = FALSE);

	KLinkStruct<KPilgarlicItem>*                     //返回当前所选择的列表类型所对应的数据集
		          GetCurrentSelectedData();

	KPilgarlicItem*                                  //返回所选择的会员
		          GetSelectedPlayer();

	void          UpdateTongInfoView();              //更新帮会讯息的显示
	void          UpdateTongListView();              //更新列表栏

	void          ClearTongData();                   //把帮会信息栏清空

	void          UpdateListCheckButton(int nType);  //三个列表CheckBox的管理
	void          OnAction(int nType);               //各种帮会操作的处理函数
	void          OnRefresh();                       //响应刷新按钮的操作函数

	void          ResponseLeave(                     //响应离帮后的操作
		                        KUiGameObjectWithName *pResult, int nbIsSucceed);

	void          ResponseDismiss(                   //响应踢人后的操作
		                        KUiGameObjectWithName *pResult, int nbIsSucceed);

	void          ResponseDemise(                    //响应传位后的操作
		                        KUiGameObjectWithName *pResult, int nbIsSucceed);
	
	void          ResponseAssign(                    //响应任命后的操作
		                        KUiGameObjectWithName *pResult, int nbIsSucceed);

	void          ResponseApply(                     //响应申请后的操作
		                        KUiGameObjectWithName *pResult, int nbIsSucceed);


private:
	static        KUiTongManager* ms_pSelf;

	static int    m_nElderDataIndex;
	static int    m_nMemberDataIndex;
	static int    m_nCaptainDataIndex;

private:
	enum RESULT_TYPE
	{
		RESULT_T_DISMISS,
		RESULT_T_ASSIGN,
		RESULT_T_DEMISE,
		RESULT_T_LEAVE,
		RESULT_T_PASSWORD_TO_DEMISE,
	};

private:
	KWndLabeledButton                                //踢人，任命，传位按钮
		          m_BtnDismiss, m_BtnAssign, m_BtnDemise;

	KWndLabeledButton
		          m_BtnLeave, m_BtnRecruit;          //离帮，招人按钮

	KWndLabeledButton
		          m_BtnApply, m_BtnCancel;           //申请，取消按钮

	KWndLabeledButton                                //长老、队长、成员列表
		          m_BtnElderList, m_BtnCaptainList, m_BtnMemberList;

	KWndButton    m_BtnClose, m_BtnRefresh;

	KWndText32    m_TongName, m_MasterName;          //帮会名字，老大名字(五月)
	KWndText32    m_Alignment;                       //阵营名字
	KWndText32    m_MemberCount, m_Money;            //人数，资金

	KWndList2     m_List;                            //中间的列表控件
	KWndScrollBar m_ListScroll;                      //列表的滚动条

	TONG_MEMBER_FIGURE
		          m_Relation;                        //打开这个界面的人与目标帮会的关系


private:
	KLinkStruct<KPilgarlicItem>
		          m_MemberData, m_CaptainData;       //成员列表和队长列表的数据

	KLinkStruct<KPilgarlicItem> m_ElderData;         //长老列表
	static KTongInfo            m_TongData;          //帮会的资料数据


private:
	unsigned int  m_uLastRequestTongDataTime;        //上一次请求帮会资料的时间
	unsigned int  m_uLastRequestElderListTime;       //上一次请求长老列表的时间
	unsigned int  m_uLastRequestMemberListTime;      //上一次请求帮众列表的时间
	unsigned int  m_uLastRequestCaptainListTime;     //上一次请求队长列表的时间

	int           m_nRelation;                       //下面那个玩家与所打开的帮会的关系
	int           m_nIndex;                          //与下面那个对应的玩家的Index
	char          m_szPlayerName[32];                //帮会界面打开，都有一个绑定的玩家，
	                                                 //或别人，或自己，这就是他(她)的
	                                                 //名字。另一个角度说，每一次打开界面
	                                                 //都绑定一个帮会，而帮会绑定到人

	int           m_nSelectIndex;                    //在列表上选择的项的下标
	int           m_nCurrentCheckBox;                //正在选择的CheckBox

	char          m_szDismiss[64], m_szAssign[64];   //踢人和任命的确认框的字符串
	char          m_szDemise[64], m_szLeave[64];     //传位和离开的确认框的字符串
	char          m_szJustice[16], m_szBalance[16];  //正和中的描述串
	char          m_szEvil[16];                      //邪的描述串
	char          m_szConfirm[16], m_szCancel[16];   //确认窗口上，确认和取消的文字

	char          m_szPassword[32];                  //MD5编码之后的密码串
	char          m_szTargetPlayerName[32];          //操作对象玩家的名字
};

unsigned long StringToHash(const char *pString, BOOL bIsCaseSensitive = TRUE);

#endif // !defined(AFX_UITONGMANAGER_H__13BA213D_11EC_4F24_BF98_F51C3F414D6D__INCLUDED_)
