// KScriptValueSet.cpp: implementation of the KScriptValueSet class.
//
//////////////////////////////////////////////////////////////////////
#include "KCore.h"

#include "KScriptValueSet.h"
#include "KIniFile.h"
KScriptValueSet g_ScriptValueSet;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KScriptValueSet::KScriptValueSet()
{
	
}

KScriptValueSet::~KScriptValueSet()
{
	
}


BOOL	KScriptValueSet::Load(char * FileName)
{
	KIniFile File;
	if (!File.Load(FileName))	return FALSE;

	char ValueName[40];
	strcpy(ValueName, "");
	while (File.GetNextKey("StringValue", ValueName, ValueName))
	{
			char StringValue[100];
			File.GetString("StringValue", ValueName, "", StringValue, 100);
			TStringValueNode * pNode = new TStringValueNode(ValueName, StringValue);
			m_StringValueList.AddTail(pNode);
	}
	
	strcpy(ValueName,"");
	while (File.GetNextKey("IntegerValue", ValueName, ValueName))
	{
		int nValue;
		File.GetInteger("IntegerValue", ValueName, 0, &nValue);
		TIntegerValueNode * pNode = new TIntegerValueNode(ValueName, nValue);
		m_IntegerValueList.AddTail(pNode);
	}
	return TRUE;
}

BOOL	KScriptValueSet::Save(char * FileName)
{
	
	KIniFile File;
	File.Load(FileName);
	
	TStringValueNode * pStrNode = (TStringValueNode*)m_StringValueList.GetHead();
	while(pStrNode)
	{
		File.WriteString("StringValue", pStrNode->ValueName, pStrNode->strValue);
		pStrNode = (TStringValueNode*)pStrNode->GetNext();
	}
	
	TIntegerValueNode * pIntNode = (TIntegerValueNode*)m_IntegerValueList.GetHead();
	while(pIntNode)
	{
		File.WriteInteger("IntegerValue", pIntNode->ValueName, pIntNode->intValue);
		pIntNode = (TIntegerValueNode * ) pIntNode->GetNext();
	}

	File.Save(FileName);
	return TRUE;
}

BOOL	KScriptValueSet::SetValue(char * pValueName, char * pValue)
{
	TStringValueNode * pNode = (TStringValueNode *) SearchValue(pValueName, 0);
	if (pNode == NULL)
	{
		AddValue(pValueName , pValue);
		return FALSE;
	}
	else 
	{
		strcpy(pNode->strValue, pValue);
		return TRUE;
	}
	return FALSE;
}

BOOL	KScriptValueSet::SetValue(char * pValueName, int	 nValue)
{
	TIntegerValueNode * pNode = (TIntegerValueNode *) SearchValue(pValueName, 1);
	
	if (pNode == NULL)
	{
		AddValue(pValueName, nValue);
		return FALSE;
	}
	else
	{
		pNode->intValue = nValue;
		return TRUE;
	}
	
	return FALSE;
}



BOOL	KScriptValueSet::GetValue(char * pValueName, char * pValue)
{
	TStringValueNode * pNode = (TStringValueNode *) SearchValue(pValueName, 0);
	if (pNode == NULL)	{strcpy(pValue,""); return FALSE;};
	strcpy(pValue, pNode->strValue);
	return TRUE;
}

BOOL	KScriptValueSet::GetValue(char * pValueName, int *pValue)
{
	TIntegerValueNode * pNode = (TIntegerValueNode *) SearchValue(pValueName, 1);
	if (pNode == NULL)	{*pValue = 0 ; return FALSE;};
	*pValue = pNode->intValue;
	return TRUE;
}


void KScriptValueSet::AddValue(char * pValueName , char * pValue)
{
	TStringValueNode * pNode = new TStringValueNode(pValueName, pValue);
	m_StringValueList.AddTail(pNode);
}

void KScriptValueSet::AddValue(char * pValueName, int Value)
{
	TIntegerValueNode * pNode = new TIntegerValueNode(pValueName, Value);
	m_IntegerValueList.AddTail(pNode);
}

KNode* KScriptValueSet::SearchValue(char * pValueName, int nType)
{
	switch(nType)
	{
	case 0:
		{
			TStringValueNode * pNode = (TStringValueNode*)m_StringValueList.GetHead();
			while (pNode)
			{
				if (!strcmp(pNode->ValueName, pValueName))		return pNode;
				pNode = (TStringValueNode*)pNode->GetNext();
			}
			
		}break;
	case 1:
		{
			TIntegerValueNode * pNode = (TIntegerValueNode*)m_IntegerValueList.GetHead();
			while (pNode)
			{
				if (!strcmp(pNode->ValueName, pValueName))				return pNode;
				pNode = (TIntegerValueNode*)pNode->GetNext();
			}
			
		}break;
	}
	return NULL;
}
