/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Event.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *        This file defines UI event structures and routines.
 *
 *****************************************************************************/

#ifndef __EVENT_H__
#define __EVENT_H__

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <DataMgr.h>
#include <Font.h>
#include <Rect.h>
#include <Window.h>
#include <CmnKeyTypes.h>
#include <EventCodes.h>

/************************************************************
 * Event Manager Errors
 *************************************************************/
#define	evtErrParamErr			(evtErrorClass | 1)
#define	evtErrQueueFull			(evtErrorClass | 2)
#define	evtErrQueueEmpty		(evtErrorClass | 3)
#define	evtErrQueueBusy			(evtErrorClass | 4)
#define	evtErrNoQueue			(evtErrorClass | 5)


#define _BA32_PADDING_16(num)	uint16_t padding_ ## num;

// define mask for all "virtual" keys
#define virtualKeyMask	(appEvtHookKeyMask | libEvtHookKeyMask | commandKeyMask)

// Event timeouts
#define  evtWaitForever    -1
#define  evtNoWait          0

struct _GenericEventType {
	uint16_t		datum[16];
};


struct FormType;

// Flags for pen down and move events, describing additional
// information in the event.
enum {
	evtPenPressureFlag	=	0x0001
};
struct _PenDownMoveEventType {
	uint16_t		flags;            // which of the following fields are valid?
	int16_t			pressure;         // 0=none, 0x1000=hard
};

struct _PenUpEventType {
	PointType		start;            // display coord. of stroke start
	PointType		end;              // display coord. of stroke start
};

// See comment in BLegacyEventQueue::AddKeyEvent before changing these
struct _KeyDownEventType {
	wchar32_t		chr;              // ascii code
	uint16_t		keyCode;
	uint16_t		modifiers;
};

struct _KeyHoldEventType {
	wchar32_t		chr;              // ascii code
	uint16_t		keyCode;
	uint16_t		modifiers;
};

struct _KeyUpEventType {
	wchar32_t		chr;              // ascii code
	uint16_t		keyCode;
	uint16_t		modifiers;
};

struct _WinEnterEventType {
	WinHandle		enterWindow;
	WinHandle		exitWindow;
};

struct _WinExitEventType {
	WinHandle		enterWindow;
	WinHandle		exitWindow;
};

struct _WinFocusGainedEventType {
	WinHandle		window;
	uint32_t		flags;		// always 0 for now.
};

struct _WinFocusLostEventType {
	WinHandle		window;
	uint32_t		flags;		// always 0 for now.
};

struct _WinUpdateEventType {
	WinHandle		window;
	RectangleType	dirtyRect;
	void *			prv;
};

struct _WinResizedEventType {
	WinHandle		window;
	RectangleType	newBounds;
};

struct _WinScreenDrawnEventType {
	RectangleType	rect;
};

// ken?
struct _TSMConfirmType {
	char *			yomiText;
	uint16_t		formID;
	uint16_t		padding_1;
};

struct _TSMFepButtonType {
	uint16_t		buttonID;
};

struct _TSMFepModeEventType {
	uint16_t		mode;					// Value is TsmFepModeType
};

enum { // must match the values for IWindow!
	windowFullyHidden = 0,
	windowPartiallyVisible = 1,
	windowFullyVisible = 2
};

// The event record.
typedef struct EventType
{
	eventsEnum		eType;
	Boolean			penDown;
	uint8_t			padding_1;
	uint16_t		padding_2;
	uint32_t		tapCount;
	Coord			screenX;
	Coord			screenY;
	union {
		struct _GenericEventType		generic;
		struct _PenDownMoveEventType	penDownMove;	// Not available before PalmOS 6.0
		struct _PenUpEventType			penUp;
		struct _KeyDownEventType		keyDown;
		struct _KeyHoldEventType		keyHold;
		struct _KeyUpEventType			keyUp;
		struct _WinEnterEventType		winEnter;
		struct _WinExitEventType		winExit;
		struct _WinFocusGainedEventType	winFocusGained;
		struct _WinFocusLostEventType	winFocusLost;
		struct _WinResizedEventType		winResized;
		struct _WinUpdateEventType		winUpdate;
		struct _WinScreenDrawnEventType	winScreenDrawn;
		struct _TSMConfirmType			tsmConfirm;
		struct _TSMFepButtonType		tsmFepButton;
		struct _TSMFepModeEventType		tsmFepMode;

		struct winVisibilityChanged {
			WinHandle			window;
			uint32_t			visibility; // one of windowFullyHidden,
											// windowPartiallyVisible or windowFullyVisible
		} winVisibilityChanged;

		struct ctlEnter {
			struct ControlType	*pControl;
			uint16_t				controlID;
			_BA32_PADDING_16(1)
		} ctlEnter;

		struct ctlSelect {
			struct ControlType	*pControl;
			uint16_t				controlID;
			uint16_t				value;				// used for slider controls only
			Boolean				on;
			uint8_t				reserved1;
			_BA32_PADDING_16(1)
		} ctlSelect;

		struct ctlRepeat {
			struct ControlType	*pControl;
			uint16_t				controlID;
			uint16_t				value;				// used for slider controls only
			uint32_t				time;
		} ctlRepeat;

		struct ctlExit {
			struct ControlType	*pControl;
			uint16_t				controlID;
			_BA32_PADDING_16(1)
		} ctlExit;

		struct fldEnter {
			struct FieldType	*pField;
			uint16_t				fieldID;
			_BA32_PADDING_16(1)
		} fldEnter;

		struct fldHeightChanged {
	         struct FieldType	*pField;
			uint16_t				fieldID;
			Coord				newHeight;
			size_t				currentPos;
		} fldHeightChanged;

		struct fldChanged {
			struct FieldType	*pField;
			uint16_t				fieldID;
			_BA32_PADDING_16(1)
		} fldChanged;

		struct fldExit {
			struct FieldType	*pField;
			uint16_t				fieldID;
			_BA32_PADDING_16(1)
		} fldExit;

		struct lstEnter {
			struct ListType		*pList;
			uint16_t				listID;
			int16_t				selection;
		} lstEnter;

		struct lstExit {
			struct ListType		*pList;
			uint16_t				listID;
			_BA32_PADDING_16(1)
		} lstExit;

		struct lstSelect {
			struct ListType		*pList;
			uint16_t				listID;
			int16_t				selection;
		} lstSelect;

		struct tblEnter {
			struct TableType	*pTable;
			uint16_t				tableID;
			int16_t				row;
			int16_t				column;
			_BA32_PADDING_16(1)
		} tblEnter;

		struct tblExit {
			struct TableType	*pTable;
			uint16_t				tableID;
			int16_t				row;
			int16_t				column;
			_BA32_PADDING_16(1)
		} tblExit;

		struct tblSelect {
			struct TableType	*pTable;
			uint16_t				tableID;
			int16_t				row;
			int16_t				column;
			_BA32_PADDING_16(1)
		} tblSelect;

		struct frmLoad {
			uint16_t		formID;
			uint16_t		reserved;
			DmOpenRef	formDatabase;
		} frmLoad;

		struct frmOpen {
			uint16_t		formID;
		} frmOpen;

		struct frmGoto {
			uint16_t		formID;
			_BA32_PADDING_16(1)
			uint32_t		recordNum;        // index of record that contains the match.
			uint32_t		recordID;		  // ID of record that contains the match.
			size_t			matchPos;         // postion in record of the match.
			size_t			matchLen;         // length of match.
			uint32_t		matchFieldNum;    // field number string was found int
			uint32_t		matchCustom;      // application specific info
		} frmGoto;

		struct frmClose {
			uint16_t		formID;
		} frmClose;

		struct frmUpdate {
			uint16_t			formID;
			uint16_t			updateCode;		// Application specific
			RectangleType	dirtyRect;
		} frmUpdate;

		struct frmTitleEnter {
			uint16_t		formID;
		} frmTitleEnter;

		struct frmTitleSelect {
			uint16_t		formID;
		} frmTitleSelect;

		struct attnIndicatorEnter {
			uint16_t		formID;
		} attnIndicatorEnter;

		struct attnIndicatorSelect {
			uint16_t		formID;
		} attnIndicatorSelect;

		struct daySelect {
			struct DaySelectorType	*pSelector;
			int16_t					selection;
			Boolean					useThisDate;
			uint8_t					reserved1;
		} daySelect;

		struct menu {
			uint16_t	itemID;
		} menu;

		struct popSelect {
			uint16_t					controlID;
			uint16_t					listID;
			struct ControlType		*controlP;
			struct ListType			*listP;
			int16_t					selection;
			int16_t					priorSelection;
		} popSelect;

		struct sclEnter {
			struct ScrollBarType	*pScrollBar;
			uint16_t				scrollBarID;
			_BA32_PADDING_16(1)
		} sclEnter;

		struct sclExit {
			struct ScrollBarType	*pScrollBar;
			uint16_t				scrollBarID;
			_BA32_PADDING_16(1)
			int32_t					value;
			int32_t					newValue;
		} sclExit;

		struct sclRepeat {
			struct ScrollBarType	*pScrollBar;
			uint16_t				scrollBarID;
			uint16_t				firstTapTime;
			int32_t					value;
			int32_t					newValue;
			uint32_t				time;
		} sclRepeat;

		struct menuCmdBarOpen {
			Boolean					preventFieldButtons;	// set to stop the field from automatically adding cut/copy/paste
			uint8_t					reserved;				// alignment padding
		} menuCmdBarOpen;

		struct menuOpen {
			uint16_t					menuRscID;
			int16_t					cause;
		} menuOpen;

		struct gadgetEnter {
			struct FormGadgetType	*gadgetP;				// must be same as gadgetMisc
			uint16_t				gadgetID;				// must be same as gadgetMisc
			_BA32_PADDING_16(1)
		} gadgetEnter;

		struct gadgetMisc {
			struct FormGadgetType	*gadgetP;				// must be same as gadgetEnter
			uint16_t 					gadgetID;				// must be same as gadgetEnter
			uint16_t					selector;
			void					*dataP;
		} gadgetMisc;

		struct tsmFepChange {
			uint32_t				creator;
		} tsmFepChange;

		struct tsmFepDisplayOptions {
			uint16_t				numOptions;
			uint16_t				curOption;
			uint16_t				maxOptionWidth;
		} tsmFepDisplayOptions;

		struct tsmFepSelectOption {
			int16_t					selection;
		} tsmFepSelectOption;

		struct gsiStateChange {
			uint16_t				lockFlags;
			uint16_t				tempShift;
		} gsiStateChange;

		struct frmObjectFocusTake {
			uint16_t			formID;
			uint16_t			objectID;
			uint32_t			dispatchHint; // system use only
		} frmObjectFocusTake;

		struct frmObjectFocusLost {
			uint16_t			formID;
			uint16_t			objectID;
			uint32_t			dispatchHint; // system use only
		} frmObjectFocusLost;

		struct frmPropertyBinder {			  // system use only
			struct FormType*			formP;
			uint16_t			objectID;
			Boolean				objectIsForm;
			Enum8				objectKind;
		} frmPropertyBinder;

	} data;	// the union
} EventType;

typedef EventType *EventPtr;

#ifdef __cplusplus
extern "C" {
#endif

// Event Loop
// ===========================================================================
//!	The event loop function
extern void	EvtGetEvent(EventType *event, int32_t timeout);

//!	Finish processing current event.
/*! You can call this when done processing the event returned by
	EvtGetEvent().  Normally you don't need to call this (EvtGetEvent()
	will call it for you), but if you are going to block (such as through
	IOSPoll()) before retrieving the next event, you must take
	care of this yourself. */
extern void EvtFinishLastEvent(void);

//!	If this returns true, EvtGetEvent will return something
extern Boolean	EvtEventAvail (void);

/*!	Returns the last window that received a winFocusGained event, or
	invalidWindowHandle if no winFocusGained event has been returned
	from EvtGetEvent() since the last winFocusLost event. */
extern WinHandle EvtGetFocusWindow(void);

//!	Unblock the main event loop.
/*!	This will result in a null event being sent to the current app. */
extern status_t	EvtWakeup(void);

//!	Unblock the main event loop.
/*!	This will NOT result in a null event being sent to the current app. */
extern status_t	EvtWakeupWithoutNilEvent(void);

/*!	Causes a nilEvent to be returned from EvtGetEvent at the time (in ticks)
	requested.  Will only replace an already set request if it is sooner
	(i.e. ticks < previouslySetTicks).  Passing 0 for ticks will cleart the
	request. */
extern Boolean	EvtSetNullEventTick(int64_t milliseconds);

/*!	Return the IOS file descriptor you can block on to wake up when events
	arrive in your queue.  Note that this only works for the main UI thread. */
extern int32_t	EvtGetEventDescriptor(void);

//!	Convenience function.
extern void		EvtEventToString(EventType *event, char *str, uint32_t bufsize);

// High Level Queue
// ===========================================================================

//!	Add an event to the high level queue
extern status_t	EvtAddEventToQueue (const EventType *event);

//!	Add an event to the high level queue, uniquely
extern status_t	EvtAddUniqueEventToQueue(const EventType *eventP, uint32_t userCookie, Boolean inPlace);

//!	Add an event to the high level queue, time is in ticks/milliseconds like EvtGetEvent, but absolute
extern status_t	EvtAddEventToQueueAtTime(uint64_t absoluteTime, const EventType *event);

//!	Add an event to the high level queue, uniquely
extern status_t	EvtAddUniqueEventToQueueAtTime(uint64_t absoluteTime, const EventType *eventP, uint32_t userCookie, Boolean inPlace);


// Multithreading Support
// ===========================================================================

//!	This is a handle on a thread's event queue.
typedef void *			EvtQueueHandle;

//!	Return the event queue for the current thread.
/*!	Can be called from the main UI thread, or from other threads
	once they have called WinStartThreadUI() to build a UI context.
	You can then use EvtAddEventToEventQueue() to add events to this
	thread's queue from any other thread in the process.  Be sure to
	use EvtReleaseEventQueue() when you are done with it. */
extern EvtQueueHandle	EvtGetThreadEventQueue(void);

//!	Release reference on event queue.
/*!	Call this on an EvtQueue returned above when you are done
	using it.  This allows the system to reclaim the queue's
	resources. */
extern void				EvtReleaseEventQueue(EvtQueueHandle queue);

//!	Add another reference to the given event queue
/*!	The reference is removed with a corresponding call to
	EvtReleaseEventQueue(). */
extern void				EvtAcquireEventQueue(EvtQueueHandle queue);

//!	Place an event into an explicit event queue.
/*!	The reply queue can be found by the receiver through EvtGetReplyEventQueue(),
	and can be supplied as NULL if no reply is required. */
extern status_t			EvtAddEventToEventQueue(
							EvtQueueHandle queue, const EventType *event,
							EvtQueueHandle replyQueue);

//!	Place a unique event into an explicit event queue.
/*!	This is like EvtAddUniqueEventToQueue(), but operates on an
	explicit queue like EvtAddEventToEventQeue() */
extern status_t			EvtAddUniqueEventToEventQueue(
							EvtQueueHandle queue, const EventType *event,
							uint32_t userCookie, Boolean inPlace);

//!	Retrieve reply queue of current event.
/*!	Return the event queue through which you can reply to the current
	event being processed.  Returns NULL if no reply is requested. */
extern EvtQueueHandle	EvtGetReplyEventQueue(void);

//!	Start an event thread in the Background Process
/*!	Returns the event queue of the new background thread.
	Events can be sent to it through the queue as in the local
	process, with the restriction that only top-level contents
	of the event structure will be copied (it can't contain pointers
	to strings or other data or objects).

	The caller queue and data are propagated to the new thread through
	the launch code as described below.  Supplying NULL for any of these
	is valid.

	Be sure to call EvtReleaseEventQueue() when you are done with this
	queue (though that by itself is NOT sufficient to make the thread
	go away). */
extern EvtQueueHandle	EvtCreateBackgroundThread(
							DatabaseID db, size_t stackSize, uint8_t priority,
							EvtQueueHandle callerQueue, MemPtr data, size_t dataSize);

/*!	EvtCreateBackgroundThread() enters PilotMain with the launch code
	sysAppLaunchCmdBackground and this data structure. */
typedef struct SysAppLaunchCmdBackgroundType {
	EvtQueueHandle	callerQueue;		/*<! As supplied above.  Will be
										     released by the system upon return. */
	MemPtr			data;				/*<! Non-NULL if supplied.  Will be
										     released by the system upon return. */
	size_t			dataSize;
} SysAppLaunchCmdBackgroundType;

//!	Publish and lookup an event queue by name.
/*!	You must call EvtReleaseEventQueue() when done with the queue
	returned by EvtLooupEventQueue().  Published queues persist across
	application switches...  though if the queue refers to a
	thread in the application process, after a switch that
	queue will be dead and return errors if used. */
extern status_t			EvtPublishEventQueue(const char* name, EvtQueueHandle queue);
extern EvtQueueHandle	EvtLookupEventQueue(const char* name);

// Key Queue
// ===========================================================================

//!	Flush the key queue
extern status_t	EvtFlushKeyQueue(void);

//!	Append a key to the key queue.
extern status_t	EvtEnqueueKey(wchar32_t ascii, uint16_t keycode, uint16_t modifiers);

//!	Return true of key queue empty.
extern Boolean	EvtKeyQueueEmpty(void);

/*!	Pop off the next key event from the key queue and fill in the given
	event record structure. Returns non-zero if there aren't any keys in the
	key queue. If peek is non-zero, key will be left in key queue. */
extern status_t	EvtDequeueKeyEvent(EventType *event, Boolean peek);


// Pen Queue
// ===========================================================================
//!	Return the stroke info for the next stroke in the pen queue.
/*!	This MUST be the first call when removing a stroke from the queue */
extern status_t	EvtDequeuePenStrokeInfo(PointType *startPtP, PointType *endPtP);

//!	Dequeue the next point from the pen queue.
/*!	Returns non-0 if no more points. The point returned will be (-1,-1) at the end
	of the stroke. */
extern status_t	EvtDequeuePenPoint(PointType *retP);

/*!	Flush the current stroke from the pen queue, or the next one if there
	is no current one */
extern status_t	EvtFlushNextPenStroke(void);

//!	Flush the entire pen queue
extern status_t	EvtFlushPenQueue(void);

//!	variation of EvtGetPen, returning coordinates using window's active coordinate system
extern status_t	EvtGetPenNative(WinHandle winH, Coord* pScreenX, Coord* pScreenY, Boolean* pPenDown);

//!	Get he current pen location
extern status_t	EvtGetPen(Coord *pScreenX, Coord *pScreenY, Boolean *pPenDown);

//!	(post-6.0.1) This enum describes how pen events can be filtered by EvtPenFilterFunc().
enum EvtDispatchTag
{
	evtDispatchAbsorb = 0,			//!< Deliver the event to the window and consume.
	evtDispatchFallthrough = 1		//!< Ignore event, send it to next window.
};
typedef uint32_t EvtDispatchType;

//!	(post-6.0.1) Pen event filter function prototype.
/*!	This function is called for each pen event delivered to the
	window, allowing you to decide what is down with it by
	selecting winPenFilterAbsorb or winPenFilterFallthrough.
	Note that once you have allowed an event to fall through,
	you will not see this or any more events in the current
	motion delivered to your window.
	\note This function is called OUTSIDE OF THE WINDOW'S
	EVENT THREAD.  You can not access any UI state from it. */
typedef EvtDispatchType (*EvtPenDispatchFunc)(const EventType* penEvent, const RectangleType* nativeFrame, void* userData);

//!	(post-6.0.1) Set filter function for the given window.
/*!	If NULL, the default filter function (which always returns
	winPenFilterAbsorb) will be used. */
extern status_t EvtSetPenDispatchFunc(WinHandle winHandle, EvtPenDispatchFunc penDispatch, void* userData);

/*!	Evaluate to true if <eventP> is a pointer to a virtual character key-
	down event. We assume that the caller has already determined the event
	is a keydown. WARNING!!! This macro is only safe to use on Palm OS 3.5
	or later. With earlier versions of the OS, use TxtGlueCharIsVirtual()
	in PalmOSGlue.lib */
#define	EvtKeydownIsVirtual(eventP)	(((eventP)->data.keyDown.modifiers & virtualKeyMask) != 0)


// Pen and Key Queues
// ===========================================================================
//!	Return true if there is either a pen or a key event available
extern Boolean	EvtSysEventAvail(Boolean ignorePenUps);

#ifdef __cplusplus
}
#endif

#endif // __EVENT_H__
