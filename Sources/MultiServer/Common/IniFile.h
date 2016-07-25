/********************************************************************
	created:	2003/04/09
	file base:	IniFile
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_INIFILE_H__
#define __INCLUDE_INIFILE_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

#pragma warning(disable:4786)

#include <vector>
#include <string>

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * CIniFile
 */

class CIniFile
{
public:

	typedef std::vector< std::string >	_VETSTR;
	
	CIniFile();
	CIniFile( LPCTSTR pFileName );

	~CIniFile();

	void SetFile( LPCTSTR pFileName );

	int	 ReadInteger( LPCTSTR lpSectionName, LPCTSTR lpKeyName, int nDefault );
	BOOL WriteInteger( LPCTSTR lpSectionName, LPCTSTR lpKeyName, int nValue );

	DWORD ReadHexNum( LPCTSTR lpSectionName, LPCTSTR lpKeyName, DWORD dwDefault );
	BOOL WriteHexNum( LPCTSTR lpSectionName, LPCTSTR lpKeyName, DWORD dwValue );

	DWORD ReadString( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPTSTR lpString, DWORD dwSize, LPCTSTR lpDefault );
	BOOL WriteString( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString );

	BOOL ReadSections( _VETSTR& stdSections );
	BOOL ReadSection( LPCTSTR lpSection, _VETSTR& stdKey );

private:
	
	char m_szFileName[MAX_PATH];

};

} // End of namespace OnlineGameLib
} // End of namespace Win32	

#endif // __INCLUDE_INIFILE_H__