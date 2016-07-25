/*****************************************************************************************
//	界面--消息中心面板
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-12-13
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "../Elem/WndMessageListBox.h"

int VerticalSplitTwoWindow(KWndWindow* pLeft, KWndWindow* pRight, int nAt);

struct KUiMsgParam;

#define MAX_CHANNELRESOURCE 10

struct KChannelResourceInfo
{
	char cTitle[32];
	char cShortTitle[MAX_CHANNELRESOURCE][32];
	KRColor uTextColor;
	KRColor uTextBorderColor;
	WORD nMenuPicIndex;
	WORD nMenuPicHeight;
	char cMenuText[32];
	WORD nMenuDeactivatePicIndex;
	WORD nMenuDeactivatePicHeight;
	char cMenuDeactivateText[32];
	KRColor uMenuBkColor;
	WORD nTextPicIndex;
	char cFormatName[32];	//用于频道订阅
	int nNeverClose;
	char szSoundFileName[80];
	unsigned int uSendMsgInterval;	//用于发送消息的时间间隔(毫秒)
	unsigned int uSendMsgNum;	//用于缓存待发送消息最大数目,最大10句
};

struct KChannelActivateInfo
{
	DWORD nChannelID;
	BYTE cost;
	bool bSubscribe;
	int ResourceIndex;	//-1表示无资源
	unsigned int uLastSendMsgTime;	//上次发消息的时间
	unsigned int uLeftSendMsgNum;	//已经缓存的代发消息
	unsigned int uBufferOffset;	//可用空间的起始位置
	char Buffer[1024 * 10];
};

struct KMSN_ChannelInfo
{
	KRColor uTextColorSelf;
	KRColor uTextBorderColorSelf;
	WORD nSelfTextPicIndex;
	KRColor uTextBKColorSelf;
	
	KRColor uTextFriendColor;
	KRColor uTextBorderFriendColor;
	WORD nFriendMenuPicIndex;
	WORD nFriendMenuPicHeight;
	KRColor uFriendMenuBkColor;
	WORD nFriendTextPicIndex;
	char szFriendSoundFileName[80];

	KRColor uTextColorUnknown;
	KRColor uTextBorderColorUnknown;
	WORD nStrangerMenuPicIndex;
	WORD nStrangerMenuPicHeight;
	KRColor uStrangerMenuBkColor;
	WORD nStrangerTextPicIndex;
};

class KSysMsgCentrePad : public KWndWindow
{
public:
	KWndButton				m_OpenSysButton;
	KWndButton				m_UpButton;
	KWndButton				m_DownButton;
	KWndMessageListBox		m_SysRoom;
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void ScrollBottom();
};

class KUiMsgCentrePad : public KWndWindow
{
public:
	//----界面面板统一的接口函数----
	static KUiMsgCentrePad* OpenWindow();					//打开窗口，返回唯一的一个类对象实例
	static void				CloseWindow(bool bDestroy);		//关闭窗口，同时可以选则是否删除对象实例
	static void				Clear();
	static void				LoadScheme(const char* pScheme);//载入界面方案
	static void				DefaultScheme(const char* pScheme);//重新初始化界面
	static void				HideAllMessage();//隐藏所有输出
	static void				ShowAllMessage();//显示所有输出
	
	static void				SystemMessageArrival(const char* pMsgBuff, unsigned short nMsgLength);

	//用于新聊天协议到达的处理
	static int				NewChannelMessageArrival(DWORD nChannelID, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength);
	static void				NewMSNMessageArrival(char* szSourceName, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength);
	static void				OpenChannel(char* channelName, DWORD nChannelID, BYTE cost);	//同时订阅
	//用于外挂的聊天
	static void				ShowSomeoneMessage(char* szSourceName, const char* pMsgBuff, unsigned short nMsgLength);
	enum  SelfChannel
	{
		ch_Team = 0,
		ch_Faction,
		ch_Tong,
		ch_Screen,
		ch_GM
	};
	static void				CloseSelfChannel(SelfChannel type);	//同时退订

	static void				QueryAllChannel();
	static void				ReplaceChannelName(char* szDest, size_t nDestSize, char* szSrc);
	static bool				GetChannelMenuinfo(int nChannelIndex, WORD* pnPicIndex = NULL, WORD* pPicHei = NULL, KRColor* puTextColor = NULL, KRColor* puBkColor = NULL, char* pszMenuText = NULL, short* pnCheckPicIndex = NULL);
	static bool				GetPeopleMenuinfo(char* szDest, WORD* pnPicIndex = NULL, WORD* pPicHei = NULL, KRColor* puTextColor = NULL, KRColor* puBkColor = NULL);
	static bool				ReplaceSpecialField(char* szDest, char* szSrc);
	static DWORD			GetChannelID(int nChannelIndex);
	static char*			GetChannelTitle(int nChannelIndex);
	static DWORD			GetChannelCount();
	static int				GetChannelIndex(char* channelName);
	static int				GetChannelIndex(DWORD dwID);
	static int				ReleaseActivateChannelAll();	//返回原来的总个数
	static bool				GetChannelSubscribe(int nChannelIndex);
	static bool				IsChannelType(int nChannelIndex, SelfChannel type);
	static int				CheckChannel(int nChannelIndex, bool b);
	static KUiMsgCentrePad* GetSelf()	{return m_pSelf;}
	static void SetFontSize(int nFontSize);
	static int GetFontSize();
	static int SetChannelTextColor(char* cTitle, DWORD uTextColor, DWORD uTextBorderColor);
	static char* GetChannelTextColor(int nIndex, DWORD& uTextColor, DWORD& uTextBorderColor);
	static void SetMSNTextColor(int nType, DWORD uTextColor, DWORD uTextBorderColor);	//nType 0 is me, 1 is friend, 2 is stranger
	static void GetMSNTextColor(int nType, DWORD& uTextColor, DWORD& uTextBorderColor);
	static BYTE	GetChannelCost(DWORD dwID);
	static int	PushChannelData(DWORD dwID, const char* Buffer, int nLen);
	static int	GetChannelData(DWORD& dwID, BYTE& cost, char*& Buffer, int& nLen);
	static int	PopChannelData(DWORD dwID);
	
	void	Breathe();								//活动函数
private:
	KUiMsgCentrePad();
	~KUiMsgCentrePad()
	{
		ReleaseActivateChannelAll();
	}
	void	Initialize();							//初始化
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	int		ChanageHeight(int nOffsetY, bool bAtTop);
	void	LoadScheme(KIniFile* pIni);
	int		PtInWindow(int x, int y);				//判断一个点是否落在窗口内,传入的是绝对坐标
	void	SetAutoDelMsgInterval(unsigned int uInterval = 0);	//设置自动删除消息的时间间隔
				//参数默认值为0，表示不按照时间间隔自动删除，时间单位毫秒
	void	DragWndSize(int nMoveOffset);
	void	DragWndPos(int nMoveOffset);
	void	PaintWindow();							//绘制窗口

	int		FindActivateChannelResourceIndex(char* cTitle);
	int		FindActivateChannelIndex(char* cTitle);
	int		FindActivateChannelIndex(DWORD nChannelID);
	BYTE	FindActivateChannelCost(DWORD nChannelID);
	int		FindChannelResourceIndex(char* cTitle);
	int		FindActivateChannelIndexByKey(char* cKey);
	int		IsNeverCloseActivateChannel(int nChannelIndex);
	void	ChannelMessageArrival(int nChannelIndex, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength, KWndMessageListBox* pM, bool bName);
	void	MSNMessageArrival(char* szSourceName, char* szSendName, const char* pMsgBuff, unsigned short nMsgLength);
	void	ShowMSNMessage(char* szName, const char* pMsgBuff, unsigned short nMsgLength, KRColor uColor, KRColor uBKColor, KRColor uBorderColor, WORD nPic);
	void	SendQueryChannelID(int nChannelResourceIndex);
	void	SendChannelSubscribe(int nChannelIndex, bool b);
	int		AddActivateChannel(const KChannelActivateInfo& Item);	//返回增加Item的Index
	int		ReleaseActivateChannel(int nChannelIndex);	//返回剩余的总个数
	void	CloseActivateChannel(int nChannelIndex);	//同时退订

	void	PopupChannelMenu(int x, int y);
	
	int		FilterTextColor(char* pMsgBuff, unsigned short nMsgLength, const KRColor& uColor);
	
	int		GetMessageSendName(KWndWindow* pWnd, int nIndex, char* szCurText);
	int		GetMessageSendName(KWndWindow* pWnd, int x, int y, char* szCurText);

private:
	static KUiMsgCentrePad*	m_pSelf;

	KWndImage			m_BorderImg;
	KWndImage			m_SizeBtn;
	KWndImage			m_MoveImg;
	KWndButton          m_BgShadowBtn;

	KSysMsgCentrePad	m_Sys;

	int					m_nMinTopPos, m_nMaxBottomPos;
	bool				m_bSizingWnd;
	bool				m_bDockLeftSide;
	bool                m_bShowShadow;

	short				m_nSizeBtnDragOffsetY;
	
	bool				m_bSizeUp;

#define SECOND_AUTODELMSG 20000
	unsigned int	m_uAutoDelMsgInterval;	//自动删除消息的间隔时间，单位毫秒
	unsigned int	m_uLastDelMsgTime;		//上次删除消息时的时间


	KScrollMessageListBox m_ChatRoom;

	KChannelResourceInfo m_ChannelsResource[MAX_CHANNELRESOURCE];
	int m_nChannelsResource;
	char m_DefaultChannelSendName[32];
	int m_nDefaultChannelResource;
	KChannelActivateInfo* m_pActivateChannel;
	int m_nActivateChannels;
	KMSN_ChannelInfo m_MSNInfo;
	short m_nCheckOnPicIndex;
	short m_nCheckOffPicIndex;

	KRColor m_NameColor;
	KRColor m_BorderNameColor;

	static int ms_DefaultHeight, ms_DefaultWidth;
};
