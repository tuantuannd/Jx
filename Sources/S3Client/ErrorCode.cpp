/**************************************************************************
文件    ：    ErrorCode.cpp
创建人  ：    Fyt(Fan Zhanpeng)
创建时间：    08-01-2003(mm-dd-yyyy)
功能描述：    弹出错误提示窗口
***************************************************************************/

#include "KWin32.h"
#include <windows.h>
#include "ErrorCode.h"
#include "KEngine.h"
#include "KiniFile.h"


#define ERROR_STRING		"\\UI\\错误描述.ini"
#define	NEXT_LINE_CHARA		'~'

//错误代码提示部分定义的全局变量
static unsigned int s_uErrorCode = 0;
static char			s_szErrorString[32] = "";
//错误代码提示部分的全局变量添加完了


void Error_SetErrorCode(unsigned int uCode)
{
	s_uErrorCode = uCode;
}

void Error_SetErrorString(const char* pcszString)
{
	strncpy(s_szErrorString, pcszString, sizeof(s_szErrorString));
	s_szErrorString[sizeof(s_szErrorString) - 1] = 0;
}

/**************************************************************************
*功能：弹出错误描述窗口
***************************************************************************/
void Error_Box()
{
	if(s_uErrorCode)
	{
    	KIniFile Ini;
    	char szFormatString[512], szOutputInfo[512], szBuf[16];

        //嗷，开始显示错误信息咯
	    Ini.Load(ERROR_STRING);

        Ini.GetString("Strings", itoa(s_uErrorCode,szBuf,10), "", szFormatString, sizeof(szFormatString));
		if(szFormatString[0] == 0)
		{
			Ini.GetString("Strings", "Unknown", "", szFormatString, sizeof(szFormatString));
			if(szFormatString[0] == 0)
			    strcpy(szFormatString, "未知的错误(错误号: %d)");
		}
		while(true)
		{
			char* pszNextLine = strchr(szFormatString, NEXT_LINE_CHARA);
			if (pszNextLine == NULL)
				break;
			*pszNextLine = '\n';
		}
		const char* pszCodePos = strstr(szFormatString, "%d");
		const char* pszStringPos = strstr(szFormatString, "%s");
		if (pszCodePos)
		{
			if (pszStringPos)
			{
				if (pszCodePos < pszStringPos)
					sprintf(szOutputInfo, szFormatString, s_uErrorCode, s_szErrorString);
				else
					sprintf(szOutputInfo, szFormatString, s_szErrorString, s_uErrorCode);
			}
			else
			{
				sprintf(szOutputInfo, szFormatString, s_uErrorCode);
			}
		}
		else if (pszStringPos)
		{
			sprintf(szOutputInfo, szFormatString, s_szErrorString);
		}
		else
			strcpy(szOutputInfo, szFormatString);

	    MessageBox(NULL, szOutputInfo, "剑侠情缘网络版", MB_OK | MB_ICONERROR);

	    s_uErrorCode = 0;
	}
	s_szErrorString[0] = 0;
}
