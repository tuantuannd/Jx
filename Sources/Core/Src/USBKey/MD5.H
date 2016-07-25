#ifndef	MD5_H_INC
#define MD5_H_INC

#ifdef	__cplusplus
extern "C" {
#endif

/*{{{ MD5 Wrapper functions.*/
unsigned char* __stdcall
MD5_Hash(unsigned char* pucData,unsigned long ulLength,unsigned char* pucDigest);

unsigned char *__stdcall
MD5_HMAC(unsigned char * pucText,        /* pointer to data stream        */
		 unsigned long   ulText_Len,     /* length of data stream         */
		 unsigned char * pucKey,         /* pointer to authentication key */
		 unsigned long   ulKey_Len,      /* length of authentication key  */
		 unsigned char * pucKey1,        /* inner hashed key (optional)   */
		 unsigned char * pucKey2,        /* outer hashed key (optional)   */
		 unsigned char * pucDigest );    /* caller digest to be filled in */
/*}}}*/

#ifdef __cplusplus
}
#endif

#endif	//	MD5_H_INC