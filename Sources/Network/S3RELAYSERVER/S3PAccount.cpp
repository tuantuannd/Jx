//-----------------------------------------//
//                                         //
//  File		: S3PAccount.cpp		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/29/2002                //
//                                         //
//-----------------------------------------//
#pragma warning(disable: 4786)
#include "S3PAccount.h"
#include "S3PDBConVBC.h"
#include "S3PResultVBC.h"
#include "COMUtil.h"

S3PAccount::S3PAccount()
{
}

S3PAccount::~S3PAccount()
{
}

DWORD GetGMID();

int S3PAccount::ServerLogin(S3PDBConVBC* pConn, const char* strAccName, const char* strPassword, const DWORD Address, const short Port, const BYTE Mac[6], DWORD& nGameID)
{
	nGameID = 0;
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "select cIP, iPort, iid, cMemo from ServerList where (cServerName = '%s') and (cPassword COLLATE Chinese_PRC_CS_AS = '%s')", strAccName, strPassword);
	S3PResultVBC* pResult = NULL;
	if (pConn->QuerySql(strSQL, &pResult))
	{
		if (pResult->num_rows() <= 0)
			iRet = E_ACCOUNT_OR_PASSWORD;
		else
		{
			iRet = E_ADDRESS_OR_PORT;
			if ((Address & 0x0000FFFF) == 0x0000a8c0)
			{
				_variant_t clientid = 0L;
				pResult->get_field_data(2, &clientid, sizeof(_variant_t));
				nGameID = clientid.lVal;
				iRet = ACTION_SUCCESS;	//Local network not check ip
			}
			else
			{
				_variant_t vaddr;
				_variant_t vaMac;
				if (pResult->get_field_data(0, &vaddr, sizeof(_variant_t)) &&
					pResult->get_field_data(3, &vaMac, sizeof(_variant_t)) &&
					vaddr.vt == VT_BSTR && vaMac.vt == VT_BSTR)
				{
					DWORD addr = inet_addr((const char *)(_bstr_t)vaddr);
					char szmac[15];
					sprintf(szmac, "%02X%02X-%02X%02X-%02X%02X", Mac[0], Mac[1], Mac[2], Mac[3],Mac[4], Mac[5]);
					szmac[14] = 0;
					if (addr == Address &&
						strcmpi((const char *)(_bstr_t)vaMac, szmac) == 0)
					{
						_variant_t gameid = 0L;
						pResult->get_field_data(2, &gameid, sizeof(_variant_t));
						nGameID = gameid.lVal;
						iRet = ACTION_SUCCESS;
					}
				}
			}
		}
	}
	
	if (pResult)
		pResult->unuse();

	return iRet;
}

int S3PAccount::CheckAddress(S3PDBConVBC* pConn, const DWORD Address, const short Port)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	if ((Address & 0x0000FFFF) == 0x0000a8c0)
		iRet = ACTION_SUCCESS;	//Local network not check ip
	else
	{
		char strSQL[MAX_PATH];
		in_addr add;
		add.s_addr = Address;
		sprintf(strSQL, "select iid from ServerList where (cIP = '%s')", inet_ntoa(add));
		S3PResultVBC* pResult = NULL;
		if (pConn->QuerySql(strSQL, &pResult))
		{
			if (pResult->num_rows() <= 0)
				iRet = E_ADDRESS_OR_PORT;
			else
				iRet = ACTION_SUCCESS;
		}
		if (pResult)
			pResult->unuse();
	}

	return iRet;
}

int S3PAccount::ServerLogout(S3PDBConVBC* pConn, DWORD ClientID, BOOL bElapse)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}

	iRet = ACTION_SUCCESS;

	return iRet;
}

int S3PAccount::GetAccountCount(S3PDBConVBC* pConn, DWORD nGameID, BOOL bOnline, DWORD& dwCount)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	iRet = ACTION_SUCCESS;
	dwCount = 0;

	char strSQL[MAX_PATH];
	strSQL[0] = 0;
	std::string strwhere;
	if (bOnline)
	{
		strwhere = "where ";
		if (nGameID > 0)
		{
			sprintf(strSQL, "(iClientID = %d)", nGameID);
		}
		else
		{
			sprintf(strSQL, "(iClientID <> 0) and (iClientID is not null)");
		}
	}
	strwhere += strSQL;
	
	sprintf(strSQL, "select count(*) from Account_info %s", strwhere.c_str());
	S3PResultVBC* pResult = NULL;
	if (pConn->QuerySql(strSQL, &pResult))
	{
		if (pResult->num_rows() > 0)
		{
			_variant_t count = 0L;
			pResult->get_field_data(0, &count, sizeof(_variant_t));
			dwCount = count.lVal;
		}
	}
	
	if (pResult)
		pResult->unuse();
	
	return iRet;
}

int S3PAccount::GetAccountGameID(S3PDBConVBC* pConn, const char* strAccName, DWORD& ClientID)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "select iClientID from Account_info where (cAccName = '%s')", strAccName);
	S3PResultVBC* pResult = NULL;
	if (pConn->QuerySql(strSQL, &pResult))
	{
		if (pResult->num_rows() <= 0)
			iRet = E_ACCOUNT_OR_PASSWORD;
		else
		{
			_variant_t lID = 0L;
			pResult->get_field_data(0, &lID, sizeof(_variant_t));
			ClientID = lID.lVal;
			iRet = ACTION_SUCCESS;
		}
	}
	
	if (pResult)
		pResult->unuse();

	return iRet;
}

int S3PAccount::UnlockAccount(S3PDBConVBC* pConn, const char* strAccName)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "update Account_info set iClientID = 0 where (cAccName = '%s')", strAccName);
	S3PResultVBC* pResult = NULL;
	if (pConn->Do(strSQL))
		iRet = ACTION_SUCCESS;
	
	return iRet;
}

int S3PAccount::FreezeAccount(S3PDBConVBC* pConn, const char* strAccName)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "update Account_info set iClientID = %d where (cAccName = '%s')", GetGMID(), strAccName);
	S3PResultVBC* pResult = NULL;
	if (pConn->Do(strSQL))
		iRet = ACTION_SUCCESS;
	
	return iRet;
}

int S3PAccount::UnlockServer(S3PDBConVBC* pConn, unsigned long nGameID)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "update Account_info set iClientID = 0 where (iClientID = %d)", nGameID);
	S3PResultVBC* pResult = NULL;
	if (pConn->Do(strSQL))
		iRet = ACTION_SUCCESS;
	
	return iRet;
}

int S3PAccount::GetAccountsTime(S3PDBConVBC* pConn, DWORD ClientID, DWORD dwMinSecond, AccountTimeList& List)
{
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "select datediff(second, getdate(), dEndDate), iLeftSecond, cAccName, datediff(second, getdate(), dLoginDate) from View_AccountMoney where (iClientID = %d) and (dLoginDate is not null)", ClientID);
	S3PResultVBC* pResult = NULL;
	if (pConn->QuerySql(strSQL, &pResult))
	{
		for (int iRow = 0; iRow < pResult->num_rows(); iRow++)
		{
			KAccountUserTimeInfo aReturn;
			aReturn.Size = sizeof(KAccountUserTimeInfo);
			aReturn.Type = AccountUserTimeInfo;
			aReturn.Operate = 0;

			_variant_t left = 0L;
			_variant_t diffDate;
			_variant_t accname;
			_variant_t usesencond;
			pResult->get_field_data(0, &diffDate, sizeof(_variant_t));
			pResult->get_field_data(1, &left, sizeof(_variant_t));
			pResult->get_field_data(2, &accname, sizeof(_variant_t));
			pResult->get_field_data(3, &usesencond, sizeof(_variant_t));
			long liLeft = left.lVal;

			if (diffDate.vt == VT_I4 && diffDate.lVal > 0)	//包月有效
			{
				liLeft += diffDate.lVal;
			}
			else if (usesencond.vt == VT_I4 && usesencond.lVal < 0)	//点卡扣点起作用了
			{
				liLeft += usesencond.lVal;
			}
			aReturn.nTime = max(0, liLeft);
			memcpy(aReturn.Account, (const char *)_bstr_t(accname.bstrVal, true), LOGIN_USER_ACCOUNT_MAX_LEN);
			aReturn.nReturn = ACTION_SUCCESS;
			if (aReturn.nTime <= dwMinSecond)
				List.push_back(aReturn);
			pResult->data_seek(1, S3PResultVBC::next);
		}
		iRet = ACTION_SUCCESS;
	}
	
	if (pResult)
		pResult->unuse();

	return iRet;
}

int S3PAccount::GetServerID(S3PDBConVBC* pConn, const char* strAccName, unsigned long& nGameID)
{
	nGameID = 0;
	int iRet = ACTION_FAILED;
	if (NULL == pConn)
	{
		return iRet;
	}
	char strSQL[MAX_PATH];
	sprintf(strSQL, "select iid from ServerList where (cServerName = '%s')", strAccName);
	S3PResultVBC* pResult = NULL;
	if (pConn->QuerySql(strSQL, &pResult))
	{
		if (pResult->num_rows() <= 0)
			iRet = E_ACCOUNT_OR_PASSWORD;
		else
		{
			_variant_t clientid = 0L;
			pResult->get_field_data(0, &clientid, sizeof(_variant_t));
			nGameID = clientid.lVal;
			iRet = ACTION_SUCCESS;	//Local network not check ip
		}
	}
	
	if (pResult)
		pResult->unuse();

	return iRet;
}
