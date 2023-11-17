/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMDefs.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_DEFS_H
#define _MM_DEFS_H

#include <PalmTypes.h>
#include <CmnErrors.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------------
    types
*/

typedef int32_t MMTypeCode;
typedef int32_t MMPropInfoType;

#define P_MM_INVALID_ID 0

// object IDs
typedef int32_t MMSessionID;
typedef int32_t MMSourceID;
typedef int32_t MMDestID;
typedef int32_t MMStreamID;
typedef int32_t MMTrackID;
typedef int32_t MMFilterID;

// class (add-on) IDs
typedef int32_t MMCodecClassID;
typedef int32_t MMSessionClassID;

typedef int32_t MMSessionState;
typedef int32_t MMSessionControlOpcode;

typedef int32_t MMEvent;

typedef int8_t  MMSeekOrigin;

/* --------------------------------------------------------------------------------
    constants
*/

/* --------------------------------------------------------------------------------
	MM Property Bases: 
*/

enum
{
	// Standard properties defined by PalmSource.
	// They will be published for 3rd party use & implementation.
	P_MM_STANDARD_PROP_BASE =		0x00010000L,
	
	// These properties are implementation-defined.  Developers
	// can use properties within this base however they see fit.
	// They should be published by each developer for public use,
	// and may have a different meaning for each component.
	P_MM_USER_PROP_BASE =			0x00020000L,

	// Properties that are private to the particular developer.
	// These will not be published, they are intended for 
	// implementing test modes, or any other private feature.
	// Like the properties beginning at P_USER_PROP_BASE, these
	// properties may be implementation specific.
	P_MM_PRIVATE_PROP_BASE =		0x00030000L
};

/* --------------------------------------------------------------------------------
	MM object types

	The top byte of the property ID refers to the type of MMLib object it
	controls.
*/

enum
{
	P_MM_PROP_OBJECT_MASK =				0xFF000000L,

	P_MM_PROP_OBJECT_SESSION =			(1L << 24),
	P_MM_PROP_OBJECT_CONTENT =			(2L << 24),
	P_MM_PROP_OBJECT_SOURCE =			(3L << 24),
	P_MM_PROP_OBJECT_DEST =				(4L << 24),
	P_MM_PROP_OBJECT_STREAM =			(5L << 24),
	P_MM_PROP_OBJECT_TRACK =			(6L << 24),
	P_MM_PROP_OBJECT_DEVICE =			(7L << 24),
	P_MM_PROP_OBJECT_SESSION_CLASS =	(8L << 24),
	P_MM_PROP_OBJECT_CODEC_CLASS =		(9L << 24)
};

enum
{
	P_MM_SESSION_PROP_BASE =		P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_SESSION,
	P_MM_CONTENT_PROP_BASE =		P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_CONTENT,
	P_MM_SOURCE_PROP_BASE =			P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_SOURCE,
	P_MM_DEST_PROP_BASE =			P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_DEST,
	P_MM_STREAM_PROP_BASE =			P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_STREAM,
	P_MM_TRACK_PROP_BASE =			P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_TRACK,
	P_MM_DEVICE_PROP_BASE =			P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_DEVICE,
	P_MM_SESSION_CLASS_PROP_BASE =	P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_SESSION_CLASS,
	P_MM_CODEC_CLASS_PROP_BASE =	P_MM_STANDARD_PROP_BASE | P_MM_PROP_OBJECT_CODEC_CLASS
};

/*
* Generic Enumeration iterators for use in Enumeration functions
*/
enum
{
	P_MM_ENUM_BEGIN =		0,
	P_MM_ENUM_END =			-1 
};

/*
* callback events
*/

enum
{
	P_MM_EVENT_SESSION_STATE_CHANGED =		0x01,
	P_MM_EVENT_SESSION_MARKER_EXPIRED =		0x02,
	P_MM_EVENT_SESSION_DELETING =			0x03,
	P_MM_EVENT_SESSION_WARNING =			0x04,
	P_MM_EVENT_CUSTOM_BASE =				0x1000
};

enum
{
	P_MM_EVENT_CAUSE_UNKNOWN =				0x01,
	P_MM_EVENT_CAUSE_REQUESTED_BY_APP =		0x02,
	P_MM_EVENT_CAUSE_END_OF_STREAM =		0x03,
	P_MM_EVENT_CAUSE_INVALID_STREAM =		0x04,
	P_MM_EVENT_CAUSE_STORAGE_FULL =			0x05,
	P_MM_EVENT_CAUSE_CUSTOM_BASE =			0x1000
};

/*
* Seek Origin : Predefined position from which to seek a relative offset time
*/
enum
{
	P_MM_SEEK_ORIGIN_BEGIN,
	P_MM_SEEK_ORIGIN_CURRENT,
	P_MM_SEEK_ORIGIN_END,
};

/*
* Property types
*/

#define P_MM_TYPE_CODE_MASK		0x7f7f7f00
#define P_MM_TYPE_CODE_SHIFT	8
#define MM_TYPE_CODE(code)		((code << P_MM_TYPE_CODE_SHIFT) & P_MM_TYPE_CODE_MASK)

/*
* standard (system) types
*/

enum
{
	P_MM_UNDEFINED_TYPE =		MM_TYPE_CODE(0),		// no data associated
	P_MM_WILD_TYPE =			MM_TYPE_CODE('wld'),	// no data associated
	P_MM_RAW_TYPE =				MM_TYPE_CODE('raw'),	// void *
	P_MM_INT8_TYPE =			MM_TYPE_CODE('i08'),	// int8_t
	P_MM_INT16_TYPE =			MM_TYPE_CODE('i16'),	// int16_t
	P_MM_INT32_TYPE =			MM_TYPE_CODE('i32'),	// int32_t
	P_MM_INT64_TYPE =			MM_TYPE_CODE('i64'),	// int64_t
	P_MM_BOOL_TYPE =			MM_TYPE_CODE('bol'),	// bool
	P_MM_STRING_TYPE =			MM_TYPE_CODE('str')		// char *
};

/*
* MMLib-specific types
*/

enum
{
	P_MM_RECT_TYPE =			MM_TYPE_CODE('Rct'),	// RectangleType
	P_MM_FORMAT_TYPE =			MM_TYPE_CODE('Fmt'),	// MMFormat
	P_MM_FORMAT_VECTOR_TYPE =	MM_TYPE_CODE('Fmv')		// MMFormatVector
};

/*
* Property info opcodes for MMPropertyInfo
*/

enum
{
	P_MM_PROP_INFO_DEFAULT,	// data type depends on property
	P_MM_PROP_INFO_MINIMUM,	// data type depends on property
	P_MM_PROP_INFO_MAXIMUM,	// data type depends on property
	P_MM_PROP_INFO_READABLE,	// P_MM_BOOL_TYPE
	P_MM_PROP_INFO_WRITABLE,	// P_MM_BOOL_TYPE
	P_MM_PROP_INFO_TYPE_CODE	// P_MM_INT32_TYPE
};

#ifdef __cplusplus
}
#endif
#endif /* _MM_DEFS_H */

