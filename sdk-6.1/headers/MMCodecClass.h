/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMDefs.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_CODEC_H
#define _MM_CODEC_H

#include <MMDefs.h>
#include <MMFormat.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
	CodecClass Property Selectors
*/

#define P_MM_CODEC_CLASS_PROP_NAME			(P_MM_CODEC_CLASS_PROP_BASE | 0x0001L)	// char *
#define P_MM_CODEC_CLASS_PROP_VERSION		(P_MM_CODEC_CLASS_PROP_BASE | 0x0002L)	// char *
#define P_MM_CODEC_CLASS_PROP_CREATOR		(P_MM_CODEC_CLASS_PROP_BASE | 0x0003L)	// char *
#define P_MM_CODEC_CLASS_PROP_SOURCE_FORMAT	(P_MM_CODEC_CLASS_PROP_BASE | 0x0004L)	// MMFormat : if it is decoder
#define P_MM_CODEC_CLASS_PROP_DEST_FORMAT	(P_MM_CODEC_CLASS_PROP_BASE | 0x0005L)	// MMFormat : if it is encoder

/*	--------------------------------------------------------------------------------
    MMCodecClass functions
*/

/*	MMFileFormatEnumerate:
	list the supported file formats (these are distinct from codecs as a given
	file format may encapsulate many kinds of encoded data.)
	Start by placing P_MM_ENUM_BEGIN in the iterator param and continue calling 
	the function until P_MM_ENUM_END is returned in the iterator.
	IN
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outFormat: the next supported movie format
	ERRORS
		sysErrBadIndex: invalid iterator value, or no more formats to list
*/
extern status_t MMFileFormatEnumerate(int32_t * ioIterator, MMFormatType * outFormat); 

/*	MMCodecClassEnumerate: iterate through the available codecs, both encoders and decoders.
	IN
		type: a potential subset of codecs to enumerate, or P_FORMAT_TYPE_UNKNOWN for all
		*ioIterate: value returned by previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outCodecClassID: ID of the next available codec
	ERRORS
		sysErrParamErr: invalid MMType
		sysErrBadIndex: invalid iterator or no more codecs to list
*/
extern status_t MMCodecClassEnumerate(
		MMFormatType type, int32_t * ioIterator, MMCodecClassID * outCodecClassID);

#ifdef __cplusplus
}
#endif
#endif /* __MM_CODEC_H */
