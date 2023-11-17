/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: WebCompPub.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _WebCompPub_h
#define _WebCompPub_h

// Public symbols for WebExgLib
#define kStrRequestUrl				"name"
#define kStrUserName				"UserName"
#define kStrPassword				"Password"
#define	kStrMethod					"Method"
#define kStrResponseUrl				"ResponseUrl"
#define	kStrUploadData				"UploadData" 
#define kStrUploadDataStore			"UploadDataStore"

#define kStrAuthRealm				"AuthRealm"
#define	kStrStatus					"Status"
#define	kStrContentType				"Content-Type"
#define	kStrHeaders					"Headers" 

#define kStrHttpFlag				"HttpFlag"
#define	kStrHttpDoNotCache			"HttpDoNotCache"
#define kStrHttpForceReload			"HttpForceReload"
#define kStrHttpDoNotRedirect		"HttpDoNotRedirect"
#define kStrHttpDoNotSendCookie		"HttpDoNotSendCookie"
#define kStrHttpDoNotStoreCookie	"HttpDoNotStoreCookie"

//--------------------------
//Error Codes

#define WebCompBaseError				(0xF0001000 )

#define	ErrWebCompAuth					(WebCompBaseError | 1)
#define ErrWebCompProxyAuth				(WebCompBaseError | 2)
#define ErrWebCompRedirected			(WebCompBaseError | 3)
#define ErrWebCompNoContent				(WebCompBaseError | 4)
#define ErrWebCompHttp					(WebCompBaseError | 5)
#define ErrWebCompConnection			(WebCompBaseError | 6)
#define ErrWebCompInvalidUrl			(WebCompBaseError | 7)
#define ErrWebSchemeNotSupported		(WebCompBaseError | 8)
#define ErrWebCompResendRequest			(WebCompBaseError | 9)
#define ErrWebCompInvalidInput			(WebCompBaseError | 10)
#define	ErrWebCompRidirectedTooManyTimes (WebCompBaseError |11)
#define	ErrWebCompDataNotReadyYet		(WebCompBaseError | 12)
#define ErrWebCompCannotSendData		(WebCompBaseError | 13)
#define ErrWebCompUnknownDataType		(WebCompBaseError | 14)
#define ErrWebCompCannotResend			(WebCompBaseError | 15)
#define ErrWebCompProxyInfoNotSet		(WebCompBaseError | 16)
#define ErrWebCompInternalError			(WebCompBaseError | 17)
#define ErrWebCompNotImplemented		(WebCompBaseError | 18)
#define ErrWebCompMaxConnection			(WebCompBaseError | 19)
#define ErrWebCompWouldBlock			(WebCompBaseError | 20)

//Private Internal Error codes
#define ErrWebCompVerifyError			(WebCompBaseError | 256)
//DOLATER Remove these error codes

#define ErrWebContentNotDecoded			(WebCompBaseError | 257)
#define ErrWebRequestAlreadyAborted		(WebCompBaseError | 258)


//=========Private Error codes


#endif //_WebCompPub_h
