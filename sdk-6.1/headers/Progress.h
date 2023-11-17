/******************************************************************************
 *
 * Copyright (c) 1996-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Progress.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *	  This header file defines a generic progress dialog interface
 *
 *****************************************************************************/

#ifndef	__PROGRESS_H__
#define	__PROGRESS_H__

#include <PalmTypes.h>						// Basic types

#include <Form.h>
#include <Rect.h>
#include <Window.h>


#define progressMaxMessage		128
#define progressMaxTitle		31			// max size for title of progress dialog
#define progressMaxButtonText	7			// max size of text in OK/Cancel button

// Progress callback function
// The progress dialog will call this function to get the text to display for the
// current status. 
// stage - the current stage of progess as defined by your app
// message - text that can be sent from the protocol 
// cancel - true if the dialog is in cancel mode
// error - current error (func should return an error message in this case...
typedef struct
{
	uint16_t	stage;					// <= current stage
	uint16_t	bitmapId;				// => resource ID of bitmap to display
	DmOpenRef	bitmapDatabase;			// => database in which to find the icon, or NULL to look in UILib.
	uint32_t	textLen;				// <= length of text buffer
	char		*textP;					// => buffer to hold text to display
	char		*message;				// <= additional text for display
	status_t	error;					// <= current error
	uint16_t	canceled:1;				// <= true if user has pressed the cancel button			
	uint16_t	showDetails:1;			// <= true if user pressed down arrow for more details
	uint16_t	textChanged:1;			// => if true then update text (defaults to true)
	uint16_t	timedOut:1;				// <= true if update caused by a timeout
	uint16_t	displaySkipBtn:1;
	uint16_t	skipped:1;
	uint16_t	spareBits1:10;
	uint16_t	padding1;

	uint32_t	timeout;				// <> timeout in milliseconds to force next update (for animation)
	uint32_t	padding2;

	//progress bar info
	uint32_t	barMaxValue;			// the maximum value for the progress bar, if = 0 then the bar is
										// not visible
	uint32_t	barCurValue;			// the current value of the progress bar, the bar will be drawn 
										// filled the percentage of maxValue \ value
	char		*barMessage;			// (Not Implemented) additional text for display below the progress bar.
	uint16_t	barFlags;				// reserved for future use.
	
	//
	// *** The following fields were added in PalmOS 3.2 ***
	//
	
	uint16_t	delay:1;				// => if true delay 1 second after updating form icon/msg
	uint16_t	spareBits2:15;
	void *		userDataP;				// <= context pointer that caller passed to PrgStartDialog
	
} PrgCallbackData, *PrgCallbackDataPtr;

typedef Boolean (*PrgCallbackFunc)  (PrgCallbackDataPtr cbP);

typedef struct ProgressType ProgressType, *ProgressPtr;

//-----------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

Boolean PrgUserCancel(const ProgressType *prgP);
	Boolean PrgUserSkip(const ProgressType *prgP);
status_t PrgGetError(const ProgressType *prgP);

ProgressPtr	PrgStartDialog(const char *title, PrgCallbackFunc textCallback, void *userDataP);

ProgressPtr	PrgStartDialogWithFlags(const char *title, PrgCallbackFunc textCallback, void *userDataP,
									WinFlagsType flags);

void PrgStopDialog(ProgressPtr prgP,Boolean force);

void	PrgUpdateDialog(ProgressPtr prgGP, status_t err, uint16_t stage,
					const char *messageP,Boolean updateNow);

Boolean	PrgHandleEvent(ProgressPtr prgGP,EventType *eventP);

#ifdef __cplusplus 
}
#endif

#endif //__PROGRESS_H__
