/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SignVfy.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Digital Signature Verification Shared Library
 *
 *****************************************************************************/

#ifndef __SIGNVFY_H__
#define __SIGNVFY_H__

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <DataMgr.h>
#include <CPMLib.h>

/******************************************************************
 * Public API Definition
 *
 * The signature verification shared library does the heavy 
 * lifting for the following tasks
 *
 *  -> Interpreting the sign resource in an applications prc.
 *  -> Extract the X.509 certificate block from the sign resource
 *  -> Verify the validity of a digital signature in a prc.
 *
 ******************************************************************/

/******************************************************************
 * Library Error Codes
 ******************************************************************/

#define signErrNoSignResource			(signErrorClass | 1)
#define signErrNoCertResource			(signErrorClass | 2)
#define signErrIndexOutOfBounds         (signErrorClass | 3)
#define signErrInvalidSignResource      (signErrorClass | 4)
#define signErrInvalidCertResource		(signErrorClass | 5)
#define signErrInvalidSignatureBlock    (signErrorClass | 6)
#define signErrDigestMismatch           (signErrorClass | 7)
#define signErrNotFound                 (signErrorClass | 8)
#define signErrInvalidParams            (signErrorClass | 9)
#define signErrBufferTooSmall           (signErrorClass | 10)
#define signErrOutOfMemory              (signErrorClass | 11)
#define signErrInvalidResourceInDB      (signErrorClass | 12)

/******************************************************************
 * Entry numbers of exported functions. Must conform to the MDF
 ******************************************************************/
#define   entryNumSignVerifySignatureByIndex    (0)
#define   entryNumSignVerifySignatureByID          (1)
#define   entryNumSignGetNumSignatures             (2)
#define   entryNumSignGetNumCertificates            (3)
#define   entryNumSignGetSignatureByIndex        (4)
#define   entryNumSignGetCertificateByIndex       (5)
#define   entryNumSignGetSignatureByID              (6)
#define   entryNumSignGetCertificateByID             (7)
#define   entryNumSignGetDigest                           (8)
#define   entryNumSignGetOverlayCertIdList         (9)
#define   entryNumSignGetShLibCertIdList             (10)

/******************************************************************
 * Public Structure Definitions
 ******************************************************************/

/**
 * CertificateIDType
 */
typedef uint8_t SignCertificateIDType[20];


/**
 * SignSignatureBlockType
 *
 * Field descriptions
 *      index           - index position  of signature block in the 
 *                        sign  resource
 *      certificateID   - ID of the certificate used to verify this 
 *                        signature
 *      signingDate     - date when the prc file was signed
 **/  
typedef struct {
    uint32_t                  index;
    SignCertificateIDType   certificateID;
    uint32_t                  signingDate;
} SignSignatureBlockType;


/**
 * SignCertificateBlockType
 *
 * Field descriptions
 *      index           - index position of the certificate block in
 *                        in the sign resource
 *      ID              - 20 byte certificate ID
 *      length          - length of data
 *      data            - starting point for certificate data blob
 **/
typedef struct {
    uint16_t				encoding;
    SignCertificateIDType   certificateID;
} SignCertificateBlockType;
 


/******************************************************************
 * Verifying the 'sign' resource
 *
 *  SignVerifySignatureByIndex  - verify the signature in the 
 *                                index position specified
 *  SignVerifySignatureByID     - verify the signature with the 
 *                                specified certificate ID
 *
 ******************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name         : SignVerifySignatureByIndex
 *
 * Description  : Verify the signature block referenced by the 
 *                specified index.  The index is the signature 
 *                blocks position  in the signature block list.
 *
 * Parameters
 *      dbP     - Pointer to an open database.  The database which 
 *                we want to verify signature from (a PRC database)
 *      index   - The index of the certificate to get.  (Its index 
 *                is its position in the certificate block list)
 *
 * Return
 *      errNone - no error, the signature block is valid
 *      signErrNoSignResource - no sign resource exists in the prc file
 *      signErrIndexOutOfBounds - the index requested is out of bounds 
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 *      signErrInvalidSignatureBlock - the signature block is invalid
 *      signErrDigestMismatch - the signed digest does not match the 
 *                             calculated prc digest 
 *
 * Notes        :
 *
 **/
extern status_t SignVerifySignatureByIndex(
        DmOpenRef dbP, 
        uint16_t index); 
 
typedef  status_t (*SignVerifySignatureByIndexPtrType)(
        DmOpenRef dbP, 
        uint16_t index); 

 
/**
 * Name         : SignVerifySignatureByID
 *
 * Description  : Verify the signature block referenced by the 
 *                specified ID.  The ID is the certificate's ID 
 *                used for verification of the digital signature block.
 *
 * Parameters
 *      dbP             - Pointer to an open database.  The database 
 *                        which we want to verify signature from (a 
 *                        PRC database)
 *      certificateID   - The 20 byte ID of the verifying certificate
 *
 * Return
 *      errNone - no error, the signature block is valid
 *      signErrNoSignResource - no sign resource exists in the prc file
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 *      signErrInvalidSignatureBlock - the signature block is invalid
 *      signErrDigestMismatch - the signed digest does not match the 
 *                             calculated prc digest
 *
 * Notes        :
 *
 **/
status_t SignVerifySignatureByID(
        DmOpenRef dbP, 
        const SignCertificateIDType certificateID); 
 
typedef  status_t (*SignVerifySignatureByIDPtrType)(
        DmOpenRef dbP, 
        const SignCertificateIDType certificateID); 

/******************************************************************
 *
 * Interpreting the 'sign' resource
 *
 *  SignGetNumSignatures		- get the number of signatures 
 *                                in the sign resource
 *  SignGetNumCertificates	    - get the number of certificates 
 *                                in the sign resource
 *  SignGetSignatureByIndex     - get a signature block by its 
 *                                index position in the signature 
 *                                block list
 *  SignGetCertificateByIndex   - get a certificate by its index 
 *                                in the certificate list
 *  SignGetSignatureByID		- get a signature block by it's 
 *                                the  ID of the signing certificate 
 *  SignGetCertificateByID	    - get a certificate by its ID 
 *
 ******************************************************************/
 
/**
 * Name         : SignGetNumSignatures
 *
 * Description  : Get the number of signature blocks in the sign 
 *                resource
 *
 * Parameters
 *      dbP       - Pointer to an open database. The database 
 *                  which we want to check signatures of 
 *                  (a PRC database)
 *      sigCountP - The number of signature blocks in the sign
 *                  resource is returned here
 *
 * Return
 *      signErrNoSignResource      - 'sign' resource not 
 *                                  found in PRC
 *      signErrInvalidSignResource - sign resource is
 *                                  malformed
 *
 * Notes        :
 *
 **/
status_t SignGetNumSignatures(
        DmOpenRef dbP, 
        uint16_t *sigCountP);

typedef  status_t (*SignGetNumSignaturesPtrType)(
        DmOpenRef dbP, 
        uint16_t *sigCountP);


/**
 * Name         : SignGetNumCertificates
 *
 * Description  : Get the number of certificates in the sign resource
 *
 * Parameters
 *      dbP       - Pointer to an open database. The database 
 *                  which we want to check signatures of 
 *                  (a PRC database)
 *      certCountP- The number of certificate in the sign
 *                  resource is returned here
 *
 * Return
 *      signErrNoSignResource      - 'sign' resource not 
 *                                  found in PRC
 *      signErrInvalidSignResource - sign resource is
 *                                  malformed
 *
 * Notes        :
 *
 **/
status_t SignGetNumCertificates(
        DmOpenRef dbP, 
        uint16_t *num); 


/**
 * Name         : SignGetSignatureByIndex
 *
 * Description  : Get a signature block structure by its index 
 *                position in the sign resource's signature block list
 *
 * Parameters
 *      dbP             - Pointer to an open database.  The database 
 *                        which we want to get signatures from (a PRC 
 *                        database)
 *      index           - The index of the signature block to get.  
 *                        (Its index is its position in the signature 
 *                        block list)
 *      signatureBlockP - The signature block is returned here, 
 *                        this is meta data about the signature.
 *
 * Return
 *      signErrNoSignResource - no sign resource exists in the prc file
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 *      signErrIndexOutOfBounds - the index requested is out of bounds
 *
 * Notes        :
 *
 **/
status_t SignGetSignatureByIndex(
        DmOpenRef dbP, 
        uint16_t index, 
        SignSignatureBlockType *signatureBlockP);

/**
 * Name         : SignGetCertificateByIndex
 *
 * Description  : Get a certificate by its index position in the 
 *                sign resource's certificate block list.
 *
 * Parameters
 *      dbP                 - Pointer to an open database.  
 *                            The database which we want to get certificates 
 *                            from (a PRC database)
 *      index               - The index of the certificate to get.  (Its 
 *                            index is its position in the certificate 
 *                            block list)
 *      certificateBlockP   - The certificate block in the PRC.  It 
 *                            includes information about the certificate 
 *                            (its ID), and the certificate in X.509 format.
 *
 * Return
 *      signErrNoSignResource - no sign resource exists in the prc files
 *      signErrInvalidSignResource - the sign resource is malformed, or 
 *                                  invalid in some form
 *      signErrIndexOutOfBounds - the index requested is out of bounds
 *
 * Notes        :
 *
 **/
status_t SignGetCertificateByIndex(
        DmOpenRef dbP,
        uint16_t index,
        SignCertificateBlockType *certificateBlockP,
		uint32_t *certificateLength,
		uint8_t *certificateData);


/**
 * Name         : SignGetSignatureByID
 *
 * Description  : Get a signature block by the ID of the certificate 
 *                used for its verification.
 *
 * Parameters
 *      dbP             - Pointer to an open database.  The database 
 *                        which we want to get signatures from (a PRC 
 *                        database)
 *      certificateID   - The 20 byte ID of the verifying certificate
 *      signatureBlockP - The signature block is returned here, this 
 *                        is meta data about the signature.
 *
 * Return
 *      signErrNoSignResource - no sign resource exists in the prc file
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 *      signErrNotFound - could not find a signature with that certificate ID
 *
 * Notes        :
 *
 **/
status_t SignGetSignatureByID(
        DmOpenRef dbP, 
        const SignCertificateIDType certificateID, 
        SignSignatureBlockType *signatureBlockP);


/**
 * Name         : SignGetCertificateByID
 *
 * Description  : Get a certificate by its ID
 *
 * Parameters
 *      dbP                 - Pointer to an open database.  The database 
 *                            which we want to get certificates from (a 
 *                            PRC database)
 *      certificateID       - The 20 byte ID of the certificate
 *      certificateBlockP   - The certificate block in the PRC.  It 
 *                            includes information about the certificate 
 *                            (its ID), and the certificate in X.509 format.
 *
 * Return
 *      signErrNoSignResource - no sign resource exists in the prc file
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 *      signErrNotFound - could not find a signature with that certificate ID
 * 
 * Notes        :
 *
 **/
status_t SignGetCertificateByID(
        DmOpenRef dbP, 
        const SignCertificateIDType certificateID, 
        SignCertificateBlockType *certificateBlockP,
		uint32_t *certificateLength,
		uint8_t *certificateData);

/**
 * Name         : SignGetOverlayCertIdList
 *
 * Description  : Get a list of certificates that are valid for overlay
 *
 * Parameters
 *      dbP                 - Pointer to an open database.  The database 
 *                            which we want to get certificates from (a 
 *                            PRC database)
 *      certIdList          - buffer to place list of cert ids (20 byte each)
 *      certIdListSize      - size in bytes on input list length
 *
 * Return
 *      signErrBufferTooSmall - need larger buffer
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 * 
 * Notes        : a call with a null certIdList or a certIdListSize of 0
 *					will return signErrBufferTooSmall, and an appropriate size
 *
 **/
status_t SignGetOverlayCertIdList(
        DmOpenRef dbP, 
		uint8_t *certIdList,
		uint32_t *certIdListSize);


/**
 * Name         : SignGetShLibCertIdList
 *
 * Description  : Get a list of certificates that are valid for shared libraries
 *
 * Parameters
 *      dbP                 - Pointer to an open database.  The database 
 *                            which we want to get certificates from (a 
 *                            PRC database)
 *      certIdList          - buffer to place list of cert ids (20 byte each)
 *      certIdListSize      - size in bytes on input list length
 *
 * Return
 *      signErrBufferTooSmall - need larger buffer
 *      signErrInvalidSignResource - the sign resource is malformed, 
 *                                  or invalid in some form
 * 
 * Notes        : a call with a null certIdList or a certIdListSize of 0
 *					will return signErrBufferTooSmall, and an appropriate size
 *
 **/
status_t SignGetShLibCertIdList(
        DmOpenRef dbP, 
		uint8_t *certIdList,
		uint32_t *certIdListSize);

typedef  status_t (*SignGetShLibCertIdListPtrType)(
        DmOpenRef dbP, 
		uint8_t *certIdList,
		uint32_t *certIdListSize);


/******************************************************************
 *
 * Utility Functions
 *
 *  SignGetDigest				- Get the digest of a PRC
 *
 ******************************************************************/
 
/**
 * Name         : SignGetDigest
 *
 * Description  : Get the digest of a PRC
 *
 * Parameters
 *      dbP                 - Pointer to an open database.  The database 
 *                            which we want to get the digest of.
 *							  (Must be resource database)
 *      hashinfo       		- Initialized CPM hash structure
 *
 * Return
 * 
 * Notes        :
 *		Caller must still call CPMLibHashFinal to get the final result.
 *
 */
status_t SignGetDigest(
	DmOpenRef dbP,
	APHashInfoType *hashinfo);

#ifdef __cplusplus 
}
#endif


#endif /* Digital Signature Verification Shared Library */
