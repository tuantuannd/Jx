// S3DBInterface.cpp : Defines the entry point for the DLL application.
//

#include "S3DBInterface.h"
#include "DBTable.h"


//需要增加一些二级的索引
//在此构造事务和复制的环境，线程的观点，需要一个独立的线程来做这件事情?
//日志数据库需要定时进行清理
ZDBLogTable *db_table;
int get_account(DB *db, const DBT *pkey, const DBT *pdata, DBT *ikey) {			//给定一个完整的buffer，得到account作为索引
	memset(ikey, 0, sizeof(DBT));
	TRoleData *pRoleData = (TRoleData *)pdata->data;
	ikey->data = pRoleData->BaseInfo.caccname;
	ikey->size = strlen(pRoleData->BaseInfo.caccname) + 1;
	return 0;
}

S3DBI_API BOOL S3DBI_InitDBInterface()
{
	db_table = new ZDBLogTable("database", "roledb"); 
	db_table->addIndex(get_account);
	if(db_table->open()) return TRUE;
	else return FALSE;
}

S3DBI_API void S3DBI_ReleaseDBInterface()		//释放数据库引擎
{
	db_table->close();
	delete db_table;
}

S3DBI_API void *	S3DBI_GetRoleInfo(IN OUT BYTE * pRoleBuffer, IN char * strUser, IN OUT int &nBufLen)
{
	int size;
	char *buffer = db_table->search(strUser, strlen(strUser) + 1, size);
	if(buffer) {
		nBufLen = size;
		memmove(pRoleBuffer, buffer, size);
		delete[] buffer;
	}
	else nBufLen = 0;
	return pRoleBuffer;
}

//保存角色的信息，如果数据库不存在该玩家，则增加该玩家
//bAutoInsertWhenNoExistUser 设为TRUE时表示，如果需要保存的该玩家在数据库中并不存在则自动加入到数据库中，FALSE则不增加直接返回错误
//注意INI文件只须存放将需要改动的数据，不需改动的数据将自动保存原状。
S3DBI_API int	S3DBI_SaveRoleInfo(IN BYTE * pRoleBuffer, IN char * strUser, BOOL bAutoInsertWhenNoExistUser)
{
//需要存放帐号首先找到数据
	TRoleData *pRoleData = (TRoleData *)pRoleBuffer;
	if(strUser[0] == 0) strUser = pRoleData->BaseInfo.szName;
	if(db_table->add(strUser, strlen(strUser) + 1, (char *)pRoleBuffer,  pRoleData->dwDataLen)) return 1;
	else return 0;
}

S3DBI_API void S3DBI_ShowDebugInfo(IN BYTE * pRoleBuffer, char * pShowString)
{
	return;
}

S3DBI_API int S3DBI_GetRoleListOfAccount(char * szAccountName, S3DBI_RoleBaseInfo * RoleBaseList, int nMaxCount)
{
	int size = 0;
	S3DBI_RoleBaseInfo *base_ptr = RoleBaseList;
	int count = 0;
	char *buffer = db_table->search(szAccountName, strlen(szAccountName) + 1, size, 0);
	while(buffer) {
		if(count < nMaxCount) {
			TRoleData *pRoleData = (TRoleData *)buffer;
			strncpy(base_ptr->szRoleName, pRoleData->BaseInfo.szName, 32);
			base_ptr->nSex = pRoleData->BaseInfo.bSex;
			base_ptr->nHelmType = pRoleData->BaseInfo.ihelmres;
			base_ptr->nArmorType = pRoleData->BaseInfo.iarmorres;
			base_ptr->nWeaponType = pRoleData->BaseInfo.iweaponres;
			base_ptr->nLevel = pRoleData->BaseInfo.ifightlevel;
			base_ptr++;
		}
		count++;
		buffer = db_table->next(size);
	}
	return count;
}

//删除角色
S3DBI_API int	S3DBI_DeleteRole(char * strUser)
{
	return 1;
}

S3DBI_API void*	S3DBI_GetList(S3DBI_LISTNAME listname,  char * strUser)
{
	return 0;
}

S3DBI_API void*	S3DBI_GetList(S3DBI_LISTNAME listname,  int iid)
{
	return 0;
}



S3DBI_API int	S3DBI_GetCountOfList(S3DBI_LISTNAME listname, void * pList)
{
	return 0;
}

S3DBI_API int	S3DBI_DeleteDataAtList(S3DBI_LISTNAME listname, void * pList, int i )
{
	return 0;
}

S3DBI_API void	S3DBI_ReleaseListBuffer(S3DBI_LISTNAME listname, void * pList)
{
	return;
}

S3DBI_API int	S3DBI_ClearList(S3DBI_LISTNAME listname, void * pList)
{
	return 0;
}

S3DBI_API int	S3DBI_DeleteData(S3_DB_TABLENAME tablename, int nId)
{
	return 0;
}
S3DBI_API int	S3DBI_DeleteData(S3_DB_TABLENAME tablename, char * strUser)
{
	return 0;
}


S3DBI_API int	S3DBI_GetListData(S3DBI_LISTNAME listname, void * pList, int i , KTabFileCtrl *pTabFile)
{
	return 0;
}

S3DBI_API int	S3DBI_InsertDataAtList(S3DBI_LISTNAME listname, void * pList, KTabFileCtrl *pTabFile)
{
	return 0;
}
S3DBI_API int	S3DBI_GetData(S3_DB_TABLENAME tablename, int nId, KTabFileCtrl *pTabFile)
{
	return 0;
}
S3DBI_API int	S3DBI_GetData(S3_DB_TABLENAME tablename, char * strUser, KTabFileCtrl * pTabFile)
{
	return 0;
}
S3DBI_API int	S3DBI_UpdateData(S3_DB_TABLENAME tablename, int nId, KTabFileCtrl *pTabFile)
{
	return 0;
}
S3DBI_API int	S3DBI_UpdateData(S3_DB_TABLENAME tablename, char * strUser, KTabFileCtrl *pTabFile)
{
	return 0;
}
S3DBI_API int	S3DBI_InsertData(S3_DB_TABLENAME tablename,  KTabFileCtrl *pTabFile)
{
	return 0;
}

