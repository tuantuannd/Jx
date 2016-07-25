// KScriptValueSet.h: interface for the KScriptValueSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KSCRIPTVALUESET_H__00036A2D_425B_4C7D_AD6F_4941B0DD1735__INCLUDED_)
#define AFX_KSCRIPTVALUESET_H__00036A2D_425B_4C7D_AD6F_4941B0DD1735__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "KEngine.h"
class TStringValueNode:public KNode
{
public:
	char ValueName[40];
	char strValue[100];
	TStringValueNode(char * pValueName){strcpy(ValueName, pValueName); strcpy(strValue, "");};
	TStringValueNode(char * pValueName, char * pValue){strcpy(ValueName,pValueName); strcpy(strValue, pValue);};
};

class TIntegerValueNode:public KNode
{
public:
	char ValueName[40];
	int intValue;
	TIntegerValueNode(char * pName){strcpy(ValueName, pName); intValue = 0;};
	TIntegerValueNode(char * pName, int iValue){	strcpy(ValueName, pName); intValue = iValue;};
};

class KScriptValueSet  
{
public:
	KScriptValueSet();
	virtual ~KScriptValueSet();
	KList m_StringValueList;
	KList m_IntegerValueList;

	BOOL	Load(char * FileName);
	BOOL	Save(char * FileName);
	
	BOOL	SetValue(char * pValueName, char * pValue);
	BOOL	SetValue(char * pValueName, int	 nValue);

	
	BOOL	GetValue(char * pValueName, char * pValue);
	BOOL 	GetValue(char * pValueName, int* Value);


private:

  void AddValue(char * pValueName , char * pValue);
  void AddValue(char * pValueName, int Value);
  KNode *SearchValue(char * pValueName, int nType);

};
extern KScriptValueSet g_ScriptValueSet;
#endif // !defined(AFX_KSCRIPTVALUESET_H__00036A2D_425B_4C7D_AD6F_4941B0DD1735__INCLUDED_)
