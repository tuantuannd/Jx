// GatewayRobot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "conio.h"

#include "Win32Exception.h"

#include "Player.h"

#include <string>
#include <list>

using namespace OnlineGameLib::Win32;


typedef std::list< IPlayer * >	PLAYER_LIST;
PLAYER_LIST				g_thePlayers;

//OnlineGameLib::Win32::CLibrary g_theRainbowLib( "rainbow.dll" );


// 要跟随的角色的名字
char g_szRoleName[32] = "";
char *g_pRoleName = g_szRoleName;

int g_AccoutIDMin = 500;
int g_AccoutIDMax = 600;

//char g_szServerIP[0x100] = "192.168.21.62";
//char g_szServerIP[0x100] = "192.168.22.105";
char g_szServerIP[0x100] = "192.168.26.1";
int  g_nServerPort = 5622;

int g_nLoginLogoutFlag = false;

int g_nDeleteRoleFlag   = false;

int g_nAddRoleFlag      = false;
int g_nAddRoleGender    = -1;   // 性别
int g_nAddRoleSeries    = -1;   // 五行
int g_nAddRolePlaceID   = -1;    // 出生地

// flying add these vars
// 指定出生坐标
POINT g_ptBirthPos;
int g_nFlgSetPos		= 0;
int g_nFlgSetAttack		= 0;
int g_nFlgSetSilence	= 0;
int g_nFlgChatTimer		= 0;
unsigned char szGMCommand[128] = {0};
char g_szGMCommandList[MAX_GM_COUNT][MAX_GM_SIZE];

int ProcessArg(int argc, char *argv[])
{
    int nResult = false;
    int nHaveArgcFlag = false;

    while (--argc)
    {
        nHaveArgcFlag = true;
        argv++;

        if (stricmp(argv[0], "-AccMin") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_AccoutIDMin = atoi(argv[1]);

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-AccMax") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_AccoutIDMax = atoi(argv[1]);

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-ServerIP") == 0)
        {
            if (argc < 2)
                goto Exit0;

            strcpy(g_szServerIP, argv[1]);

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-ServerPort") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_nServerPort = atoi(argv[1]);

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-FollowRoleName") == 0)
        {
            if (argc < 2)
                goto Exit0;

            strncpy(g_szRoleName, argv[1], 32);
            g_szRoleName[31] = '\0';

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-LoginLogout") == 0)
        {
            g_nLoginLogoutFlag = true;

            continue;
        }


        if (stricmp(argv[0], "-DeleteRole") == 0)
        {
            g_nDeleteRoleFlag = true;

            continue;
        }

        if (stricmp(argv[0], "-AddRole") == 0)
        {
            g_nAddRoleFlag= true;

            continue;
        }
        
        if (stricmp(argv[0], "-AddRoleGender") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_nAddRoleGender = atoi(argv[1]);
            if (g_nAddRoleGender > 1)
                g_nAddRoleGender = -1;

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-AddRoleSeries") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_nAddRoleSeries = atoi(argv[1]);
            if (g_nAddRoleSeries >= 5)
                g_nAddRoleSeries = -1;

            argc--;
            argv++;
            continue;
        }

        if (stricmp(argv[0], "-AddRolePlaceID") == 0)
        {
            if (argc < 2)
                goto Exit0;

            g_nAddRolePlaceID = atoi(argv[1]);

            argc--;
            argv++;
            continue;
        }


		// flying add this branch. SetPos
		if (stricmp(argv[0], "-SetPos") == 0)
		{
			if (argc < 3)
				goto Exit0;

			g_ptBirthPos.x = atoi(argv[1]);
			g_ptBirthPos.y = atoi(argv[2]);
			g_nFlgSetPos = 1;
			argc -= 2;
			argv += 2;
			continue;
		}

		// flying add this branch, set auto attack
		if (stricmp(argv[0], "-SetAttack") == 0)
		{
			g_nFlgSetAttack = 1;
			continue;
		}

		// flying add this branch, Let all robots shut up
		if (stricmp(argv[0], "-Shutup") == 0)
		{
			g_nFlgSetSilence= 1;
			continue;
		}
		
		// flying add this branch, pass a gm command to the robot
		// this branch was removed, use "LoadGM" instead.
//		if (stricmp(argv[0], "-SetGM") == 0)
//		{
//			g_nFlgSetGM = 1;
//			strncpy((char *)szGMCommand, argv[1], 128);
//			argc--;
//			argv++;
//			continue;
//		}

		// load gm command list from a file
		if (stricmp(argv[0], "-LoadGM") == 0)
		{
			char szGMFile[MAX_PATH];
			FILE* fp = NULL;

			memset(szGMFile, 0, MAX_PATH);
			strncpy(szGMFile, argv[1], strlen(argv[1]));
			fp = fopen(szGMFile, "r");
			if (fp == NULL)
			{
				printf("GM Command list file %s is not exist!\n", szGMFile);
				goto Exit0;
			}
			printf("Load GM command........\n");
			int nSize = 0;
			for (int i = 0; i < MAX_GM_COUNT; i++)
			{
				fgets(g_szGMCommandList[i], MAX_GM_SIZE, fp);
				nSize = strlen(g_szGMCommandList[i]);
				if (nSize == 0)
					break;
				if (g_szGMCommandList[i][nSize-1] == '\r' 
					|| g_szGMCommandList[i][nSize-1] == '\n')
					g_szGMCommandList[i][nSize-1] = 0;
				printf("+ %s\n", g_szGMCommandList[i]);
			}
			fclose(fp);
			fp = NULL;
			argc--;
			argv++;
			continue;
		}

		// flying add this branch, set the timer of chatting. The 
		// currently timer is 0.8 s
		if (stricmp(argv[0], "-ChatTimer") == 0)
		{
			g_nFlgChatTimer= 1;
			continue;
		}


        if (
            (stricmp(argv[0], "-Help") == 0)  || 
            (stricmp(argv[0], "-?") == 0)
        )
        {
            goto Exit0;
        }

        goto Exit0;

    }

    if (!nHaveArgcFlag)
        goto Exit0;     // goto Help

    nResult = true;

Exit0:
    return nResult;
}

int Help()
{
    printf("GatewayRobot for Sword Online Game V1.0\n");
    printf("    Usage:\n");
    printf("    GatewayRobot -AccMin dddd -AccMax dddd\n"); 
    printf("                 -ServerIP ddd.ddd.ddd.ddd -ServerPort dddd\n");
    printf("                 -FollowRoleName Name\n");
    printf("                 -LoginLogout\n");
    printf("                 -DeleteRole\n");
    printf("                 -AddRole\n");
    printf("                 -AddRoleGender     0(man)/1(woman)\n");
    printf("                 -AddRoleSeries     0..4\n");
    printf("                 -AddRolePlaceID    dddd\n");
    printf("\n");
    printf("    -AccMin:            Account Min Number\n");
    printf("    -AccMax:            Account Max Number\n");
    printf("    -ServerIP:          Gateway Server IP\n");
    printf("    -ServerPort:        Gateway Server Port\n");
    printf("    -LoginLogout:       Test Login / Logout\n");
    printf("    -DeleteRole:        Delete All Role(maybe)\n");
    printf("    -AddRole:           Add one Role\n");
    printf("    -FollowRoleName:    Robot follow Role Name\n");
	// flying add this line, to put the robots at a specified point.
	printf("    -SetPos:            X, Y\n");
	printf("    -SetAttack\n");
	printf("    -SetGM \"?gm xx ???\"\n");
	printf("    -LoadGM GM_COMMANDS_FILE\n");
	printf("    -Shutup\n");
	printf("    -ChatTimer\n");
    printf("\n");
    printf("    dddd: digtal number\n");
    printf("    ddd.ddd.ddd.ddd: IP address");

    return true;
}

int main(int argc, char* argv[])
{
    int nResult = false;
    int nRetCode = false;

    int i = 0;
	printf("%s\n", argv[0]);
  	srand( (unsigned)time( NULL ) );
	
	memset(g_szGMCommandList, 0, MAX_GM_COUNT * MAX_GM_SIZE);
    nRetCode = ProcessArg(argc, argv);
    if (!nRetCode)
    {
        Help();
        goto Exit0;
    }


    //g_thePlayers.empty();

	for (i = g_AccoutIDMin; i <= g_AccoutIDMax; i++)
	{
		char szAccountName[0x100];
        
        sprintf(szAccountName, "Robot%04d", i);

        printf("Create %s ... ", szAccountName);

		IPlayer *pPlayer = new CPlayer( 
            g_szServerIP, g_nServerPort,
			szAccountName
        );
		//Sleep(300);
        if (!pPlayer)
        {
            printf("fail!\n");
            continue;
        }
        printf("ok!\n");

		// flying add these, make the robot at the specified position
		if (g_nFlgSetPos)
			pPlayer->SetPos(g_ptBirthPos.x, g_ptBirthPos.y);
		// flying add these, make the robot auto attack to the NPCs
		if (g_nFlgSetAttack)
			pPlayer->SetAttack();
		pPlayer->SetSilence(g_nFlgSetSilence);
		pPlayer->SetChatTimer(g_nFlgChatTimer);
		pPlayer->ConnectToGateway();

        g_thePlayers.push_back( pPlayer );
	}

    printf("Press \'Q\' to End Program\n");

    while (true)
    {
        int ch = 0;

	    PLAYER_LIST::iterator it;
	    for ( it = g_thePlayers.begin(); it != g_thePlayers.end(); it ++ )
	    {
            while (true)
            {
                if (!kbhit())
                    break;

                ch = getch();

                ch = toupper(ch);
            
                if ((ch == 'Q') || (ch == 27)) // ESC
                    break;  
            }

            if ((ch == 'Q') || (ch == 27)) // ESC
                break;  

            //Sleep(rand() % 40);

		    CPlayer *pPlayer = (CPlayer *)(*it);

            if (pPlayer == NULL)
            {
                Sleep(100);
                continue;
            }

            if ((pPlayer->GetStatus()) != CPlayer::enumExitGame)
                continue;

		    OnlineGameLib::Win32::_tstring sAccountName;

            pPlayer->GetAccountName(sAccountName);

            delete pPlayer;
            pPlayer = NULL;
            *it = NULL;
            printf("Destory %s ... ok!\n", sAccountName.c_str());

            if (g_nAddRoleFlag || g_nDeleteRoleFlag)
                continue;
        
            printf("ReCreate %s ... ", sAccountName.c_str());

		    pPlayer = new CPlayer( 
                g_szServerIP, g_nServerPort,
			    sAccountName.c_str()
            );
			Sleep(300);
            if (!pPlayer)
            {
                printf("fail!\n");
                continue;
            }
            printf("ok!\n");

		    pPlayer->ConnectToGateway();

            *it = pPlayer;
	    }

        if ((ch == 'Q') || (ch == 27)) // ESC
            break;  

        Sleep(rand() % 1000);
    }	

    {
	PLAYER_LIST::iterator it;
	for ( it = g_thePlayers.begin(); it != g_thePlayers.end(); it ++ )
	{
		IPlayer *pPlayer = ( *it );

		if ( pPlayer )
		{
			delete pPlayer;
		}
	}
    }

    nResult = true;

Exit0:
    return nResult;
}

