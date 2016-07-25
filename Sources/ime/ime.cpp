#include "Ime.h"

CIme::CIme():m_lpCandList(NULL){
	DisableIme(); ;//通过DisableIme初始化一些数据
	m_pszText=m_szText;
    memset(m_pszText,0,MAX_BUFFER);
	m_nTextCursor=0;
	m_nStrLen=0;
}
CIme::~CIme(){
	DisableIme();
	if( m_lpCandList ){
		GlobalFree( (HANDLE)m_lpCandList );
		m_lpCandList = NULL;
	}
}

void CIme::SetCursorPos(int nCursorPos){
	if (nCursorPos<0) 
		m_nTextCursor=0;
	else if (nCursorPos>m_nStrLen) 
	    m_nTextCursor=m_nStrLen;
	else
		m_nTextCursor=nCursorPos;
}

bool CIme::SetText(LPSTR pszBuffer)
{
	if(pszBuffer)
	{
		m_nStrLen=g_StrLen(pszBuffer);
		if (m_nStrLen <= MAX_BUFFER)
		{
			memcpy(m_pszText,pszBuffer,m_nStrLen);
			m_nTextCursor=m_nStrLen;
			return true;
		}
	};
	return false;
} 

bool CIme::GetText(LPSTR pszBuffer,int * MaxCount)
{
	if (*MaxCount > m_nStrLen && pszBuffer)
	{
		memcpy(pszBuffer, m_pszText, m_nStrLen);
		pszBuffer[m_nStrLen] = 0;
		return true;
	}
	else
	{
		*MaxCount = m_nStrLen + 1;
		return false;
	}
};

int CIme::GetCursorPos()
{
	return m_nTextCursor;
}

void CIme::DisableIme()
{
	while( ImmIsIME( GetKeyboardLayout( 0 )))
		ActivateKeyboardLayout(( HKL )HKL_NEXT, 0 );//如果ime打开通过循环切换到下一个关闭
	m_bIme = false;
	m_szImeName[ 0 ] = 0;
	m_szCompStr[ 0 ] = 0;
	m_szCompReadStr[ 0 ] = 0;
	m_szCandList[ 0 ] = 0;
	m_nImeCursor = 0;
}

void CIme::EnableIme()
{
	m_bIme = true;
}

void CIme::NextIme(){
	if( m_bIme )ActivateKeyboardLayout(( HKL )HKL_NEXT, 0 );
}
 
void CIme::SharpIme( HWND hWnd ){
	ImmSimulateHotKey( hWnd, IME_CHOTKEY_SHAPE_TOGGLE );
}

void CIme::SymbolIme( HWND hWnd ){
	ImmSimulateHotKey( hWnd, IME_CHOTKEY_SYMBOL_TOGGLE );
}



void CIme::ConvertCandList( CANDIDATELIST *pCandList, char *pszCandList ){//转换CandidateList到一个串，\t分隔每一项
	unsigned int i;
	if( pCandList->dwCount < pCandList->dwSelection ){
		pszCandList[ 0 ] = 0;
		return;
	}
		//待选字序号超出总数，微软拼音第二次到选字表最后一页后再按PageDown会出现这种情况，并且会退出选字状态，开始一个新的输入
		//但微软拼音自己的ime窗口可以解决这个问题，估计微软拼音实现了更多的接口，所以使用了这种不太标准的数据
		//我现在无法解决这个问题，而且实际使用中也很少遇到这种事，而且其它标准输入法不会引起这种bug
		//非标准输入法估计实现的接口比较少，所以应该也不会引起这种bug
	for( i = 0; ( i < pCandList->dwCount - pCandList->dwSelection )&&( i < pCandList->dwPageSize ); i++ ){
		*pszCandList++ = ( i % 10 != 9 )? i % 10 + '1' : '0';//每项对应的数字键
		*pszCandList++ = '.';//用'.'分隔
		strcpy( pszCandList, (char*)pCandList
			+ pCandList->dwOffset[ pCandList->dwSelection + i ] );//每项实际的内容
		pszCandList += strlen( pszCandList );
		*pszCandList++ = '\t';//项之间以'\t'分隔
	}
	*( pszCandList - 1 )= 0;//串尾，并覆盖最后一个'\t'
}
bool CIme::onWM_KEYDOWN(WPARAM wParam){
	switch(wParam){
		case VK_LEFT:
		{
			m_nTextCursor--;
			if (m_nTextCursor<0) m_nTextCursor=0;
			if (*(BYTE*)(m_pszText+m_nTextCursor)>=0xA1){
			m_nTextCursor--;
			if (m_nTextCursor<0) m_nTextCursor=0;
			}
			break;
		}
		case VK_RIGHT:
		{
			m_nTextCursor++;
			if (m_nTextCursor>m_nStrLen) m_nTextCursor=m_nStrLen;
			if (*(BYTE*)(m_pszText+m_nTextCursor)>=0xA1){
			m_nTextCursor++;
			if (m_nTextCursor>m_nStrLen) m_nTextCursor=m_nStrLen;
			}
			break;
		}
		case VK_DELETE:
		{
			
			break;
		}
		case VK_INSERT:
		{
			m_bInsert=!m_bInsert;
			break;
		}
	};
	return true;
}

bool CIme::onWM_CHAR(WPARAM wParam){

    switch(wParam){
	case VK_BACK:
		{
            if (m_nTextCursor==0) break;
			int OldCursor;
			OldCursor=m_nTextCursor;
			m_nStrLen--;
			if (m_nStrLen<0) m_nStrLen=0;
			m_nTextCursor--;
			if (m_nTextCursor<0) m_nTextCursor=0;

			if (*(BYTE*)(m_pszText+m_nTextCursor)>=0xA1)
			{
				m_nTextCursor--;
				if (m_nTextCursor<0) m_nTextCursor=0;
				m_nStrLen--;
				if (m_nStrLen<0) m_nStrLen=0;
			};
			if((m_nStrLen-m_nTextCursor)==0)
			{
			 *(m_pszText+m_nTextCursor)=0;
			}
			else
			{
			 memcpy(m_pszText+m_nTextCursor,m_pszText+OldCursor,m_nStrLen-m_nTextCursor);	
             *(m_pszText+m_nStrLen)=0 ;
			};
		break;
		};
	default:
		{
		if (m_bInsert){
		if ((m_nStrLen-m_nTextCursor)>0){
			memcpy(m_pszText+m_nTextCursor+1,m_pszText+m_nTextCursor,m_nStrLen-m_nTextCursor);
			};
		};
		*(m_pszText+m_nTextCursor++) = (char)wParam;
		m_nStrLen++;
		}
    }
    return TRUE;
}


int	CIme::WndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int	nRet = 0;
	switch(uMsg)
	{
	case WM_KEYDOWN:
		nRet=onWM_KEYDOWN(wParam);
		break;
	case WM_CHAR:
		nRet = onWM_CHAR(wParam);
		break;
   	case WM_IME_SETCONTEXT:
		nRet = OnWM_IME_SETCONTEXT();
		break;
	case WM_INPUTLANGCHANGEREQUEST:
		nRet = OnWM_INPUTLANGCHANGEREQUEST();
		break;
	case WM_INPUTLANGCHANGE:
		nRet = OnWM_INPUTLANGCHANGE(hWnd);
		break;
	case WM_IME_STARTCOMPOSITION:
		nRet = OnWM_IME_STARTCOMPOSITION();
		break;
	case WM_IME_ENDCOMPOSITION:
		nRet =OnWM_IME_ENDCOMPOSITION();
		break;
	case WM_IME_NOTIFY:
		nRet = OnWM_IME_NOTIFY( hWnd, wParam );
		break;
	case WM_IME_COMPOSITION:
		nRet = OnWM_IME_COMPOSITION( hWnd, lParam );
		break;
	}
	return nRet;
}

bool CIme::OnWM_INPUTLANGCHANGEREQUEST(){
	return !m_bIme;//如果禁止ime则返回false，此时窗口函数应返回0，否则DefWindowProc会打开输入法		HIMC hIMC = ImmGetContext( hWnd );
}

bool CIme::OnWM_INPUTLANGCHANGE( HWND hWnd ){
	//ime改变
	HKL hKL = GetKeyboardLayout( 0 );
	if( ImmIsIME( hKL )){
		HIMC hIMC = ImmGetContext( hWnd );
	ImmEscape( hKL, hIMC, IME_ESC_IME_NAME, m_szImeName );//取得新输入法名字
		DWORD dwConversion, dwSentence;
		ImmGetConversionStatus( hIMC, &dwConversion, &dwSentence );
		m_bImeSharp = ( dwConversion & IME_CMODE_FULLSHAPE )? true : false;//取得全角标志
		m_bImeSymbol = ( dwConversion & IME_CMODE_SYMBOL )? true : false;//取得中文标点标志
		ImmReleaseContext( hWnd, hIMC );
	}
	else//英文输入
		m_szImeName[ 0 ] = 0;
	return false;//总是返回false，因为需要窗口函数调用DefWindowProc继续处理
}
bool CIme::OnWM_IME_NOTIFY( HWND hWnd, WPARAM wParam ){
	HIMC hIMC;
	DWORD dwSize;
	DWORD dwConversion, dwSentence;
	switch( wParam ){
		case IMN_SETCONVERSIONMODE://全角/半角，中/英文标点改变
			hIMC = ImmGetContext( hWnd );
			ImmGetConversionStatus( hIMC, &dwConversion, &dwSentence );
			m_bImeSharp = ( dwConversion & IME_CMODE_FULLSHAPE )? true : false;
			m_bImeSymbol = ( dwConversion & IME_CMODE_SYMBOL )? true : false;
			ImmReleaseContext( hWnd, hIMC );
			break;
		case IMN_OPENCANDIDATE://进入选字状态
		case IMN_CHANGECANDIDATE://选字表翻页
			hIMC = ImmGetContext( hWnd );
			if( m_lpCandList ){
				GlobalFree( (HANDLE)m_lpCandList );
				m_lpCandList = NULL;
			}//释放以前的选字表
			if( dwSize = ImmGetCandidateList( hIMC, 0, NULL, 0 )){
				m_lpCandList = (LPCANDIDATELIST)GlobalAlloc( GPTR, dwSize );
				if( m_lpCandList )
					ImmGetCandidateList( hIMC, 0, m_lpCandList, dwSize );
			}//得到新的选字表
			ImmReleaseContext( hWnd, hIMC );
			if( m_lpCandList )ConvertCandList( m_lpCandList, m_szCandList );//选字表整理成串
			break;
		case IMN_CLOSECANDIDATE://关闭选字表
			if( m_lpCandList ){
				GlobalFree( (HANDLE)m_lpCandList );
				m_lpCandList = NULL;
			}//释放
			m_szCandList[ 0 ] = 0;
			break;
	}
	return true;//总是返回true，防止ime窗口打开
}
bool CIme::OnWM_IME_COMPOSITION( HWND hWnd, LPARAM lParam ){//输入改变
	HIMC hIMC;
	DWORD dwSize;
	hIMC = ImmGetContext( hWnd );
	if( lParam & GCS_COMPSTR ){
		dwSize = ImmGetCompositionString( hIMC, GCS_COMPSTR, (void*)m_szCompStr, sizeof( m_szCompStr ));
		m_szCompStr[ dwSize ] = 0;
	}//取得szCompStr
	if( lParam & GCS_COMPREADSTR ){
		dwSize = ImmGetCompositionString( hIMC, GCS_COMPREADSTR, (void*)m_szCompReadStr, sizeof( m_szCompReadStr ));
		m_szCompReadStr[ dwSize ] = 0;
	}//取得szCompReadStr
	if( lParam & GCS_CURSORPOS ){
		m_nImeCursor = 0xffff & ImmGetCompositionString( hIMC, GCS_CURSORPOS, NULL, 0 );
	}//?〉?.nImeCursor
	if( lParam & GCS_RESULTSTR ){
		unsigned char str[ MAX_PATH ];
		dwSize = ImmGetCompositionString( hIMC, GCS_RESULTSTR, (void*)str, sizeof( str ));//取得汉字输入串
		str[ dwSize ] = 0;
		unsigned char *p = str;
		while( *p )PostMessage( hWnd, WM_CHAR, (WPARAM)(*p++), 1 );//转成WM_CHAR消息
	}
	ImmReleaseContext( hWnd, hIMC );
	return true;//总是返回true，防止ime窗口打开
}

char* CIme::GetImeName(){
	return m_szImeName[ 0 ]? m_szImeName : NULL;
}


bool CIme::IfImeSharp(){//是否全角
	return m_bImeSharp;
}

bool CIme::IfImeSymbol(){//是否中文标点
	return m_bImeSymbol;
}

bool CIme::IfInsert(){
	return m_bInsert;
}

void CIme::GetImeInput( char **pszCompStr, char **pszCompReadStr, int *pnImeCursor, char **pszCandList ){
	if( pszCompStr )*pszCompStr = m_szCompStr;
	if( pszCompReadStr )*pszCompReadStr = m_szCompReadStr;
	if( pnImeCursor )*pnImeCursor = m_nImeCursor;
	if( pszCandList )*pszCandList = m_szCandList;
}