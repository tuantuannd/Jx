// ClientClone.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>

#include "..\..\Sword3PaySys\S3AccServer\AccountLoginDef.h"
#include "..\..\s3client\login\LoginDef.h"

#include "..\..\RoleDBManager\kroledbheader.h"
#include "S3DBInterface.h"
#include "KProtocolDef.h"
#include "KProtocol.h"

#include "IClient.h"
#include "RainbowInterface.h"

#include "Macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"
#include "Library.h"
#include "Console.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::Console::setcolor;
using OnlineGameLib::Win32::Console::gotoxy;
using OnlineGameLib::Win32::Console::getxy;

const size_t g_nTitleSpace = 3;
const size_t g_nBufferSize = 256;
_tstring g_sServerIPAddress;
unsigned short g_nServerPort = 5622;

_tstring g_sAccount, g_sPassword, g_sRoleName;

CLibrary g_theRainbowLibrary( "rainbow.dll" );

IClient *pClient = NULL;
IClient *pGameSvrClient = NULL;

#define CONFIG_FILENAME			_T( "config.ini" )
#define SVR_SETTING_FILENAME	_T( "settings\\ServerList.ini" )

#include <iostream>
#include <strstream>

using namespace std;

/*
 * Using directives
 */
using std::auto_ptr;
using std::endl;

typedef HRESULT ( __stdcall * pfnCreateClientInterface )(
			REFIID	riid,
			void	**ppv
		);

void __stdcall ClientEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:

		cout << "Connection was created!" << endl;
		break;

	case enumServerConnectClose:

		cout << "Connection was destroy!" << endl;
		break;
	}
}

/*
 * Global variable
 */
_tstring	g_sAppFullPath;

/*
 * Helper function
 */
void ShowTitle( const char *pTile );

void SelectServerList();
void InputAccountAndPassword();
UINT Login();
int SelectRoleList();
void WaitForEnterGameServer();
void ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid );

void PlayGame();
void InitGameWorld();
void ConnectServer();
void DisconnectServer();

int	g_nProtocolSize[MAX_PROTOCOL_NUM] = 
{
	-1,							// s2c_login,
	-1,							// s2c_logout,
	sizeof(BYTE),				// s2c_syncend,
	sizeof(CURPLAYER_SYNC),		// s2c_synccurplayer,
	-1,							// s2c_synccurplayerskill
	sizeof(CURPLAYER_NORMAL_SYNC),// s2c_synccurplayernormal
	-1,							// s2c_newplayer,
	-1,							// s2c_removeplayer,
	sizeof(WORLD_SYNC),			// s2c_syncworld,
	sizeof(PLAYER_SYNC),		// s2c_syncplayer,
	sizeof(PLAYER_NORMAL_SYNC),	// s2c_syncplayermin,
	-1,	//sizeof(NPC_SYNC),			// s2c_syncnpc,
	sizeof(NPC_NORMAL_SYNC),	// s2c_syncnpcmin,
	sizeof(NPC_PLAYER_TYPE_NORMAL_SYNC),	// s2c_syncnpcminplayer,

	sizeof(OBJ_SYNC_ADD),		// s2c_objadd,
	sizeof(OBJ_SYNC_STATE),		// s2c_syncobjstate,
	sizeof(OBJ_SYNC_DIR),		// s2c_syncobjdir,
	sizeof(OBJ_SYNC_REMOVE),	// s2c_objremove,
	sizeof(OBJ_SYNC_TRAP_ACT),	// s2c_objTrapAct,

	sizeof(NPC_REMOVE_SYNC),	// s2c_npcremove,
	sizeof(NPC_WALK_SYNC),		// s2c_npcwalk,
	sizeof(NPC_RUN_SYNC),		// s2c_npcrun,
	-1,							// s2c_npcattack,
	-1,							// s2c_npcmagic,
	sizeof(NPC_JUMP_SYNC),		// s2c_npcjump,
	-1,							// s2c_npctalk,
	sizeof(NPC_HURT_SYNC),		// s2c_npchurt,
	sizeof(NPC_DEATH_SYNC),		// s2c_npcdeath,
	sizeof(NPC_CHGCURCAMP_SYNC),// s2c_npcchgcurcamp,
	sizeof(NPC_CHGCAMP_SYNC),	// s2c_npcchgcamp,
	sizeof(NPC_SKILL_SYNC),		// s2c_skillcast,
	-1,							// s2c_playertalk,
	sizeof(PLAYER_EXP_SYNC),	// s2c_playerexp,

	sizeof(PLAYER_SEND_TEAM_INFO),			// s2c_teaminfo,
	sizeof(PLAYER_SEND_SELF_TEAM_INFO),		// s2c_teamselfinfo,
	sizeof(PLAYER_APPLY_TEAM_INFO_FALSE),	// s2c_teamapplyinfofalse,
	sizeof(PLAYER_SEND_CREATE_TEAM_SUCCESS),// s2c_teamcreatesuccess,
	sizeof(PLAYER_SEND_CREATE_TEAM_FALSE),	// s2c_teamcreatefalse,
	sizeof(PLAYER_TEAM_OPEN_CLOSE),			// s2c_teamopenclose,
	sizeof(PLAYER_APPLY_ADD_TEAM),			// s2c_teamgetapply,
	sizeof(PLAYER_TEAM_ADD_MEMBER),			// s2c_teamaddmember,
	sizeof(PLAYER_LEAVE_TEAM),				// s2c_teamleave,
	sizeof(PLAYER_TEAM_CHANGE_CAPTAIN),		// s2c_teamchangecaptain,
	sizeof(PLAYER_FACTION_DATA),			// s2c_playerfactiondata,
	sizeof(PLAYER_LEAVE_FACTION),			// s2c_playerleavefaction,
	sizeof(PLAYER_FACTION_SKILL_LEVEL),		// s2c_playerfactionskilllevel,
	-1,//sizeof(PLAYER_SEND_CHAT_SYNC),			// s2c_playersendchat,
	sizeof(PLAYER_LEAD_EXP_SYNC),			// s2c_playersyncleadexp
	sizeof(PLAYER_LEVEL_UP_SYNC),			// s2c_playerlevelup
	sizeof(PLAYER_TEAMMATE_LEVEL_SYNC),		// s2c_teammatelevel
	sizeof(PLAYER_ATTRIBUTE_SYNC),			// s2c_playersyncattribute
	sizeof(PLAYER_SKILL_LEVEL_SYNC),		// s2c_playerskilllevel
	sizeof(ITEM_SYNC),						// s2c_syncitem
	sizeof(ITEM_REMOVE_SYNC),				// s2c_removeitem
	sizeof(PLAYER_MONEY_SYNC),				// s2c_syncmoney
	sizeof(PLAYER_MOVE_ITEM_SYNC),			// s2c_playermoveitem
	-1,										// s2c_playershowui
	sizeof(CHAT_APPLY_ADD_FRIEND_SYNC),		// s2c_chatapplyaddfriend
	sizeof(CHAT_ADD_FRIEND_SYNC),			// s2c_chataddfriend
	-1,//sizeof(CHAT_REFUSE_FRIEND_SYNC),		// s2c_chatrefusefriend
	sizeof(CHAT_ADD_FRIEND_FAIL_SYNC),		// s2c_chataddfriendfail
	sizeof(CHAT_LOGIN_FRIEND_NONAME_SYNC),	// s2c_chatloginfriendnoname
	-1,//sizeof(CHAT_LOGIN_FRIEND_NAME_SYNC),	// s2c_chatloginfriendname
	sizeof(CHAT_ONE_FRIEND_DATA_SYNC),		// s2c_chatonefrienddata
	sizeof(CHAT_FRIEND_ONLINE_SYNC),		// s2c_chatfriendinline
	sizeof(CHAT_DELETE_FRIEND_SYNC),		// s2c_chatdeletefriend
	sizeof(CHAT_FRIEND_OFFLINE_SYNC),		// s2c_chatfriendoffline
	sizeof(ROLE_LIST_SYNC),					// s2c_syncrolelist
	sizeof(TRADE_CHANGE_STATE_SYNC),		// s2c_tradechangestate
	-1, // NPC_SET_MENU_STATE_SYNC			   s2c_npcsetmenustate
	sizeof(TRADE_MONEY_SYNC),				// s2c_trademoneysync
	sizeof(TRADE_DECISION_SYNC),			// s2c_tradedecision
	-1, // sizeof(CHAT_SCREENSINGLE_ERROR_SYNC)s2c_chatscreensingleerror
	sizeof(NPC_SYNC_STATEINFO),				// s2c_syncnpcstate,
	-1,	// sizeof(TEAM_INVITE_ADD_SYNC)		   s2c_teaminviteadd
	sizeof(TRADE_STATE_SYNC),				// s2c_tradepressoksync
	sizeof(PING_COMMAND),					// s2c_ping
	sizeof(NPC_SIT_SYNC),					// s2c_npcsit
	sizeof(SALE_BOX_SYNC),					// s2c_opensalebox
	sizeof(NPC_SKILL_SYNC),					// s2cDirectlyCastSkill
	-1,										// s2c_msgshow
	-1,										// s2c_syncstateeffect
	sizeof(BYTE),							// s2c_openstorebox
	sizeof(NPC_REVIVE_SYNC),				// s2c_playerrevive
	sizeof(NPC_REQUEST_FAIL),				// s2c_requestnpcfail
	sizeof(TRADE_APPLY_START_SYNC),			// s2c_tradeapplystart
	sizeof(tagNewDelRoleResponse),			// s3c_rolenewdelresponse
};

int main(int argc, char* argv[])
{
	ShowTitle( "Welcome to JX.MMOG client" );

	g_sAppFullPath = GetAppFullPath( NULL );

	cout << "The current directory : " 
		<< endl 
		<< _strlwr( const_cast< char * >( g_sAppFullPath.c_str() ) )
		<< endl;

	SelectServerList();

	ConnectServer();

_LOGIN_ROUTE:

	InputAccountAndPassword();

	switch ( Login() )
	{
	case LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR:
	case LOGIN_R_TIMEOUT:
	case LOGIN_R_ACCOUNT_EXIST:
	case LOGIN_R_FAILED:

		goto _LOGIN_ROUTE;

		break;

	case LOGIN_R_SUCCESS:
		break;
	}

	int nRoleIndex = SelectRoleList();

	WaitForEnterGameServer();

	InitGameWorld();

	PlayGame();

	DisconnectServer();
	
	return 0;
}

void PlayGame()
{
	ShowTitle( "Enter the game" );

	cout << "Input 'Exit' or 'exit' to quit." << endl;

	_tstring sCmd;

	while ( sCmd.compare( "exit" ) && sCmd.compare( "Exit" ) )
	{
		cin >> sCmd;
	}
}

void InitGameWorld()
{
	bool ok = false;

	while ( !ok )
	{
		size_t datalength = 0;	
		const char *pData = ( const char * )pGameSvrClient->GetPackFromServer( datalength );

		const char *pMsg = pData;

		while ( pMsg < ( pData + datalength ) )
		{
			BYTE cProtocol = *( BYTE * )pMsg;

			ASSERT( cProtocol > s2c_clientbegin && cProtocol < s2c_end );

			int nProtocolSize = g_nProtocolSize[ cProtocol - s2c_clientbegin - 1 ];			

			cout << nProtocolSize << "; ";

			switch ( cProtocol )
			{
			case s2c_syncend:
				{
					BYTE cData = c2s_syncend;
					pGameSvrClient->SendPackToServer( ( const void * )&cData, sizeof( BYTE ) );

					cout << endl << "Init game finish!" << endl;

					ok = true;
				}
				break;
			}

			if ( ok )
			{
				break;
			}
			
			/*
			 * Get next protocol
			 */
			if ( nProtocolSize >= 0 )
			{
				pMsg += nProtocolSize;
			}
			else
			{
				unsigned short usMsgSize = *( unsigned short * )( pMsg + sizeof( BYTE ) );

				pMsg += sizeof( BYTE ) + usMsgSize;
			}
		}

		::Sleep( 1 );
	}
}

void ShowTitle( const char *pTile )
{
	ASSERT( pTile );

	setcolor( enumBlackonGray );
	
	int count = strlen( pTile );
	int size = 1;

	_tstring sLine;

	sLine = '+';

	sLine.resize( g_nTitleSpace * 2 + count + size, '-' );
	sLine += '+';
	
	_tstring sTitle;
	sTitle = '|';
	sTitle.resize( g_nTitleSpace + size, ' ' );
	sTitle += pTile;
	size = sTitle.size();
	sTitle.resize( g_nTitleSpace + size, ' ' );
	sTitle += '|';

	/*
	 * Show some information for user
	 */
	cout << sLine << endl;
	cout << sTitle << endl;
	cout << sLine << endl;

	setcolor( enumDefault );
}

void SelectServerList()
{

_BEGIN_SELECT_SERVER_LIST:

	ShowTitle( "Select server" );

	/*
	 * Get setup infomation from ini-file
	 */
	_tstring sConfigFile, buffer, key, result;

	sConfigFile = g_sAppFullPath + SVR_SETTING_FILENAME;

	CIniFile iniFile( sConfigFile.c_str() );

	key.resize( g_nBufferSize );
	buffer.resize( g_nBufferSize );

	size_t nSvrCount = iniFile.ReadInteger( "List", "Count", 0 );

	setcolor( enumYellowonBlack );

	int i = 0;
	for ( ; i<nSvrCount; i++ )
	{
		sprintf( const_cast< char * >( key.c_str() ), "%d", i );

		iniFile.ReadString( key.c_str(), "Title", const_cast< char * >( buffer.c_str() ), g_nBufferSize, "" );

		result = buffer.c_str();
		result += _T(" [");

		iniFile.ReadString( key.c_str(), "Address", const_cast< char * >( buffer.c_str() ), g_nBufferSize, "" );

		result += buffer.c_str();
		result += "]";

		cout << i << ") " << result << endl;
	}

	cout << i << ") " << "Exit game" << endl;

	setcolor( enumDefault );

	cout << "What are you select? Press number to be continue!" << endl;

	int nSelRoleIndex = 0;

	while ( true )
	{
		cin >> buffer;

		nSelRoleIndex = atoi( buffer.c_str() );

		/*
		 * The last one is option that you can exit application
		 */
		if ( nSelRoleIndex >= 0 && nSelRoleIndex < nSvrCount + 1 )
		{
			break;
		}

		cout << "Input incorrect, please input again." << endl;
	}

	/*
	 * Exit game if you select the last option
	 */
	if ( nSelRoleIndex == nSvrCount )
	{
		cout << "Are you exit game? < Y : yes>";

		_tstring sInformation;

		cin >> sInformation;

		if ( 0 == sInformation.compare( "Y" ) || 0 == sInformation.compare( "y" ) )
		{
			exit( 0 );
		}

		/*
		 * Repeat this route
		 */
		goto _BEGIN_SELECT_SERVER_LIST;
	}

	/*
	 * Read information by you select
	 */
	sprintf( const_cast< char * >( key.c_str() ), "%d", nSelRoleIndex );
	
	iniFile.ReadString( key.c_str(), "Title", const_cast< char * >( buffer.c_str() ), g_nBufferSize, "" );
	
	result = buffer.c_str();
	result += _T(" [");
	
	iniFile.ReadString( key.c_str(), "Address", const_cast< char * >( buffer.c_str() ), g_nBufferSize, "" );

	g_sServerIPAddress = buffer.c_str();
	
	result += buffer.c_str();

/*	sConfigFile = g_sAppFullPath + CONFIG_FILENAME;
	
	iniFile.SetFile( sConfigFile.c_str() );

	g_nServerPort = iniFile.ReadInteger( "Server", "GameServPort", g_nServerPort );
*/
	result += _T( ":" ) + ToString(g_nServerPort) + _T( "]" );

	cout << "Your select " << result << endl;
	//cout << "Please wait for a moment..." << endl;

}

UINT Login()
{
	_tstring buffer;

	UINT nRetValue = ( UINT )( -1 );

	/*
	 * Send login command to gateway
	 */
	KLoginAccountInfo lai;

	ZeroMemory( &lai, sizeof( KLoginAccountInfo ) );

	/*
	 * Account
	 */
	size_t used = sizeof( lai.Account );
	used = ( used > g_sAccount.length() ) ? g_sAccount.length() : used;

	memcpy( lai.Account, g_sAccount.c_str(), used );

	/*
	 * Password
	 */
	used = sizeof( lai.Password );
	used = ( used > g_sPassword.length() ) ? g_sPassword.length() : used;

	memcpy( lai.Password, g_sPassword.c_str(), used );

	/*
	 * The other info
	 */
	lai.Param = LOGIN_A_LOGIN | LOGIN_R_REQUEST;

	lai.Size = sizeof( lai );

	buffer.resize( lai.Size + 1/* protocol id */ );

	BYTE bProtocol = c2s_login;
	memcpy( const_cast< char * >( buffer.c_str() ), &bProtocol, sizeof( BYTE ) );
	memcpy( const_cast< char * >( buffer.c_str() + 1 ), &lai, sizeof( lai ) );

	pClient->SendPackToServer( buffer.c_str(), buffer.size() );

	/*
	 * Get verify info get from gateway
	 */
	bool ok = false;

	int xPos = 0;
	int yPos = 0;

	getxy( xPos, yPos );

	DWORD dwOlderTimer = GetTickCount();

	while ( true )
	{
		static int s_nPos = 0;

		DWORD dwNewTimer = GetTickCount();
		if ( dwNewTimer - dwOlderTimer > 300 )
		{
			dwOlderTimer = dwNewTimer;

			if ( s_nPos++ > 5 )
			{
				s_nPos = 0;
			}
		}

		_tstring	info;
		strstream	str;

		gotoxy( xPos, yPos );
		
		info.resize( s_nPos, '.' );
		cout << "Wait for purify your account" << info.c_str() << "      " << endl;
 
		size_t datalength = 0;
		
		const void *pData = pClient->GetPackFromServer( datalength );
		
		while ( pData && datalength )
		{
			BYTE cProtocol = *( const BYTE * )pData;

			switch ( cProtocol )
			{
			case s2c_login:
				{
					KLoginAccountInfo *pLAI = ( KLoginAccountInfo * )( ( const BYTE * )pData + 1 );

					if ( ( pLAI->Param | LOGIN_A_LOGIN ) &&
						 0 == g_sAccount.compare( pLAI->Account ) &&
						 0 == g_sPassword.compare( pLAI->Password ) )
					{
						if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR )
						{
							nRetValue = LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR;

							cout << "An incorrect account or password!" << endl;
						}
						else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_FAILED )
						{
							nRetValue = LOGIN_R_FAILED;

							cout << "Login failed!" << endl;
						}
						else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_SUCCESS )
						{
							nRetValue = LOGIN_R_SUCCESS;

							cout << "Login successful!" << endl;
						}
						else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_ACCOUNT_EXIST )
						{
							nRetValue = LOGIN_R_ACCOUNT_EXIST;

							cout << "Login failed! your accout was existed!" << endl;
						}
						else if ( ( pLAI->Param & 0xFFFF ) == LOGIN_R_TIMEOUT )
						{
							nRetValue = LOGIN_R_TIMEOUT;

							cout << "Login failed! no money!" << endl;
						}

						ok = true;
					}
				}
				break;

			default:
				break;
			}
			
			pData = pClient->GetPackFromServer( datalength );
		}

		if ( ok )
		{
			break;
		}

		Sleep( 1 );
	}

	return nRetValue;
}

void InputAccountAndPassword()
{
	ShowTitle( "Login" );

	/*
	 * Wait for a correct account and password
	 */
	_tstring sInformation;
	
	while ( true )
	{
		cout << "Please input your account : ";
		cin >> g_sAccount;
		
		cout << "please input your password for this account : ";
		cin >> g_sPassword;
		
		cout << endl;
		cout << "Account : [" << g_sAccount << "] Password : [" << g_sPassword << "]" << endl;
		
		cout << "Is it right? < Y : yes>";
		cin >> sInformation;

		if ( 0 == sInformation.compare( "Y" ) || 0 == sInformation.compare( "y" ) )
		{
			break;
		}
	}
}

int SelectRoleList()
{
	int nRoleCount = 0;
	const S3DBI_RoleBaseInfo *pRoleList = NULL;

	size_t datalength = 0;
	
	const void *pData = pClient->GetPackFromServer( datalength );
	
	while ( true )
	{
		if ( pData && datalength )
		{
			BYTE cProtocol = *( BYTE * )pData;
			
			if ( s2c_roleserver_getrolelist_result == cProtocol )
			{
				TProcessData *pPA = ( TProcessData * )pData;
				
				nRoleCount = *( char * )( pPA->pDataBuffer );
				
				pRoleList = ( const S3DBI_RoleBaseInfo * )( ( const char * )( pPA->pDataBuffer ) + 1/* sizeof( char ) */ );
				const size_t datalength = sizeof( S3DBI_RoleBaseInfo ) * nRoleCount;
				
				break;
			}
		}

		pData = pClient->GetPackFromServer( datalength );

		::Sleep( 1 );
	}

	ShowTitle( "Select role" );

	setcolor( enumYellowonBlack );

	int nIndex = 0;

	while ( nIndex < nRoleCount && pRoleList )
	{
		cout << nIndex << " ) " << pRoleList[nIndex].szRoleName << endl;

		nIndex ++;
	}

	setcolor( enumDefault );

	_tstring sRoleIndex;

	cout << "What are you select? Press number to be continue!" << endl;

	int nSelRoleIndex = -1;

	while ( true )
	{
		cin >> sRoleIndex;

		nSelRoleIndex = atoi( sRoleIndex.c_str() );

		/*
		 * The last one is option that you can exit application
		 */
		if ( nSelRoleIndex >= 0 && nSelRoleIndex < nRoleCount )
		{
			break;
		}

		cout << "Input incorrect, please input again." << endl;
	}

	/*
	 * Send this info to gateway
	 */
	tagDBSelPlayer tDBSP;

	tDBSP.cProtocol = c2s_dbplayerselect;

	/*
	 * nSelRoleIndex >= 0 && 
	 * nSelRoleIndex < nRoleCount && 
	 * sizeof( dpc.szRoleName ) >= strlen( pRoleList[nSelRoleIndex].szRoleName )
	 */
	size_t nRoleNameLen = strlen( pRoleList[nSelRoleIndex].szRoleName );
	memcpy( tDBSP.szRoleName, pRoleList[nSelRoleIndex].szRoleName, nRoleNameLen );
	tDBSP.szRoleName[nRoleNameLen] = '\0';

	pClient->SendPackToServer( &tDBSP, sizeof( tagDBSelPlayer ) );

	return nSelRoleIndex;
}

void WaitForEnterGameServer()
{
	cout << endl << "Wait for a respond for log on to gameserver..." << endl;

	size_t datalength = 0;	
	const void *pData = pClient->GetPackFromServer( datalength );
	
	while ( true )
	{
		if ( pData && datalength )
		{
			BYTE cProtocol = *( BYTE * )pData;
			
			if ( s2c_notifyplayerlogin == cProtocol )
			{
				tagNotifyPlayerLogin *pNPL = ( tagNotifyPlayerLogin * )pData;

				cout << "Find a valid game server and connect to it!" << endl;

				const char *pIPAddr = OnlineGameLib::Win32::net_ntoa( pNPL->nIPAddr );

				cout << "IP : " << pIPAddr << " PORT : " << pNPL->nPort << endl;

				g_sRoleName = ( const char * )pNPL->szRoleName;

				ConnectToGameSvr( pIPAddr, pNPL->nPort, pNPL->guid );

				break;
			}
		}

		pData = pClient->GetPackFromServer( datalength );

		::Sleep( 1 );
	}
}

void ConnectToGameSvr( const char *pIPAddr, unsigned short nPort, GUID &guid )
{
	/*
	 * Query network interface
	 */
	pfnCreateClientInterface pFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 50, 1024 * 16 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &pGameSvrClient ) );
		
		pClientFactory->Release();
	}

	if ( !pGameSvrClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		exit( -1 );
	}

	/*
	 * Create a connection
	 */
	pGameSvrClient->Startup();

	//pGameSvrClient->RegisterMsgFilter( reinterpret_cast< void * >( pClient ), NULL );

	if ( FAILED( pGameSvrClient->ConnectTo( pIPAddr, nPort ) ) )
	{
		cout << "Connection failed! Don't connect to game server." << endl;

		exit( -1 );
	}

	cout << "It's successful in connection! Prepare to enter this game..." << endl;

	tagLogicLogin ll;

	ll.cProtocol = c2s_logiclogin;
	memcpy( &ll.guid, &guid, sizeof( GUID ) );

	pGameSvrClient->SendPackToServer( &ll, sizeof( tagLogicLogin ) );
}

void ConnectServer()
{
	/*
	 * Query network interface
	 */
	pfnCreateClientInterface pFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 50, 1024 * 16 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &pClient ) );
		
		pClientFactory->Release();
	}

	if ( !pClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		exit( -1 );
	}

	/*
	 * Create a connection
	 */
	pClient->Startup();

	pClient->RegisterMsgFilter( reinterpret_cast< void * >( pClient ), ClientEventNotify );

	if ( FAILED( pClient->ConnectTo( g_sServerIPAddress.c_str(), g_nServerPort ) ) )
	{
		cout << "Connection failed! Don't connect to server." << endl;

		exit( -1 );
	}

	cout << "It's successful in connection! Prepare to enter this game..." << endl;
}

void DisconnectServer()
{
	/*
	 * Destroy the connection
	 */
	pGameSvrClient->Shutdown();

	pGameSvrClient->Cleanup();

	pGameSvrClient->Release();

	pClient->Shutdown();

	pClient->Cleanup();

	pClient->Release();
}