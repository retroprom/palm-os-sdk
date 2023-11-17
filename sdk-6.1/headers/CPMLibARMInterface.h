/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CPMLibARMInterface.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _CPMLIB_ARM_INTERFACE_H
#define _CPMLIB_ARM_INTERFACE_H
#include "CPMLibCommon.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Library initialization and shutdown
 */
status_t CPMLibOpen(uint16_t *numProviders);
status_t CPMLibClose(void);
status_t CPMLibSleep(void);
status_t CPMLibWake(void);

/* 
 * Library settings
 */
status_t CPMLibGetInfo(CPMInfoType *infoP);
status_t CPMLibSetDebugLevel(uint8_t debugLevel);

/* 
 * Provider information
 */
status_t CPMLibEnumerateProviders(uint32_t providerIDs[], uint16_t *numProviders);
status_t CPMLibGetProviderInfo(uint32_t providerID, APProviderInfoType *providerInfoP);
status_t CPMLibSetDefaultProvider(uint32_t providerID);

/*
 * Random bytes
 */
status_t CPMLibGenerateRandomBytes(uint8_t *bufferP, uint32_t *bufLenP);
status_t CPMLibAddRandomSeed(uint8_t *seedDataP, uint32_t dataLen);

/*
 * Symmetric key generation operations
 */
status_t CPMLibGenerateKey(uint8_t *keyDataP, uint32_t dataLen, APKeyInfoType *keyInfoP);
status_t CPMLibImportKeyInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APKeyInfoType *keyInfoP);
status_t CPMLibExportKeyInfo(APKeyInfoType *keyInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseKeyInfo(APKeyInfoType *keyInfoP);

/*
 * Asymmetric key generation operations
 */
status_t CPMLibGenerateKeyPair(uint8_t *keyDataP, uint32_t dataLen, APKeyInfoType *privateKeyInfoP, APKeyInfoType *publicKeyInfoP);
status_t CPMLibImportKeyPairInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APKeyInfoType *privateKeyInfoP, APKeyInfoType *publicKeyInfoP);
status_t CPMLibExportKeyPairInfo(APKeyInfoType *privateKeyInfoP, APKeyInfoType *publicKeyInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);

/*
 * Derived key generation operations (PBE)
 */
status_t CPMLibDeriveKeyData(APDerivedKeyInfoType *derivedKeyInfoP, uint8_t *keyDataP, uint32_t *dataLen);

/*
 * Message digest operations
 */
status_t CPMLibHashInit(APHashInfoType *hashinfo);
status_t CPMLibHashUpdate(APHashInfoType *hashinfo, uint8_t *bufIn, uint32_t bufInLen);
status_t CPMLibHashFinal(APHashInfoType *hashinfo, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibHash(APHashEnum type, APHashInfoType *hashinfo, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibImportHashInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APHashInfoType *hashInfoP);
status_t CPMLibExportHashInfo(APHashInfoType *hashInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseHashInfo(APHashInfoType *hashInfoP);

/*
 * Cipher operations
 */
status_t CPMLibEncryptInit(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP);
status_t CPMLibEncryptUpdate(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibEncryptFinal(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibEncrypt(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibDecryptInit(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP);
status_t CPMLibDecryptUpdate(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibDecryptFinal(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibDecrypt(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibImportCipherInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APCipherInfoType *cipherInfoP);
status_t CPMLibExportCipherInfo(APCipherInfoType *cipherInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseCipherInfo(APCipherInfoType *cipherInfoP);

/*
 * MAC operations
 */
status_t CPMLibMACInit(APKeyInfoType *keyInfoP, APHashInfoType *hashInfoP, APMACInfoType *macInfoP);
status_t CPMLibMACUpdate(APMACInfoType *macInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibMACFinal(APMACInfoType *macInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibMAC(APKeyInfoType *keyInfoP, APHashInfoType *hashInfoP, APMACEnum type, APMACInfoType *macInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP);
status_t CPMLibImportMACInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APMACInfoType *macInfoP);
status_t CPMLibExportMACInfo(APMACInfoType *macInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseMACInfo(APMACInfoType *macInfoP);

/*
 * Verification operations
 */
status_t CPMLibVerifyInit(APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP);
status_t CPMLibVerifyUpdate(APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, uint8_t *bufIn, uint32_t bufInLen);
status_t CPMLibVerifyFinal(APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP, uint8_t *signature, uint32_t signatureLen, VerifyResultType *verifyResultP);
status_t CPMLibVerify(APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP, uint8_t *signature, uint32_t signatureLen, VerifyResultType *verifyResultP);
status_t CPMLibImportVerifyInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APVerifyInfoType *verifyInfoP);
status_t CPMLibExportVerifyInfo(APVerifyInfoType *verifyInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseVerifyInfo(APVerifyInfoType *verifyInfoP);

/*
 * Signing operations
 */
status_t CPMLibSignInit(APKeyInfoType *keyInfoP, APSignInfoType *signInfoP);
status_t CPMLibSignUpdate(APKeyInfoType *keyInfoP, APSignInfoType *signInfoP, uint8_t *bufIn, uint32_t bufInLen);
status_t CPMLibSignFinal(APKeyInfoType *keyInfoP, APSignInfoType *signInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP, uint8_t *signature, uint32_t *signatureLenP);
status_t CPMLibSign(APKeyInfoType *keyInfoP, APSignInfoType *signInfoP, uint8_t *bufIn, uint32_t bufInLen, uint8_t *bufOut, uint32_t *bufOutLenP, uint8_t *signature, uint32_t *signatureLenP);
status_t CPMLibImportSignInfo(uint8_t encoding, uint8_t *importDataP, uint32_t dataLen, APSignInfoType *signInfoP);
status_t CPMLibExportSignInfo(APSignInfoType *signInfoP, uint8_t encoding, uint8_t *exportDataP, uint32_t *dataLenP);
status_t CPMLibReleaseSignInfo(APSignInfoType *signInfoP);

#ifdef  __cplusplus
}
#endif

#endif /* _CPMLIB_ARM_INTERFACE_H */
