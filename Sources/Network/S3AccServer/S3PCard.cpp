// S3PCard.cpp: implementation of the S3PCard class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PCard.h"

#include "S3PCardInfoDAO.h"
#include "S3PDBConnector.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PCard::S3PCard(int iType,string strIID)
{
	if ( iType == 1 )   //通过iid做索引
	{
		findByPrimaryKey(strIID);	
	}
	else if ( iType == 2 ) //通过Code做索引
	{
		findByCardCode(strIID);
	}
}

S3PCard::S3PCard(ColumnAndValue CardInfoProp)
{
	std::string strCardCode = CardInfoProp["ccardcode"];
	if ( beExist("select * from cardinfo where ccardcode like \'" + strCardCode + "\' "))
	{
		findByCardCode(strCardCode);
		return;
	}
	int iResult = create(CardInfoProp);
	if ( iResult == 1 )
	{
		m_CardProp = CardInfoProp;
	}
	
}

S3PCard::~S3PCard()
{
	m_CardProp.clear();
}


std::list<ColumnAndValue> S3PCard::getCardList(int iType)
{
	std::list<ColumnAndValue> lstReturn;

	std::string strSQL;
	strSQL = " select * from cardinfo ";
	switch (iType)
	{
	case 0:
		break;
	case 1:
		strSQL = strSQL + "where iflag = 1 ";
		break;
	case 2:
		strSQL = strSQL + "where iflag = 2 ";
		break;
	}
	strSQL = strSQL + "order by ccardcode ";

	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);
		S3PResult result;
		if ( card.Query(strSQL,result) > 0 )
		{
			for ( int i = 0; i < result.size(); i++)
			{
				ColumnAndValue cav = result[i];
				lstReturn.push_front(cav);
			}
		}
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return lstReturn;
}

bool S3PCard::beExist(std::string strSQL)
{
	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	bool bReturn = false;
	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);

		S3PResult result;
		if ( card.Query(strSQL,result) > 0 )
		{
			if ( result.size() > 0 )
			{
				bReturn = true;
			}
		}
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return bReturn;
}

ColumnAndValue S3PCard::load(std::string strSQL)
{
	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	ColumnAndValue cav;
	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);

		S3PResult result;
		if ( card.Query(strSQL, result) > 0)
		{
			if ( result.size() == 1)
			{
				cav = result[0];
			}
		}
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return cav;
}

int S3PCard::store(ColumnAndValue CardProp)
{
	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	int iReturn = 0 ;
	if ( NULL != pCardCon )
	{
		S3PCardInfoDAO card( pCardCon );
		S3PRow row( card.GetTableName(), &CardProp,pCardCon);
		ColumnAndValue cavWhere;
		cavWhere["iid"] = CardProp["iid"];
		S3PRow rowWhere( card.GetTableName(), &cavWhere, pCardCon);
		iReturn = card.Update(&row, &rowWhere);
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return iReturn;
}

int S3PCard::create(ColumnAndValue NewCard)
{
	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);
		
		S3PRow row(card.GetTableName(), &NewCard, pCardCon);
		if ( card.Add(&row) <= 0 )
		{
			pCardCon->Close();
			return -1;    //添加数据出错
		}
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return 0;
}

int S3PCard::deletes(std::string CardCode)
{
	S3PDBConnection* pCardCon =		// 连接点卡数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	int iReturn = 0;
	if ( NULL != pCardCon )
	{
		S3PCardInfoDAO card(pCardCon);
		ColumnAndValue cav;
		cav["ccardcode"] = CardCode;
		S3PRow row (card.GetTableName(), &cav,pCardCon);
		iReturn = card.Delete(&row);
	}
	if ( NULL != pCardCon )
	{
		pCardCon->Close();
	}
	return iReturn;
}

int S3PCard::findByPrimaryKey(std::string primKey)
{
	std::string strSQL = "select * from cardinfo where iid = " + primKey ;
	m_CardProp = load(strSQL);
	return 0;
}

int S3PCard::findByCardCode(std::string CardCode)
{
	std::string strSQL = "select * from cardinfo where iid like \'" + CardCode + "\'";
	m_CardProp = load(strSQL);
	return 0;
}

int S3PCard::setCardProp(ColumnAndValue CardProp)
{
	store(CardProp);
	m_CardProp = CardProp;
	return 0;
}

ColumnAndValue S3PCard::getCardProp()
{
	return m_CardProp;
}

int S3PCard::remove()
{
	std::string strCode = m_CardProp["ccardcode"];
	deletes(strCode);
	m_CardProp.clear();
	return 0;
}
