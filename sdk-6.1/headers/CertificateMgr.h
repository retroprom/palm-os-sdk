/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: CertificateMgr.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _CERT_MANAGER_H_
#define _CERT_MANAGER_H_

#include <PalmTypes.h>
#include <MemoryMgr.h>
#include <DateTime.h>

#define NON_PORTABLE
#include "CPMLib.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define CertMgrServiceName      "pSysCertificateManager"

/* A certificate (and other things) is a collection of CertMgrLibElement's.
     The CertMgrLibElement's are collected into an CertMgrLibElementList.
 */
typedef struct
{
  uint16_t type;
  uint16_t field;
  uint16_t dataType;
  uint16_t length;
  uint32_t offset;
} CertMgrElementType;

typedef struct
{
  uint32_t length;
  uint32_t count;
  CertMgrElementType element[1];
} CertMgrElementListType;

typedef uint32_t CertMgrCertElementEnum;

/* In a CertMgrElementType, the "type" can be one of the following.
 */
#define apCertMgrElementTypeX509Cert          32
#define apCertMgrElementTypeRSA               33
#define apCertMgrElementTypeRDN               34
#define apCertMgrElementTypeX509Extensions    35

/* In a CertMgrElementType, the "field" can be one of the following.
 */

/* This group is associated with apCertMgrElementTypeX509Cert.
 */
#define apCertMgrElementFieldEntireCert       17
#define apCertMgrElementFieldInnerDER         1
#define apCertMgrElementFieldVersion          2
#define apCertMgrElementFieldSerialNumber     3
#define apCertMgrElementFieldIssuerRDN        4
#define apCertMgrElementFieldSubjectRDN       5
#define apCertMgrElementFieldNotBefore        6
#define apCertMgrElementFieldNotAfter         7
#define apCertMgrElementFieldPubKeyBER        8
#define apCertMgrElementFieldSigAlgID         91
#define apCertMgrElementFieldSigOID           12
#define apCertMgrElementFieldSigParams        13
#define apCertMgrElementFieldSignature        14
#define apCertMgrElementFieldIssuerID         15
#define apCertMgrElementFieldSubjectID        16
#define apCertMgrElementFieldIssuerUniqueID   21
#define apCertMgrElementFieldSubjectUniqueID  22
#define apCertMgrElementFieldExtensions       23
#define apCertMgrElementFieldExtension        200

/* This group is associated with apCertMgrElementTypeRSA.
 */
#define apCertMgrElementFieldRSAModulus  16
#define apCertMgrElementFieldRSAPubExpo  17

/* This group is associated with apCertMgrElementTypeRDN.
 */
#define apCertMgrElementFieldRDNOID       4
#define apCertMgrElementFieldRDNValue     5
#define apCertMgrElementFieldRDNOIDN(n)   (apCertMgrElementFieldRDNOID+(n)*4)
#define apCertMgrElementFieldRDNValueN(n) (apCertMgrElementFieldRDNValue+(n)*4)

/* This group is associated with apCertMgrElementTypeX509Extensions.
 */
#define apCertMgrElementFieldX509ExOID          0 /* First extension OID */
#define apCertMgrElementFieldX509ExCritical     1 /* First critical flag */
#define apCertMgrElementFieldX509ExBytes        2 /* First data bytes */
#define apCertMgrElementFieldX509ExOIDN(n)      (n*3)
#define apCertMgrElementFieldX509ExCriticalN(n) (1+(n*3))
#define apCertMgrElementFieldX509ExBytesN(n)    (2+(n*3))

/* In an CertMgrElementType, the "dataType" can be one of the following.
 */
#define apCertMgrElementDataTypeASN1Eoc            0
#define apCertMgrElementDataTypeASN1Boolean        1
#define apCertMgrElementDataTypeASN1Integer        2
#define apCertMgrElementDataTypeASN1BitString      3
#define apCertMgrElementDataTypeASN1OctetString    4
#define apCertMgrElementDataTypeASN1Null           5
#define apCertMgrElementDataTypeASN1OID            6
#define apCertMgrElementDataTypeASN1ObjDesc        7
#define apCertMgrElementDataTypeASN1External       8
#define apCertMgrElementDataTypeASN1Real           9
#define apCertMgrElementDataTypeASN1Enumerated     10
#define apCertMgrElementDataTypeASN1EmbeddedPDV    11
#define apCertMgrElementDataTypeASN1UTF8String     12
#define apCertMgrElementDataTypeASN1Sequence       16
#define apCertMgrElementDataTypeASN1Set            17
#define apCertMgrElementDataTypeASN1NumericString  18
#define apCertMgrElementDataTypeASN1PrintString    19
#define apCertMgrElementDataTypeASN1T61String      20
#define apCertMgrElementDataTypeASN1VideoTexString 21
#define apCertMgrElementDataTypeASN1IA5String      22
#define apCertMgrElementDataTypeASN1UTCTime        23
#define apCertMgrElementDataTypeASN1GenTime        24
#define apCertMgrElementDataTypeASN1GraphicString  25
#define apCertMgrElementDataTypeASN1ISO64String    26
#define apCertMgrElementDataTypeASN1GenString      27
#define apCertMgrElementDataTypeASN1UnivString     28
#define apCertMgrElementDataTypeASN1BmpString      30

/* These flags are used to tell the GetField function which field to get.
 */
typedef uint32_t CertMgrCertFieldEnum;

#define apCertMgrFieldVersion          apCertMgrElementFieldVersion
#define apCertMgrFieldSerialNumber     apCertMgrElementFieldSerialNumber
#define apCertMgrFieldIssuerRDN        apCertMgrElementFieldIssuerRDN
#define apCertMgrFieldSubjectRDN       apCertMgrElementFieldSubjectRDN
#define apCertMgrFieldNotBefore        apCertMgrElementFieldNotBefore
#define apCertMgrFieldNotAfter         apCertMgrElementFieldNotAfter
#define apCertMgrFieldPubKeyBER        apCertMgrElementFieldPubKeyBER
#define apCertMgrFieldSigOID           apCertMgrElementFieldSigOID
#define apCertMgrFieldSigParams        apCertMgrElementFieldSigParams
#define apCertMgrFieldSignature        apCertMgrElementFieldSignature
#define apCertMgrFieldIssuerID         apCertMgrElementFieldIssuerID
#define apCertMgrFieldSubjectID        apCertMgrElementFieldSubjectID
#define apCertMgrFieldIssuerUniqueID   apCertMgrElementFieldIssuerUniqueID
#define apCertMgrFieldSubjectUniqueID  apCertMgrElementFieldSubjectUniqueID
#define apCertMgrFieldExtensions       apCertMgrElementFieldExtensions

/* Certificates are stored and manipulated in CertMgrCertInfoType's.
 */
typedef struct
{
  uint32_t    ref;
  uint16_t    format;    // x5.09 vs. XML, for example  
  uint16_t    padding;
} CertMgrCertInfoType;

/* The following list the values CertMgrCertInfoTypeType.format can be.
 */
#define apCertMgrFormatX509  1
#define apCertMgrFormatXML   2

/* An CertMgrCertChain is simply a collection of cert's.
 */
typedef struct
{
  CertMgrCertInfoType *certs;
  uint32_t count;
} CertMgrCertChainType;

/* A cert can be valid or invalid. But if it is invalid, it is important to know
     why. The type VerifyResultType is a uint32_t. The value is 0 for verifies, and
     a nonzero code. The following lists the codes  which indicate why a cert
     does not verify.
 */
#define CertMgrVerifyFail                   (certErrorClass+0x80)
#define CertMgrVerifyFailure(err) \
  (((err) >= CertMgrVerifyFail) && \
   ((err) <= CertMgrVerifyFail+20))
#define CertMgrVerifyFailUnknown            (CertMgrVerifyFail+0)
#define CertMgrVerifyFailUnknownIssuer      (CertMgrVerifyFail+1)
#define CertMgrVerifyFailUnknownSigAlg      (CertMgrVerifyFail+2)
#define CertMgrVerifyFailSignature          (CertMgrVerifyFail+3)
#define CertMgrVerifyFailSelfSigned         (CertMgrVerifyFail+4)
#define CertMgrVerifyFailNotBefore          (CertMgrVerifyFail+5)
#define CertMgrVerifyFailNotAfter           (CertMgrVerifyFail+6)
#define CertMgrVerifyFailKeyUsage           (CertMgrVerifyFail+7)
#define CertMgrVerifyFailBasicConstraints   (CertMgrVerifyFail+8)
#define CertMgrVerifyFailCriticalExtension  (CertMgrVerifyFail+9)

typedef struct {
  uint32_t               failureCode;  // set to 0 on 2nd call

  CertMgrCertInfoType  cert;         // where we failed
  uint32_t               depth;        // how deep is this failed cert
  uint32_t               state;        // verify state

  DateTimeType         verifyTime;   // against what time is the cert verified
  uint16_t             padding;
} CertMgrVerifyResultType; 

/* These flags are used to tell the FindCert routine which certs to find.
 */
typedef uint32_t CertMgrCertSearchEnum;

#define apCertMgrSearchCert               1000
#define apCertMgrSearchCertID             1001
#define apCertMgrSearchSubjectRDN         apCertMgrFieldSubjectRDN

/* Error codes.
   Look at the CertMgrVerfigFail #defines for more 'error' codes.
 */
#define certMgrErrNotImplemented		(certErrorClass | 0x01)
#define certMgrErrInvalidEncoding		(certErrorClass | 0x02)
#define certMgrErrOutOfMemory			(certErrorClass | 0x03)
#define certMgrErrInvalidParam			(certErrorClass | 0x04)
#define certMgrErrServiceNotStarted		(certErrorClass | 0x05)
#define certMgrErrOutOfResources		(certErrorClass | 0x06)
#define certMgrErrBufTooSmall			(certErrorClass | 0x07)
#define certMgrErrFieldNotFound			(certErrorClass | 0x08)
#define certMgrErrCertNotFound			(certErrorClass | 0x09)
#define certMgrErrNotExportable			(certErrorClass | 0x0A)
#define certMgrErrDatabaseFail			(certErrorClass | 0x0B)
#define certMgrErrBackupInProgress		(certErrorClass | 0x0C)
#define certMgrErrNotRemovable			(certErrorClass | 0x0D)

/* Function calls for the Cert Manager
 */

/* Take the DER encoding of an X.509 certificate and build an APCertInfoType.
 */
status_t CertMgrImportCert (
   uint8_t *certData,
   uint32_t certDataLen,
   CertMgrCertInfoType *certInfoP
);

/* Create the DER encoding of an X.509 certificate from the given APCertInfoType.
 */
status_t CertMgrExportCert (
   CertMgrCertInfoType *certInfoP,
   uint8_t *certData,
   uint32_t *certDataLen
);

/* The Cert Manager will free up any memory it allocated in working with the
     given APCertInfoType.
 */
status_t CertMgrReleaseCertInfo (
   CertMgrCertInfoType *certInfoP
);

/* Extract the given fieldType from the certInfo.
 */
status_t CertMgrGetField (
   CertMgrCertInfoType *certInfoP,
   CertMgrCertElementEnum elementType,
   CertMgrCertFieldEnum fieldType, // if 0, return all fields
   CertMgrElementListType *result, // can be NULL
   uint32_t *resultLengthP           // can be 0
);

/* Verify a certificate. The Cert Manager will check the following.
     signature
     validity dates
     ????????
 */
status_t CertMgrVerifyCert (
   CertMgrCertInfoType *certInfoP,
   CertMgrCertChainType *certChainP,
   CertMgrVerifyResultType *verifyResult
);

/* Add the given cert to the secure cert database. This function will verify
     the cert.
 */
status_t CertMgrAddCert (
   CertMgrCertInfoType *certInfoP,
   Boolean compress,
   CertMgrVerifyResultType *verifyResult
);

/* Remove the given cert from the secure cert database.
 */
status_t CertMgrRemoveCert (
   CertMgrCertInfoType *certInfoP
);

/* Retrieve a cert from the secure cert database. Pass in an address of the index.
     The Cert Manager will begin searching at that index. If the Cert Manager
     finds the requested cert, it will return the database index of that cert.
     The serach flag indicates what reference to search on (subject name, public
     key, etc.).
 */
status_t CertMgrFindCert (
   uint32_t *index,
   CertMgrCertSearchEnum searchFlag,
   uint8_t *reference,
   uint32_t referenceLen,
   CertMgrCertInfoType *certInfoP
);

#ifdef  __cplusplus
}
#endif

#endif /* _CERT_MANAGER_H_ */
