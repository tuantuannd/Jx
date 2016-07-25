#ifndef S3ImeH
#define	S3ImeH

#pragma comment ( lib, "imm32.lib" )
#include <windows.h>
#include <imm.h>
#include "kEngine.h"
#include "KStrBase.h"

const int MAX_BUFFER=1024;

class CIme{
private:
	bool m_bIme;//ime允许标志

	char m_szText[MAX_BUFFER] ;
    char* m_pszText;

	int  m_nTextCursor;
    int  m_nStrLen;

	char m_szCompStr[ MAX_PATH ];//存储转换后的串
	char m_szCompReadStr[ MAX_PATH ];//存储输入的串


	char m_szCandList[ MAX_PATH ];//存储整理成字符串选字表
	int  m_nImeCursor;//存储转换后的串中的光标位置
	

	CANDIDATELIST *m_lpCandList;//存储标准的选字表
	char m_szImeName[ 64 ];//存储输入法的名字

	bool m_bImeSharp;//全角标志
	bool m_bImeSymbol;//中文标点标志

	bool m_bInsert;//插??改写状态


	void ConvertCandList( CANDIDATELIST *pCandList, char *pszCandList );//将选字表整理成串

	bool OnWM_INPUTLANGCHANGEREQUEST();
	bool OnWM_INPUTLANGCHANGE( HWND hWnd );
	bool OnWM_IME_SETCONTEXT(){ return true; }
	bool OnWM_IME_STARTCOMPOSITION(){ return true; }
	bool OnWM_IME_ENDCOMPOSITION(){ return true; }
	bool OnWM_IME_NOTIFY( HWND hWnd, WPARAM wParam );
	bool OnWM_IME_COMPOSITION( HWND hWnd, LPARAM lParam );
	bool onWM_CHAR(WPARAM wParam);
	bool onWM_KEYDOWN(WPARAM wParam);


public:
	CIme();
	~CIme();
	//控制函数
	void DisableIme();//关闭并禁止输入法，如ime已经打开则关闭，此后玩家不能用热键呼出ime
	void EnableIme();//允许输入法，此后玩家可以用热键呼出ime
	void NextIme();//切换到下一种输入法，必须EnableIme后才有效
	void SharpIme( HWND hWnd );//切换全角/半角
	void SymbolIme( HWND hWnd );//切换中/英文标点
	//状态函数
	char* GetImeName();//得到输入法名字，如果当前是英文则返回NULL

//  void GetTextInfo(char **pszBufferStr,int *pnTextCursor);//取得当前已输入字串
//	void SetTextInfo(char *pszBufferStr,int pnTextCursor);//设置已经输入的字符串
    bool GetText(LPSTR pszBuffer,int * nMaxCount);
	int	 GetCursorPos();

	bool SetText(LPSTR pszBuffer);
	void SetCursorPos(int nCursorPos);

    bool IfInsert();//是否插入状态

	bool IfImeSharp();//是否全角
	bool IfImeSymbol();//是否中文标点

	void GetImeInput( char **pszCompStr, char **pszCompReadStr, int *pnImeCursor, char **pszCandList );
		//得到输入法状态，四个指针任意可为NULL则此状态不回返回
		//在pszCompStr中返回转换后的串

	//响应输入窗口消息，如果返回是true，则窗口函数应直接返回0，否则应传递给DefWindowProc
	int	WndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	
};
#endif







