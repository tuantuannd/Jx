//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KIniFile.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KIniFile_H
#define KIniFile_H
//---------------------------------------------------------------------------
#include "KMemStack.h"
//---------------------------------------------------------------------------
typedef struct tagKeyNode {
	DWORD			dwID;
	LPSTR			pKey;
	LPSTR			pValue;
	tagKeyNode*		pNextNode;
} KEYNODE;
//---------------------------------------------------------------------------
typedef struct tagSecNode {
	DWORD			dwID;
	LPSTR			pSection;
	tagKeyNode		pKeyNode;
	tagSecNode*		pNextNode;
} SECNODE;
//---------------------------------------------------------------------------
ENGINE_API void g_SetIniCodec(int nCodec);
//---------------------------------------------------------------------------
class ENGINE_API KIniFile
{
private:
	SECNODE		m_Header;
	LONG		m_Offset;
	KMemStack	m_MemStack;
private:
	void		CreateIniLink(LPVOID Buffer,LONG Size);
	void		ReleaseIniLink();
	DWORD		String2Id(LPCSTR pString);
	BOOL		ReadLine(LPSTR Buffer,LONG Size);
	BOOL		IsKeyChar(char ch);
	LPSTR		SplitKeyValue(LPSTR pString);
	BOOL		SetKeyValue(LPCSTR pSection,LPCSTR pKey,LPCSTR pValue);
	BOOL		GetKeyValue(LPCSTR pSection,LPCSTR pKey,LPSTR pValue,DWORD dwSize);
public:
	KIniFile();
	~KIniFile();
	BOOL		Load(LPCSTR FileName);
	BOOL		Save(LPCSTR FileName);
	BOOL		LoadPack(LPCSTR FileName);
	BOOL		SavePack(LPCSTR FileName);
	void		Clear();
	BOOL		IsSectionExist(LPCSTR lpSection);
	void		EraseSection(LPCSTR lpSection);
	void		EraseKey(LPCSTR lpSection, LPCSTR lpKey);
	BOOL		GetString(
				LPCSTR	lpSection,		// points to section name
				LPCSTR	lpKeyName,		// points to key name
				LPCSTR	lpDefault,		// points to default string
				LPSTR	lpRString,		// points to destination buffer
				DWORD	dwSize			// size of string buffer
				);
	BOOL		GetInteger(
				LPCSTR	lpSection,		// points to section name
				LPCSTR	lpKeyName,		// points to key name
				int		nDefault,		// default value
				int		*pnValue        // return value
				);
	void		GetInteger2(
				LPCSTR	lpSection,		// points to section name
				LPCSTR	lpKeyName,		// points to key name
				int		*pnValue1,		// value 1
				int		*pnValue2       // value 2
				);
	BOOL		GetFloat(
				LPCSTR	lpSection,		// points to section name
				LPCSTR	lpKeyName,		// points to key name
				float	fDefault,		// default value
				float	*pfValue        // return value
				);
	void		GetFloat2(
				LPCSTR	lpSection,		// points to section name
				LPCSTR	lpKeyName,		// points to key name
				float	*pfValue1,		// value 1
				float	*pfValue2       // value 2
				);
	void		GetStruct(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				LPVOID	lpStruct,		// pointer to buffer that contains data to add
				DWORD 	dwSize			// size, in bytes, of the buffer
				);
	void		GetRect(
				LPCSTR	lpSection,
				LPCSTR	lpKeyName,
				RECT	*pRect
				);
	void		GetFloat3(
				LPCSTR	lpSection,
				LPCSTR	lpKeyName,
				float	*pFloat
				);
	void		GetFloat4(
				LPCSTR	lpSection,
				LPCSTR	lpKeyName,
				float	*pRect
				);
	void		GetBool(
				LPCSTR	lpSection,
				LPCSTR	lpKeyName,
				BOOL	*pBool
				);
	void		WriteString(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				LPCSTR	lpString		// pointer to string to add
				);
	void		WriteInteger(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				int	    Value			// value to write
				);
	void		WriteInteger2(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				int	    Value1,			// value 1 to write
				int		Value2			// value 2 to write
				);
	void		WriteFloat(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				float	fValue			// value to write
				);
	void		WriteFloat2(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				float	fValue1,		// value 1 to write
				float	fValue2			// value 2 to write
				);
	void		WriteStruct(
				LPCSTR	lpSection,		// pointer to section name
				LPCSTR	lpKeyName,		// pointer to key name
				LPVOID	lpStruct,		// pointer to buffer that contains data to add
				DWORD 	dwSize			// size, in bytes, of the buffer
				);
	BOOL		GetNextSection(LPCSTR pSection, LPSTR pNextSection);
	BOOL		GetNextKey(LPCSTR pSection, LPCSTR pKey, LPSTR pNextKey);	
	int			GetSectionCount();
	
};
//---------------------------------------------------------------------------
#endif
