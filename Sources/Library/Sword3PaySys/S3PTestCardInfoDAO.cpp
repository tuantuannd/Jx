// S3PTestCardInfoDAO.cpp: implementation of the S3PTestCardInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestCardInfoDAO.h"

#include "S3PDBConnection.h"
#include "S3PRow.h"
#include "S3PCardInfoDAO.h"
#include "S3PDBConnector.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTestCardInfoDAO::~S3PTestCardInfoDAO()
{

}

void S3PTestCardInfoDAO::testAdd()
{
	S3PDBConnection* pCardCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );

	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);
		ColumnAndValue cav;
		
		cav["cCardCode"] = "001";
		cav["cCardPassWord"] = "1111";
		cav["iFlag"]="1";
		cav["iHoldSecond"]="1000";
		cav["iHoldMonth"]="6";
		
		S3PRow row(card.GetTableName(), &cav, pCardCon);
		if ( card.Add(&row) <= 0 )
		{
			assert(false);
		}

		pCardCon->Close();
	}
}

void S3PTestCardInfoDAO::testUpdate()
{
/*	S3PDBConnection* pCardCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );
	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);
		ColumnAndValue cav,cavWhere;
		cav["cCardPassWord"] = "101010";
		cavWhere["cCardCode"] = "001";
		S3PRow row(card.GetTableName(), &cav,pCardCon);
		S3PRow rowWhere(card.GetTableName(),&cavWhere,pCardCon);
		card.Update(&row,&rowWhere);
		S3PResult * res;
		std::string strQuery;
		strQuery = "select * from cardinfo where ccardcode like \'001\' ";
		card.GetCardInfoResult(strQuery,res);
		if ( res->rows() != 1)
		{
			assert(false);
		}
		else
		{
			ColumnAndValue cavPassWord;
			cavPassWord = ( * res )[0];
			if ( cavPassWord["ccardpassword"] != "101010" )
			{
				assert( false );
			}
		}

		pCardCon->Close();
	}*/
}

void S3PTestCardInfoDAO::testDelete()
{
	S3PDBConnection* pCardCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_CARDDB );

	if (NULL != pCardCon)
	{
		S3PCardInfoDAO card(pCardCon);
		ColumnAndValue cav;
		
		cav["cCardCode"] = "001";
		//cav["cCardPassWord"] = "1111";
		//cav["iFlag"]="1";
		//cav["iHoldSecond"]="1000";
		//cav["iHoldMonth"]="6";
		
		S3PRow row(card.GetTableName(), &cav, pCardCon);
		if ( card.Delete(& row) <= 0 )
		{
			assert(false);
		}

		pCardCon->Close();
	}
}


Test *S3PTestCardInfoDAO::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test CardInfo Table Manipulator");

	testSuite->addTest (new TestCaller <S3PTestCardInfoDAO> ("testCardInfoAdd", testAdd));
    testSuite->addTest (new TestCaller <S3PTestCardInfoDAO> ("testCardInfoUpdate", testUpdate));
    testSuite->addTest (new TestCaller <S3PTestCardInfoDAO> ("testCardInfoDelete", testDelete));
    
	return testSuite;
}

void S3PTestCardInfoDAO::setUp()
{
}
