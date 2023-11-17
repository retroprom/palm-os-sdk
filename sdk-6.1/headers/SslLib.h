/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SslLib.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef __SslLibARM_H__
#define __SslLibARM_H__

#include <PalmTypesCompatibility.h>

#ifdef __cplusplus
extern	"C" {
#endif 

/********************************************************************
 * Constants
 ********************************************************************/

//#include <NetMgr.h>

#define kSslLibType		sysFileTLibrary
#define kSslLibCreator		'ssl0'
#define kSslDBName		"SslLib"

/********************************************************************
 * Types
 ********************************************************************/

typedef struct SslLib_st		SslLib;
typedef struct SslContext_st		SslContext;
typedef uint32_t SslAttribute;

/* This strucure is a template used with the SslContextSet_IoStruct() and
 * SslContextGet_IoStruct() calls */
typedef struct SslSocket_st
	{
	int32_t socket;
	uint16_t flags;
	uint16_t addrLen;
	status_t err;
	int32_t timeout;
	unsigned char addr[8];
	} SslSocket;

/* These structures are used to define the InfoCallback
 * and the VerifyCallback */
typedef struct SslCallback_st SslCallback;
typedef int32_t (*SslCallbackFunc)(SslCallback *,
		int32_t,int32_t,void *);
struct SslCallback_st
	{
	void *reserved;
	SslCallbackFunc callback;
	void *data;
	SslContext *ssl;
	};

/* The protocol versions supported by SslLib.  This value is used with the
 * ProtocolHello/ProtocolVersion and SslCipherSuiteInfo attributes */
#define sslVersionSSLv3		0x0300
#define sslVersionTLSv1		0x0301

/* The protocol variants supported by the library.  This differest from
 * the version, because the ProtocolHello/ProtooclVersion is what we want to
 * talk to the peer, this determines what sections of the library code
 * is turned on/off.  It is important to not disable things you don't know
 * anything about :-).  This is used with the ProtocolSupport attribute */
/* DO NOT turn off the Ex512/Ex1024 bits without also removing the
 * relevent ciphers from the cipher suite list, otherwise we will send the
 * export cipher in the list and be unable to process the protocol when
 * the server uses an export cipher. */
#define sslSupport_RSAKeyExchange		0x0001 /* Implemented */
#define sslSupport_DHKeyExchange		0x0002 /* Not implemented */
#define sslSupport_anonDHKeyExchange		0x0008 /* Not implemented */
#define sslSupport_RSASign			0x0004 /* Implemented */
#define sslSupport_DSASign			0x0010 /* Not implemented */
#define sslSupport_Ex512			0x0020 /* Implemented */
#define sslSupport_Ex1024			0x0040 /* Implemented */
#define sslSupport_SSLv3Protocol		0x0100 /* Implemented */
#define sslSupport_TLSv1Protocol		0x0200 /* Implemented */
#define sslSupport_SSLv2Header			0x0400 /* Not Implemented */

#define _sslSupport_SslLib (sslSupport_RSAKeyExchange|sslSupport_RSASign|\
			sslSupport_Ex512|sslSupport_Ex1024)
/* It is suggested to use these defines to enable SSLv3, TLSv1 or both
 * protocols.  The default is sslSupport_Both */
#define sslSupport_SSLv3 (_sslSupport_SslLib|sslSupport_SSLv3Protocol)
#define sslSupport_TLSv1 (_sslSupport_SslLib|sslSupport_TLSv1Protocol)
#define sslSupport_Both	(sslSupport_SSLv3Protocol|sslSupport_TLSv1Protocol)

/* These are the possible errors returned from SslLib.
 * In addition to these errors, any NetLib errors that occur while SslLib
 * is performing network IO will be imediatly returned to the application */
//#define sslErrorClass                         0x3900
#define sslErrOk				(sslErrorClass+0) /* 3900 */
#define sslErrFailed				(sslErrorClass+1) /* 3901 */
#define sslErrEof				(sslErrorClass+2) /* 3902 */
#define sslErrOutOfMemory			(sslErrorClass+3) /* 3903 */
#define sslErrCbAbort				(sslErrorClass+4) /* 3904 */
#define sslErrIo				(sslErrorClass+5) /* 3905 */
#define sslErrNotFound				(sslErrorClass+6) /* 3906 */
#define sslErrDivByZero				(sslErrorClass+7) /* 3907 */
#define sslErrNoModInverse			(sslErrorClass+8) /* 3908 */
#define sslErrBadDecode				(sslErrorClass+9) /* 3909 */
#define sslErrInitNotCalled			(sslErrorClass+10) /* 390A */
#define sslErrBufferTooSmall			(sslErrorClass+11) /* 390B */
#define sslErrNullArg				(sslErrorClass+12) /* 390C */
#define sslErrBadLength				(sslErrorClass+13) /* 390D */
#define sslErrNoDmem				(sslErrorClass+14) /* 390E */
#define sslErrNoMethodSet			(sslErrorClass+15) /* 390F */
#define sslErrNoRandom				(sslErrorClass+16) /* 3910 */
#define sslErrBadArgument			(sslErrorClass+17) /* 3911 */
#define sslErrBadOption				(sslErrorClass+18) /* 3912 */
#define sslErrNotImplemented			(sslErrorClass+19) /* 3913 */
#define sslErrReallocStaticData			(sslErrorClass+20) /* 3914 */
#define sslErrInternalError			(sslErrorClass+21) /* 3915 */
#define sslErrRecordError			(sslErrorClass+37) /* 3925 */
#define sslErrCsp				(sslErrorClass+38) /* 3926 */
#define sslErrCert				(sslErrorClass+39) /* 3927 */
#define sslErrHandshakeEncoding			(sslErrorClass+40) /* 3928 */
#define sslErrMissingProvider			(sslErrorClass+41) /* 3929 */
#define sslErrHandshakeProtocol			(sslErrorClass+42) /* 392A */
#define sslErrExtraHandshakeData		(sslErrorClass+43) /* 392B */
#define sslErrWrongMessage			(sslErrorClass+44) /* 392C */
#define sslErrFatalAlert			(sslErrorClass+45) /* 392D */
#define sslErrBadPeerFinished			(sslErrorClass+46) /* 392E */
#define sslErrBadSignature			(sslErrorClass+47) /* 392F */

#define sslErrUnexpectedRecord			(sslErrorClass+49) /* 3931 */
#define sslErrReadAppData			(sslErrorClass+50) /* 3932 */
#define sslErrCertDecodeError			(sslErrorClass+51) /* 3933 */
#define sslErrUnsupportedCertType		(sslErrorClass+52) /* 3934 */
#define sslErrUnsupportedSignatureType		(sslErrorClass+53) /* 3935 */
#define sslErrUnsupportedProtocol		(sslErrorClass+54) /* 3936 */

#define sslErrMissingCipherSuite		(sslErrorClass+80) /* 3950 */

/* There are additional error code that can be returned from the CertManageri
 * See the CertMgrVerifyFail codes in CertificateMgr.h */
/* This error is used if the verify callback routine wants to 'report'
 * an error.  So it should not originate from inside SslLib, but rather
 * from an Application callback */
#define sslErrVerifyCallback			(sslErrorClass+134) /* 3986 */

/* These options can be passed to SslOpen() */
#define sslOpenModeClear		0x0001/* Set_Mode(s,sslModeClear) */
#define sslOpenModeSsl			0x0002/* Set_Mode(s,sslModeSsl) */
#define sslOpenNewConnection		0x0004/* Set_Session(s,NULL) */
#define sslOpenNoAutoFlush		0x0008/* Set_AutoFlush(s,0) */
#define sslOpenUseDefaultTimeout	0x0020/* Ignore the timeout parameter */
#define sslOpenBufferedReuse		0x0040/* Set_BufferedReuse(s,1) */
#define sslOpenDelayHandshake		0x0080/* Set_BufferedReuse(s,1) */

/* Options to pass to SslClose() */
#define sslCloseUseDefaultTimeout	0x0020/* Ignore the timeout parameter */
#define sslCloseDontSendShutdown	0x0001/* Don't send a shutdown msg */
#define sslCloseDontWaitForShutdown	0x0002/* Don't wait for a shutdown msg*/

/* Values to use when calling SslXxxSet_Mode();  The sslModeFlush value
 * will only affect SslContextSet_Mode() and will cause the SslContexts
 * data bufferes to be flushed if they contain any data bytes (due to the
 * SslContext being reused */
#define sslModeClear            0x0000
#define sslModeSsl              0x0002
#define sslModeSslClient        0x000A
#define sslModeFlush        	0x8000

/* Values that can be passed to any SslCallback functions */
#define sslCmdNew		0x0001
#define sslCmdFree		0x0002
#define sslCmdReset		0x0003
#define sslCmdGet		0x0004
#define sslCmdSet		0x0005

/* Values that can be passed to specific SslCallback types */
#define sslCmdRead		0x0010	/* Internal use */
#define sslCmdWrite		0x0011	/* Internal use */
#define sslCmdInfo		0x0012	/* InfoCallback */
#define sslCmdVerify		0x0013	/* VerifyCallback */

/* Passed to SslXxxSet_InfoInterest() to specify which 'interest'
 * events we wish to be notified about */
#define sslFlgInfoAlert		0x0001
#define sslFlgInfoHandshake	0x0002
#define sslFlgInfoIo		0x0004
#define sslFlgInfoCert		0x0008

/* These values are the argi values to sslCmdInfo callback calls
 * (the InfoCallback) */
#define sslArgInfoHandshake	0x0001
#define sslArgInfoAlert		0x0002
#define sslArgInfoReadBefore	sslCmdRead		/* 0x0010 */
#define sslArgInfoReadAfter	(sslCmdRead|0x8000)	/* 0x8010 */
#define sslArgInfoWriteBefore	sslCmdWrite		/* 0x0011 */
#define sslArgInfoWriteAfter	(sslCmdWrite|0x8000)	/* 0x8011 */
#define sslArgInfoCert		0x0003

/* Turn on bug compatability, use with
 * SslXxxSet_Compat() */
#define sslCompatReuseCipherBug         0x0001
#define sslCompatNetscapeCaDnBug        0x0002
/* This-one is needto to interoperate with Stronghold when using a <1k write
 * buffer */
#define sslCompat1RecordPerMessage      0x0004
#define sslCompatBigRecords		0x0008
#define sslCompatAll                    0xffff

/* Specify the mode to operate in, use with
 * SslXxxSet_Mode() */
#define sslModeClear                    0x0000
#define sslModeSsl                      0x0002
#define sslModeSslClient                0x000A

/* Possible handshake states for the SSL protocol handshake.
 * Refer to the SSLv3 specification for an idea as to what the
 * states mean.  These values are returned by
 * SslContextGet_HsState(ssl); */
#define sslHsStateNone			0
#define sslHsStateStart			1
#define sslHsStateClientHello		2
#define sslHsStateServerHello		3
#define sslHsStateFlush			4
#define sslHsStateWriteFlush		5
#define sslHsStateWrite			6
#define sslHsStateCert			7
#define sslHsStateCertB			8
#define sslHsStateSkEx			9
#define sslHsStateSkExRsa		10
#define sslHsStateSkExDh		11
#define sslHsStateSkExAnonDh		12
#define sslHsStateCertReq		13
#define sslHsStateCertReqB		14
#define sslHsStateServerDone		15
#define sslHsStateClientCert		16
#define sslHsStateCkEx			17
#define sslHsStateWriteCcs		18
#define sslHsStateFinished		19
#define sslHsStateReadCcs		20
#define sslHsStateGenerateKeys		21
#define sslHsStateReadFinished		22
#define sslHsStateReadFinishedB		23
#define sslHsStateReadFinishedC		24
#define sslHsStateCleanup		25
#define sslHsStateDone			26
#define sslHsStateShutdown		27
#define sslHsStateClosed		28
#define sslHsStateHelloRequest		29
#define sslHsStateWriteClose		30

/* These are the defined Sslv3/TLSv1 alerts as definined in the SSLv3 and
 * TLSv1 specifications.  For their meanings, refer to those specifications.
 * These values can be returned by
 * SslContextGet_LastAlert(ssl); */
#define sslAlertCloseNotify		(0x0100+ 0)	/* SSL3 */
#define sslAlertUnexpectedMessage	(0x0200+10)	/* SSL3 */
#define sslAlertBadRecordMac		(0x0200+20)	/* SSL3 */
#define sslAlertDecryptionFailed       	(0x0200+21)	/* TLS */
#define sslAlertRecordOverflow         	(0x0200+22)	/* TLS */
#define sslAlertDecompressionFailure	(0x0200+30)	/* SSL3 */
#define sslAlertHandshakeFailure	(0x0200+40)	/* SSL3 */
#define sslAlertNoCertificate		(0x0100+41)	/* SSL3 */
#define sslAlertBadCertificate		(0x0100+42)	/* SSL3 */
#define sslAlertUnsupportedCertificate	(0x0100+43)	/* SSL3 */
#define sslAlertCertificateRevoked     	(0x0100+44)	/* SSL3 */
#define sslAlertCertificateExpired     	(0x0100+45)	/* SSL3 */
#define sslAlertCertificateUnknown     	(0x0100+46)	/* SSL3 */
#define sslAlertIllegalParameter       	(0x0200+47)	/* SSL3 */
#define sslAlertUnknownCa              	(0x0200+48)	/* TLS fatal */
#define sslAlertAccessDenied           	(0x0200+49)	/* TLS fatal */
#define sslAlertDecodeError            	(0x0200+50)	/* TLS fatal */
#define sslAlertDecryptError           	(0x0200+51)	/* TLS */
#define sslAlertExportRestricion       	(0x0200+60)	/* TLS fatal */
#define sslAlertProtocolVersion        	(0x0200+70)	/* TLS fatal */
#define sslAlertInsufficientSecurity   	(0x0200+71)	/* TLS fatal */
#define sslAlertInternalError          	(0x0200+80)	/* TLS fatal */
#define sslAlertUserCancled            	(0x0100+90)	/* TLS */
#define sslAlertNoRenegotiation        	(0x0100+100)	/* TLS */

/* The last IO operation performed by a SslContext, return values from
 * SslContextGet_LastIo(ssl); */
#define sslLastIoNone           0x00
#define sslLastIoRead           0x01
#define sslLastIoWrite          0x02

/* The last SslLib API call made that could cause IO activity, returned by
 * SslContextGet_LastApi(ssl); */
#define sslLastApiNone          0x00
#define sslLastApiOpen		0x01
#define sslLastApiRead          0x02
#define sslLastApiWrite         0x03
#define sslLastApiFlush         0x04
#define sslLastApiShutdown      0x05

/* SSLv3 Cipher suite identification strings.  These values are relevent to
 * the CipherSuites, CipherSuite, CipherSuiteInfo, SslSession and
 * SslCipherSuiteInfo attributes */
#define sslCs_RSA_RC4_40_MD5	0x00,0x03
#define sslCs_RSA_RC4_128_MD5	0x00,0x04
#define sslCs_RSA_RC4_128_SHA1	0x00,0x05
#define sslCs_RSA_RC4_56_SHA1	0x00,0x64
/* These are only supported if the CPM supports DES/3DES */
#define sslCs_RSA_DES_40_SHA1	0x00, 0x08
#define sslCs_RSA_DES_56_SHA1	0x00, 0x09
#define sslCs_RSA_3DES_168_SHA1	0x00, 0x0A
/* These are only supported if the CPM supports AES128/AES256 */
#define sslCs_RSA_AES_128_SHA1	0x00, 0x2F
#define sslCs_RSA_AES_256_SHA1	0x00, 0x35

/* The identifiers are expanded because the C pre-processor may not support
 * string concatination */
// StrongCiphers == sslCs_AES_256_SHA1 sslCs_3DES_168_SHA1 sslCs_AES_128_SHA1
//			sslCs_RSA_RC4_128_SHA1	sslCs_RSA_RC4_128_MD5
#define sslCs_StrongCiphers "\x00\0x35\x00\x06\x00\x2F\x00\x0A\x00\x05\x00\x04"

// Export Ciphers == sslCs_RSA_RC4_56_SHA1 sslCs_RSA_RC4_40_MD5
//		        sslCs_RSA_DES_40_SHA1
#define sslCs_ExportCiphers	"\x00\x06\x00\x64\x00\x03\x00\x08"

// Weak Export Ciphers == sslCs_RSA_RC5_40_MD5 sslCs_RSA_DES_40_SHA1
// Export included the '64bit/1024bit export ciphers, this one does not
#define sslCs_WeakExportCiphers	"\x00\x04\x00\x03\x00\x08"

/* These values are used to decode some of the fields in the
 * SslCipherSuiteInfo structure */
#define sslCsiKeyExchNull	0x00
#define sslCsiKeyExchRsa	0x01
#define sslCsiAuthNULL		0x00
#define sslCsiAuthRsa		0x01
#define sslCsiDigestNull	0x00
#define sslCsiDigestMd5		0x01
#define sslCsiDigestSha1	0x02
#define sslCsiDigestMd2		0x03
#define sslCsiCipherNull	0x00
#define sslCsiCipherRc4		0x01

typedef struct SslCipherSuiteInfo_st {
	uint8_t cipherSuite[2];
	uint16_t cipher;			/* sslCsiCipherXXX */
	uint16_t digest;			/* sslCsiDigestXXX */
	uint16_t keyExchange;		/* sslCsiKeyExchXXX */
	uint16_t authentication;		/* sslCsiAuthXXX */
	uint16_t version;
	uint16_t cipherBitLength;
	uint16_t cipherKeyLength;
	uint16_t keyExchangeLength;
	uint16_t authenticationLength;
	uint16_t exportCipher;
	} SslCipherSuiteInfo;

typedef struct SslIoBuf_st
	{
	SslContext *ssl;
	uint8_t *ptr;
	uint32_t outNum;
	uint32_t inNum;
	uint32_t max;
	uint32_t err;
	uint32_t flags;
	} SslIoBuf;

typedef struct SslSession_st
        {
        /* Lets hope these all pack correctly with 0 padding */
        uint32_t length;
        uint16_t version;
        unsigned char cipherSuite[2];
        unsigned char compression;
        unsigned char sessionId[33];   /* First byte is the length */
        unsigned char masterSecret[48];/* Master secret */
        unsigned char time[8];          /* Start time - host specific */
        unsigned char timeout[4];       /* Timeout in seconds */
        /* Optional Peer certificate, it the offset to a SslExtendedItems
         * If the offset if 0, it does not exist */
        uint16_t certificateOffset;
        /* Can be used to store anything, perhaps hostname of peer?
         * Application defined */
        uint16_t extraData;
        /* Extra bytes are located on the end.  The offsets are from
         * the front of the structure */
        uint16_t padding;
        } SslSession;
        
#include "SslLibMac.h"

/********************************************************************
 * Prototypes
 ********************************************************************/

/* Following two functions SslLibOpen and SslLibClose should not be used in code,
these functions are for backward compatibilty only*/
extern status_t SslLibOpen(void);
extern status_t SslLibClose(void);

extern status_t SslLibCreate(SslLib **lib); 
extern void SslLibDestroy(SslLib *lib);
extern status_t SslLibSetLong(SslLib *lib,SslAttribute attr,int32_t value);
extern int32_t SslLibGetLong(SslLib *lib,SslAttribute attr);
extern status_t SslLibSetPtr(SslLib *lib,SslAttribute attr,void *value);
extern status_t SslLibGetPtr(SslLib *lib,SslAttribute attr,void **value);

extern status_t SslContextCreate(SslLib *lib,SslContext **ctx);
extern void SslContextDestroy(SslContext *ctx);

extern status_t SslContextSetLong(SslContext *lib,SslAttribute attr, long value);
extern int32_t SslContextGetLong(SslContext *lib,SslAttribute attr);
extern status_t SslContextSetPtr(SslContext *lib,SslAttribute attr, void *value);
extern status_t SslContextGetPtr(SslContext *lib,SslAttribute attr, void **value);

extern status_t SslOpen(SslContext *ctx,uint16_t mode,uint32_t timeout);
extern status_t SslClose(SslContext *ctx,uint16_t mode,uint32_t timeout);
extern int16_t SslSend(SslContext *ctx,const void *buffer,uint16_t bufferLen,uint16_t flags,
		void *toAddr, uint16_t toLen, int32_t timeout,status_t *errRet);
extern int16_t SslReceive(SslContext *ctx,void *buffer,uint16_t bufferLen,
		uint16_t flags, void *fromAddr, uint16_t *fromLen,
		int32_t timeout,status_t *errRet);
extern int32_t SslRead(SslContext *ctx,void *buffer,
		int32_t bufferLen, status_t *errRet);
extern int32_t SslWrite(SslContext *ctx,const void *buffer,
		int32_t bufferLen, status_t *errRet);
extern status_t SslPeek(SslContext *ctx,void **buffer_ptr,
		int32_t *availableBytes, int32_t readSize);
extern void SslConsume(SslContext *ctx, int32_t number);
extern status_t SslFlush(SslContext *ctx,int32_t *outstanding);

#ifdef __cplusplus
}
#endif  //_cplusplus

#endif
