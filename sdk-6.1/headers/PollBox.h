/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PollBox.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              Public header file for PollBox users
 *
 *****************************************************************************/

#ifndef _POLLBOX_H_
#define _POLLBOX_H_

#include <PalmTypes.h>
#include <IOS.h>

#ifdef __cplusplus
extern "C" {
#endif 


struct PollBox; // keep the arm compiler happy

//--------------------------------------
// Callback procedure type.
//
typedef void PbxCallback(
	struct PollBox*	pbx,		// the PollBox
	struct pollfd*	pollFd,		// fd, event mask, and received events
	void*			context		// user context
);

//--------------------------------------
// The semi-opaque PollBox.
//

typedef struct
{
	PbxCallback*	callback;
	void*			context;
	uint16_t		flags;
	uint16_t		_padding;
}
PbxInfo;

typedef struct PollBox
{
	struct pollfd*	pollTab;	// public:  per-fd table of structs for IOSPoll()
	uint16_t		count;		// public:  number of fds in the box
	uint16_t		capacity;	// private: current max number of fds
	uint16_t		flags;		// private: internal flags
	uint16_t		nCallbacks;	// private: number of fds with callbacks
	PbxInfo*		infoTab;	// private: per-fd table for callbacks, etc.
}
PollBox;

//--------------------------------------
// Create a PollBox.
// Returns:
//		NULL .... memory allocation failed
//		other ... pointer to the new PollBox
//
PollBox* PbxCreate( void );

//--------------------------------------
// Destroy a PollBox -- close any contained file descriptors,
// and deallocate memory.
//
void PbxDestroy(
	PollBox*		pbx			// the PollBox to destroy
);

//--------------------------------------
// Add a file descriptor to a PollBox.
// Returns:
//		0 ...................... success
//		memErrNotEnoughSpace ... memory allocation failed
//
status_t PbxAddFd(
	PollBox*		pbx,		// the PollBox
	int32_t			fd,			// the file descriptor to add
	int16_t			eventMask,  // events of interest on this fd
	PbxCallback*	callback,   // the callback procedure for this fd
	void*			context		// context variable for this fd
);

//--------------------------------------
// Remove a file descriptor from a PollBox.
//
void PbxRemoveFd(
	PollBox*		pbx,		// the PollBox
	int32_t			fd			// the file descriptor to remove
);

//--------------------------------------
// Poll the file descriptors in a PollBox.
//
// If there are no more file descriptors in the PollBox, then this function
// sets *nReady to zero and returns zero.
// 
// Otherwise, this function blocks until one or more file descriptors have
// events of interest, or the given timeout expires. For each file descriptor
// that has events, the associated callback procedure (if there is one) is
// called. The number of file descriptors that have events is returned in
// *nReady.
// 
// Note that if this function returns zero and sets *nReady to zero, then
// either there are no more file descriptors in the PollBox, or the timeout
// expired. You can check pbx->count to determine if there are any file
// descriptors remaining in the PollBox.
// 
// Returns:
//		0 ....... success; *nReady >= is the number of fds with events
//		other ... error; *nReady == -1
//
status_t PbxPoll(
	PollBox*	pbx,		// the PollBox
	int32_t		timeout,	// timeout in milliseconds, -1 means "infinity"
	int32_t*	nReady		// receives the count of ready fds; -1 if error
);

//--------------------------------------
// Run an event loop using a PollBox. At each iteration call PbxPoll()
// without an "infinite" timeout. The loop stops and this function returns
// if there are no more file descriptors, or some unexpected error occurs.
//
// Returns:
//		0 ....... there are no more file descriptors
//		other ... error
//
status_t PbxRun(
	PollBox*	pbx 		// the PollBox
);

#ifdef __cplusplus
}
#endif

#endif // _POLLBOX_H_
