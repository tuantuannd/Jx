#ifndef ServerInfo_H
#define ServerInfo_H
#include "SynDataSet.h"
struct ServerInfo
{
SynDataSet_var ServerRef;
CORBA::Object_var Obj;
char ServerName[20];
char ServerType[20];
char ServerObjName[40];
int From;
int To;

long ActionNum;//每次服务在服务端的次数
long LoopNum;//总共完成多少次。

//CORBA::ORB::RequestSeq ReqSeq;
};

#endif