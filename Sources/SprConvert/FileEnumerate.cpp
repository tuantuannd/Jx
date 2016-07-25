#include "stdafx.h"
#include <string.h>
#include "FileEnumerate.h"

void DirectorySearch(LPSTR lpszDir, FILE_PROCESS_CALLBACK fnCallBack)
{
	HANDLE			hFind;
	WIN32_FIND_DATA	FindData;
	char szPath[512];
	char cc[512];

	strcpy(szPath, lpszDir);
	if(szPath[strlen(szPath)-1] != '\\')
		strcat(szPath, "\\");

	sprintf(cc, "%s*", szPath);
	hFind = FindFirstFile(cc, &FindData);

	do
	{
		if (strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0)
			continue;

		sprintf(cc, "%s%s", szPath, FindData.cFileName);
		if (GetFileAttributes(cc) & FILE_ATTRIBUTE_DIRECTORY)
		{
			DirectorySearch(cc, fnCallBack);
		}
		else
		{
			sprintf(cc, "%s%s", szPath, FindData.cFileName);
			(*fnCallBack)(cc);
		}
	}while(FindNextFile(hFind, &FindData));

	FindClose(hFind);
}