// RoleNameFilter.cpp: implementation of the CRoleNameFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoleNameFilter.h"

//////////////////////////////////////////////////////////////////////

const char file_rolenameflt[] = "goddess_rolename.flt";


//////////////////////////////////////////////////////////////////////

const size_t EXP_MAXLEN = 1024;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoleNameFilter::CRoleNameFilter()
	: m_pTextFilter(NULL)
{

}

CRoleNameFilter::~CRoleNameFilter()
{

}


BOOL CRoleNameFilter::Initialize()
{
	if (m_pTextFilter)
		return FALSE;

	FILE* pFileRoleNameFlt = fopen(file_rolenameflt, "rt");
	if (pFileRoleNameFlt == NULL)
		return FALSE;

	extern CFilterTextLib g_libFilterText;
	if (SUCCEEDED(g_libFilterText.CreateTextFilter(&m_pTextFilter)))
	{
		while (!feof(pFileRoleNameFlt))
		{
			char exp[EXP_MAXLEN];
			if (!fgets(exp, EXP_MAXLEN, pFileRoleNameFlt))
				break;

			if (!exp[0])
				continue;

			//cut the '\r\n' of tail
			size_t explen = strlen(exp);
			while (explen > 0 && (exp[explen - 1] == '\r' || exp[explen - 1] == '\n'))
				exp[--explen] = 0;
			if (explen <= 0)
				continue;

			m_pTextFilter->AddExpression(exp);
		}
	}

	fclose(pFileRoleNameFlt);

	return m_pTextFilter != NULL;
}

BOOL CRoleNameFilter::Uninitialize()
{
	if (m_pTextFilter)
	{
		m_pTextFilter->Release();
		m_pTextFilter = NULL;
	}

	return TRUE;
}

BOOL CRoleNameFilter::IsTextPass(LPCTSTR text)
{
	if (!m_pTextFilter)
		return FALSE;

	return m_pTextFilter->IsTextPass(text);
}

