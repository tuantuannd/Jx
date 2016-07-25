#ifndef __KTABFILECTRL_H__
#define __KTABFILECTRL_H__
#include "KEngine.h"
#include "KNode.h"
#include "KITabFile.h"


class TTabColNode: public KNode
{
	
public :
	char * m_Str;
	~TTabColNode(){ delete m_Str;};
	
};

class TTabLineNode: public KNode
{
public:
	KList * pList;
	~TTabLineNode()
	{
		while(pList->GetHead())	
		{
			TTabColNode * pDelNode = (TTabColNode*) pList->GetHead();
			pList->RemoveHead();
			delete pDelNode;
		}
	};
};

class ENGINE_API KTabFileCtrl:public KITabFile
{
private:
	
	BOOL		SetValue(int nRow, int nColumn, LPSTR lpRString, DWORD dwSize, BOOL bCanCreateNew = TRUE);
	BOOL		GetValue(int nRow, int nColumn, LPSTR& lpRString, DWORD dwSize);
	BOOL		GetValue(LPSTR RowName, LPSTR ColumnName, LPSTR szValue, DWORD dwSize)
	{return GetValue(FindRow(RowName),FindColumn(ColumnName), szValue, dwSize);};
	int			Str2Col(LPSTR);
public:
	KTabFileCtrl();
	~KTabFileCtrl();
	KList		m_RowList;
	BOOL		Load(LPSTR FileName);//加载文件
	BOOL		Save(LPSTR FileName);//保存文件
	BOOL		LoadPack(LPSTR FileName);
	int			FindRow(LPSTR szRow);//返回以1为起点的值
	int			FindColumn(LPSTR szColumn);//返回以1为起点的值
	LPSTR		GetRowName(int nRow);
	LPSTR		GetColName(int nCol);
	int			InsertNewCol(LPSTR strNewCol);//返回新生成的Col编号，错误则返回-1


	BOOL		InsertAfter(int nRow);
	
	BOOL		InsertAfter(LPSTR szRow, BOOL bColumnLab = TRUE)
	{
		if (!bColumnLab)
			return InsertAfter(Str2Col(szRow));
		else
			return InsertAfter(FindRow(szRow));
	}

	BOOL		InsertBefore(int nRow);
	
	BOOL		InsertBefore(LPSTR szRow, BOOL bColumnLab = TRUE)
	{
		if (!bColumnLab)
			return InsertBefore(Str2Col(szRow));
		else
			return InsertBefore(FindRow(szRow));
	}
	
	
	
	int			GetWidth()//获得第一行的宽度
	{
		TTabLineNode * pLineNode = (TTabLineNode*)m_RowList.GetHead();
		if (!pLineNode) return 0;
		return pLineNode->pList->GetNodeCount();
	};
	int			GetHeight() { return m_RowList.GetNodeCount();};//获得总长度
	
	BOOL		GetString(int nRow, int nColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize);
	
	BOOL		GetString(int nRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize, BOOL bColumnLab = TRUE)
	{	
		if (!bColumnLab) //该段以exl文件的a1,b1,c1字符串为
			return GetString(nRow, Str2Col(szColumn), lpDefault, lpRString, dwSize);	
		else
			return GetString(nRow, FindColumn(szColumn), lpDefault, lpRString, dwSize);	
	};
	
	BOOL		GetString(LPSTR szRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize)
				{			return GetString(FindRow(szRow), FindColumn(szColumn), lpDefault, lpRString, dwSize);	};
	
	BOOL		GetInteger(int nRow, int nColumn, int nDefault, int *pnValue);
	
	BOOL		GetInteger(int nRow, LPSTR szColumn, int nDefault, int *pnValue, BOOL bColumnLab = TRUE)
				{			if (!bColumnLab)
				return GetInteger( nRow, Str2Col(szColumn),	nDefault, pnValue);
				else
					return GetInteger( nRow, FindColumn(szColumn),	nDefault, pnValue);
				};
	
	BOOL		GetInteger(LPSTR szRow, LPSTR szColumn, int nDefault, int *pnValue)
	{
		return GetInteger(FindRow(szRow), FindColumn(szColumn), nDefault, pnValue);
	}
	
	
	BOOL		GetFloat(int nRow, int nColumn, float fDefault, float *pfValue);
	BOOL		GetFloat(int nRow, LPSTR szColumn, float fDefault, float *pfValue, BOOL bColumnLab = TRUE)
	{
		if (!bColumnLab)
			return GetFloat( nRow, Str2Col(szColumn),	fDefault, pfValue);
		else
			return GetFloat( nRow, FindColumn(szColumn),	fDefault, pfValue);
	};
	
	BOOL		GetFloat(LPSTR szRow, LPSTR szColumn, float fDefault, float *pfValue)
	{
		return GetFloat( FindRow(szRow), FindColumn(szColumn),	fDefault, pfValue);
	}
	
	
	
	BOOL		WriteString(int nRow, int nColumn,  LPSTR lpString,  DWORD dwSize = 0);
	
	BOOL		WriteString(int nRow, LPSTR szColumn, LPSTR lpString,  BOOL bColumnLab = TRUE)
	{	
		if (!bColumnLab) //该段以exl文件的a1,b1,c1字符串为
			return WriteString(nRow, Str2Col(szColumn), lpString);	
		else
			return WriteString(nRow, FindColumn(szColumn), lpString);	
	};
	
	BOOL		WriteString(LPSTR szRow, LPSTR szColumn,  LPSTR lpRString)
	{			return WriteString(FindRow(szRow), FindColumn(szColumn), lpRString);	};
	
	BOOL		WriteInteger(int nRow, int nColumn, int nValue);
	
	BOOL		WriteInteger(int nRow, LPSTR szColumn, int nValue, BOOL bColumnLab = TRUE)
	{			if (!bColumnLab)
	return WriteInteger( nRow, Str2Col(szColumn),	 nValue);
				else
					return WriteInteger( nRow, FindColumn(szColumn), nValue);
	};
	
	BOOL		WriteInteger(LPSTR szRow, LPSTR szColumn, int nValue)
	{
		return WriteInteger(FindRow(szRow), FindColumn(szColumn), nValue);
	}
	
	
	BOOL		WriteFloat(int nRow, int nColumn,float fValue);
	BOOL		WriteFloat(int nRow, LPSTR szColumn, float fValue, BOOL bColumnLab = TRUE)
	{
		if (!bColumnLab)
			return WriteFloat( nRow, Str2Col(szColumn),	fValue);
		else
			return WriteFloat( nRow, FindColumn(szColumn), fValue);
	};
	
	BOOL		WriteFloat(LPSTR szRow, LPSTR szColumn,  float fValue)
	{
		return WriteFloat( FindRow(szRow), FindColumn(szColumn),	fValue);
	}
	
	BOOL		Remove(int nRow);
	BOOL		Remove(LPSTR szRow, BOOL bColumnLab = TRUE)
	{
		if (!bColumnLab)
			return Remove(Str2Col(szRow));
		else 
			return Remove(FindRow(szRow));
	};
	void		Clear();

};


#endif //__KTABFILECTRL_H__
