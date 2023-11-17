/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SysEventCompatibility.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *        THIS IS FOR BACKWARDS COMPATIBILITY ONLY.
 *        All event-related APIs are now located in Event.h.
 *
 *****************************************************************************/

#ifndef __SYSEVENT_H__
#define __SYSEVENT_H__

#include <Event.h>

// This enum is here for compatibility only.  Use the codes
// defined in EventCodes.h
enum SysEventsEnumTag {
	sysEventNilEvent				= nilEvent,
	sysEventPenDownEvent			= penDownEvent,
	sysEventPenUpEvent				= penUpEvent,
	sysEventPenMoveEvent			= penMoveEvent,
	sysEventKeyDownEvent			= keyDownEvent,
	sysEventWinEnterEvent			= winEnterEvent,
	sysEventWinExitEvent			= winExitEvent,
	sysEventAppStopEvent			= appStopEvent,
	sysEventTsmConfirmEvent			= tsmConfirmEvent,
	sysEventTsmFepButtonEvent		= tsmFepButtonEvent,
	sysEventTsmFepModeEvent			= tsmFepModeEvent,
	// sysEventFrmTitleChangedEvent	= ,						// This is not used anymore?

	sysEventNextUIEvent				= 0x0800,
	sysEventFirstINetLibEvent		= firstINetLibEvent,
	sysEventFirstWebLibEvent		= firstWebLibEvent,
	sysEventFirstUserEvent			= firstUserEvent,
	sysEventLastUserEvent			= lastUserEvent
} ;
typedef uint32_t SysEventsEnum ;


// The event record.
typedef struct SysEventType {
	SysEventsEnum	eType;
	Boolean			penDown;
	uint8_t			padding_1;
	uint16_t		padding_2;
	uint32_t		tapCount;
	Coord			screenX;
	Coord			screenY;
	union {
		struct _GenericEventType		generic;

		struct _PenUpEventType			penUp;
		struct _KeyDownEventType		keyDown;
		struct _WinEnterEventType		winEnter;
		struct _WinExitEventType		winExit;
		struct _WinFocusGainedEventType	winFocusGained;
		struct _WinFocusLostEventType	winFocusLost;
		struct _WinUpdateEventType		winUpdate;
		struct _WinResizedEventType		winResized;
		struct _TSMConfirmType			tsmConfirm;
		struct _TSMFepButtonType		tsmFepButton;
		struct _TSMFepModeEventType		tsmFepMode;
	} data;

} SysEventType;

#define	SysEventGet(eventP, timeOut)					EvtGetEvent((EventType *)eventP, timeOut)
#define SysEventAvail									EvtEventAvail
#define	SysEventAddToQueue(eventP)						EvtAddEventToQueue((const EventType *)eventP)
#define	SysEventAddUniqueToQueue(eventP, id, inPlace)	EvtAddUniqueEventToQueue((const EventType *)eventP, id, inPlace)

#endif // __SYSEVENT_H__
