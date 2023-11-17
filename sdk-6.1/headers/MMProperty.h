/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMProperty.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_PROPERTY_H
#define _MM_PROPERTY_H

#include <MMDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
    MMProperty functions
	all MMProperty functions take any valid ID (MMCodecClassID, MMSourceID,
	MMDestID, MMTrackID, MMSessionID, MMSessionClassID) as the first argument.
*/


/*	MMPropertySet: set a property value of the given object.
	IN
		id: a valid ID
		property: a valid property ID
		typeCode: the type of the given data
		value, length: value is a pointer to a the desired property value.  if
			typeCode specifies a variable-length value with no delimiter
			(P_MM_TYPE_RAW,) length should indicate the size of the value in bytes.
			otherwise length will be ignored and should be set to 0.
	ERRORS
		sysErrParamErr: invalid property or object ID
		sysErrBadType: the given typeCode does not match the property's type
*/
extern status_t MMPropertySet(int32_t id, int32_t property,
		MMTypeCode typeCode, const void * value, uint32_t length);

/*	MMPropertyGet: get the current value of the given property of the given object.
	IN
		id: a valid ID
		property: a valid property ID
		typeCode: the type of data expected
		ioValue, ioLength: ioValue is a pointer to a buffer of size (*ioLength),
			or a buffer the caller guarantees to be large enough (if ioLength == NULL),
			or ioValue is NULL
	OUT
		*ioValue: the property value
        *ioLength: the length of the property data
	ERRORS
		sysErrParamErr: invalid property or object ID
		sysErrBadType: the given typeCode does not match the property's type
        memErrNotEnoughSpace: the buffer size indicated by *ioLength was insufficient;
        	the buffer was filled to capacity with a partial value and *ioLength has been
        	set to indicate the required capacity
*/
extern status_t MMPropertyGet(int32_t id, int32_t property,
		MMTypeCode typeCode, void * outValue, int32_t * ioValueLen);

/*	MMPropertyInfo: get metadata about a property.
	IN
		id: a valid ID
		infoType: P_MM_PROP_INFO_DEFAULT, P_MM_PROP_INFO_MINIMUM, P_MM_PROP_INFO_MAXIMUM,
			P_MM_PROP_INFO_READABLE, P_MM_PROP_INFO_WRITABLE or P_MM_PROP_INFO_TYPE_CODE.
		property: a valid property ID
		typeCode: the type of data expected
		ioValue, ioLength: ioValue is a pointer to a buffer of size (*ioLength),
			or a buffer the caller guarantees to be large enough (if ioLength == NULL),
			or ioValue is NULL
	OUT
		*ioValue: the property value
        *ioLength: the length of the property data
	ERRORS
		sysErrParamErr: invalid property or object ID
		sysErrBadType: the given typeCode does not match the info field's type
        memErrNotEnoughSpace: the buffer size indicated by *ioLength was insufficient;
        	the buffer was filled to capacity with a partial value and *ioLength has been
        	set to indicate the required capacity
*/
extern status_t MMPropertyInfo(int32_t id, MMPropInfoType infoType, int32_t property,
		MMTypeCode typeCode, void * outValue, int32_t * ioValueLen);
		      		      
/*	MMPropertyEnumerate:
	list the supported values of the given property for the given object
	IN
		id: a valid ID
		property: a valid property ID
		typeCode: the type of data expected
		*ioIterator: a previously-returned iterator value, or P_MM_ENUM_BEGIN for the first call
		outValue, ioLength: ioValue is a pointer to pointer of size (*ioLength)
	OUT
		*outValue: the property value
		*ioLength: the length of that property value
	ERRORS
        sysErrBadIndex: invalid iterator value, or past the last item in the set
        sysErrNotAllowed: cannot enumerate this property
		sysErrParamErr: invalid property or object ID
		sysErrBadType: the given typeCode does not match the info field's type
        memErrNotEnoughSpace: the buffer size indicated by *ioLength was insufficient;
        	the buffer was filled to capacity with a partial value and *ioLength has been
        	set to indicate the required capacity
*/
extern status_t MMPropertyEnumerate(int32_t id, int32_t property, int32_t * ioIterator,
		MMTypeCode typeCode, void * outValue, int32_t * ioLength);
		
#ifdef __cplusplus
}
#endif
#endif /* _MM_PROPERTY_H */

