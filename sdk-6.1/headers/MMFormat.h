/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. and Sony, Inc.  All rights reserved.
 *
 * File: FormatDefs.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_FORMAT_H
#define _MM_FORMAT_H

#include <MMDefs.h>
#include <MMFormatDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t MMFormat;
typedef int32_t MMFormatType;
typedef int32_t MMFormatVector;

#define P_MM_INVALID_FORMAT 0

/*	MMFormatCreate: create a new MMFormat object.  The caller is responsible
	for calling MMFormatDelete on this MMFormat when they no longer need it.
	OUT
		*outFormat: a valid MMFormat
*/
status_t MMFormatCreate(MMFormat * outFormat);

/*	MMFormatCopy: create a new MMFormat object by copying an existing MMFormat.
	The caller is responsible for calling MMFormatDelete on this MMFormat when
	they no longer need it.
	IN
		source: a valid MMFormat
	OUT
		*outDest: a valid MMFormat identical to 'source'
*/
status_t MMFormatCopy(MMFormat * outDest, MMFormat source);

/*	MMFormatDelete: deletes an MMFormat object.
	IN
		format: a valid MMFormat
*/
status_t MMFormatDelete(MMFormat format);

/*	MMFormatsCompatible: tests whether the two given MMFormat objects are
	compatible by matching their types and terms.
	IN
		a: a valid MMFormat
		b: a valid MMFormat
	RETURNS
		errNone if the formats are compatible, mediaErrFormatMismatch if not,
		or sysErrParamErr if either format has not been initialized.
*/
status_t MMFormatsCompatible(MMFormat a, MMFormat b);

/*	MMFormatGetType: returns the type of an MMFormat object.
	IN
		format: a valid MMFormat
	RETURNS
		the type of the format
*/
status_t MMFormatGetType(MMFormat format, MMFormatType * outType);

/*	MMFormatSetType: changes the basic format type of an MMFormat object.
	IN
		format: a valid MMFormat
		type: the new type
*/
status_t MMFormatSetType(MMFormat format, MMFormatType type);

/*	MMFormatGetTermType: returns the type of a term in an MMFormat.
	IN
		format: a valid MMFormat
		key: the key to search for
	OUT
		*outType: the term type
	RETURNS
		sysErrBadIndex: the key was not found
*/
status_t MMFormatGetTermType(MMFormat format, const char * key, MMTypeCode * outType);

/*	MMFormatGetTerm: returns the value of a term in an MMFormat.
	IN
		format: a valid MMFormat
		key: the key to search for
		valueType: the expected type of data
		outValue: a buffer capable of storing the given type of data
		ioValueSize: if typeCode specifies a variable-length value with
			no delimiter (P_MM_TYPE_RAW,) *ioLength should indicate the size of buffer
			in bytes.  otherwise *ioLength may be NULL.
	OUT
		*outValue: if found, and enough storage space was provided, the value
		*ioValueSize: the actual size of the value
	RETURNS
		sysErrParamErr: the given MMFormat was invalid
		sysErrBadIndex: the key was not found
		sysErrBadType: the given typeCode does not match the terms's type
        memErrNotEnoughSpace: the buffer size indicated by *ioLength was insufficient;
        	the buffer was filled to capacity with a partial value and *ioLength has been
        	set to indicate the required capacity.
*/
status_t MMFormatGetTerm(MMFormat format, const char * key, 
	MMTypeCode typeCode, void * outValue, int32_t * ioLength);

/*	MMFormatGetTermInt32: returns the value of a term in an MMFormat as an int32_t.
	IN
		format: a valid MMFormat
		key: the key to search for
		outValue: storage for the retrieved value
	OUT
		*outValue: if a term was found and representable as an int32_t (not a
			P_MM_TYPE_WILD, for example), the value
	RETURNS
		sysErrParamErr: the given MMFormat was invalid
		sysErrBadIndex: the key was not found
		sysErrBadType: the term's value could not be converted to int32_t
*/
status_t MMFormatGetTermInt32(
	MMFormat format, const char * key, int32_t * outValue);

/*	MMFormatSetTerm: sets the value of a term in an MMFormat, replacing any
	existing value for the given key.
	IN
		format: a valid MMFormat
		key: the key to set a value for
		typeCode: the type of data provided
		value: the data
		length: if typeCode specifies a variable-length value with no
			delimiter (such as P_MM_TYPE_RAW,) length should indicate the size
			of the value in bytes.  otherwise length will be ignored and
			should be set to 0.
	RETURNS
		sysErrParamErr: the given MMFormat was invalid
*/
status_t MMFormatSetTerm(
	MMFormat format, const char * key,
	MMTypeCode typeCode, const void * value, int32_t length);

/*	MMFormatSetTermInt32: sets the value of a term in an MMFormat to the
	given int32_t value, replacing any existing value for the given key.
	IN
		format: a valid MMFormat
		key: the key to set a value for
		value: the data
	RETURNS
		sysErrParamErr: the given MMFormat was invalid
*/
status_t MMFormatSetTermInt32(
	MMFormat format, const char * key, int32_t value);

/*	MMFormatEnumerateTerms: list the terms in the given MMFormat object
	IN
		format: a valid MMFormat
		*ioIterator: a previously-returned iterator value, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outKey: the key for the current term.  outKey must point to a buffer of size
			P_FORMAT_MAX_KEY_LENGTH or larger.
		*outType: the data type of the current term.  unspecified 'wildcard' terms may
			have type P_MM_TYPE_WILD (in which case they contain no data.)
	ERRORS
		sysErrParamErr: the given MMFormat was invalid
        sysErrBadIndex: invalid iterator value, or past the last item in the set
*/
status_t MMFormatEnumerateTerms(
	MMFormat format, int32_t * ioIterator, char * outKey, MMTypeCode * outType);

/*	some common formats (these are owned by the framework -- do not call MMFormatDelete()
 	on them.)  MMSessionCopy() may be used to create a format based on these formats,
	just like any other.
*/

MMFormat MMFormatRawAudio(void);

MMFormat MMFormatRawVideo(void);

MMFormat MMFormatRawStill(void);


/*	MMFormatVector read-only API: this allows MultimediaLib clients to read
	sets of formats (typically used to describe multiple allowed formats
	that the application may choose from.)
*/

/*	MMFormatVectorSize: return the number of MMFormat objects in the vector
	IN
		formatVector: a valid MMFormatVector
	RETURNS
		>= 0: the size of the vector
		sysErrParamErr: formatVector was not valid
*/
ssize_t MMFormatVectorSize(MMFormatVector formatVector);

/*	MMFormatVectorGetFormat: retrieve an MMFormat from the vector.  The
	caller is responsible for calling MMFormatDelete() on the format.

	IN
		formatVector: a valid MMFormatVector
		index: the index of the format to retrieve
	RETURNS
		a valid MMFormat, or
		P_MM_INVALID_FORMAT if the formatVector was not valid or index wa
		out of range.
*/
MMFormat MMFormatVectorGetFormat(MMFormatVector formatVector, ssize_t index);

/*	MMFormatVectorDelete: as with MMFormat, clients are responsible for
	explicitly deleting MMFormatVectors.  Call this function when you are
	done using a given MMFormatVector.

	IN
		formatVector: a valid MMFormatVector
	RETURNS
		errNone on success, or sysErrParamErr if formatVector was not valid.
*/
status_t MMFormatVectorDelete(MMFormatVector formatVector);

#ifdef __cplusplus
}
#endif
#endif // _MM_FORMAT_H

