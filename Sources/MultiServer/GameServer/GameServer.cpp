// GameServer.cpp : Defines the entry point for the console application.
//

/*#ifdef _STANDALONE
#ifndef __linux
//#include <winsock2.h>
#endif
#endif*/
#include "StdAfx.h"
#include "KSOServer.h"

extern KSwordOnLineSever g_SOServer;
/*#ifdef __linux
#include <unistd.h>
#include <signal.h> 
#include <sys/param.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
void init_daemon(void) { 
	int pid; 
	int i; 
	if(pid=fork()) exit(0);			//是父进程，结束父进程 
	else if(pid< 0) exit(1);		//fork失败，退出 
//是第一子进程，后台继续执行 
	setsid();//第一子进程成为新的会话组长和进程组长 
//并与控制终端分离 
	if(pid = fork()) exit(0);//是第一子进程，结束第一子进程 
	else if(pid< 0) 
	exit(1);//fork失败，退出 
//是第二子进程，继续 
//第二子进程不再是会话组长 

	for(i=0; i< NOFILE; ++i) close(i); 
	umask(0);//重设文件创建掩模 
	return; 
} 

#endif*/
int g_nPort = 0;
int main(int argc, char* argv[])
{
	BOOL bRunning = TRUE;
	if (argc == 2)
	{
		g_nPort = atoi(argv[1]);
	}
/*#ifdef __linux
	init_daemon();
#endif*/

	if (!g_SOServer.Init())
		return 0;

	while(bRunning)
	{
		bRunning = g_SOServer.Breathe();
	}

	g_SOServer.Release();
	return 1;
}

#ifdef _STANDALONE
extern "C"
{
void lua_outerrmsg(const char * szerrmsg)
{
	fprintf(stderr, szerrmsg);
}
};
#endif