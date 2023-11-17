/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: EventCodes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *        This file defines the enum that contains all of the event
 *        codes.  This is in a separate file from the structs because
 *        this enum needs to be managed all in one place.
 *
 *        When adding new codes to this file, give the identifier
 *        a specific value, so nothing conflicts.  Also, add them
 *        at specified location.  Or if you're adding a library
 *        that generates events, put a base into the library section.
 *
 *****************************************************************************/

#ifndef __EVENT_CODES_H__
#define __EVENT_CODES_H__


enum eventsEnumTag {
	nilEvent						= 0,
	penDownEvent					= 1,
	penUpEvent						= 2,
	penMoveEvent					= 3,
	keyDownEvent					= 4,
	winEnterEvent					= 5,
	winExitEvent					= 6,
	ctlEnterEvent					= 7,
	ctlExitEvent					= 8,
	ctlSelectEvent					= 9,
	ctlRepeatEvent					= 10,
	lstEnterEvent					= 11,
	lstSelectEvent					= 12,
	lstExitEvent					= 13,
	popSelectEvent					= 14,
	fldEnterEvent					= 15,
	fldHeightChangedEvent			= 16,
	fldChangedEvent					= 17,
	tblEnterEvent					= 18,
	tblSelectEvent					= 19,
	daySelectEvent					= 20,
	menuEvent						= 21,
	appStopEvent					= 22,
	frmLoadEvent					= 23,
	frmOpenEvent					= 24,
	frmGotoEvent					= 25,
	frmUpdateEvent					= 26,
	frmSaveEvent					= 27,
	frmCloseEvent					= 28,
	frmTitleEnterEvent				= 29,
	frmTitleSelectEvent				= 30,
	tblExitEvent					= 31,
	sclEnterEvent					= 32,
	sclExitEvent					= 33,
	sclRepeatEvent					= 34,
	tsmConfirmEvent					= 35,
	tsmFepButtonEvent				= 36,
	tsmFepModeEvent					= 37,
	attnIndicatorEnterEvent			= 38,	// for attention manager's indicator
	attnIndicatorSelectEvent		= 39,	// for attention manager's indicator
	
	// Add future System level evets in this numeric space -- 100 through 2047
	invalidEvent					= 100,	// This should never be returned, but is used for error checking
	winResizedEvent					= 101,
	winUpdateEvent					= 102,
	winFocusGainedEvent				= 105,
	winFocusLostEvent				= 106,
	insertionPointOnEvent			= 107,
	insertionPointOffEvent			= 108,
	amWorkerDoneEvent				= 109,
	tsmFepChangeEvent				= 110,
	certMgrWorkerDoneEvent			= 111,
	keyUpEvent						= 112,
	keyHoldEvent					= 113,
	gsiStateChangeEvent				= 114,
	exgLocalEvtNotify				= 115,
	exgLocalEvtDie					= 116,
	tunneledEvent					= 117,
	reservedFindEvent				= 118,	// Workaround for dialogs blocking the main thread, private
	winVisibilityChangedEvent		= 119,
	sysClearUIEvent					= 120,	// This is sent when all windows in the UI should
											// be removed due to a high-level action such as
											// an app switch.  SysHandleEvent() will turn this
											// into an appStop event outside of the main UI
											// thread.
	stringInputEvent				= 121,	// For system use -- not seen by applications
	listenStoppedEvent				= 122,	// for system use
	playbackEvent					= 123,	// for system use
	discoverEvent					= 124,	// for system use
	discoverStoppedEvent			= 125,	// for system use
	propertiesEvent					= 126,	// for system use
	propertyGetEvent				= 127,	// for system use
	propertySetEvent				= 128,	// for system use
	winScreenChangedEvent			= 129,	// Screen dimensions changed (i.e., switch from portait
											// to landscape).  You should general NOT do anything
											// with this event, but let your window constraints take
											// care of things.  Note that this is different than the
											// old winDisplayChangedEvent, which told about screen
											// layout changes (DIA up or down) but not about the
											// actual physical screen changing.
	testBeginSelfTestEvent			= 130,  // Begin self test. Used for test only
							// Do NOT change the value of this event! This is important for prebuilt test harness
							// to work!
	winScreenDrawnEvent				= 131,	// private

	// Add future UI level events in this numeric space -- 2048 through 4095
	menuCmdBarOpenEvent				= 2048,	// 0x0800
	menuOpenEvent					= 2049,
	menuCloseEvent					= 2050,
	frmGadgetEnterEvent				= 2051,
	frmGadgetMiscEvent				= 2052,
	tsmFepDisplayOptionsEvent		= 2053,
	tsmFepSelectOptionEvent			= 2054,
	frmScrollPrvRefreshEvent		= 2055,
	prgUpdateEvent					= 2056,
	frmControlPrvRefreshEvent		= 2057,
	menuCmdBarTimeoutEvent			= 2058,
	debugEvent						= 2059,
	frmStopDialogEvent				= 2060,	// Return from FrmDoForm() with the default button,
											// but do not leave an appStopEvent in the queue.

	prgMakeCallback					= 2061, // Private event DO NOT USE!
	prgUpdateDialog					= 2062, // Private event DO NOT USE!
	
	FIND_START_SEARCH				= 2063, // Private event DO NOT USE!
	FIND_CANCEL_SEARCH				= 2064, // Private event DO NOT USE!
	FIND_GOTO_MATCH					= 2065, // Private event DO NOT USE!
	FIND_RESULTS_DRAW				= 2066, // Private event DO NOT USE!
	FIND_STOP_SEARCH				= 2067, // Private event DO NOT USE!
	
	// Provide the bases of library events here
	firstINetLibEvent				= 0x1000,
	firstWebLibEvent				= 0x1100,
	telAsyncReplyEvent				= 0x1200, 

	// These events were added to the 68k SDK for for 5-way navigation;
	// because PACE could not be changed, that had to added here.  ARM
	// native applications should use the ones above instead.  (PACE translates
	// between the ARM value and these.)
	keyUpEvent5 					= 0x4000,
	keyHoldEvent5 					= 0x4001,
	// event defined for frmObjectFocusTakeEvent, but not supported in 6.0
	frmObjectFocusTakeEvent			= 0x4002,
	// event defined for frmObjectFocusLostEvent, but not supported in 6.0
	frmObjectFocusLostEvent	 		= 0x4003,

	// Used by 5.x's PINS API for winDisplayChangedEvent
	reservedEventCode3				= 0x4101,

	// BGT, 06/24/2003 Clarify the range reserved for licensees
	firstLicenseeEvent				= 0x5000,
	lastLicenseeEvent				= 0x5FFF,

	// user event range
	firstUserEvent					= 0x6000,
	lastUserEvent					= 0x7FFF
	
	// Don't go above 0x7FFF because there is code that depends on these
	// numbers being a signed int16_t
};

#include <stdint.h>

// Can I do this?
//typedef SignedEnum16 eventsEnum;
typedef uint32_t eventsEnum;


#endif // __EVENT_CODES_H__
