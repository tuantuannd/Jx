/***************************************************************************
*
* (C) Copyright 1999-2001 Feitian Technologies, Inc. All rights reserved.
*
* Description:  ePass C interface and definitions
*
* Environment:  Ring 3
*
* Revision History:
*
*   03/30/2001: Xuying - created
* 	05/08/2001: Xuying - modified
****************************************************************************/
#ifndef	EPASSAPI_H_INC
#define EPASSAPI_H_INC

#pragma pack(push,pk1,1)

#ifdef __cplusplus
extern "C" {
#endif


#ifndef	EPAS_STATUS
#define EPAS_STATUS		unsigned long
#define EPAS_API		__stdcall
typedef void*			EPAS_HANDLE;
#endif

#define	EPAS_API_VERSION	0x200

////
///	Status Definition
//

#define	FT_SUCCESS					0x00
#define FT_CANNOT_OPEN_DRIVER       0x01
#define FT_INVALID_DRVR_VERSION     0x02
#define FT_INVALID_COMMAND          0x03
#define FT_ACCESS_DENIED            0x04
#define FT_ALREADY_ZERO             0x05
#define FT_UNIT_NOT_FOUND           0x06
#define FT_DEVICE_REMOVED           0x07
#define FT_COMMUNICATIONS_ERROR     0x08
#define FT_DIR_NOT_FOUND            0x09
#define FT_FILE_NOT_FOUND           0x0A
#define FT_MEM_CORRUPT              0x0B
#define FT_INTERNAL_HW_ERROR        0x0C
#define FT_INVALID_RESP_SIZE        0x0D
#define FT_PIN_EXPIRED              0x0E
#define FT_ALREADY_EXISTS           0x0F
#define FT_NOT_ENOUGH_MEMORY        0x10
#define FT_INVALID_PARAMETER        0x11
#define FT_INPUT_TOO_LONG           0x13
#define FT_INVALID_FILE_SELECTED    0x14
#define FT_DEVICE_IN_USE            0x15
#define FT_INVALID_API_VERSION      0x16
#define FT_TIME_OUT_ERROR           0x17
#define FT_ITEM_NOT_FOUND           0x18
#define FT_COMMAND_ABORTED          0x19
#define FT_INVALID_STATUS           0xff

////
///	Reserved IDs
//

#define	EPAS_ROOT_DIR			0x0000
#define EPAS_7816_ROOT_DIR		0x3F00
#define	EPAS_AUTO_ID_BASE		0xF000
#define	EPAS_INDEX_FILE			0xFFFF
#define EPAS_TOKEN_NAME_FILE	0xFFFE

////
///	ePass File System Type
//

#define EPAS_SYS_TYPE1		1

////
///	Maximum size of names and labels
//

#define EPAS_APP_NAME_SIZE			32
#define EPAS_FRIENDLY_NAME_SIZE		32

////
/// Flags used for directory type
//

#define EPAS_DIR_BY_ID				0x00000100
#define EPAS_DIR_BY_LONG_ID			0x00000200
#define EPAS_DIR_BY_NAME			0x00000300
#define EPAS_DIR_BY_GUID			0x00000400
#define EPAS_DIR_BY_GUID_STR		0x00000500

#define EPAS_DIR_BY_MASK            0x00000f00

////
///	EPAS_ACCESSINFO structure definition
//
typedef struct tagACCESSINFO
{
	unsigned char	ucMaxPinRetries;		//Maximum USER PIN authentication failures.
	unsigned char	ucCurPinCounter;		//Currently allowed USER PIN authentication failures
	unsigned char	ucFlags;				//Reserved,MUST be zero
	unsigned char	ucReserved1;			//MUST be zero
	unsigned char	ucCreateAccess;		//Dir\file's creation rights
	unsigned char	ucDeleteAccess;		//Dir\file's creationg rights
	unsigned char	ucReserved2[10];		//Reserved,MUST be zero
}EPAS_ACCESSINFO,*PEPAS_ACCESSINFO;

////
///	EPAS_VERSIONINFO structure definition.
//

typedef struct tagVERSIONINFO
{
	unsigned long	ulDriverVersion;
	unsigned char	ucFwVerMajor;
	unsigned char	ucFwVerMinor;
	unsigned char	ucProductCode;
	unsigned char	ucConfiguration;
}EPAS_VERSIONINFO,*PEPAS_VERSIONINFO;

////
///	EPAS_SYSINFO structure definition
//

typedef struct tagSYSINFO
{
	unsigned long ulFlags;
	unsigned long ulFreeSpace;
	unsigned long ulReserved1[2];
	unsigned char ucFileSysType;
	unsigned char ucMaxDirLevels;
	unsigned char ucReserved2[14];
}EPAS_SYSINFO,*PEPAS_SYSINFO;

////
/// EPAS_DIRINFO structure definition
//
typedef struct tagDIRINFO
{
	unsigned long ulID;
	unsigned long ulFlags;
	unsigned char ucReserved[40];
}EPAS_DIRINFO,*PEPAS_DIRINFO;

////
///	EPAS_FILEINFO structure definition
//
typedef struct tagFILEINFO
{
	unsigned long ulID;			//File ID.
	unsigned long ulFlags;		//Reserved.MUST be zero
	unsigned long ulFileSize;		
	unsigned char ucFileType;
	unsigned char ucReadAccess;
	unsigned char ucWriteAccess;
	unsigned char ucCryptAccess;
	unsigned char ucReserved1[12];
	unsigned char ucGrantedAccess;
	unsigned char ucReserved2[15];
}EPAS_FILEINFO,*PEPAS_FILEINFO;

/////
//// FILEINFO.FileType
/// File type definition
//
#define EPAS_FILETYPE_UNUSED		0x00
#define EPAS_FILETYPE_DIR			0x01
#define EPAS_FILETYPE_DATA			0x02
#define EPAS_FILETYPE_KEY			0x04
#define EPAS_FILETYPE_UNKNOWN		0xFF

/////
//// FILEINFO.ReadAccess,FILEINFO.WriteAccess,FILEINFO.CryptAccess
/// File access types
//
#define EPAS_ACCESS_ANYONE			0x00
#define EPAS_ACCESS_USER			0x01
#define EPAS_ACCESS_OFFICER			0x02
#define EPAS_ACCESS_NONE			0x07

////
///  FILEINFO.GrantedAccess 
//
#define EPAS_ACCESS_READ			0x01
#define EPAS_ACCESS_WRITE			0x02
#define EPAS_ACCESS_CRYPT			0x04

////
///	ePass Capabilities definitions
//
#define EPAS_CAPS_MD5HMAC			0x00000001

/////////////////////////////////////////////////////////////
///  Device Access functions
/////////////////////////////////////////////////////////////
EPAS_STATUS EPAS_API 
epas_CreateContext(EPAS_HANDLE* hContextHandle/*[out]*/,
				   unsigned long ulFlags/*[in]*/,
				   unsigned long ulApiVersion/*[in]*/
				   );

EPAS_STATUS EPAS_API
epas_DeleteContext(EPAS_HANDLE hContextHandle/*[in]*/);

EPAS_STATUS EPAS_API
epas_OpenDevice(EPAS_HANDLE hContextHandle/*[in]*/,
				unsigned long ulFlags/*[in]*/,
				void* pAppID/*[in]*/
				);
////
///	Flags used by epas_OpenDevice()
//
#define EPAS_OPEN_NEXT				0x00000000
#define EPAS_OPEN_FIRST				0x00000001
#define EPAS_OPEN_CURRENT			0x00000002
#define EPAS_OPEN_SPECIFIC			0x00000003

#define EPAS_OPEN_MASK				0x00000007

#define EPAS_OPEN_BY_NAME			0x00000100
#define EPAS_OPEN_BY_GUID			0x00000200
#define EPAS_OPEN_BY_GUID_STR		0x00000300

#define EPAS_OPEN_BY_MASK			0x00000F00


EPAS_STATUS EPAS_API
epas_CloseDevice(EPAS_HANDLE hContextHandle/*[in]*/);


////////////////////////////////////////////////////////////
/// ePass File Operation functions
///////////////////////////////////////////////////////////

EPAS_STATUS EPAS_API
epas_CreateFile(EPAS_HANDLE	hContextHandle/*[in]*/,
				unsigned long  ulFlags/*[in]*/,
				PEPAS_FILEINFO pFileInfo/*[in]*/,
				unsigned long  ulSizeOfFileInfo/*[in]*/);

EPAS_STATUS EPAS_API
epas_DeleteFile(EPAS_HANDLE hContextHandle/*[in]*/,
				unsigned long ulFlags/*[in]*/,
				unsigned long ulFileID/*[in]*/
				);

EPAS_STATUS EPAS_API
epas_OpenFile(EPAS_HANDLE hContextHandle/*[in]*/,
			  unsigned long  ulFlags/*[in]*/,
			  unsigned long  ulFileID/*[in]*/,
			  PEPAS_FILEINFO pFileInfo/*[out]*/,
			  unsigned long  ulSizeOfFileInfo/*[in]*/
			  );

#define EPAS_FILE_READ			0x00000010
#define EPAS_FILE_WRITE			0x00000020
#define EPAS_FILE_CRYPT			0x00000040

EPAS_STATUS EPAS_API
epas_CloseFile(EPAS_HANDLE hContextHandle/*[in]*/);

EPAS_STATUS EPAS_API
epas_GetFileInfo(EPAS_HANDLE	hContextHandle/*[in]*/,
				 unsigned long	ulFlags/*[in]*/,
				 unsigned long	ulID/*[in]*/,
				 unsigned char*	pucName/*[in]*/,
				 PEPAS_FILEINFO	pFileInfo/*[out]*/,
				 unsigned long	ulFileInfoSize/*[in]*/
				 );

EPAS_STATUS EPAS_API
epas_Read(EPAS_HANDLE		hContextHandle/*[in]*/,
		  unsigned long		ulFlags/*[in]*/,
		  unsigned long		ulOffset/*[in]*/,
		  unsigned char*	pucBuffer/*[out]*/,
		  unsigned long		ulBytesToRead/*[in]*/,
		  unsigned long*	pulBytesRead/*[out]*/
		  );

EPAS_STATUS EPAS_API
epas_Write(EPAS_HANDLE	hContextHandle/*[in]*/,
		   unsigned long	ulFlags/*[in]*/,
		   unsigned long	ulOffset/*[in]*/,
		   unsigned char*	pucBuffer/*[in]*/,
		   unsigned long	ulBytesToWrite/*[in]*/,
		   unsigned long*	pulBytesWritten/*[out]*/
		   );

////////////////////////////////////////////////////////////
/// ePass Directory Operation functions
///////////////////////////////////////////////////////////

EPAS_STATUS EPAS_API
epas_CreateDir(EPAS_HANDLE hContextHandle/*[in]*/,
			   unsigned long	ulFlags/*[in]*/,
			   unsigned char*	pucName/*[in]*/,
			   unsigned char*	pucGuid/*[in]*/,
			   PEPAS_DIRINFO	pDirInfo/*[in][out]*/,
			   unsigned long	ulSizeOfDirInfo/*[in]*/
			   );

#define	EPAS_CREATE_AUTO_ID		0x00010000		//can be zero

EPAS_STATUS	EPAS_API
epas_DeleteDir(EPAS_HANDLE hContextHandle/*[in]*/,
			   unsigned	long	ulFlags/*in]*/,
			   unsigned long	ulDirID/*[in]*/,
			   unsigned char*	pucName/*[in]*/
			   );

#define	EPAS_DELETE_RECURSIVE	0x00010000

EPAS_STATUS	EPAS_API
epas_GetCurrentDir(EPAS_HANDLE	hContextHandle/*[in]*/,
				   unsigned long	ulFlags/*[in]*/,
				   unsigned	long*	pulPath/*[out]*/,
				   unsigned long	ulMaxLevels/*[in]*/,
				   unsigned long*	pulRetLevels/*[out]*/
				   );

EPAS_STATUS EPAS_API
epas_ChangeDir(EPAS_HANDLE hContextHandle/*[in]*/,
			   unsigned long	ulFlags/*[in]*/,
			   unsigned	long	ulDirID/*[in]*/,
			   unsigned char*	pucDirName/*[in]*/
			   );
#define	EPAS_DIR_FROM_MF			0x00000000
#define EPAS_DIR_FROM_CUR_DF		0x00000010
#define	EPAS_DIR_TO_PARENT			0x00000020

EPAS_STATUS EPAS_API
epas_Directory(EPAS_HANDLE	hContextHandle/*[in]*/,
			   unsigned long	ulFlags/*[in]*/,
			   unsigned long	ulIndex/*[in]*/,
			   PEPAS_FILEINFO	pFileInfo/*[out]*/,
			   unsigned long	ulSizeOfFileInfo/*[in]*/
			   );

///////////////////////////////////////////////////////////////////////////////////////
///	ePass	Cryptographic	Functions
//////////////////////////////////////////////////////////////////////////////////////
EPAS_STATUS	EPAS_API
epas_GenRandom(EPAS_HANDLE	hContextHandle/*[in]*/,
			   unsigned long	ulFlags/*[in]*/,
			   unsigned char*	pucBuffer/*[in][out]*/,
			   unsigned long	ulBufferSize/*[in]*/
			   );

EPAS_STATUS	EPAS_API
epas_HashToken(EPAS_HANDLE	hContextHandle/*[in]*/,
			   unsigned long	ulFlags/*[in]*/,
			   unsigned	long	ulRefData/*[in]*/,
			   unsigned	char*	pucBuffer/*[in]*/,
			   unsigned long	ulBufferSize/*[in]*/,
			   unsigned char*	pucResponse/*[out]*/,
			   unsigned long	ulRespSize/*[in]*/,
			   unsigned	long*	pulReturnedBytes/*[out]*/
			   );
////
///	Flags of epas_HashToken(...)
//

#define EPAS_HASH_MD5_HMAC			0x01

#define EPAS_HASH_TYPE_MASK			0xFF

EPAS_STATUS EPAS_API
epas_MD5_HMAC(EPAS_HANDLE		hContextHandle/*[in]*/,
			  unsigned long		ulFileID1/*[in]*/,
			  unsigned long		ulFileID2/*[in]*/,
			  unsigned char*	pucText/*[in]*/,
			  unsigned long		ulTextLen/*[in]*/,
			  unsigned char*	pucDigest/*[out]*/
			  );

///////////////////////////////////////////////////////////////////////////////////////////
///	ePass Authentication Functions
///////////////////////////////////////////////////////////////////////////////////////////

EPAS_STATUS EPAS_API
epas_Verify(EPAS_HANDLE			hContextHandle/*[in]*/,
			unsigned long		ulFlags/*[in]*/,
			unsigned char*		pucData/*[in]*/,
			unsigned long		ulSize/*[in]*/
			);

////
///		Flags of epas_Verify()
//

#define	EPAS_VERIFY_USER_PIN			0x00000000
#define	EPAS_VERIFY_SO_PIN				0x00000001

#define EPAS_VERIFY_NO_HASH				0x00001000
#define EPAS_VERIFY_HASH_UPIN			0x00002000

EPAS_STATUS EPAS_API
epas_ChangeCode(EPAS_HANDLE		hContextHandle/*[in]*/,
				unsigned long	ulFlags/*[in]*/,
				unsigned char*	pucOldCode/*[in]*/,
				unsigned long	ulOldCodeLen/*[in]*/,
				unsigned char*	pucNewCode/*[in]*/,
				unsigned long	ulNewCodeLen/*[in]*/
				);
////
///		Flags	of epas_ChangeCode
//

#define	EPAS_CHANGE_USER_PIN				0x00000000
#define	EPAS_UNBLOCK_USER_PIN				0x00000001
#define EPAS_CHANGE_SO_PIN					0x00000002

#define EPAS_CHANGE_MASK					0x0000000F
#define	EPAS_CHANGE_NO_HASH					0x00001000
#define EPAS_CHANGE_HASH_UPIN				0x00002000

EPAS_STATUS EPAS_API
epas_ResetSecurityState(EPAS_HANDLE		hContextHandle/*[in]*/,
						unsigned long	ulFlags/*[in]*/
						);
////
///		Flags of epas_ResetSecurityState()
//

#define	EPAS_SCOPE_MF				0x00000000

//////////////////////////////////////////////////////////////////////////////////////////////////
///		Misc.Funcionts
/////////////////////////////////////////////////////////////////////////////////////////////////

EPAS_STATUS EPAS_API
epas_GetProperty(EPAS_HANDLE			hContextHandle/*[in]*/,
				 unsigned long			ulFlag/*[in]*/,
				 void*					pRefData/*[in]*/,
				 void*					pPropData/*[in][out]*/,
				 unsigned long			ulPropSize/*[in]*/
				 );

EPAS_STATUS EPAS_API
epas_SetProperty(EPAS_HANDLE	hContextHandle/*[in]*/,
				 unsigned long	ulFlags/*[in]*/,
				 void*			pRefData/*[in]*/,
				 void*			pPropData/*[in]*/,
				 unsigned long	ulPropSize/*[in]*/
				 );
////
///		Flags of epas_SetProperty() and epas_GetProperty()
// 
#define	EPAS_PROP_CAPABILITIES		0x00
#define EPAS_PROP_MEM_SIZE			0x01
#define	EPAS_PROP_ACCESSINFO		0x03
#define EPAS_PROP_APP_NAME			0x04
#define EPAS_PROP_APP_GUID			0x05
#define EPAS_PROP_VERSIONINFO		0x06
#define EPAS_PROP_SERNUM			0x07
#define EPAS_PROP_LED_ON			0x08
#define EPAS_PROP_LED_OFF			0x09
#define EPAS_PROP_FRIENDLY_NAME		0x0B
#define EPAS_PROP_SYSINFO			0x0C


EPAS_STATUS EPAS_API
epas_Cleanup(EPAS_HANDLE		hContextHandle/*[in]*/,
			 unsigned long		ulFlags/*[in]*/
			 );


unsigned long EPAS_API epas_GetLibVersion(void);

unsigned long EPAS_API epas_GetDriverVersion(void);



#ifdef __cplusplus
}
#endif

#include "md5.h"

#pragma pack(pop,pk1)

#endif	//	EPASSAPI_H_INC