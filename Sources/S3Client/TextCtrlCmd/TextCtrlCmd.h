/*****************************************************************************************
//	输入消息文字第一重过滤处理。
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-1-10
------------------------------------------------------------------------------------------
    把输入文字划分为聊天文字与控制命令，并处理界面、调试等控制命令。
*****************************************************************************************/

//命令前缀字符
#define		TEXT_CTRL_CMD_PREFIX	'?'
//聊天前缀字符
#define		TEXT_CTRL_CHAT_PREFIX	'/'
//频道前缀字符
#define		TEXT_CTRL_CHANNEL_PREFIX	'&'
//表情前缀字符
#define		TEXT_CTRL_EMOTE_PREFIX	'.'

BOOL TextMsgFilter(const char* pText, int nLen);
