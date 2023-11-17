/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CPMLibProvider.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef CPMLIB_PROVIDER_H
#define CPMLIB_PROVIDER_H

#include <CPMLibCommon.h>

/** @def cpmProviderResourceID
 * defines the resource ID of the code resource
 */
#define cpmProviderResourceID 0

/** @def cpmProviderResourceType
 * defines the type (creator/type) of the provider
 */
#define cpmProviderResourceType 'cpmp'

/** @enum APCmdType
 *
 * Algorithm provider command enumerations
 */
typedef enum {
  apOpen,
  apClose,
  apStatus,
  apZero,
  apGetInfo,
  apGetProviderInfo,

  apGenerateKey,
  apImportKeyInfo,
  apExportKeyInfo,
  apReleaseKeyInfo,

  apGenerateKeyPair,
  apImportKeyPairInfo,
  apExportKeyPairInfo,

  apDeriveKeyData,

  apHashInit,
  apHashUpdate, 
  apHashFinal,
  apHash,
  apImportHashInfo,
  apExportHashInfo,
  apReleaseHashInfo,

  apMACInit,
  apMACUpdate,
  apMACFinal,
  apMAC,
  apImportMACInfo,
  apExportMACInfo,
  apReleaseMACInfo,

  apEncryptInit, 
  apEncryptUpdate, 
  apEncryptFinal, 
  apEncrypt,

  apDecryptInit,
  apDecryptUpdate,
  apDecryptFinal, 
  apDecrypt,

  apImportCipherInfo,
  apExportCipherInfo,
  apReleaseCipherInfo,

  apSignInit, 
  apSignUpdate, 
  apSignFinal, 
  apSign,
  apImportSignInfo,
  apExportSignInfo,
  apReleaseSignInfo,

  apVerifyInit, 
  apVerifyUpdate, 
  apVerifyFinal, 
  apVerify,
  apImportVerifyInfo,
  apExportVerifyInfo,
  apReleaseVerifyInfo,
  apLast
} APCmdType;


/** @union APCmdPBType
 * Algorithm Provider Interface Command Parameter Blocks
 */
typedef union APCmdPBType {
#if 0
  /** @struct APGetInfo
   * @param info (output) pointer to the CPMInfoType structure
   *
   * The command paramter blcck for the GetInfo call. The
   * <code>info</code> parameter is filled in with the appropriate
   * information. This call is not passed to the provider and should be
   * handled by the CPM library.
   */
  struct {
    CPMInfoType *infoP;
  } APGetInfo;
#endif
  /** @struct APGetProviderInfo 
   * @param info (output) pointer to the APProviderInfoType structure
   * 
   * The command paramater blcok for the GetProviderInfo call. The
   * <code>info</code> parameter is filled in with appropriate
   * information. The provider is responsible for filling the correct
   * information.
   */
  struct {
    APProviderInfoType *infoP;
  } APGetProviderInfo;

  /** @struct APGenerateKey 
   * @param type (input) the type of algorithm for which this key will
   * be used
   * @param keyinfo (input/output) an opaque handle to the key generated
   * by the provider. 
   * 
   * The provider generates a key of the specified @c type and returns
   * an opaque handle the key in @c key. 
   */
  struct {
    uint8_t *keyDataP;
    uint32_t dataLen;
    APKeyInfoType *keyInfoP;
  } APGenerateKey;

  /** @struct APImportKey 
   * @param encoding (input) the encoding of the import data
   * @param keydata (input) the data for the import operation
   * @param keyinfo (input/output) an opaque handle to the key generated
   * by the provider. 
   * 
   * The provider imports a key as specified by the data in @c keydata
   * and returns an opaque handle the key in @c key. 
   */
  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APKeyInfoType *keyInfoP;
  } APImportKeyInfo;

  /** @struct APExportKey 
   * @param encoding (input) the encoding desired for the exported data
   * @param keydata (input) the data for the import operation
   * @param keyinfo (input/output) an opaque handle to the key generated
   * by the provider. 
   * 
   * The provider imports a key as specified by the data in @c keydata
   * and returns an opaque handle the key in @c key. 
   *
   * @todo Match these parameters to export
   */
  struct {
    uint8_t encoding;
    APKeyInfoType *keyInfoP;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
  } APExportKeyInfo;

  /** @struct APReleaseKey 
   * @param keyinfo (input/output) an opaque handle to the key generated
   * by the provider. 
   * 
   * The provider releases the key specified by the opaque handle @c
   * keyinfo. After this call, @c keyinfo is no longer valid for any operations. 
   */
  struct {
    APKeyInfoType *keyInfoP;
  } APReleaseKeyInfo;

  struct {
    uint8_t *keyDataP;
    uint32_t dataLen;
    APKeyInfoType *privateKeyInfoP;
    APKeyInfoType *publicKeyInfoP;
  } APGenerateKeyPair;

  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APKeyInfoType *privateKeyInfoP;
    APKeyInfoType *publicKeyInfoP;
  } APImportKeyPairInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APKeyInfoType *privateKeyInfoP;
    APKeyInfoType *publicKeyInfoP;
  } APExportKeyPairInfo;

  struct {
    APKeyDerivationEnum kdType;
    APKeyDerivationUsageEnum kdUsage;
    uint8_t *saltP;
    uint32_t saltLen;
    uint32_t iterationCount;
    void *kdInfo;
    uint8_t *keyDataP;
    uint32_t *keyDataLenP;
  } APDeriveKeyData;

  /** @struct APHashInit
   * @param type (input) the type of hash requested
   * @param hashinfo (input/output) a pointer to a hash info structure
   * to pass to the other members of the Hash functions.
   *
   * The provider is responsible for initializing a context of the
   * appropriate @c type for the operation and saving it in @c
   * hashinfo. This Init operation must be concluded with a single Final
   * operation with zero or more Update operations in between. 
   */
  struct {
    APHashInfoType *hashInfoP;
  } APHashInit;

  /** @struct APHashUpdate
   * @param hashinfo (input) a valid context for this operation initialized by the Init operation
   * @param buffIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this operation
   *
   * The @c hashinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. The provider
   * uses the @c hashinfo to update the operation with the data in @c buffer of
   * length @c buflen. The provider also updates the @c hashinfo to reflect
   * the Update. Any number of Update operations (including zero) may
   * occur between an Init and a Final operation. The parameters @c
   * buffer and @c buflen must be valid for the operation or an error is
   * returned. 
   */
  struct {
    APHashInfoType *hashInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
  } APHashUpdate;

  /** @struct APHashFinal
   * @param hashinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the buffer where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the buffer specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c hashinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * If the @c bufInLen is non-zero, the provider uses the @c hashinfo to
   * update the operation with the data in @c bufIn of length @c
   * bufInLen.
   *
   * The provider finalizes the operation which was initialized by the
   * Init operation and places the results of the operation in @c
   * bufOut. The provider updates @c bufOutLenP with the actual number
   * of bytes placed in @c bufOut. The provider is responsible for
   * cleaning up the @c hashinfo parameter. 
   */
  struct {
    APHashInfoType *hashInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APHashFinal;

  /** @struct APHash
   * @param type (input) the type of hash requested for this operation
   * @param hashinfo (input/output) the hash context information for this operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c hashinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * The parameter @c bufInLen may not be zero and @c bufIn must be valid.
   *
   * The provider performs a complete operation of @c type using the
   * data in @c bufIn of length @c bufInLen and puts the results in @c
   * bufOut. The actual number of bytes placed in @c bufOut is placed in
   * @c bufOutLenP. 
   */
  struct {
    APHashEnum type;
    APHashInfoType *hashInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APHash;

  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APHashInfoType *hashInfoP;
  } APImportHashInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APHashInfoType *hashInfoP;
  } APExportHashInfo;

  struct {
    APHashInfoType *hashInfoP;
  } APReleaseHashInfo;

  struct {
    APKeyInfoType keyInfoP;
    APHashInfoType hashInfoP;
    APMACInfoType *macInfoP;
  } APMacInit;

  struct {
    APMACInfoType *macInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APMacUpdate;

  struct {
    APMACInfoType *macInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APMacFinal;

  struct {
    APKeyInfoType *keyInfoP;
    APHashInfoType *hashInfoP;
    APMACEnum type;
    APMACInfoType *macInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APMac;

  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APMACInfoType *macInfoP;
  } APImportMACInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APMACInfoType *macInfoP;
  } APExportMACInfo;

  struct {
    APMACInfoType *macInfoP;
  } APReleaseMACInfo;

  /** @struct APEncryptInit
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input/output) a handle to a context to pass to the
   * other members of the Encrypt functions.
   *
   * The provider is responsible for initializing a context of the
   * appropriate @c type with the specified @c key for the operation
   * and saving it in @c cipherinfo. This Init operation must be concluded
   * with a single Final operation with zero or more Update operations
   * in between.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
  } APEncryptInit;

  /** @struct APEncryptUpdate
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this operation
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. The provider
   * uses the @c cipherinfo to update the operation with the data in @c bufIn of
   * length @c bufInLen. The provider also updates the @c cipherinfo to reflect
   * the Update. Any number of Update operations (including zero) may
   * occur between an Init and a Final operation. The parameters @c
   * bufIn and @c bufInLen must be valid for the operation or an error is
   * returned. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APEncryptUpdate;

  /** @struct APEncryptFinal
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * If the @c bufInLen is non-zero, the provider uses the @c cipherinfo to
   * update the operation with the data in @c bufIn of length @c
   * bufInLen.
   *
   * The provider finalizes the operation which was initialized by the
   * Init operation and places the results of the operation in @c
   * bufOut. The provider updates @c bufOutLenP with the actual number
   * of bytes placed in @c bufOut. The provider is responsible for
   * cleaning up the @c cipherinfo parameter. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APEncryptFinal;

  /** @struct APEncrypt
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input/output) the context information for this operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * The parameter @c bufInLen may not be zero and @c bufIn must be valid.
   *
   * The provider performs a complete operation of @c type using the
   * data in @c bufIn of length @c bufInLen and puts the results in @c
   * bufOut. The actual number of bytes placed in @c bufOut is placed in
   * @c bufOutLenP. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APEncrypt;

  /** @struct APDecryptInit
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input/output) a handle to a context to pass to the
   * other members of the Decrypt functions.
   *
   * The provider is responsible for initializing a context of the
   * appropriate @c type with the specified @c keyinfo for the operation
   * and saving it in @c cipherinfo. This Init operation must be concluded
   * with a single Final operation with zero or more Update operations
   * in between.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
  } APDecryptInit;

  /** @struct APDecryptUpdate
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this operation
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. The provider
   * uses the @c cipherinfo to update the operation with the data in @c bufIn of
   * length @c bufInLen. The provider also updates the @c cipherinfo to reflect
   * the Update. Any number of Update operations (including zero) may
   * occur between an Init and a Final operation. The parameters @c
   * bufIn and @c bufInLen must be valid for the operation or an error is
   * returned. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APDecryptUpdate;

  /** @struct APDecryptFinal
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * If the @c bufInLen is non-zero, the provider uses the @c cipherinfo to
   * update the operation with the data in @c bufIn of length @c
   * bufInLen.
   *
   * The provider finalizes the operation which was initialized by the
   * Init operation and places the results of the operation in @c
   * bufOut. The provider updates @c bufOutLenP with the actual number
   * of bytes placed in @c bufOut. The provider is responsible for
   * cleaning up the @c cipherinfo parameter.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APCipherInfoType *cipherInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
  } APDecryptFinal;

  /** @struct APDecrypt
   * @param type (input) the type of algorithm requested for this
   * operation
   * @param keyinfo (input) the key to be used for the operation
   * @param cipherinfo (input/output) the context information forthis operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c cipherinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * The parameter @c bufInLen may not be zero and @c bufIn must be valid.
   *
   * The provider performs a complete operation of @c type using the
   * data in @c bufIn of length @c bufInLen and puts the results in @c
   * bufOut. The actual number of bytes placed in @c bufOut is placed in
   * @c bufOutLenP. 
   */ 
 struct {
   APKeyInfoType *keyInfoP;
   APCipherInfoType *cipherInfoP;
   uint8_t *bufIn;
   uint32_t bufInLen;
   uint8_t *bufOut;
   uint32_t *bufOutLenP;
  } APDecrypt;

  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APCipherInfoType *cipherInfoP;
  } APImportCipherInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APCipherInfoType *cipherInfoP;
  } APExportCipherInfo;

  struct {
    APCipherInfoType *cipherInfoP;
  } APReleaseCipherInfo;

  /** @struct APSignInit
   * @param keyinfo (input) the key to be used for the operation
   * @param signinfo (input/output) a handle to a context to pass to the
   * other members of the Sign functions.
   *
   * The provider is responsible for initializing a context 
   * with the  specified @c keyinfo for the operation
   * and saving it in @c signinfo. This Init operation must be concluded
   * with a single Final operation with zero or more Update operations
   * in between.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APSignInfoType *signInfoP;
  } APSignInit;

  /** @struct APSignUpdate
   * @param keyinfo (input) the key to be used for the operation
   * @param signinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this operation
   *
   * The @c signinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. The provider
   * uses the @c signinfo to update the operation with the data in @c bufIn of
   * length @c bufInLen. The provider also updates the @c signinfo to reflect
   * the Update. Any number of Update operations (including zero) may
   * occur between an Init and a Final operation. The parameters @c
   * bufIn and @c bufInLen must be valid for the operation or an error is
   * returned. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APSignInfoType *signInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
  } APSignUpdate;

  /** @struct APSignFinal
   * @param keyinfo (input) the key to be used for the operation
   * @param signinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c signinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * If the @c bufInLen is non-zero, the provider uses the @c signinfo to
   * update the operation with the data in @c bufIn of length @c
   * bufInLen.
   *
   * The provider finalizes the operation which was initialized by the
   * Init operation and places the results of the operation in @c
   * bufOut. The provider updates @c bufOutLenP with the actual number
   * of bytes placed in @c bufOut. The provider is responsible for
   * cleaning up the @c signinfo parameter.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APSignInfoType *signInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
    uint8_t *signature;
    uint32_t *signatureLenP;
  } APSignFinal;

  /** @struct APSign
   * @param keyinfo (input) the key to be used for the operation
   * @param signinfo (input/output) the context information for this operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c signinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * The parameter @c bufInLen may not be zero and @c bufIn must be valid.
   *
   * The provider performs a complete operation using the
   * data in @c bufIn of length @c bufInLen and puts the results in @c
   * bufOut. The actual number of bytes placed in @c bufOut is placed in
   * @c bufOutLenP. 
   */ 
  struct {
    APKeyInfoType *keyInfoP;
    APSignInfoType *signInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
    uint8_t *signature;
    uint32_t *signatureLenP;
  } APSign;

  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APSignInfoType *signInfoP;
  } APImportSignInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APSignInfoType *signInfoP;
  } APExportSignInfo;

  struct {
    APSignInfoType *signInfoP;
  } APReleaseSignInfo;

  /** @struct APVerifyInit
   * @param keyinfo (input) the key to be used for the operation
   * @param verifyinfo (input/output) a handle to a context to pass to the
   * other members of the Verify functions.
   *
   * The provider is responsible for initializing a context 
   * with the specified @c keyinfo for the operation
   * and saving it in @c verifyinfo. This Init operation must be concluded
   * with a single Final operation with zero or more Update operations
   * in between.
   */
  struct {
    APKeyInfoType *keyInfoP;
    APVerifyInfoType *verifyInfoP;
  } APVerifyInit;

  /** @struct APVerifyUpdate
   * @param keyinfo (input) the key to be used for the operation
   * @param verifyinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this operation
   *
   * The @c verifyinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. The provider
   * uses the @c verifyinfo to update the operation with the data in @c bufIn of
   * length @c bufInLen. The provider also updates the @c verifyinfo to reflect
   * the Update. Any number of Update operations (including zero) may
   * occur between an Init and a Final operation. The parameters @c
   * bufIn and @c bufInLen must be valid for the operation or an error is
   * returned. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APVerifyInfoType *verifyInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
  } APVerifyUpdate;

  /** @struct APVerifyFinal
   * @param keyinfo (input) the key to be used for the operation
   * @param verifyinfo (input) a valid context for this operation initialized by the Init operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c verifyinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * If the @c bufInLen is non-zero, the provider uses the @c verifyinfo to
   * update the operation with the data in @c bufIn of length @c
   * bufInLen.
   *
   * The provider finalizes the operation which was initialized by the
   * Init operation and places the results of the operation in @c
   * bufOut. The provider updates @c bufOutLenP with the actual number
   * of bytes placed in @c bufOut. The provider is responsible for
   * cleaning up the @c verifyinfo parameter. 
   */
  struct {
    APKeyInfoType *keyInfoP;
    APVerifyInfoType *verifyInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP;
    uint8_t *signature;
    uint32_t signatureLen;
    VerifyResultType *verifyResultP;
  } APVerifyFinal;

  /** @struct APVerify
   * @param keyinfo (input) the key to be used for the operation
   * @param verifyinfo (input/output) the context information for this operation
   * @param bufIn (input) the data to add to the operation
   * @param bufInLen (input) the length of the data to add to this
   * operation
   * @param bufOut (input/output) the bufIn where the results of the
   * operation are to be placed. 
   * @param bufOutLenP (input/output) the size of the bufIn specified
   * by the @c bufOut parameter. Also returns the actual number of bytes
   * placed in @c bufOut on return
   *
   * The @c verifyinfo parameter specifies an initialized context which was
   * created in the Init of this class of functions. 
   * 
   * The parameter @c bufInLen may not be zero and @c bufIn must be valid.
   *
   * The provider performs a complete operation of @c type using the
   * data in @c bufIn of length @c bufInLen and puts the results in @c
   * bufOut. The actual number of bytes placed in @c bufOut is placed in
   * @c bufOutLenP. 
   */ 
  struct {
    APKeyInfoType *keyInfoP;
    APVerifyInfoType *verifyInfoP;
    uint8_t *bufIn;
    uint32_t bufInLen;
    uint8_t *bufOut;
    uint32_t *bufOutLenP; 
    uint8_t *signature;
    uint32_t signatureLen;
    VerifyResultType *verifyResultP;
 } APVerify;
  
  struct {
    uint8_t encoding;
    uint8_t *importDataP;
    uint32_t dataLen;
    APVerifyInfoType *verifyInfoP;
  } APImportVerifyInfo;

  struct {
    uint8_t encoding;
    uint8_t *exportDataP;
    uint32_t *dataLenP;
    APVerifyInfoType *verifyInfoP;
  } APExportVerifyInfo;

  struct {
    APVerifyInfoType *verifyInfoP;
  } APReleaseVerifyInfo;

} APCmdPBType, *APCmdPBPtr;

/*
 * #if BUILD_TYPE == BUILD_TYPE_DEBUG
 * #else
 * figure this one out
 * #endif
 */
typedef status_t (*CPMDebugOutProcPtr)(void *appContext, uint8_t level, char *fmtstring, ...);
typedef status_t (*CPMDispatcherProcPtr)(void *appContext, APCmdType cmd, APCmdPBType *cmdPB, uint32_t *id);
typedef status_t (*CPMGenerateRandomBytesProcPtr)(void *appContext, uint8_t *buffer, uint32_t *buflenP);
typedef status_t (*CPMAddRandomSeedProcPtr)(void *appContext, uint8_t *buffer, uint32_t buflen);

/** @struct CPMCallerInfoType
 * 
 * The CPMCallerInfoType structure is passed to the provider's
 * dispatcher. The CPMCallerInfoType contains all the information 
 * necessary for the provider to call back into the CPM's framework and
 * use the CPM's resources. 
 */
typedef struct _CPMCallerInfoType {
  void *appContext;
  CPMDebugOutProcPtr debugout;
  CPMDispatcherProcPtr dispatcher;
  CPMGenerateRandomBytesProcPtr generateRandom;
  CPMAddRandomSeedProcPtr addSeed;
} CPMCallerInfoType;

typedef CPMCallerInfoType *CPMCallerInfoPtr;

/** @typedef APDispatchProcPtr
 * 
 * Each provider must export a function matching this signature as a
 * dispatcher for the commands and command parameter blocks that will be
 * sent to the provider by the CPM. 
 */
typedef status_t (*APDispatchProcPtr)(CPMCallerInfoPtr info, APCmdType cmd, APCmdPBPtr pbP);

#endif /* CPMLIB_PROVIDER_H */
