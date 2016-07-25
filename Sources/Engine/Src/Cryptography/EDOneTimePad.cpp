/*****************************************************************************************
//	译码与解码--“一次一密码乱本(one-time pad)”的变体
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-22
*****************************************************************************************/
#include "KWin32.h"
#include "memory.h"

//--------------------------------------------------------------------------
//	功能：一次一密码乱本译码函数
//--------------------------------------------------------------------------
extern "C" ENGINE_API
void EDOneTimePad_Encipher(char* pPlaintext, int nPlainLen)
{
	if (pPlaintext && nPlainLen > 0)
	{	
		unsigned char	*pContent = (unsigned char*)pPlaintext;
		#define	NUM_FIX_CIPHER_CHARA	3
		unsigned char cCipher[NUM_FIX_CIPHER_CHARA + NUM_FIX_CIPHER_CHARA] = { 'W', 'o', 'o', 'y', 'u', 'e'};
		if (nPlainLen > NUM_FIX_CIPHER_CHARA)
		{
			cCipher[0] = pContent[nPlainLen - 1] % 4;
			if (cCipher[0])
				cCipher[0] = cCipher[cCipher[0] + 2];
			else
				cCipher[0] = pContent[nPlainLen - 1];

			if (nPlainLen > NUM_FIX_CIPHER_CHARA + 1)
			{
				cCipher[1] = 'l';
				if (nPlainLen > NUM_FIX_CIPHER_CHARA + 2)
				{
					cCipher[2] = pContent[nPlainLen - 2];
					if (nPlainLen > NUM_FIX_CIPHER_CHARA + 3)
						cCipher[1] =pContent[nPlainLen - 3];
				}
			}
		}
		for (int nPos = 0; nPos < nPlainLen; nPos++)
		{
			pContent[nPos] = (pContent[nPos] - 0x20 + (0xff - cCipher[cCipher[0] % 3])) % 0xdf + 0x20;
			cCipher[0] = cCipher[1];	cCipher[1] = cCipher[2];
			cCipher[2] = pContent[nPos];
		}
	}
}

//--------------------------------------------------------------------------
//	功能：一次一密码乱本解码函数
//--------------------------------------------------------------------------
extern "C" ENGINE_API
void EDOneTimePad_Decipher(char* pCiphertext, int nCiphertextLen)
{
	if (pCiphertext && nCiphertextLen > 0)
	{
		unsigned char* pContent = (unsigned char*)pCiphertext;
		#define	NUM_FIX_CIPHER_CHARA	3
		unsigned char cCipher[NUM_FIX_CIPHER_CHARA + NUM_FIX_CIPHER_CHARA] = { 'W', 'o', 'o', 'y', 'u', 'e'};
		int nPos;
		for (nPos = nCiphertextLen - 1; nPos >= NUM_FIX_CIPHER_CHARA; nPos--)
			pContent[nPos] = (pContent[nPos] - 0x20 + (pContent[nPos - 3 + (pContent[nPos - 3] % 3)] - 0x20)) % 0xdf + 0x20;
		if (nCiphertextLen > NUM_FIX_CIPHER_CHARA)
		{
			cCipher[0] = pContent[nCiphertextLen - 1] % 4;
			if (cCipher[0])
				cCipher[0] = cCipher[cCipher[0] + 2];
			else
				cCipher[0] = pContent[nCiphertextLen - 1];
			
			if (nCiphertextLen > NUM_FIX_CIPHER_CHARA + 1)
			{
				cCipher[1] = 'l';
				if (nCiphertextLen > NUM_FIX_CIPHER_CHARA + 2)
				{
					cCipher[2] = pContent[nCiphertextLen - 2];
					if (nCiphertextLen > NUM_FIX_CIPHER_CHARA + 3)
						cCipher[1] =pContent[nCiphertextLen - 3];
				}
			}
		}

		if (nPos)
			memcpy(cCipher + NUM_FIX_CIPHER_CHARA, pContent, nPos);

		for (;nPos >= 0; nPos--)
		{
			pContent[nPos] = (pContent[nPos] - 0x20 + (cCipher[nPos + (cCipher[nPos] % 3)] - 0x20)) % 0xdf + 0x20;
		}
	}
}
