#ifndef __FILEENUMERATE_H__
#define __FILEENUMERATE_H__

typedef void (*FILE_PROCESS_CALLBACK)(LPSTR lpszFile);
void DirectorySearch(LPSTR lpszDir, FILE_PROCESS_CALLBACK fnCallBack);

#endif