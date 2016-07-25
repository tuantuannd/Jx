/*****************************************************************************************
//	界面--聊天快捷输入短语
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-3-23
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once

#define	MAX_PHRASE_COUNT	10

class KUiChatPhrase
{
public:
	KUiChatPhrase();
	~KUiChatPhrase();
	int		GetPhrase(int nIndex, char* pszPhrase);
	int		SetPhrase(int nIndex, char* pszPhrase, int nLen);
	int		GetPhraseCount();
	int		SavePrivateSetting(KIniFile* pFile, LPCSTR lpSection, int nStart);
	void	ClearAllPhrase();

	enum EMOTE_TYPE
	{
		EMOTE_T_PHRASE = 0,
		EMOTE_T_MENU,
	};
	//载入所有聊天动作文件
	int     LoadEntireEmote();
	//设置聊天动作，有给出index的就改index的，没有就根据Cmd寻找要替换的
	int     SetEmote(char *szCmd, char *szName, char *szStringTarget, char *szStringSelf, int nIndex = -1);
	//
	int     GetEmoteCount(EMOTE_TYPE eType);
	//根据命令获取一个动作描述(pBuff)，返回描述长度，这里是包括了MENU和PHRASE的
	int     GetEmote(const char *szCmd, char *pBuff, int nBuffLen, int nType = 0);
	//根据索引获取一个动作描述(pBuff)，返回描述长度，，这里是包括了MENU和PHRASE的
	int     GetEmote(int nIndex, char *pBuff, int nBuffLen, int nType = 0);
	//获取一个用于菜单上的动作描述(pBuff)，返回描述长度，仅仅MENU
	int     GetMenuEmote(const char *szCmd, char *pBuff, int nBuffLen, int nType = 0);
	int     GetMenuEmote(int nIndex, char *pBuff, int nBuffLen, int nType = 0);
	//取出指定索引的动作名字
	int     GetMenuEmoteName(int nIndex, char *pBuff, int nBuffLen);
	//制作字符串
	int     ConvertEmoteString(char *szString, int nStringLen, const char *szMyName, const char *szTarName);

	//把读取到的Emote信息构造进去Shortcut系统中的函数别名表
	int     ConstructFunctionAlias();

private:
	struct  EMOTE_PHRASE
	{
		char    szName[16];
		int     nNameLen;
		char    szString[128];
		int     nStringLen;
		char    szStringMe[128];
		int     nStringMeLen;
		char    szCmd[64];
	} *m_pEmotePhrase, *m_pMenuEmote;
	
	int     LoadEmotePhrase(KIniFile *pIni);
	int     LoadMenuEmote(KIniFile *pIni);
	int     HandleLoad(KIniFile *pIni, EMOTE_PHRASE *pData, int nCount, const char *szSection);
	int     FindEmote(EMOTE_TYPE eType, const char *szCmd);

private:
	struct	CHAT_PHRASE
	{
		char	szString[64];
		int		nStringLen;
	}		m_PhraseList[MAX_PHRASE_COUNT];

	int m_nEmotePhraseCount;
	int m_nMenuEmoteCount;

	unsigned int m_uMyNameColor;
	unsigned int m_uTargetColor;
};

extern KUiChatPhrase	g_UiChatPhrase;