//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   CRC32.H
//  Creater     :   Freeway Chen
//  Date        :   2000-6-15 18:25:43
//  Comment     :   Get source code from Zlib Project
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef CRC32_H
#define CRC32_H

// most CRC first value is 0
extern "C" unsigned CRC32(unsigned CRC, const void *pvBuf, unsigned uLen);
extern "C" inline unsigned CRC32_16BYTES(unsigned CRC, const void *pvBuf);
extern "C" inline unsigned CRC32_48BYTES(unsigned CRC, const void *pvBuf);


#endif  // CRC32_H