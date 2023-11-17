/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Form.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This file defines form routines.
 *
 *****************************************************************************/

#ifndef __FORM_H__
#define __FORM_H__

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <Event.h>
#include <Window.h>
#include <GcRender.h>

#include <Field.h>

#define frmInvalidObjectId		(0xffff)
#define noFocus					(frmInvalidObjectId)
#define frmNoSelectedControl	0xff
#define frmNavTitleID ((uint16_t)(0xFFFC))

// Update code send as part of a frmUpdate event.
#define frmRedrawUpdateCode		0x8000

// Magic button IDs used by FrmCustomResponseAlert callbacks
#define frmResponseCreate		1974
#define frmResponseQuit			((int16_t) 0xBEEF)


// Alert constants and structures
enum alertTypes {
	informationAlert,
	confirmationAlert,
	warningAlert,
	errorAlert
};
typedef Enum8 AlertType;

typedef struct AlertTemplateTag AlertTemplateType;


// Types of object in a dialog box
enum formObjects {
	frmFieldObj,
	frmControlObj,
	frmListObj,
	frmTableObj,
	frmBitmapObj,
	frmLineObj,
	frmFrameObj,
	frmRectangleObj,
	frmLabelObj,
	frmTitleObj,
	frmPopupObj,
	frmGraffitiStateObj,
	frmGadgetObj,
	frmScrollBarObj };
typedef Enum8 FormObjectKind;

// Gadget support:
#define formGadgetDrawCmd			0	// paramP is unspecified
#define formGadgetEraseCmd			1	// paramP is unspecified
#define formGadgetHandleEventCmd	2	// paramP is an EventType *for the relevant event.
#define formGadgetDeleteCmd			3	// paramP is unspecified.

// access to this is allowed only within the gadget callback, and not otherwise.
typedef struct FormGadgetAttrTag
{
	uint16_t usable			:1;		// Set if part of ui - "should be drawn"
	uint16_t extended		:1;		// Set if the structure is an "Extended" gadget (i.e., the 'handler' field is present)
	uint16_t visible		:1;		// Set if drawn - "has been drawn" or "must do work to erase"
	uint16_t reserved		:13;	// pad it out
}
FormGadgetAttrType;

// What is a FormGadgetView?  Well it all depends...
#ifdef __cplusplus

#if _SUPPORTS_NAMESPACE
namespace palmos {
namespace view {
#endif
class IView;
#if _SUPPORTS_NAMESPACE
} }
typedef palmos::view::IView FormGadgetView;
#else
typedef IView FormGadgetView;
#endif

#else

typedef void FormGadgetView;

#endif

#ifdef __GNUC__
struct FormGadgetTypeInCallback; // forward declaration
#else
typedef struct FormGadgetTypeInCallback FormGadgetTypeInCallback; // forward declaration
#endif

typedef Boolean (FormGadgetHandlerType) (struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

// access to this is allowed only within the gadget callback, and not otherwise.
typedef struct FormGadgetTypeInCallback
{
	uint16_t					id;
	FormGadgetAttrType			attr;
	RectangleType				rect;
	const void *				data;
	FormGadgetHandlerType *		handler;
} 
FormGadgetTypeInCallback;

typedef Boolean FormEventHandlerType (EventType *eventP);

typedef FormEventHandlerType *FormEventHandlerPtr;

typedef struct FormLabelType FormLabelType;
typedef struct FormGadgetType FormGadgetType;
typedef struct FormBitmapType FormBitmapType;
typedef struct FormGraffitiStateTag FrmGraffitiStateType;

typedef struct FormType FormType;
typedef FormType *FormPtr;

// FormActiveStateType: this structure is passed to FrmActiveState for
// saving and restoring active form/window state; this structure's
// contents are abstracted because the contents will differ significantly
// as PalmOS evolves
// Added for PalmOS 3.0
//
// NOTE:
// Never turn paddingNeverUse into a real data item.  The reason is
// that in the 68K world, this public struct was declared as
//     uint16_t data[11];
// meaning that it was one uint16_t smaller than this definition.  We had
// to redefine it, though, because the real (private) struct contains
// members that require 4-byte alignment, and so we needed any
// FormActiveStateType that people declare to have sufficient alignment.
// You can't ever actually use the extra uint16_t at the end, though,
// because if such a struct ever made a trip through 68K code (via PACE),
// we would lose the uint16_t at the end (since 68K apps will have
// allocated only 22 bytes for this struct, we can't copy the whole
// struct into 68K code anymore).
typedef struct FormActiveStateType {
	uint32_t	data[5];
	uint16_t	dataLast;
	uint16_t	paddingNeverUse; // Do not ever change this to a real data item!
} FormActiveStateType;

// FrmCustomResponseAlert callback routine prototype
typedef Boolean FormCheckResponseFuncType 
		(int16_t button, char * attempt);

typedef FormCheckResponseFuncType *FormCheckResponseFuncPtr;

#define customAlertInsertionTag			((char*)"^ErrMsg")

typedef enum ErrDlgResultTag
{
	errDlgResOK,
	errDlgResCancel,
	errDlgResRetry,
	errDlgResYes,
	errDlgResNo
} ErrDlgResultType;

// alert template types (defines a default combination of title, icon and buttons)...
#define	errDlgWarning			0x00000000
#define	errDlgError				0x00000001
#define	errDlgInformation		0x00000002
#define	errDlgConfirmation		0x00000003

// alert button types (used to override default buttons for any of the predefined alert template types)...
#define	errDlgOK				0x00000000
#define	errDlgOKCancel			0x00000010
#define	errDlgYesNo				0x00000020
#define	errDlgYesNoCancel		0x00000030
#define	errDlgRetryCancel		0x00000040

#define	errDlgTemplateMask		0x0000000F
#define	errDlgButtonsMask		0x000000F0


/************************************************************
 * UI library error codes
 * the constant uilibErrorClass is defined in CmnErrors.h
 *************************************************************/
#define uilibErrInvalidParam		(uilibErrorClass | 1)
#define uilibErrCurrentFocusInvalid	(uilibErrorClass | 2)
#define	uilibErrObjectFocusModeOff	(uilibErrorClass | 3)
#define uilibErrObjectNotFound		(uilibErrorClass | 4)
#define uilibErrNoNavInfoForForm	(uilibErrorClass | 5)
#define uilibErrInvalidFocusObject	(uilibErrorClass | 6)
#define uilibErrFormDoesNotHaveFocus (uilibErrorClass | 7)


//*********************************************************
// Nav Flags (used with navFlags field of FrmNavHeaderType)
//*********************************************************
typedef uint32_t	FrmNavHeaderFlagsType;

#define	kFrmNavHeaderFlagsObjectFocusStartState		  0x00000001
#define	kFrmNavHeaderFlagsAppFocusStartState		  0x00000002
#define kFrmNavHeaderFlagsAutoGenerated				  0x80000000

#define kFrmNavHeaderFlagsStartStateMask			  0x00000003
#define	kFrmNavHeaderFlagsDefaultStartStateValue	  0x00000000
#define	kFrmNavHeaderFlagsObjectFocusStartStateValue  0x00000001
#define	kFrmNavHeaderFlagsAppFocusStartStateValue	  0x00000002
#define kFrmNavHeaderFlagsInvalidStartStateValue	  0x00000003

//*********************************************************
// Object Flags (used with objectFlags field of
//	FrmNavOrderEntryType)
//*********************************************************
typedef uint16_t	FrmNavObjectFlagsType;

#define kFrmNavObjectFlagsSkip					0x0001
#define	kFrmNavObjectFlagsForceInteractionMode	0x0002


//*********************************************************
// Nav State Flags (used with stateFlags parameter of 
//	FrmGetNavState and FrmSetNavState API functions)
//*********************************************************
typedef uint32_t	FrmNavStateFlagsType;

#define kFrmNavStateFlagsInteractionMode		0x00000001
#define kFrmNavStateFlagsObjectFocusMode		0x00000002


//*********************************************************
// The current version of the navigation structures
//  (FrmNavOrderEntryType and FrmNavHeaderType)
//*********************************************************
#define kFrmNavInfoVersion						1

//*********************************************************
// No extra info assosiated with current nav object
//*********************************************************
#define frmNavFocusRingNoExtraInfo				(0xffff)

//*********************************************************
// Structures used with Navigation API functions
//*********************************************************

typedef struct FrmNavOrderEntryTag
{
	uint16_t				objectID;
	FrmNavObjectFlagsType	objectFlags;
	uint16_t				aboveObjectID;
	uint16_t				belowObjectID;
} FrmNavOrderEntryType;

typedef struct FrmNavHeaderTag
{
	uint16_t				version;				// This is version 1
	uint16_t				numberOfObjects;
	uint16_t				headerSizeInBytes;		// 20 for the version 1 structure
	uint16_t				listElementSizeInBytes; // 8 for the version 1 structure
	FrmNavHeaderFlagsType	navFlags;
	uint16_t				initialObjectIDHint;
	uint16_t				jumpToObjectIDHint;
	uint16_t				bottomLeftObjectIDHint;
	uint16_t				padding1;
} FrmNavHeaderType;


//-----------------------------------------------
//  Macros
//-----------------------------------------------

#if BUILD_TYPE == BUILD_TYPE_DEBUG
#define ECFrmValidatePtr(formP) FrmValidatePtr(formP)
#else
#define ECFrmValidatePtr(formP) 
#endif

//--------------------------------------------------------------------
//
// Form Function
//
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern FormType * FrmInitForm (DmOpenRef database, uint16_t rscID);

// If the target form has a constraints resource, this form overrides
// the window flags specified there.
extern FormType * FrmInitFormWithFlags (DmOpenRef database, uint16_t rscID, WinFlagsType);

extern void FrmDeleteForm (FormType *formP);

extern void FrmDrawForm (FormType *formP);

extern void FrmEraseForm (FormType *formP);

extern FormType * FrmGetActiveForm (void);

extern void FrmSetActiveForm (FormType *formP);

extern uint16_t FrmGetActiveFormID (void);

extern FieldType* FrmGetActiveField(const FormType* formP);

extern uint16_t FrmGetFocus (const FormType *formP);

extern void FrmSetFocus (FormType *formP, uint16_t fieldIndex);

extern Boolean FrmHandleEvent (FormType *formP, EventType *eventP);

extern void FrmGetFormBounds (const FormType *formP, RectangleType *rP);

extern WinHandle FrmGetWindowHandle (const FormType *formP);

extern uint16_t FrmGetFormId (const FormType *formP);

extern FormType *FrmGetFormPtr (uint16_t formId);

extern FormType *FrmGetFirstForm (void);

extern uint16_t FrmGetNumberOfObjects (const FormType *formP);

extern uint16_t FrmGetObjectIndex (const FormType *formP, uint16_t objID);

extern uint16_t FrmGetObjectIndexFromPtr (const FormType *formP, const void* objP);

extern uint16_t FrmGetObjectId (const FormType *formP, uint16_t objIndex);

extern FormObjectKind FrmGetObjectType (const FormType *formP, uint16_t objIndex);

extern void *FrmGetObjectPtr (const FormType *formP, uint16_t objIndex);

extern void FrmGetObjectBounds (const FormType *formP, uint16_t objIndex, RectangleType *rP);

extern void FrmHideObject (FormType *formP, uint16_t objIndex);

extern void FrmShowObject (FormType *formP, uint16_t objIndex);

extern void FrmGetObjectPosition (const FormType *formP, uint16_t objIndex, 
	Coord *x, Coord *y);

extern void FrmSetObjectPosition (FormType *formP, uint16_t objIndex, 
	Coord x, Coord y);

extern void FrmSetObjectBounds (FormType *formP, uint16_t objIndex, 
	const RectangleType *bounds);

extern int16_t FrmGetControlValue (const FormType *formP, uint16_t objIndex);

extern void FrmSetControlValue (const FormType *formP, uint16_t objIndex, 
	int16_t newValue);

extern uint16_t FrmGetControlGroupSelection (const FormType *formP, 
	uint8_t groupNum);

extern void FrmSetControlGroupSelection (const FormType *formP, 
	uint8_t groupNum, uint16_t controlID);

extern void FrmCopyLabel (FormType *formP, uint16_t labelID, 
	const char *newLabel);

extern const char *FrmGetLabel (const FormType *formP, uint16_t labelID);

extern void FrmSetCategoryLabel (const FormType *formP, uint16_t objIndex, 
	char *newLabel);

extern const char *FrmGetTitle (const FormType *formP);

extern void FrmSetTitle (FormType *formP, char *newTitle);

extern void FrmCopyTitle (FormType *formP, const char *newTitle);

extern void *FrmGetGadgetData (const FormType *formP, uint16_t objIndex);

extern void FrmSetGadgetData (FormType *formP, uint16_t objIndex, 
	const void *data);

extern FormGadgetView* FrmGetGadgetView (const FormType *formP, uint16_t objIndex);

extern void FrmSetGadgetView(FormType *formP, uint16_t objIndex, 
	FormGadgetView* view);

extern void FrmSetGadgetHandler (FormType *formP, uint16_t objIndex, 
	FormGadgetHandlerType *attrP);

extern uint16_t FrmDoDialog (FormType *formP);

extern uint16_t FrmAlert (DmOpenRef database, uint16_t alertId);
							
extern uint16_t FrmAlertWithFlags (DmOpenRef database, uint16_t alertId, WinFlagsType windowFlags);

extern uint16_t FrmCustomAlert (DmOpenRef database, uint16_t alertId, const char *s1, 
	const char *s2, const char *s3);

extern uint16_t FrmCustomAlertWithFlags (DmOpenRef database, uint16_t alertId, WinFlagsType windowFlags,
	const char *s1, const char *s2, const char *s3);

extern uint16_t FrmUIAlert(uint16_t alertId);

extern void FrmHelp (DmOpenRef database, uint16_t helpMsgId);

extern void FrmHelpWithFlags (DmOpenRef database, uint16_t helpMsgId, WinFlagsType windowFlags);

extern void FrmUpdateScrollers (FormType *formP, uint16_t upIndex, 
	uint16_t downIndex, Boolean scrollableUp, Boolean scrollableDown);

extern Boolean FrmVisible (const FormType *formP);

extern void FrmSetEventHandler (FormType *formP, FormEventHandlerType *handler);

extern Boolean FrmDispatchEvent (EventType *eventP);



extern void FrmPopupForm (DmOpenRef database, uint16_t formId);

extern void FrmGotoForm (DmOpenRef database, uint16_t formId);

extern void FrmUpdateForm (uint16_t formId, uint16_t updateCode);
							
extern void FrmReturnToForm  (uint16_t formId);
							
extern void FrmCloseAllForms (void);

extern void FrmSaveAllForms (void);



extern Boolean FrmPointInTitle (const FormType *formP, Coord x, Coord y);

extern void FrmSetMenu (FormType *formP, DmOpenRef database, uint16_t menuRscID);

extern Boolean FrmValidatePtr (const FormType *formP);

extern status_t FrmRemoveObject (FormType **formPP, uint16_t objIndex);

extern FormType *FrmNewForm (uint16_t formID, const char *titleStrP, 
	Coord x, Coord y, Coord width, Coord height, Boolean modal, 
	uint16_t defaultButton, DmOpenRef helpDatabase, uint16_t helpRscID,
	DmOpenRef menuDatabase, uint16_t menuRscID);

extern FormType *FrmNewFormWithConstraints (uint16_t formID, const char *titleStrP, 
	uint32_t win_flags, const WinConstraintsType* constraints,
	uint16_t defaultButton, DmOpenRef helpDatabase, uint16_t helpRscID,
	DmOpenRef menuDatabase, uint16_t menuRscID);

extern FormLabelType *FrmNewLabel (FormType **formPP, uint16_t ID, const char *textP, 
	Coord x, Coord y, FontID font);

extern FormBitmapType *FrmNewBitmap (FormType **formPP, DmOpenRef database, 
	uint16_t rscID, Coord x, Coord y);

extern FormGadgetType *FrmNewGadget (FormType **formPP, uint16_t id, 
	Coord x, Coord y, Coord width, Coord height);

extern uint16_t FrmCustomResponseAlert (DmOpenRef database, uint16_t alertId, const char *s1, const char *s2, 
	const char *s3, char *entryStringBuf, int16_t entryStringBufLength,
	FormCheckResponseFuncPtr callback);

extern uint16_t FrmCustomResponseAlertWithFlags (DmOpenRef database, uint16_t alertId, WinFlagsType windowFlags,
	const char *s1, const char *s2, const char *s3,
	char *entryStringBuf, int16_t entryStringBufLength, FormCheckResponseFuncPtr callback);

extern FrmGraffitiStateType *FrmNewGsi (FormType **formPP, Coord x, Coord y);

extern status_t FrmSaveActiveState(FormActiveStateType *stateP);

extern status_t FrmRestoreActiveState(FormActiveStateType *stateP);



extern Boolean FrmGetObjectUsable(const FormType *formP, uint16_t objIndex);

extern uint16_t FrmGetDefaultButtonID (const FormType *formP);

extern void FrmSetDefaultButtonID (FormType *formP, uint16_t defaultButton);

extern uint16_t FrmGetHelpID (const FormType *formP);

extern void FrmSetHelpID (FormType *formP, DmOpenRef database, uint16_t helpRscID);

extern uint16_t FrmGetMenuBarID (const FormType *formP);

extern FontID FrmGetLabelFont (const FormType *formP, uint16_t labelID);

extern void FrmSetLabelFont (FormType *formP, uint16_t labelID, FontID fontID);

extern FormEventHandlerType* FrmGetEventHandler (const FormType *formP);

extern uint16_t FrmGetObjectIdFromObjectPtr(void* formObjP, FormObjectKind objKind);

extern FormType *FrmGetFormWithWindow (WinHandle winH);

// These alerts (ErrXXX) are UILib dependent and may fail for system error conditions that prevent the UILib
// from functioning correctly (low memory, etc.). Severe errors should be reported using the UILib independent
// ErrFatalDisplay(If)/ErrDebugDisplay(If)...
uint16_t 
ErrAlert(DmOpenRef appDbRef, 
		 status_t errCode);

status_t 
ErrGetErrorMsg(DmOpenRef appDbRef, 
			   status_t errCode, 
			   char* errMsgP, 
			   int32_t errMsgLen);

void 	FrmSetPenTracking(FormType *formP, Boolean track, const void *frmObj);
Boolean FrmAmIPenTracking(FormType *formP, uint16_t objectID);

// These functions allow you to take advantage of a cache
// of loaded bitmaps, which is shared with sliders, graphic controls,
// and bitmap objects in the form.  The first time a bitmap
// (identified uniquely by the set database/bitmapType/bitmapID/flags)
// is requested from FrmGetBitmapHandle(), GcLoadBitmap (with the given
// flags passed through) will be used to load it; later calls will
// immediately return the previously loaded bitmap.
//
// Note that the system may flush old bitmaps from the cache as needed
// to reclaim memory, so upon calling FrmGetBitmapHandle() you must
// assume that any previously retrieved bitmaps are now invalid.
// (In debug builds, the cache is turned off, so only the most recent
// bitmap is ever valid.)
extern GcBitmapHandle FrmGetBitmapHandle(FormType* form, DmOpenRef database,
	DmResourceType bitmapType, DmResourceID bitmapID, uint32_t flags);

// This function can be used to forcibly remove a bitmap from the cache.
extern void FrmRemoveBitmapHandle(FormType* form, DmOpenRef database,
	DmResourceType bitmapType, DmResourceID bitmapID, uint32_t flags);

// One-handed navigation support

void
FrmClearFocusHighlight(void);

void
FrmSetFocusHighlight(WinHandle focusWindow, const RectangleType *hlRegionP, uint16_t cornerRadius);

uint16_t
FrmCountObjectsInNavOrder (const FormType * formP);

status_t
FrmGetNavOrder (const FormType* formP, FrmNavHeaderType* navHeaderP,
				FrmNavOrderEntryType* navOrderP, uint16_t* numObjectsP);

status_t
FrmSetNavOrder (FormType* formP, FrmNavHeaderType* navHeaderP,
				FrmNavOrderEntryType* navOrderP);

status_t
FrmGetNavEntry (const FormType* formP, uint16_t targetObjectID,
				uint16_t* afterObjectIDP, uint16_t* aboveObjectIDP,
				uint16_t* belowObjectIDP, FrmNavObjectFlagsType* objectFlagsP);

status_t
FrmSetNavEntry (FormType* formP, uint16_t targetObjectID, 
				uint16_t afterObjectID, uint16_t aboveObjectID, 
				uint16_t belowObjectID, FrmNavObjectFlagsType objectFlags);

status_t
FrmGetNavState (const FormType* formP, FrmNavStateFlagsType* stateFlagsP);

status_t
FrmSetNavState (FormType* formP, FrmNavStateFlagsType stateFlags);

status_t
FrmCalculateNavOrder (FormType *formP);

void
FrmSetTransparentObjects(FormType *formP, Boolean value);

void
FrmNavObjectTakeFocus (const FormType* formP, uint16_t objID);

#ifdef __cplusplus 
}
#endif

#endif // __FORM_H__
