//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketParser.h      //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#if !defined(AFX_S3PDBSOCKETPARSER_H__8B70E8D1_C829_49AA_95E9_E8E092F44010__INCLUDED_)
#define AFX_S3PDBSOCKETPARSER_H__8B70E8D1_C829_49AA_95E9_E8E092F44010__INCLUDED_

#include "KStdAfx.h"
#include "LoginDef.h"

typedef struct tag_PARSEDDATAPACKET
{
	DWORD m_dwParam;
	IBYTE* m_lpData;
	DWORD m_dwSize;	// The size of data packet not including
					// the place occupid by tag and KLoginStructHead.
}_PARSEDDATAPACKET, *_LPPARSEDDATAPACKET;
typedef const tag_PARSEDDATAPACKET* _LPCPARSEDDATAPACKET;

class S3PDBSocketParser : public _PARSEDDATAPACKET
{
public:
	S3PDBSocketParser();
	virtual ~S3PDBSocketParser();

	inline operator _LPCPARSEDDATAPACKET() const
	{
		return this;
	}
	inline operator _LPPARSEDDATAPACKET()
	{
		return this;
	}
	static BOOL Encode( const IBYTE* pRawData/* Input */, unsigned long ulRawSize/* Input */,
		IBYTE* pEncodedData/* Output */, unsigned long& ulEncodedSize/* Output */ );
	virtual BOOL Parse( IBYTE* lpData, DWORD dwSize, BOOL bEncrypt = TRUE );
protected:
	virtual BOOL Decode( IBYTE* lpData/*Input*/, DWORD& dwSize/*Input/Output*/, IBYTE* lpRetData/*Output*/ );
	virtual void Init();
	virtual void Release();
};

#endif // !defined(AFX_S3PDBSOCKETPARSER_H__8B70E8D1_C829_49AA_95E9_E8E092F44010__INCLUDED_)
