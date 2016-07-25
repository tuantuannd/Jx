// ED1EditorDoc.cpp : implementation of the CED1EditorDoc class
//

#include "stdafx.h"
#include "ED1Editor.h"

#include "ED1EditorDoc.h"
#include "malloc.h"

#include "../../Engine/Src/Cryptography/EDOneTimePad.h"
#ifdef _DEBUG
#pragma comment(lib, "../../Engine/Debug/Engine.lib")
#else
#pragma comment(lib, "../../Engine/Release/Engine.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////

static const TCHAR STR_RN[] = _T("\r\n");
static const TCHAR CH_LO = (TCHAR)0x020;
static const TCHAR CH_HI = (TCHAR)0x0FE;

//////////////////////////////////////////////

inline const TCHAR* __x_tcsnotrg(const TCHAR *string, TCHAR minCh, TCHAR maxCh)
{
	const _TUCHAR ucMinCh = minCh, ucMaxCh = maxCh;
	for (const TCHAR* q = string; *q ; q++)
	{
		const _TUCHAR v = *q;
		if (v < ucMinCh || v > ucMaxCh)
			return q;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CED1EditorDoc

IMPLEMENT_DYNCREATE(CED1EditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CED1EditorDoc, CDocument)
	//{{AFX_MSG_MAP(CED1EditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CED1EditorDoc construction/destruction

CED1EditorDoc::CED1EditorDoc()
{
	// TODO: add one-time construction code here

}

CED1EditorDoc::~CED1EditorDoc()
{
}

BOOL CED1EditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CED1EditorDoc serialization

void CED1EditorDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	//((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);

	CEditView* pEditView = (CEditView*)m_viewList.GetHead();
	if (pEditView == NULL)
		return;

	BOOL bUnlegal = FALSE;

	if (ar.IsStoring())
	{
		UINT buflen = pEditView->GetBufferLength();
		if (buflen <= 0)
			return;

		LPCTSTR szBuff = pEditView->LockBuffer();
		if (szBuff != NULL)
		{
			TCHAR	lb[2];
			lb[0] = _T('\r');
			lb[1] = _T('\n');

			for (LPCTSTR szP = szBuff, szRN = NULL; szP[0] != 0; szP = szRN + 1)
			{
				szRN = __x_tcsnotrg(szP, CH_LO, CH_HI);

				size_t linelen = 0;
				if (szRN == NULL)
					linelen = _tcslen(szP);
				else
					linelen = szRN - szP;

				if (linelen > 0)
				{
					size_t linesize = linelen * sizeof(TCHAR);

					TCHAR* szLine = (TCHAR*)_alloca(linesize + sizeof(TCHAR));
					memcpy(szLine, szP, linesize);
					szLine[linelen] = 0;

					//encode
					EDOneTimePad_Encipher(szLine, linelen);


					ar.Write(szLine, linesize);
				}

				if (szRN == NULL)
					break;

				if (szRN[0] != _T('\n'))
					continue;

				ar.Write(&lb, 2 * sizeof(TCHAR));
			}
		}

		pEditView->UnlockBuffer();
	}
	else
	{
		CFile* pFile = ar.GetFile();
		if (pFile == NULL)
			return;

		DWORD filesize = pFile->GetLength();
		if (filesize <= 0)
			return;

		TCHAR* szBuff = (TCHAR*)_alloca(filesize + sizeof(TCHAR));

		UINT readch = ar.Read(szBuff, filesize) / sizeof(TCHAR);
		ASSERT(readch <= filesize / sizeof(TCHAR));
		if (readch <= 0)
			return;
		szBuff[readch] = 0;


		CString strAll;

		for (LPTSTR szLine = szBuff, szRN = NULL; szLine[0] != 0; szLine = szRN + 1)
		{
			szRN = (TCHAR*)__x_tcsnotrg(szLine, CH_LO, CH_HI);

			size_t linelen = 0;
			if (szRN == NULL)
				linelen = _tcslen(szLine);
			else
				linelen = szRN - szLine;

			if (linelen > 0)
			{
				//decode
				EDOneTimePad_Decipher(szLine, linelen);

				strAll += CString(szLine, linelen);
			}

			if (szRN == NULL)
				break;

			strAll += *szRN;
		}


		pEditView->SetWindowText(strAll);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CED1EditorDoc diagnostics

#ifdef _DEBUG
void CED1EditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CED1EditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CED1EditorDoc commands
