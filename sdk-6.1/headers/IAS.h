/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: IAS.h
 *
 * Release: Palm OS 6.0.1
 *
 *****************************************************************************/

#ifndef IAS_H
#define IAS_H

#include <IrDA.h>

#ifdef __cplusplus
extern "C" {
#endif


/* IAS-related constants */

// values for 'IASAttribValueType::attribType'
typedef enum {
	iasAttribMissing			= 0,
	iasAttribInteger			= 1,
	iasAttribOctetString		= 2,
	iasAttribUserString			= 3,
} IASAttribTypeType;

// maximum length for an octet-stream IAS attribute
#define iasMaxOctetStringLen	1024

// values for 'IASAttribValueType::value::userString::charSet'
typedef enum {
	iasCharSetASCII				= 0,
	iasCharSetISO8859_1			= 1,
	iasCharSetISO8859_2			= 2,
	iasCharSetISO8859_3			= 3,
	iasCharSetISO8859_4			= 4,
	iasCharSetISO8859_5			= 5,
	iasCharSetISO8859_6			= 6,
	iasCharSetISO8859_7			= 7,
	iasCharSetISO8859_8			= 8,
	iasCharSetISO8859_9			= 9,
	iasCharSetUnicode			= 0xff,
} IASCharSetType;

// maximum length for an user-string IAS attribute
#define iasMaxUserStringLen		255

// maximum length for an IAS object's class name
#define iasMaxClassNameLen		60

// maximum length for an IAS attribute's name
#define iasMaxAttribNameLen		60

// errors returned by IAS functions
#define iasErrBase					(IrErrorClass | 0xa0)
#define iasErrNoSuchClass			(iasErrBase | 1)	// ias query failed to find specified class
#define iasErrNoSuchAttribute		(iasErrBase | 2)	// ias query failed to find specified attribute
#define iasErrObjectTooLarge		(iasErrBase | 3)	// flattened object is too large
#define iasErrInvalidSocket			(iasErrBase | 4)	// invalid socket passed to IAS function
#define iasErrNoSuchObject			(iasErrBase | 5)	// invalid object id passed to IAS function


/* IAS-specific types */

// the below structures are tightly packed
#ifdef _MSC_VER
	#pragma pack(1)
#endif

typedef PACKED struct IASAttribValueTag {
	uint8_t attribType;				// see 'IASAttribTypeType'
	PACKED union {
		// value of integer attribute
		uint32_t integer;

		// value of octet-string attribute
		PACKED struct {
			uint16_t length;
			uint8_t bytes[1];		// 'length' bytes
		} octetString;

		// value of user-string attribute
		PACKED struct {
			uint8_t charSet;		// see 'IASCharSetType'
			uint8_t length;			// number of bytes, not chars
			uint8_t chars[1];		// 'length' bytes
		} userString;
	} value;
} IASAttribValueType;

#ifdef _MSC_VER
	#pragma pack()
#endif


typedef struct IASObjectTag {

	// user-supplied class name
	const char *className;

	// bits to be included in IrLMP's "IrLMPSupport" attribute
	const uint8_t *hintsBits;

	uint8_t __pad0[2];

	// number of attributes included in this object
	uint16_t attribCount;

	// array of attribute names, with 'attribCount' entries
	const char **attribNames;

	// array of pointers to attribute values, also with
	// 'attribCount' entries
	IASAttribValueType **attribValues;

} IASObjectType;

typedef struct IASQueryTag {

	// device to query
	IrLapDeviceAddrType addr;

	// query parameters
	const char *className;
	const char *attribName;

	// user-supplied buffer to hold query results
	uint8_t *resultBuf;
	uint32_t resultBufLen;

	uint8_t __pad0[2];

	// query results (filled in by IASQueryValueByClass())
	uint16_t attribCount;
	IASAttribValueType **attribValues;	// 'attribCount' entries
	uint16_t *objectIDs;					// 'attribCount' entries

	// total response length
	uint32_t resultSize;

} IASQueryType;


/* IAS query functions */

status_t IASGetValueByClass(IASQueryType *ioQuery);


/* IAS entry management functions */

int32_t IASRegisterObject(int iSocket,
						  const IASObjectType *iObject,
						  Boolean iExclusive);

int32_t IASRegisterService(int iSocket,
						   const char *iServiceClass,
						   const uint8_t *iHintsBits,
						   Boolean iExclusive);

status_t IASUnregisterObject(int iSocket,
							 uint16_t iObjectID);

// flatten IASObjectType for delivery to IrLMP
status_t IASFlattenObject(const IASObjectType *iObject,
						  uint8_t *oBuf,
						  uint32_t *ioBufLen);


#ifdef __cplusplus
}	// extern "C"
#endif

#endif	// IAS_H
