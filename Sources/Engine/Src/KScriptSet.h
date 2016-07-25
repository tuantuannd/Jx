//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KScriptSet.h
// Date:	2001-10-31 10:50:28
// Code:	Romandou
// Desc:	
//---------------------------------------------------------------------------
#ifndef _KSCRIPTSET_H
#define _KSCRIPTSET_H
#include "KScript.h"
#include "KLuaScript.h"
#define MAXSCRIPTNUM 100 // 最大脚本量
#define KEY_NUMBER  0
#define KEY_STRING  1

struct TScript  
{
	char szKey[32];	//关键字可以是脚本对应的文件名或其它
	DWORD nKey;//关键字为数字
	KScript * pScript;	//对应的脚本实例的地址
	TScript * pParent;		//父
	TScript * pLeftChild;	//左子
	TScript * pRightChild;	//右子
	TScript * pMoreRecent;	//比之最近的
	TScript * pLessRecent;	//比之最迟的
};//按照文件名或其它属性作为脚本标识的排序二叉树


//二叉树结构标准函数
extern TScript * BTSearch(TScript * pParentTScript, TScript * pTScript, char * szKey, BOOL * pResult);
extern DWORD BTPreorder(TScript * pTScript);//中序遍历排序二叉树
extern TScript * BTInsert(TScript *pTScript, char * szKey);
extern TScript * BTFindLess(TScript * pTScript);
extern TScript * BTSearch(TScript * pParentTScript, TScript * pTScript, DWORD nKey, BOOL * pResult);
extern DWORD BTPreorder(TScript * pTScript);//中序遍历排序二叉树
extern TScript * BTInsert(TScript *pTScript, DWORD nKey);
extern TScript *  BTDelete(TScript * pTScript, TScript ** ppRootTScript, int nKeyStyle);


//---------------------------------------------------------------------------
class ENGINE_API KScriptSet //脚本集合控制类
{		
public:		
	KScriptSet();
	~KScriptSet();

	KScriptSet(int Key_Style);

	
	KScript * GetScript(char * szKey, BOOL nNeedCreate, BOOL nNeedUpdateRecent);	//根据关键字获得脚本	
	KScript * GetScript(DWORD nKey, BOOL nNeedCreate, BOOL nNeedUpdateRecent);
	DWORD  ListScriptsKey();
	DWORD  ListRecent(int order);
	
	TScript *  SearchScript(char * szKey, BOOL *pnResult);//查找关键字的结点，若存在则*pnResult = 1，返回该结点；否则*pnResult = 0,返回接近点。
	TScript *  SearchScript(DWORD nKey, BOOL *pnResult);//查找关键字的结点，若存在则*pnResult = 1，返回该结点；否则*pnResult = 0,返回接近点。
	BOOL DeleteScript(char * szKey);//删除关键字的结点
	BOOL DeleteScript(DWORD szKey);//删除关键字的结点
	
	DWORD GetCount(){return ListRecent(0);};	//获得当前结点数量
	int	  GetKeyStyle(){return m_nKeyStyle;	};
	
	virtual BOOL	Run(char * szKey);
	virtual BOOL	Run(DWORD nKey);

	virtual BOOL	RunFunction(DWORD nKey, char * szFuncName, char * szFormat, ...);
	virtual BOOL	RunFunction(char * szKey, char * szFuncName, char * szFormat, ...);
		 
//protected:
	TScript * m_pRootList;		//二叉树根支点
	TScript * m_pMostRecent;	//最新使用脚本
	TScript * m_pLestRecent;	//最老使用脚本
	
	virtual KScript *  CreateScript(char * szKey , int StackSize); 
	virtual KScript *  CreateScript(DWORD nKey , int StackSize); 
	virtual	char * GetScriptFileNameFromKey(char * szKey){ return szKey;};
	virtual char * GetScriptFileNameFromKey(DWORD nKey){return NULL;}
	
	virtual int  GetInitStackSize(char * szKey);//在进行初始化时，获得脚本堆的大小
	virtual int  GetInitStackSize(DWORD nKey);//在进行初始化时，获得脚本堆的大小
	
	TScript * InsertScript(char * szKey);//插入以该关键字的结点
	TScript * InsertScript(DWORD nKey);//插入以该关键字的结点
	BOOL DeleteScript(TScript * pTScript);//删除结点
	
	void UpdateRecent(BOOL bExistedScript, TScript * pTScript);
	int m_nKeyStyle ;//关键字的类型 有两种 数字与字符串，值分别为 KEY_NUMER KEY_STRING
	
	

};
//---------------------------------------------------------------------------
#endif
