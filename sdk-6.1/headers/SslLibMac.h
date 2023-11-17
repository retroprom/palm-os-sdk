/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SslLibMac.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __SslLibMac_H__
#define __SslLibMac_H__

#define sslAttrLibCompat		0x0F010103
#define sslAttrLibInfoInterest		0x0F020203
#define sslAttrLibProtocolVersion	0x0F030303
#define sslAttrLibMode			0x0F040403
#define sslAttrLibInfoCallback		0x0F080805
#define sslAttrLibVerifyCallback	0x0F0A0A05
#define sslAttrLibReadStreaming		0x0F0B0B02
#define sslAttrLibAutoFlush		0x0F0C0C02
#define sslAttrLibBufferedReuse		0x0F0D0D02
#define sslAttrLibAppPtr		0x0F0E0E01
#define sslAttrLibAppInt32		0x0F0F0F03
#define sslAttrLibRbufSize		0x0F101003
#define sslAttrLibWbufSize		0x0F111103
#define sslAttrLibDontSendShutdown	0x0F121202
#define sslAttrLibDontWaitForShutdown	0x0F131302
#define sslAttrLibDelayReadServerFinished 0x0F141402
#define sslAttrLibHelloVersion		0x0F151503
#define sslAttrLibProtocolSupport	0x0F161603

#define sslAttrCompat			0x0F010113
#define sslAttrInfoInterest		0x0F020213
#define sslAttrProtocolVersion		0x0F030313
#define sslAttrMode			0x0F048013
#define sslAttrErrorState		0x0F05FF14
#define sslAttrReadStreaming		0x0F060612
#define sslAttrAutoFlush		0x0F070712
#define sslAttrBufferedReuse		0x0F080812
#define sslAttrAppPtr			0x0F090911
#define sslAttrAppInt32			0x0F0A0A13
#define sslAttrInfoCallback		0x0F0E0E15
#define sslAttrVerifyCallback		0x0F101015
#define sslAttrError			0x0F111113
#define sslAttrHsState			0x0F12FF13
#define sslAttrLastAlert		0x0F131313
#define sslAttrSessionReused		0x0F14FF12
#define sslAttrWriteBufPending		0x0F15FF13
#define sslAttrReadBufPending		0x0F16FF13
#define sslAttrReadRecPending		0x0F17FF13
#define sslAttrReadOutstanding		0x0F18FF13
#define sslAttrRbufSize			0x0F1B8113
#define sslAttrWbufSize			0x0F1C8213
#define sslAttrStreaming		0x0F1DFF12
#define sslAttrLastIo			0x0F1EFF12
#define sslAttrLastApi			0x0F1FFF12
#define sslAttrClientCertRequest	0x0F20FF12
#define sslAttrDontSendShutdown		0x0F212112
#define sslAttrDontWaitForShutdown	0x0F222212
#define sslAttrDelayReadServerFinished	0x0F232312
#define sslAttrHelloVersion		0x0F242412
#define sslAttrProtocolSupport		0x0F252512

#define sslAttrCspSslSession		0x00808001
#define sslAttrCspCipherSuites		0x00008101
#define sslAttrCspCipherSuite		0x0001FF01
#define sslAttrCspCipherSuiteInfo	0x0082FF01

#define sslAttrCertPeerCert		0x0100FF01
#define sslAttrCertSslVerify		0x0101FF04
#define sslAttrCertMgrVerifyResult	0x0102FF04
#define sslAttrCertPeerCommonName	0x0180FF01
#define sslAttrCertPeerCertInfoType	0x0102FF01
#define sslAttrCertVerifyChain		0x0103FF01

#define sslAttrIoStruct			0x04008001
#define sslAttrIoSocket			0x04010103
#define sslAttrIoTimeout		0x04020203
#define sslAttrIoFlags			0x04030303

#define SslLibGet_Compat(lib) \
	SslLibGetLong((lib),sslAttrLibCompat)
#define SslLibSet_Compat(lib,v) \
	SslLibSetLong((lib),sslAttrLibCompat,(v))
#define SslLibGet_InfoInterest(lib) \
	SslLibGetLong((lib),sslAttrLibInfoInterest)
#define SslLibSet_InfoInterest(lib,v) \
	SslLibSetLong((lib),sslAttrLibInfoInterest,(v))
#define SslLibGet_ProtocolVersion(lib) \
	SslLibGetLong((lib),sslAttrLibProtocolVersion)
#define SslLibSet_ProtocolVersion(lib,v) \
	SslLibSetLong((lib),sslAttrLibProtocolVersion,(v))
#define SslLibGet_Mode(lib) \
	SslLibGetLong((lib),sslAttrLibMode)
#define SslLibSet_Mode(lib,v) \
	SslLibSetLong((lib),sslAttrLibMode,(v))
#define SslLibGet_InfoCallback(lib,v) \
	SslLibGetPtr((lib),sslAttrLibInfoCallback,(void **)(v))
#define SslLibSet_InfoCallback(lib,v) \
	SslLibSetPtr((lib),sslAttrLibInfoCallback,(void *)(v))
#define SslLibGet_VerifyCallback(lib,v) \
	SslLibGetPtr((lib),sslAttrLibVerifyCallback,(void **)(v))
#define SslLibSet_VerifyCallback(lib,v) \
	SslLibSetPtr((lib),sslAttrLibVerifyCallback,(void *)(v))
#define SslLibGet_ReadStreaming(lib) \
	SslLibGetLong((lib),sslAttrLibReadStreaming)
#define SslLibSet_ReadStreaming(lib,v) \
	SslLibSetLong((lib),sslAttrLibReadStreaming,(v))
#define SslLibGet_AutoFlush(lib) \
	SslLibGetLong((lib),sslAttrLibAutoFlush)
#define SslLibSet_AutoFlush(lib,v) \
	SslLibSetLong((lib),sslAttrLibAutoFlush,(v))
#define SslLibGet_BufferedReuse(lib) \
	SslLibGetLong((lib),sslAttrLibBufferedReuse)
#define SslLibSet_BufferedReuse(lib,v) \
	SslLibSetLong((lib),sslAttrLibBufferedReuse,(v))
#define SslLibGet_AppPtr(lib,v) \
	SslLibGetPtr((lib),sslAttrLibAppPtr,(void **)(v))
#define SslLibSet_AppPtr(lib,v) \
	SslLibSetPtr((lib),sslAttrLibAppPtr,(void *)(v))
#define SslLibGet_AppInt32(lib) \
	SslLibGetLong((lib),sslAttrLibAppInt32)
#define SslLibSet_AppInt32(lib,v) \
	SslLibSetLong((lib),sslAttrLibAppInt32,(v))
#define SslLibGet_RbufSize(lib) \
	SslLibGetLong((lib),sslAttrLibRbufSize)
#define SslLibSet_RbufSize(lib,v) \
	SslLibSetLong((lib),sslAttrLibRbufSize,(v))
#define SslLibGet_WbufSize(lib) \
	SslLibGetLong((lib),sslAttrLibWbufSize)
#define SslLibSet_WbufSize(lib,v) \
	SslLibSetLong((lib),sslAttrLibWbufSize,(v))
#define SslLibGet_DontSendShutdown(lib) \
	SslLibGetLong((lib),sslAttrLibDontSendShutdown)
#define SslLibSet_DontSendShutdown(lib,v) \
	SslLibSetLong((lib),sslAttrLibDontSendShutdown,(v))
#define SslLibGet_DontWaitForShutdown(lib) \
	SslLibGetLong((lib),sslAttrLibDontWaitForShutdown)
#define SslLibSet_DontWaitForShutdown(lib,v) \
	SslLibSetLong((lib),sslAttrLibDontWaitForShutdown,(v))
#define SslLibGet_CipherSuites(lib,v) \
	SslLibGetPtr((lib),sslAttrCspCipherSuites,(void **)(v))
#define SslLibSet_CipherSuites(lib,v) \
	SslLibSetPtr((lib),sslAttrCspCipherSuites,(void *)(v))

#define SslLibGet_DelayReadServerFinished(lib) \
	SslLibGetLong((lib),sslAttrLibDelayReadServerFinished)
#define SslLibSet_DelayReadServerFinished(lib,v) \
	SslLibSetLong((lib),sslAttrLibDelayReadServerFinished,(v))
#define SslLibGet_HelloVersion(lib) \
	SslLibGetLong((lib),sslAttrLibHelloVersion)
#define SslLibSet_HelloVersion(lib,v) \
	SslLibSetLong((lib),sslAttrLibHelloVersion,(v))
#define SslLibGet_ProtocolSupport(lib) \
	SslLibGetLong((lib),sslAttrLibProtocolSupport)
#define SslLibSet_ProtocolSupport(lib,v) \
	SslLibSetLong((lib),sslAttrLibProtocolSupport,(v))

#define SslContextGet_Compat(ssl) \
	SslContextGetLong((ssl),sslAttrCompat)
#define SslContextSet_Compat(ssl,v) \
	SslContextSetLong((ssl),sslAttrCompat,(v))
#define SslContextGet_InfoInterest(ssl) \
	SslContextGetLong((ssl),sslAttrInfoInterest)
#define SslContextSet_InfoInterest(ssl,v) \
	SslContextSetLong((ssl),sslAttrInfoInterest,(v))
#define SslContextGet_ProtocolVersion(ssl) \
	SslContextGetLong((ssl),sslAttrProtocolVersion)
#define SslContextSet_ProtocolVersion(ssl,v) \
	SslContextSetLong((ssl),sslAttrProtocolVersion,(v))
#define SslContextGet_Mode(ssl) \
	SslContextGetLong((ssl),sslAttrMode)
#define SslContextSet_Mode(ssl,v) \
	SslContextSetLong((ssl),sslAttrMode,(v))
#define SslContextGet_ReadStreaming(ssl) \
	SslContextGetLong((ssl),sslAttrReadStreaming)
#define SslContextSet_ReadStreaming(ssl,v) \
	SslContextSetLong((ssl),sslAttrReadStreaming,(v))
#define SslContextGet_AutoFlush(ssl) \
	SslContextGetLong((ssl),sslAttrAutoFlush)
#define SslContextSet_AutoFlush(ssl,v) \
	SslContextSetLong((ssl),sslAttrAutoFlush,(v))
#define SslContextGet_BufferedReuse(ssl) \
	SslContextGetLong((ssl),sslAttrBufferedReuse)
#define SslContextSet_BufferedReuse(ssl,v) \
	SslContextSetLong((ssl),sslAttrBufferedReuse,(v))
#define SslContextGet_AppPtr(ssl,v) \
	SslContextGetPtr((ssl),sslAttrAppPtr,(void **)(v))
#define SslContextSet_AppPtr(ssl,v) \
	SslContextSetPtr((ssl),sslAttrAppPtr,(void *)(v))
#define SslContextGet_AppInt32(ssl) \
	SslContextGetLong((ssl),sslAttrAppInt32)
#define SslContextSet_AppInt32(ssl,v) \
	SslContextSetLong((ssl),sslAttrAppInt32,(v))
#define SslContextGet_InfoCallback(ssl,v) \
	SslContextGetPtr((ssl),sslAttrInfoCallback,(void **)(v))
#define SslContextSet_InfoCallback(ssl,v) \
	SslContextSetPtr((ssl),sslAttrInfoCallback,(void *)(v))
#define SslContextGet_VerifyCallback(ssl,v) \
	SslContextGetPtr((ssl),sslAttrVerifyCallback,(void **)(v))
#define SslContextSet_VerifyCallback(ssl,v) \
	SslContextSetPtr((ssl),sslAttrVerifyCallback,(void *)(v))
#define SslContextGet_Error(ssl) \
	SslContextGetLong((ssl),sslAttrError)
#define SslContextSet_Error(ssl,v) \
	SslContextSetLong((ssl),sslAttrError,(v))
#define SslContextGet_HsState(ssl) \
	SslContextGetLong((ssl),sslAttrHsState)
#define SslContextGet_LastAlert(ssl) \
	SslContextGetLong((ssl),sslAttrLastAlert)
#define SslContextSet_LastAlert(ssl,v) \
	SslContextSetLong((ssl),sslAttrLastAlert,(v))
#define SslContextGet_SessionReused(ssl) \
	SslContextGetLong((ssl),sslAttrSessionReused)
#define SslContextGet_WriteBufPending(ssl) \
	SslContextGetLong((ssl),sslAttrWriteBufPending)
#define SslContextGet_ReadBufPending(ssl) \
	SslContextGetLong((ssl),sslAttrReadBufPending)
#define SslContextGet_ReadRecPending(ssl) \
	SslContextGetLong((ssl),sslAttrReadRecPending)
#define SslContextGet_ReadOutstanding(ssl) \
	SslContextGetLong((ssl),sslAttrReadOutstanding)
#define SslContextGet_RbufSize(ssl) \
	SslContextGetLong((ssl),sslAttrRbufSize)
#define SslContextSet_RbufSize(ssl,v) \
	SslContextSetLong((ssl),sslAttrRbufSize,(v))
#define SslContextGet_WbufSize(ssl) \
	SslContextGetLong((ssl),sslAttrWbufSize)
#define SslContextSet_WbufSize(ssl,v) \
	SslContextSetLong((ssl),sslAttrWbufSize,(v))
#define SslContextGet_Streaming(ssl) \
	SslContextGetLong((ssl),sslAttrStreaming)
#define SslContextGet_LastIo(ssl) \
	SslContextGetLong((ssl),sslAttrLastIo)
#define SslContextGet_LastApi(ssl) \
	SslContextGetLong((ssl),sslAttrLastApi)
#define SslContextGet_ClientCertRequest(ssl) \
	SslContextGetLong((ssl),sslAttrClientCertRequest)

#define SslContextGet_DontSendShutdown(ssl) \
	SslContextGetLong((ssl),sslAttrDontSendShutdown)
#define SslContextSet_DontSendShutdown(ssl,v) \
	SslContextSetLong((ssl),sslAttrDontSendShutdown,(v))
#define SslContextGet_DontWaitForShutdown(ssl) \
	SslContextGetLong((ssl),sslAttrDontWaitForShutdown)
#define SslContextSet_DontWaitForShutdown(ssl,v) \
	SslContextSetLong((ssl),sslAttrDontWaitForShutdown,(v))

#define SslContextGet_DelayReadServerFinished(ssl) \
	SslContextGetLong((ssl),sslAttrDelayReadServerFinished)
#define SslContextSet_DelayReadServerFinished(ssl,v) \
	SslContextSetLong((ssl),sslAttrDelayReadServerFinished,(v))
#define SslContextGet_HelloVersion(ssl) \
	SslContextGetLong((ssl),sslAttrHelloVersion)
#define SslContextSet_HelloVersion(ssl,v) \
	SslContextSetLong((ssl),sslAttrHelloVersion,(v))
#define SslContextGet_ProtocolSupport(ssl) \
	SslContextGetLong((ssl),sslAttrProtocolSupport)
#define SslContextSet_ProtocolSupport(ssl,v) \
	SslContextSetLong((ssl),sslAttrProtocolSupport,(v))

#define SslContextGet_SslSession(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCspSslSession,(void **)(v))
#define SslContextSet_SslSession(ssl,v) \
	SslContextSetPtr((ssl),sslAttrCspSslSession,(void *)(v))
#define SslContextGet_CipherSuites(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCspCipherSuites,(void **)(v))
#define SslContextSet_CipherSuites(ssl,v) \
	SslContextSetPtr((ssl),sslAttrCspCipherSuites,(void *)(v))
#define SslContextGet_CipherSuite(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCspCipherSuite,(void **)(v))
#define SslContextGet_CipherSuiteInfo(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCspCipherSuiteInfo,(void **)(v))
#define SslContextGet_PeerCert(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertPeerCert,(void **)(v))
#define SslContextGet_SslVerify(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertSslVerify,(void **)(v))
#define SslContextGet_CertMgrVerifyResult(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertMgrVerifyResult,(void **)(v))
#define SslContextGet_PeerCommonName(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertPeerCommonName,(void **)(v))
#define SslContextGet_PeerCertInfoType(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertPeerCertInfoType,(void **)(v))
#define SslContextGet_CertChain(ssl,v) \
	SslContextGetPtr((ssl),sslAttrCertVerifyChain,(void **)(v))

#define SslContextGet_IoStruct(ssl,v) \
	SslContextGetPtr((ssl),sslAttrIoStruct,(void **)(v))
#define SslContextSet_IoStruct(ssl,v) \
	SslContextSetPtr((ssl),sslAttrIoStruct,(void *)(v))

#define SslContextGet_Socket(ssl) \
	SslContextGetLong((ssl),sslAttrIoSocket)
#define SslContextSet_Socket(ssl,v) \
	SslContextSetLong((ssl),sslAttrIoSocket,(v))

#define SslContextGet_IoTimeout(ssl) \
	SslContextGetLong((ssl),sslAttrIoTimeout)
#define SslContextSet_IoTimeout(ssl,v) \
	SslContextSetLong((ssl),sslAttrIoTimeout,(v))
#define SslContextGet_IoFlags(ssl) \
	SslContextGetLong((ssl),sslAttrIoFlags)
#define SslContextSet_IoFlags(ssl,v) \
	SslContextSetLong((ssl),sslAttrIoFlags,(v))

#endif /* __SslLibMac_H__ */
