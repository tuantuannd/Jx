// S3PTestRole.cpp: implementation of the S3PTestRole class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestRole.h"
#include "S3PRole.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
S3PTestRole::~S3PTestRole()
{

}

Test *S3PTestRole::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test Role");

	testSuite->addTest (new TestCaller <S3PTestRole> ("testLoad", testLoad));
	testSuite->addTest (new TestCaller <S3PTestRole> ("testAdd", testAdd));
    
	return testSuite;
}

void S3PTestRole::testAdd()
{
	S3PRole role("lwsdd");
	S3PTableObjList<S3PTask>* pTaskList = role.GetTaskList();
	for(int i=0; i<pTaskList->Size(); i++)
	{
		ColumnAndValue cav = (*pTaskList)[i]->GetProperties();
		printf("task_list itaskcode: %s\r\n", cav["itaskcode"].c_str());
		printf("task_list cUserCode: %s\r\n", cav["cusercode"].c_str());
	}

	S3PTableObjList<S3PFriend>* pFriendList = role.GetFriendList();
	assert(pFriendList);
	//ColumnAndValue cav;
	//cav["cFriendCode"] = "lws123";
	//pFriendList->Add(cav);

	for(i=0; i<pFriendList->Size(); i++)
	{
		ColumnAndValue cav = (*pFriendList)[i]->GetProperties();
		printf("cusercode :  %s\r\n", cav["cusercode"].c_str());
		printf("cfriendcode :  %s\r\n", cav["cfriendcode"].c_str());
	}

	S3PTableObjList<S3PFightSkill>* pFightSkillList = role.GetFightSkillList();
	assert(pFightSkillList);
	//ColumnAndValue cav;
	//cav["iFightSkill"] = "12";
	//cav["iFightSkillLevel"] = "1111";
	//pFightSkillList->Add(cav);

	for(i=0; i<pFightSkillList->Size(); i++)
	{
		ColumnAndValue cav = (*pFightSkillList)[i]->GetProperties();
		printf("cusercode :  %s\r\n", cav["cusercode"].c_str());
		printf("iFightSkill :  %s\r\n", cav["ifightskill"].c_str());
		printf("iFightSkillLevel :  %s\r\n", cav["ifightskilllevel"].c_str());
	}

	S3PTableObjList<S3PLifeSkill>* pLifeSkillList = role.GetLifeSkillList();
	assert(pLifeSkillList);
	//ColumnAndValue cav;
	//cav["iLifeSkill"] = "12";
	//cav["iLifeSkillLevel"] = "1111";
	//pLifeSkillList->Add(cav);

	for(i=0; i<pLifeSkillList->Size(); i++)
	{
		ColumnAndValue cav = (*pLifeSkillList)[i]->GetProperties();
		printf("cusercode :  %s\r\n", cav["cusercode"].c_str());
		printf("iLifeSkill :  %s\r\n", cav["ilifeskill"].c_str());
		printf("iLifeSkillLevel :  %s\r\n", cav["ilifeskilllevel"].c_str());
	}

	S3PTableObjList<S3PEquipment> * pEquimentList = role.GetEquipmentList();
	assert(pEquimentList);
	ColumnAndValue cav;
	cav["iEquipCode"] = "111";
	pEquimentList->Add(cav);

	for(i=0; i<pEquimentList->Size(); i++)
	{
		ColumnAndValue cav = (*pEquimentList)[i]->GetProperties();
		printf("cusercode :  %s\r\n", cav["cusercode"].c_str());
		printf("iEquipClassCode :  %s\r\n", cav["iequipclasscode"].c_str());
	}

	S3PEquipment * pEquipment = (*pEquimentList)[0];
	assert(pEquipment);

	S3PTableObjList<S3PEquipRequireInfo> * pEquipRequireInfoList=pEquipment->GetEquipRequireInfoList();
	assert(pEquipRequireInfoList);

	ColumnAndValue cavERI;
	cavERI["iClass"]="123";
	cavERI["cInfoText"]="requireInfo";
	pEquipRequireInfoList->Add(cavERI);

	for(i=0; i<pEquipRequireInfoList->Size(); i++)
	{
		ColumnAndValue cav = (*pEquipRequireInfoList)[i]->GetProperties();
		printf("iMainID :  %s\r\n", cav["imainid"].c_str());
		printf("iClass :  %s\r\n", cav["iclass"].c_str());
		printf("cInfoText :  %s\r\n", cav["cinfotext"].c_str());
	}

	S3PTableObjList<S3PEquipBaseInfo> * pEquipBaseInfoList=pEquipment->GetEquipBaseInfoList();
	assert(pEquipBaseInfoList);

	ColumnAndValue cavEBI;
	cavEBI["iClass"]="123";
	cavEBI["cInfoText"]="baseInfo";
	pEquipBaseInfoList->Add(cavEBI);

	for(i=0; i<pEquipBaseInfoList->Size(); i++)
	{
		ColumnAndValue cav = (*pEquipBaseInfoList)[i]->GetProperties();
		printf("iMainID :  %s\r\n", cav["imainid"].c_str());
		printf("iClass :  %s\r\n", cav["iclass"].c_str());
		printf("cInfoText :  %s\r\n", cav["cinfotext"].c_str());
	}

	S3PTableObjList<S3PEquipEfficInfo> * pEquipEfficInfoList=pEquipment->GetEquipEfficInfoList();
	assert(pEquipEfficInfoList);

	ColumnAndValue cavEEI;
	cavEEI["iClass"]="123";
	cavEEI["cInfoText"]="EfficInfo";
	pEquipEfficInfoList->Add(cavEEI);

	for(i=0; i<pEquipEfficInfoList->Size(); i++)
	{
		ColumnAndValue cav = (*pEquipEfficInfoList)[i]->GetProperties();
		printf("iMainID :  %s\r\n", cav["imainid"].c_str());
		printf("iClass :  %s\r\n", cav["iclass"].c_str());
		printf("cInfoText :  %s\r\n", cav["cinfotext"].c_str());
	}


}

void S3PTestRole::testLoad()
{
	S3PRole role("lws123");
	ColumnAndValue cav = role.GetProperties();
	cout << "bSex:" << cav["bsex"].c_str() <<endl;
}