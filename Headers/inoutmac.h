// -------------------------------------------------------------------------
//	文件名		：	inoutmac.h
//	创建者		：	谢茂培 (Hsie)
//	创建时间	：	2003-07-20 14:47:18
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __INOUTMAC_H__
#define __INOUTMAC_H__


#include "malloc.h"

#define MAX_ADAPTER_DESCRIPTION_LENGTH  128 // arb.
#define MAX_ADAPTER_NAME_LENGTH         256 // arb.
#define MAX_ADAPTER_ADDRESS_LENGTH      8   // arb.


typedef struct {
    char String[4 * 4];
} K_IP_ADDRESS_STRING, *K_PIP_ADDRESS_STRING, K_IP_MASK_STRING, *K_PIP_MASK_STRING;

typedef struct _K_IP_ADDR_STRING {
    struct _K_IP_ADDR_STRING* Next;
    K_IP_ADDRESS_STRING IpAddress;
    K_IP_MASK_STRING IpMask;
    DWORD Context;
} K_IP_ADDR_STRING, *K_PIP_ADDR_STRING;

typedef struct _K_IP_ADAPTER_INFO {
    struct _K_IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
    UINT Type;
    UINT DhcpEnabled;
    K_PIP_ADDR_STRING CurrentIpAddress;
    K_IP_ADDR_STRING IpAddressList;
    K_IP_ADDR_STRING GatewayList;
    K_IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    K_IP_ADDR_STRING PrimaryWinsServer;
    K_IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
} K_IP_ADAPTER_INFO, *K_PIP_ADAPTER_INFO;



//此函数会确保两个地址均有
//返回值为网卡数量
inline int gGetMacAndIPAddress(BYTE* pMacAddressIn, DWORD* pIPAddressIn, BYTE* pMacAddressOut, DWORD* pIPAddressOut, DWORD nMask = 0x0000a8c0, DWORD nMacLength = 6)
{
	typedef DWORD(CALLBACK * PGAINFO)(K_PIP_ADAPTER_INFO, PULONG);//GetAdaptersInfo

    //加载IP Helper API 所需的库文件
    HINSTANCE hInst = LoadLibrary("iphlpapi.dll");
    if(!hInst)
		return -1;
    PGAINFO pGAInfo = (PGAINFO)GetProcAddress(hInst,"GetAdaptersInfo");
	if (!pGAInfo)
		return -1;
	K_PIP_ADAPTER_INFO pInfo = NULL;

	ULONG ulSize = 0;

	pGAInfo(pInfo, &ulSize);//第一次调用，获取缓冲区大小

	pInfo = (K_PIP_ADAPTER_INFO)alloca(ulSize);

	pGAInfo(pInfo, &ulSize);

	bool bIn = false;
	char* pMacAddressInX = (char*)alloca(nMacLength + 1);
	DWORD nIPAddressInX = 0;
	bool bOut = false;
	char* pMacAddressOutX = (char*)alloca(nMacLength + 1);
	DWORD nIPAddressOutX = 0;

	DWORD nMAcNum = 0;
    //遍历每一张网卡

    while (pInfo && (!bIn || !bOut))
    {
		K_PIP_ADDR_STRING pAddTemp = &(pInfo->IpAddressList);       
		
		while (pAddTemp)/*遍历IP列表中的每一个元素*/
		{
			DWORD nAddress = inet_addr(pAddTemp->IpAddress.String);
			if (!bIn &&
				((nAddress & 0x0000FFFF) == nMask)
				)
			{
				bIn = true;
				nIPAddressInX = nAddress;
				//物理地址的长度
				if (pInfo->AddressLength == nMacLength)
					memcpy(pMacAddressInX, pInfo->Address, nMacLength);
				else
					memset(pMacAddressInX, 0, nMacLength);
			}
			
			if (!bOut &&
				((nAddress & 0x0000FFFF) != nMask)
				)
			{
				bOut = true;
				nIPAddressOutX = nAddress;
				//物理地址的长度
				if (pInfo->AddressLength == nMacLength)
					memcpy(pMacAddressOutX, pInfo->Address, nMacLength);
				else
					memset(pMacAddressOutX, 0, nMacLength);
			}
			pAddTemp = pAddTemp->Next;
		}
		//将当前指针移向下一个
		pInfo = pInfo->Next;

		nMAcNum++;
	}
	
	if (bIn)
	{
		if (pMacAddressIn)
			memcpy(pMacAddressIn, pMacAddressInX, nMacLength);
		if (pIPAddressIn)
			*pIPAddressIn = nIPAddressInX;

		if (!bOut)
		{
			if (pMacAddressOut)
				memcpy(pMacAddressOut, pMacAddressInX, nMacLength);
			if (pIPAddressOut)
				*pIPAddressOut = nIPAddressInX;
		}
	}

	if (bOut)
	{
		if (pMacAddressOut)
			memcpy(pMacAddressOut, pMacAddressOutX, nMacLength);
		if (pIPAddressOut)
			*pIPAddressOut = nIPAddressOutX;

		if (!bIn)
		{
			if (pMacAddressIn)
				memcpy(pMacAddressIn, pMacAddressOutX, nMacLength);
			if (pIPAddressIn)
				*pIPAddressIn = nIPAddressOutX;
		}
	}

	FreeLibrary(hInst);

	return nMAcNum;
}


#endif // __INOUTMAC_H__
