/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Encrypt.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *    Equates for encryption/digestion routines in pilot
 *
 *****************************************************************************/

#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__

// Include elementary types
#include <PalmTypes.h>          // Basic types
#include <CmnErrors.h>


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// Perform reversible encryption or decryption of 8 byte string in
//  srcP using 8 byte key keyP. Place 8 byte result in dstP.
status_t EncDES(uint8_t *srcP, uint8_t *keyP, uint8_t *dstP, Boolean encrypt);



// Digest a string of bytes and produce a 128 bit result using
//   the MD4 algorithm.
status_t EncDigestMD4(uint8_t *strP, uint16_t strLen, uint8_t digestP[16]);



// Digest a string of bytes and produce a 128 bit result using
//   the MD5 algorithm.
status_t EncDigestMD5(uint8_t *strP, uint16_t strLen, uint8_t digestP[16]);
#ifdef __cplusplus 
}
#endif
#endif  //__ENCRYPT_H__
