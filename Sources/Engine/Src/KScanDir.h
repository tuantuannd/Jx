//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KScanFile.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KScanFile_H
#define KScanFile_H
//---------------------------------------------------------------------------
class ENGINE_API KScanFile
{
private:
	FILE*		m_pFile;
	int			m_nPathLen;
	char		m_FileExt[8];
	void		SearchDirectory();
	BOOL		CheckFileExt(LPSTR FileName);
	void		OutputFileName(LPSTR FileName);
public:
	KScanFile();
	void		DiscardFileExt(LPSTR lpFileExt);
	BOOL		RunSearch(LPSTR lpRootPath, LPSTR lpScanPath, LPSTR lpOutFile);
};
//---------------------------------------------------------------------------
#endif