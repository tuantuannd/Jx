#ifndef __KITABFILE_H__
#define __KITABFILE_H__
#include "KEngine.h"

class ENGINE_API KITabFile//TabFile½Ó¿ÚÀà
{
public:
	KITabFile(){};
	virtual  ~KITabFile() {};

	virtual BOOL		Load(LPSTR FileName) = 0;
	virtual BOOL		Save(LPSTR FileName) = 0;
	virtual int			FindRow(LPSTR szRow) = 0;
	virtual int			FindColumn(LPSTR szColumn)	=0;
	virtual int			GetWidth() 	=0;
	virtual int			GetHeight() =0;
	virtual BOOL		GetString(int nRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize, BOOL bColumnLab = TRUE)	=0;
	virtual BOOL		GetString(int nRow, int nColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize)	=0;
	virtual BOOL		GetString(LPSTR szRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize)	=0;
	virtual BOOL		GetInteger(int nRow, LPSTR szColumn, int nDefault, int *pnValue, BOOL bColumnLab = TRUE)	=0;
	virtual BOOL		GetInteger(int nRow, int nColumn, int nDefault, int *pnValue)	=0;
	virtual BOOL		GetInteger(LPSTR szRow, LPSTR szColumn, int nDefault, int *pnValue)	=0;
	virtual BOOL		GetFloat(int nRow, LPSTR szColumn, float fDefault, float *pfValue, BOOL bColumnLab = TRUE)	=0;
	virtual BOOL		GetFloat(int nRow, int nColumn, float fDefault, float *pfValue)	=0;
	virtual BOOL		GetFloat(LPSTR szRow, LPSTR szColumn, float fDefault, float *pfValue)	=0;
	virtual void		Clear()	=0;


};

#endif //__KITABFILE_H__
