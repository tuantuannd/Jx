// LogFile.cpp: implementation of the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFile.h"
#include "assert.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////

const char dirsuffix_log[] = "";
const char filesuffix_log[] = ".log";


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogFile::CLogFile()
	: m_threshold(0), m_lBatch(0), m_hFile(INVALID_HANDLE_VALUE)
{

}

CLogFile::~CLogFile()
{
	assert(m_threshold == 0);
}


BOOL CLogFile::Initialize(const std::string& root, size_t threshold)
{
	if (m_threshold != 0)
		return FALSE;

	if (threshold == 0)
		return FALSE;


	std::string strRoot = root;
	if (!strRoot.empty())
	{
		if (*strRoot.rbegin() != CH_SEPARATOR)
			strRoot += CH_SEPARATOR;

		DWORD attr = ::GetFileAttributes(strRoot.c_str());
		if (attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
			return FALSE;
	}
	m_strRoot = strRoot;

	m_threshold = threshold;


	return TRUE;
}

BOOL CLogFile::Uninitialize()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
	m_lBatch = 0;

	m_threshold = 0;
	m_strRoot.resize(0);

	return TRUE;
}

size_t CLogFile::TraceLog(const void* pData, size_t size)
{
	if (m_threshold == 0)
		return 0;

	if (!PrepareFile() && m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD writesize = 0;
	if (!::WriteFile(m_hFile, pData, size, &writesize, NULL))
		return 0;

	return writesize;
}


BOOL CLogFile::PrepareFile()
{
	assert(m_threshold != 0);

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		if (m_lBatch)
			return TRUE;
		if (m_threshold == -1)
			return TRUE;

		DWORD hisize = 0;
		DWORD losize = ::GetFileSize(m_hFile, &hisize);
		if (hisize <= 0 && losize <= m_threshold)
			return TRUE;
	}


	SYSTEMTIME systm;
	::GetLocalTime(&systm);

	char szDate[_MAX_PATH];
	sprintf(szDate, "%04d_%02d_%02d", systm.wYear, systm.wMonth, systm.wDay, CH_SEPARATOR);
	std::string path;
	if (!OpenUniDirectory(m_strRoot + szDate, &path))
		return FALSE;

	char szTime[_MAX_PATH];
	sprintf(szTime, "\\%02d_%02d_%02d_%03d", systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds);
	std::string file;
	HANDLE hFile = CreateUniFile(path + szTime, &file);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (m_hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFile);
	m_hFile = hFile;


	return TRUE;
}

ULONG CLogFile::BeginBatch()
{
	if (m_threshold == 0)
		return 0;

	if (m_lBatch == -1)
		return 0;

	if (!PrepareFile())
		return 0;

	return ++m_lBatch;
}

ULONG CLogFile::EndBatch()
{
	if (m_threshold == 0)
		return -1;

	if (m_lBatch == 0)
		return -1;
	return --m_lBatch;
}

BOOL CLogFile::IsBatched()
{
	if (m_threshold == 0)
		return FALSE;

	return m_lBatch;
}

BOOL CLogFile::OpenUniDirectory(const std::string& dirname, std::string* pNewdir)
{
	assert(!dirname.empty() && *dirname.rbegin() != CH_SEPARATOR);

	std::string path = dirname + dirsuffix_log;

	DWORD attr = ::GetFileAttributes(path.c_str());
	if (attr != -1)
	{
		if (attr & FILE_ATTRIBUTE_DIRECTORY)
			goto on_skipcreate;

		for (DWORD c = 0; ; c++)
		{
			char buff[9];
			sprintf(buff, "%08X", c);

			std::string exp = dirname;
			exp += CH_EXTSPLIT;
			exp += buff;
			exp += dirsuffix_log;
			DWORD attr2 = ::GetFileAttributes(exp.c_str());
			if (attr2 == -1)
			{
				path = exp;
				break;
			}
			else
			{
				if (attr2 & FILE_ATTRIBUTE_DIRECTORY)
				{
					path = exp;
					goto on_skipcreate;
				}
			}

			if (c == -1)
				return FALSE;
		}
	}
	if (!::CreateDirectory(path.c_str(), NULL))
		return FALSE;

on_skipcreate:
	if (pNewdir != NULL)
		*pNewdir = path;

	return TRUE;
}

HANDLE CLogFile::CreateUniFile(const std::string& filename, std::string* pNewfile)
{
	assert(!filename.empty() && *filename.rbegin() != CH_SEPARATOR);

	std::string file = filename + filesuffix_log;

	if (::GetFileAttributes(file.c_str()) != -1)
	{
		for (DWORD c = 0; ; c++)
		{
			char buff[9];
			sprintf(buff, "%08X", c);

			std::string exf = filename;
			exf += CH_EXTSPLIT;
			exf += buff;
			exf += filesuffix_log;
			if (::GetFileAttributes(exf.c_str()) == -1)
			{
				file = exf;
				break;
			}
			if (c == -1)
				return FALSE;
		}
	}

	HANDLE hFile = ::CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (pNewfile != NULL)
		*pNewfile = file;

	return hFile;
}
