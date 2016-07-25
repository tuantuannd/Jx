// S3PTestCard.cpp: implementation of the S3PTestCard class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestCard.h"
#include "S3PCard.h"
#include "S3PRow.h"
#include <iostream.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTestCard::~S3PTestCard()
{

}

void S3PTestCard::testCardInit()
{
	ColumnAndValue cav;
	cav["iid"] = "1";
	cav["ccardcode"] = "0001";
	cav["ccardpassword"] = "12345";
	cav["iflag"] = "1";
	cav["iholdsecond"] = "1000";
	cav["iholdmonth"] = "12";
	S3PCard * card = new S3PCard(cav);
	S3PCard * cardInTable = new S3PCard(1,"0001");
	ColumnAndValue cavInTable = cardInTable->getCardProp();
	cout << "Card Code is:" << cavInTable["ccardcode"].c_str() << endl;
	assert(cav == cavInTable);
	delete card;
	delete cardInTable;
}

void S3PTestCard::testRemove()
{
	ColumnAndValue cav;
	cav["iid"] = "2";
	cav["ccardcode"] = "0002";
	cav["ccardpassword"] = "12345";
	cav["iflag"] = "2";
	cav["iholdsecond"] = "1200";
	cav["iholdmonth"] = "21";
	S3PCard * card = new S3PCard(cav);
	cav.clear();
	delete card;
	S3PCard * cardOld = new S3PCard(2,"2");
	ColumnAndValue cavInfo = cardOld->getCardProp();
	cout <<"testRemove: Card code is ==>" << cavInfo["ccordcode"].c_str() << endl;
	cavInfo.clear();
	int iReturn = cardOld->remove();
	if ( iReturn < 0 )
	{
		assert(false);
	}
	delete cardOld;
}

void S3PTestCard::testCardSetInfo()
{
	ColumnAndValue cav;
	cav["iid"] = "1";
	cav["ccardcode"] = "0001";
	cav["iholdsecond"] = "2400";
	S3PCard * card = new S3PCard(1,"0001");
	ColumnAndValue cavInfo = card->getCardProp();
	cout << "testSetInfo: Card code first is ==>" << cavInfo["ccardcode"].c_str() << endl;
	card->setCardProp(cav);
	cavInfo = card->getCardProp();
	cout << "testSetInfo: Card code last is ==>" << cavInfo["ccardcode"].c_str() << endl;
	cout << "             Card iHoldsecond is ===>" << cavInfo["iholdsecond"].c_str() << endl;
	cavInfo.clear();
	delete card;
}

void S3PTestCard::testGetCardList()
{
	std::list<ColumnAndValue> lstResult;
	lstResult = S3PCard::getCardList(0);
	while (!lstResult.empty())
	{
		ColumnAndValue cav = lstResult.front();
		cout << "testGetCardList is ====>" << cav["ccardcode"].c_str() << endl;
		lstResult.pop_front();
	}
	lstResult.clear();
}

Test *S3PTestCard::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test Card Object:");

	//testSuite->addTest (new TestCaller <S3PTestCard> ("testCardInit", testCardInit));
    //testSuite->addTest (new TestCaller <S3PTestCard> ("testCardGetInfo", testCardSetInfo));
    //testSuite->addTest (new TestCaller <S3PTestCard> ("testRemove", testRemove));
	testSuite->addTest (new TestCaller <S3PTestCard> ("testGetCardList", testGetCardList));
    
	return testSuite;
}

void S3PTestCard::setUp()
{
}
