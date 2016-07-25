// S3PCard.h: interface for the S3PCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PCARD_H__6CEE334A_4D80_4BCE_B8F0_B0DF8C3DB449__INCLUDED_)
#define AFX_S3PCARD_H__6CEE334A_4D80_4BCE_B8F0_B0DF8C3DB449__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <string>
#include <list>
#include "S3PRow.h"

class S3PCard  
{
public:
	S3PCard(int iType,std::string strIID);
	S3PCard(ColumnAndValue CardInfoProp);
	virtual ~S3PCard();
	virtual ColumnAndValue getCardProp();
	virtual int setCardProp(ColumnAndValue CardProp);
	virtual int remove();

	static std::list<ColumnAndValue> getCardList(int iType);
protected:
	virtual int findByPrimaryKey(std::string primKey);
	virtual int findByCardCode(std::string CardCode);
private:
	ColumnAndValue m_CardProp;
	virtual ColumnAndValue load(std::string strSQL);
	virtual int store(ColumnAndValue CardProp);
	virtual int create(ColumnAndValue NewCard);
	virtual int deletes(std::string CardCode);
	virtual bool beExist(std::string strSQL);
};

#endif // !defined(AFX_S3PCARD_H__6CEE334A_4D80_4BCE_B8F0_B0DF8C3DB449__INCLUDED_)
