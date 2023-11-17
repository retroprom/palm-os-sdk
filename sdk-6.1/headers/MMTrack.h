/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMTrack.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_TRACK_H
#define _MM_TRACK_H

#include <MMDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
	Track Property Selectors
*/ 

// MMFormat
#define P_MM_TRACK_PROP_SOURCE_FORMAT	(P_MM_TRACK_PROP_BASE | 0x0001L)

// MMFormat
#define P_MM_TRACK_PROP_DEST_FORMAT		(P_MM_TRACK_PROP_BASE | 0x0002L)

// Boolean
#define P_MM_TRACK_PROP_ENABLE			(P_MM_TRACK_PROP_BASE | 0x0003L)

// int32_t: MMSourceID
#define P_MM_TRACK_PROP_SOURCE			(P_MM_TRACK_PROP_BASE | 0x0004L)

// int32_t: MMStreamID
#define P_MM_TRACK_PROP_SOURCE_STREAM	(P_MM_TRACK_PROP_BASE | 0x0005L)

// int32_t: MMDestID
#define P_MM_TRACK_PROP_DEST			(P_MM_TRACK_PROP_BASE | 0x0006L)

// int32_t: MMStreamID
#define P_MM_TRACK_PROP_DEST_STREAM		(P_MM_TRACK_PROP_BASE | 0x0007L)

// int32_t: MMCodecClassID
#define P_MM_TRACK_PROP_CODEC_CLASS		(P_MM_TRACK_PROP_BASE | 0x0008L)

// int64_t: current position in nanoseconds
#define P_MM_TRACK_PROP_CURRENT_TIME	(P_MM_TRACK_PROP_BASE | 0x0009L)

/*	--------------------------------------------------------------------------------
	Audio Track Property Selectors
*/

// int32_t; only 16 bits are used at present:
// * both gain and attenuation values are linear in terms of loudness, not amplitude.
// 0-1024: attenuation:
// 		0.09375 dB per step.
//		0 = -infinity dB (special case)
//		1 = -95.90625dB (that is, the first non-silent volume level)
//		1024 = 0dB (no attenuation)
// 1025-4096: gain
//		gain: .01041666667 dB per step.
//		1024 = 0dB (no gain)
//		4096 = 32dB gain

#define P_MM_TRACK_PROP_VOLUME			(P_MM_TRACK_PROP_BASE | 0x0020L)

/*	--------------------------------------------------------------------------------
	Video Track Property Selectors
*/

// RectangleType
#define P_MM_TRACK_PROP_SOURCE_RECT		(P_MM_TRACK_PROP_BASE | 0x0030L)
// RectangleType
#define P_MM_TRACK_PROP_DEST_RECT		(P_MM_TRACK_PROP_BASE | 0x0031L)

/*	--------------------------------------------------------------------------------
	Track Filters
*/

typedef struct _FilterCallbackInfo
{
	int64_t	timeStamp;		// in nanoseconds
	size_t	bufferSize;		// in bytes
} FilterCallbackInfo;

typedef void (*MMFilterCallbackFn)(
	MMTrackID track, void * buffer, FilterCallbackInfo * info, void * userdata);

/*	MMTrackInsertCallbackFilter:
	add a callback function to process data on the given track.  only one
	callback may be installed on a given track.
*/
extern status_t MMTrackInsertCallbackFilter(
		MMTrackID track, MMFilterCallbackFn callback, void * userdata);

/*	MMTrackRemoveCallbackFilter:
	remove a previously installed callback function from the given track.
*/
extern status_t MMTrackRemoveCallbackFilter(MMTrackID track);

#ifdef __cplusplus
}
#endif
#endif /* _MM_TRACK_H */


