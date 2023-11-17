/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMDefs.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_SESSION_H
#define _MM_SESSION_H

#include <MMDefs.h>
#include <MMFormat.h>
#include <MemoryMgr.h>
#include <DataMgr.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
    types
*/

// Struct to pass to callbacks and launches
typedef struct MMSessionEventTag
{
	MMSessionID		sessionRef;
	MMEvent			eventCode;
	int32_t			eventCause;
} MMSessionEvent;

typedef void (*MMSessionCallbackFn)(const MMSessionEvent * event, void * userdata);


/*	--------------------------------------------------------------------------------
	Session Control Operations 
*/

enum
{
	P_MM_SESSION_CTL_RUN =			0x01,
	P_MM_SESSION_CTL_PAUSE =		0x02,
	P_MM_SESSION_CTL_STOP =			0x03,
	P_MM_SESSION_CTL_PREFETCH =		0x04,
	P_MM_SESSION_CTL_GRAB =			0x05,
	P_MM_SESSION_CTL_REFRESH =		0x06,

	P_MM_SESSION_CTL_CUSTOM_BASE =	0x1000
};

/*	--------------------------------------------------------------------------------
	Session States
*/

enum
{
	P_MM_SESSION_NOT_INITIALIZED =	0x01,
	P_MM_SESSION_READY =			0x02,
	P_MM_SESSION_PREFETCHING =		0x03,
	P_MM_SESSION_PAUSED =			0x04,
	P_MM_SESSION_RUNNING =			0x05
};

/*	--------------------------------------------------------------------------------
	Session Properties
*/

// Boolean: 1 if the session should be enumerable; 0 (default) if it's not publicly visible
#define P_MM_SESSION_PROP_PUBLIC			(P_MM_SESSION_PROP_BASE | 0x0001L)

// int64_t: stream position in nanoseconds
#define P_MM_SESSION_PROP_CURRENT_TIME		(P_MM_SESSION_PROP_BASE | 0x0002L)

// int16_t: multiplier (1 == normal, 2 == double speed, etc.)
#define P_MM_SESSION_PROP_PLAYBACK_RATE		(P_MM_SESSION_PROP_BASE | 0x0003L)

// int64_t: prefetch time in nanoseconds
#define P_MM_SESSION_PROP_PREFETCH_TIME		(P_MM_SESSION_PROP_BASE | 0x0004L)

// Boolean: true if the session runs in the current process
#define P_MM_SESSION_PROP_IS_LOCAL			(P_MM_SESSION_PROP_BASE | 0x0005L)

// int64_t: stream position in nanoseconds at which marker event will be sent
#define P_MM_SESSION_PROP_MARKER			(P_MM_SESSION_PROP_BASE | 0x0006L)

// int64_t: playback start position in nanoseconds
#define P_MM_SESSION_PROP_START_TIME		(P_MM_SESSION_PROP_BASE | 0x0007L)

// int64_t: playback end position in nanoseconds
#define P_MM_SESSION_PROP_END_TIME			(P_MM_SESSION_PROP_BASE | 0x0008L)

// Boolean: enable playback repeat (return to start position when end position reached)
#define P_MM_SESSION_PROP_REPEAT_ENABLE		(P_MM_SESSION_PROP_BASE | 0x0009L)

// int32_t: the session class ID used to create this session
#define P_MM_SESSION_PROP_SESSION_CLASS		(P_MM_SESSION_PROP_BASE | 0x000AL)

/*	--------------------------------------------------------------------------------
	track defaults: applied automatically when tracks are added to the session
*/

// Boolean : enable audio tracks by default
#define P_MM_SESSION_DEFAULT_AUDIO_ENABLE	(P_MM_SESSION_PROP_BASE | 0x0020L)

// Boolean : enable video tracks by default
#define P_MM_SESSION_DEFAULT_VIDEO_ENABLE	(P_MM_SESSION_PROP_BASE | 0x0021L)

// RectangleType : position/size of source video rectangle
#define P_MM_SESSION_DEFAULT_SOURCE_RECT	(P_MM_SESSION_PROP_BASE | 0x0022L)

// RectangleType : position/size of screen video rectangle
#define P_MM_SESSION_DEFAULT_DEST_RECT		(P_MM_SESSION_PROP_BASE | 0x0023L)

// int32_t volume (see MMTrack.h for details)
#define P_MM_SESSION_DEFAULT_AUDIO_VOLUME	(P_MM_SESSION_PROP_BASE | 0x0024L)

/*	--------------------------------------------------------------------------------
	Source Properties
*/

// string
#define P_MM_SOURCE_PROP_URL				(P_MM_SOURCE_PROP_BASE | 0x0001L)
// MMFormat
#define P_MM_SOURCE_PROP_FILE_FORMAT		(P_MM_SOURCE_PROP_BASE | 0x0002L)

/*	--------------------------------------------------------------------------------
	Camera Source Property Selectors
*/

#define P_MM_EXPOSURE_AUTO	0

// int32_t: reciprocal of exposure in seconds or P_MM_EXPOSURE_AUTO
#define P_MM_SOURCE_PROP_CAMERA_EXPOSURE			(P_MM_SOURCE_PROP_BASE | 0x0010L)
	
// int32_t: exposure is multiplied by this value
#define P_MM_SOURCE_PROP_CAMERA_EXPOSURE_SCALE		(P_MM_SOURCE_PROP_BASE | 0x0011L)	
// int32_t: f-stop value * 1000
#define P_MM_SOURCE_PROP_CAMERA_APERTURE			(P_MM_SOURCE_PROP_BASE | 0x0012L)

enum
{
	P_MM_FLASH_MODE_OFF,
	P_MM_FLASH_MODE_AUTO,
	P_MM_FLASH_MODE_FRONT,
	P_MM_FLASH_MODE_SLOW,
	P_MM_FLASH_MODE_REAR
};

// int32_t: P_MM_FLASH_MODE_XXX
#define P_MM_SOURCE_PROP_CAMERA_FLASH_MODE			(P_MM_SOURCE_PROP_BASE | 0x0013L)

// Boolean
#define P_MM_SOURCE_PROP_CAMERA_RED_EYE_REDUCTION	(P_MM_SOURCE_PROP_BASE | 0x0014L)

enum
{
	P_MM_WHITE_BALANCE_AUTO,
	P_MM_WHITE_BALANCE_INDOOR,
	P_MM_WHITE_BALANCE_OUTDOOR,
	P_MM_WHITE_BALANCE_FLUORESCENT
};

#define P_MM_SOURCE_PROP_CAMERA_WHITE_BALANCE		(P_MM_SOURCE_PROP_BASE | 0x0015L)

enum
{
	P_MM_FOCUS_AUTO =		0,
	P_MM_FOCUS_INFINITY =	LONG_MAX
};

// int32_t: millimeters, or P_MM_FOCUS_AUTO, P_MM_FOCUS_INFINITY
#define P_MM_SOURCE_PROP_CAMERA_FOCUS				(P_MM_SOURCE_PROP_BASE | 0x0016L)

// int32_t: level/1000
#define P_MM_SOURCE_PROP_CAMERA_ZOOM				(P_MM_SOURCE_PROP_BASE | 0x0017L)

enum
{
	P_MM_ISO_SENSITIVITY_AUTO =	0
};

// int32_t: ISO ASA number or P_MM_ISO_SENSITIVITY_AUTO
#define P_MM_SOURCE_PROP_CAMERA_ISO_SENSITIVITY		(P_MM_SOURCE_PROP_BASE | 0x0018L)

/*	--------------------------------------------------------------------------------
	Dest Properties
*/

// string
#define P_MM_DEST_PROP_URL					(P_MM_DEST_PROP_BASE | 0x0001L)
// MMFormatType (int32_t)
#define P_MM_DEST_PROP_FILE_FORMAT			(P_MM_DEST_PROP_BASE | 0x0002L)

/*	--------------------------------------------------------------------------------
	Stream Properties
*/

// MMFormat
#define P_MM_STREAM_PROP_FORMAT				(P_MM_STREAM_PROP_BASE | 0x0001L)
// string
#define P_MM_STREAM_PROP_LANGUAGE			(P_MM_STREAM_PROP_BASE | 0x0002L)
// Boolean, defined for source streams: if true, this is a preview stream that
// may be connected to a render device; if false, this is a capture stream that
// may be connected to a media-stream destination (such as a file.)
#define P_MM_STREAM_PROP_IS_PREVIEW			(P_MM_STREAM_PROP_BASE | 0x0003L)
// MMFormatVector
#define P_MM_STREAM_PROP_SUPPORTED_FORMATS	(P_MM_STREAM_PROP_BASE | 0x0004L)

/*	--------------------------------------------------------------------------------
	Content Properties
	these are exposed by MMSession to describe the content being performed or captured
*/

#define P_MM_CONTENT_PROP_DURATION			(P_MM_CONTENT_PROP_BASE | 0x0001L)	// int32_t msecs
#define P_MM_CONTENT_PROP_TITLE				(P_MM_CONTENT_PROP_BASE | 0x0002L)	// string
#define P_MM_CONTENT_PROP_ARTIST			(P_MM_CONTENT_PROP_BASE | 0x0003L)	// string 
#define P_MM_CONTENT_PROP_PLAYLIST			(P_MM_CONTENT_PROP_BASE | 0x0004L)	// string
#define P_MM_CONTENT_PROP_GENRE				(P_MM_CONTENT_PROP_BASE | 0x0005L)	// string
#define P_MM_CONTENT_PROP_ALBUM				(P_MM_CONTENT_PROP_BASE | 0x0006L)	// string
#define P_MM_CONTENT_PROP_TRACK_NUMBER		(P_MM_CONTENT_PROP_BASE | 0x0007L)	// int32_t

/*	--------------------------------------------------------------------------------
	MMSession Constants
*/

enum
{
	P_MM_SESSION_CREATE_ANY_PROCESS,	// required for background playback
	P_MM_SESSION_CREATE_LOCAL_PROCESS
};

#define P_MM_DEFAULT_SOURCE 0
#define P_MM_DEFAULT_DEST 0

#define P_MM_NULL_URL					"palmdev:///Media/Null"
	
#define P_MM_DEFAULT_AUDIO_CAPTURE_URL	"palmdev:///Media/Default/AudioIn"
#define P_MM_DEFAULT_VIDEO_CAPTURE_URL	"palmdev:///Media/Default/VideoIn"
#define P_MM_DEFAULT_STILL_CAPTURE_URL	"palmdev:///Media/Default/StillIn"
#define P_MM_DEFAULT_AUDIO_RENDER_URL	"palmdev:///Media/Default/AudioOut"
#define P_MM_DEFAULT_VIDEO_RENDER_URL	"palmdev:///Media/Default/VideoOut"

/*	--------------------------------------------------------------------------------
	MMSession Functions
*/

/*	MMSessionCreate: create a new session
	OUT
		*outSession: a valid MMSessionID
	ERRORS
		sysErrNoFreeResource: no sessions available
*/
extern status_t MMSessionCreate(MMSessionClassID sessionClass, int32_t flags, MMSessionID * outSession);

/*	MMSessionDelete: delete a session
 	IN
		session: a valid MMSessionID
	SIDE EFFECTS
		on success the given MMSessionID will no longer be valid.
*/
extern status_t MMSessionDelete(MMSessionID session);

/*	MMSessionEnumerate: iterate through the current public sessions (only the sessions for
	which the value of P_MM_SESSION_PROP_PUBLIC is nonzero.)
	IN
		*ioIterator: valid iterator value from a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outSession: a valid MMSessionID for the next session in the set
	ERRORS
		sysErrBadIndex: invalid iterator value, or at end of set
*/
extern status_t MMSessionEnumerate(int32_t * ioIterator, MMSessionID * outSession);

/*	MMSessionReleaseOwnership:
	when a session is created, it is initially referenced by the process
	which created it.  when that process exits, the session is automatically
	deleted.  to make the session persist after the calling process exits,
	call this function (it must be called from the process which created or
	currently owns the session.)  the session will persist in the background
	until it is explicitly deleted or MMSessionAcquireOwnership() is called.
	IN
		session: a valid MMSessionID, created from or owned by this process
*/
extern status_t MMSessionReleaseOwnership(MMSessionID session);

/*	MMSessionAcquireOwnership:
	make the given session owned by the calling process (it will be deleted
	automatically when that process exits.)  the session must currently not
	be owned by another process.
	IN
		session: a valid MMSessionID, on which MMSessionReleaseOwnership()
		was previously called.
*/
extern status_t MMSessionAcquireOwnership(MMSessionID session);


/*	MMSessionRegisterCallback:
 	register a callback function to "observe" this session by handling events.
	the implementation will call your function asynchronously, meaning that it's safe
	to make MM calls from your function.  there may be multiple callback functions
	registered for a session at the same time.
	IN
		session: a valid MMSessionID
		callback: a pointer to a handler function
		userdata: arbitrary user-provided data, or NULL
		eventFlags: currently unused (must be 0)
	ERRORS
		sysErrNotAllowed : too many callbacks already for this session
		sysErrParamErr: invalid session
*/
extern status_t MMSessionRegisterCallback(MMSessionID session, MMSessionCallbackFn callback, void * userdata,
		uint32_t eventFlags);

/*	MMSessionUnregisterCallback:
	remove the given callback function/context pair from the list of observers.
	IN
		session: a valid MMSessionID
		callback: the function to call
		userdata: arbitrary user-provided data, or NULL
	ERRORS
		sysErrParamErr: invalid session
		sysErrBadData: the given callback/context pair was not found
*/
extern status_t MMSessionUnregisterCallback(MMSessionID session, MMSessionCallbackFn callback, void * userdata);

/*	MMSessionRegisterLaunch:
	register a handler application to be sublaunched when an event occurs.
	the application will be sublaunched with code sysAppLaunchCmdMultimediaEvent,
	with a pointer to an MMSessionEvent.
	IN
		session: a valid MMSessionID
		dbID: ID of the application resource database
		eventFlags: currently unused (must be 0)
	ERRORS
		sysErrParamErr: invalid session 
*/
extern status_t MMSessionRegisterLaunch(MMSessionID session, DatabaseID dbID, uint32_t eventFlags);

/*	MMSessionUnregisterLaunch:
	remove a registered event handler
	IN
		session: a valid MMSessionID
		cardNo : pass 0
		dbID   : ID of the application resource database
	ERRORS
		sysErrParamErr: invalid session
		sysErrBadData: no matching registration was found
*/
extern status_t MMSessionUnregisterLaunch(MMSessionID session, DatabaseID dbID);

/*	MMSessionAddSource:
	add a data source to the session, using the given URL.  on success, an
	MMSourceID will be returned.  some session classes may expose additional
	source properties that may be set before calling MMSourceFinalize() to open
	the source and create streams.
	IN
		session: a valid MMSessionID
		sourceURL: an URL to the data source to perform from.
	OUT
		*outSource: a valid MMSourceID
	ERRORS
		sysErrParamErr: invalid session
		sysErrUnsupported: no more sources may be added to this session
*/
extern status_t MMSessionAddSource(MMSessionID session, const char * sourceURL, MMSourceID * outSource);

/*	MMSourceFinalize:
	attempt to open the given data source and create streams.  on success, the
	streams may be enumerated with MMSourceEnumerateStreams().
*/
extern status_t MMSourceFinalize(MMSourceID source);

/*	MMSessionEnumerateSources:
	lists the sources in this session.
	IN
		session: a valid MMSessionID
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outSource: the next source in the set.  the MMSourceID will remain valid until
			the session is deleted or MMSessionRemoveAll() is called.
*/
extern status_t MMSessionEnumerateSources(MMSessionID session, int32_t * ioIterator, MMSourceID * outSource);

/*	MMSourceEnumerateStreams:
	enumerates the streams available in a given source.  the source must
	be finalized for this call to succeed.
	IN
		source: a valid MMSourceID
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outStream: the next stream in the set.  the MMStreamID will remain valid until
			the session is deleted or MMSessionRemoveAll() is called.
*/
extern status_t MMSourceEnumerateStreams(MMSourceID source, int32_t * ioIterator, MMStreamID * outStream);

/*	MMSessionAddDest:
	add a data destination to the session, using the URL.  on success, an
	MMDestID will be returned.  some session classes may expose additional
	destination properties that may be set before calling MMDestFinalize() to open
	the destination and create streams.
	IN
		session: a valid MMSessionID
		destURL: an URL to the destination data source.
	OUT
		*outDest: a valid MMDestID
	ERRORS
		sysErrParamErr: invalid session
		sysErrUnsupported: no more destinations may be added to this session
*/
extern status_t MMSessionAddDest(MMSessionID session, const char * destURL, MMDestID * outDest);

/*	MMDestFinalize:
	attempt to open the given data destination and create streams.  on success, the
	streams may be enumerated with MMDestEnumerateStreams().
*/
extern status_t MMDestFinalize(MMDestID dest);

/*	MMSessionEnumerateDests:
	lists the destinations in this session.
	IN
		session: a valid MMSessionID
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outDest: the next destination in the set.  the MMDestID will remain valid until
			the session is deleted or MMSessionRemoveAll() is called.
*/
extern status_t MMSessionEnumerateDests(MMSessionID session, int32_t * ioIterator, MMDestID * outDest);

/*	MMDestEnumerateStreams:
	enumerates the streams available in a given destination.  the destination must
	be finalized for this call to succeed.
	IN
		dest: a valid MMDestID
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outStream: the next stream in the set.  the MMStreamID will remain valid until
			the session is deleted or MMSessionRemoveAll() is called.
*/
extern status_t MMDestEnumerateStreams(MMDestID dest, int32_t * ioIterator, MMStreamID * outStream);

/*	MMSessionAddTrack:
	add a track to the session.  a track represents a particular data-processing route,
	which, depending on the session class, may be used for playback (rendering)
	or capture (storage to a local file or network stream.)
	IN
		session: a valid MMSessionID
		sourceStream: a valid MMSourceID
		sourceFormat: a valid MMFormat, or P_MM_INVALID_FORMAT
		destStream: a valid MMDestID
		destFormat: a valid MMFormat, or P_MM_INVALID_FORMAT
	OUT
		*outTrack: if successful, the MMTrackID for the created track
*/
extern status_t MMSessionAddTrack(MMSessionID session,
		MMStreamID sourceStream, MMFormat sourceFormat,
		MMStreamID destStream, MMFormat destFormat,
		MMTrackID * outTrack);

/*	MMSessionAddDefaultTracks:
	add all tracks applicable to this session, using the given source and/or
	destination.  for example, a playback session will add all the tracks it
	can play concurrently.  after a call to this function succeeds the caller
	may use MMSessionEnumerateTracks() to inspect the created tracks and futher
	configure them prior to calling MMSessionFinalize().  if P_MM_DEFAULT_SOURCE
	or P_MM_DEFAULT_DEST are passed, sources and/or destinations will be added
	and finalized as necessary.  the caller may use MMSessionEnumerateSources()
	and MMSessionEnumerateDests() to retrieve MMSourceIDs and MMDestIDs for
	those objects.
	IN
		session: a valid MMSessionID
		source: a valid MMSourceID, or P_MM_DEFAULT_SOURCE
		dest: a valid MMDestID, or P_MM_DEFAULT_DEST
*/
extern status_t MMSessionAddDefaultTracks(MMSessionID session, MMSourceID source, MMDestID dest);

/*	MMSessionRemoveAll:
	remove sources, destinations, and the tracks which connect them.
	this will reset	the session state to P_MM_SESSION_NOT_INITIALIZED.
	non-content properties will be unchanged.
	IN
		session: a valid MMSessionID
	ERRORS
		sysErrParamErr: invalid session
*/
extern status_t MMSessionRemoveAll(MMSessionID session);

/*	MMSessionRemoveTracks:
	remove all tracks.  existing source and destination streams may be
	used to create new tracks. this will reset	the session state to 
	remove sources, destinations, and the tracks which connect them.
	this will reset	the session state to P_MM_SESSION_NOT_INITIALIZED.
	non-content properties will be unchanged.
	IN
		session: a valid MMSessionID
	ERRORS
		sysErrParamErr: invalid session
*/
extern status_t MMSessionRemoveTracks(MMSessionID session);

/*	MMSessionRemoveSource:
	remove an individual source from the session.  any tracks using
	this source will also be removed.  if no tracks are left, this
	will reset the session state to P_MM_SESSION_NOT_INITIALIZED.
	non-content properties will be unchanged.
	IN
		session: a valid MMSessionID
		source: a valid MMSourceID from the given session
	ERRORS
		sysErrParamErr: invalid session or source
*/
extern status_t MMSessionRemoveSource(MMSessionID session, MMSourceID source);

/*	MMSessionRemoveDest:
	remove an individual destination from the session.  any tracks using
	this destination will also be removed.  if no tracks are left, this
	will reset the session state to P_MM_SESSION_NOT_INITIALIZED.
	non-content properties will be unchanged.
	IN
		session: a valid MMSessionID
		dest: a valid MMDestID from the given session
	ERRORS
		sysErrParamErr: invalid session or dest
*/
extern status_t MMSessionRemoveDest(MMSessionID session, MMDestID dest);

/*	MMSessionRemoveTrack:
	remove an individual track from the session.  if no tracks are left,
	this will reset the session state to P_MM_SESSION_NOT_INITIALIZED.
	non-content properties will be unchanged.
	IN
		session: a valid MMSessionID
		track: a valid MMTrackID from the given session
	ERRORS
		sysErrParamErr: invalid session or dest
*/
extern status_t MMSessionRemoveTrack(MMSessionID session, MMTrackID track);

/*	MMSessionFinalize:
	Finalize the set of tracks for this session.  If there were no previously
	finalized tracks, then after this call succeeds the session will enter the
	P_MM_SESSION_READY state.  If this function is called after adding new
	tracks to a session that was already running, the state will not change
	and the new tracks will begin running.
	IN
		session: a valid MMSessionID
	ERRORS
		sysErrParamErr: invalid session
*/
extern status_t MMSessionFinalize(MMSessionID session);

/*	MMSessionEnumerateTracks:
	list the tracks created in this session.
	IN
		session: a valid MMSessionID
		*ioIterator: value returned by a previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outTrack: the next track in the set.  the MMTrackID will remain valid until
			the session is deleted or MMSessionRemoveAll() is called.
	ERRORS
		sysErrParamErr: invalid session
		sysErrBadIndex: invalid iterator value, or no more tracks to list
*/
extern status_t MMSessionEnumerateTracks(MMSessionID session, int32_t * ioIterator, MMTrackID * outTrack);

/*	MMSessionControl: sends control opcodes for playback, capture, and preview
	IN
		session: a valid MMSessionID
		sessionCtl: the opcode
	ERRORS
		sysErrParamErr: invalid session
		sysErrNotAllowed: operation not allowed for this session
*/
extern status_t MMSessionControl(MMSessionID session, MMSessionControlOpcode sessionCtl);

/*	MMSessionSeek: Seek to a different location in the current session. This is equivalent to 
	skipping forward or backward until a designated time. If the file is currently being 
	played, playback will resume from the new file offset position. Offset adjustment is made 
	either relative to the current position of the file or relative to the beginning of the 
	file. The adjustment is specified in number of nanoseconds where a positive value means 
	going forward and a negative value means going backward. Out of range file offset is 
	considered as an error.
	After a successful seek operation the session will be stopped (enter P_MM_SESSION_READY state)
	whether or not it was playing previous to the call.
	IN
		session: a valid MMSessionID
		position: position or distance to seek
		origin: point to offset from
	ERRORS
		sysErrParamErr: invalid session
		sysErrOutOfRange: position out of range
		sysErrUnsupported: seeking not supported in current session (capture or some
			streaming types.)
*/
extern status_t MMSessionSeek(MMSessionID session, MMSeekOrigin origin, int64_t position);

/*	MMSessionGetState: return the current state of a session
	IN
		session: a valid MMSessionID
	OUT
		*outState: the current state -- one of:
			P_MM_SESSION_NOT_INITIALIZED
			P_MM_SESSION_READY
			P_MM_SESSION_PREFETCHING
			P_MM_SESSION_PAUSED
			P_MM_SESSION_RUNNING		
	ERRORS
		sysErrParamErr: invalid session
*/
extern status_t MMSessionGetState(MMSessionID session, MMSessionState * outState); 

#ifdef __cplusplus
}
#endif
#endif /* _MM_SESSION_H */

