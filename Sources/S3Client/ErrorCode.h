/**************************************************************************
文件    ：    ErrorCode.h
创建人  ：    Fyt(Fan Zhanpeng)
创建时间：    08-01-2003(mm-dd-yyyy)
功能描述：    错误号
****************************************************************************
已处理的错误代码的函数的层次关系(在图中的函数表示该函数中已添加错误处理代码)

S3Client.cpp*GameInit()*
 |->FilterText.cpp *Initialize()*
 |->ChatFilter.cpp *Initialize()*

****************************************************************************/

#if !defined(AFX_ERRORCODE_H__A17EADC0_A233_400A_A8F4_86A333698154__INCLUDED_)
#define AFX_ERRORCODE_H__A17EADC0_A233_400A_A8F4_86A333698154__INCLUDED_

enum ERROR_CODE
{
	ERR_T_FILE_NO_FOUND = 1,
	ERR_T_LOAD_MODULE_FAILED,
	ERR_T_MODULE_UNCORRECT,
	ERR_T_MODULE_INIT_FAILED,
	ERR_T_REPRESENT2_INIT_FAILED,
	ERR_T_REPRESENT3_INIT_FAILED,
};

void Error_Box();
void Error_SetErrorCode(unsigned int uCode);
void Error_SetErrorString(const char* pcszString);

#endif // !defined(AFX_ERRORCODE_H__A17EADC0_A233_400A_A8F4_86A333698154__INCLUDED_)
