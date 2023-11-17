/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: FormLayout.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Simple form manager algorithmic layout API.
 *
 *	  Layout is described in terms of how the edge of a control should
 *	  follow the edges of its window.  Each side of the control can move
 *	  in one of two ways:
 *	    frmAttachLeftTop: the edge follows the left or top side of the
 *	      window.  That is, it doesn't move.
 *	    frmAttachRightBottom: the edge follows the right or bottom side
 *	      of the window.  I.e., if a window grows by 2 pixels then
 *	      this edge will move out by two pixels, maintaining the
 *	      same distance from the window edge.
 *
 *****************************************************************************/

#ifndef _FORM_LAYOUT_H_
#define _FORM_LAYOUT_H_

#include <Form.h>

enum {
	frmAttachLeftTop = 0,		// Object side should follow left/top edge of window
	frmAttachRightBottom = 1,	// Object side should follow right/bottom edge of window
	frmAttachCenter = 2			// Object side should follow center of window
};

// Build rule for all four sides of an object.
#define frmLayoutRule(left, top, right, bottom) (((left) << 12) | ((top) << 8) | ((right) << 4) | (bottom))

// Some standard layout rules.
enum {
	frmFollowNone		= 0,

	frmFollowAllSides	= frmLayoutRule(frmAttachLeftTop, frmAttachLeftTop, frmAttachRightBottom, frmAttachRightBottom),

	frmFollowLeft		= frmLayoutRule(frmAttachLeftTop, 0, frmAttachLeftTop, 0),
	frmFollowRight		= frmLayoutRule(frmAttachRightBottom, 0, frmAttachRightBottom, 0),
	frmFollowLeftRight	= frmLayoutRule(frmAttachLeftTop, 0, frmAttachRightBottom, 0),

	frmFollowTop		= frmLayoutRule(0, frmAttachLeftTop, 0, frmAttachLeftTop),
	frmFollowBottom		= frmLayoutRule(0, frmAttachRightBottom, 0, frmAttachRightBottom),
	frmFollowTopBottom	= frmLayoutRule(0, frmAttachLeftTop, 0, frmAttachRightBottom),
};

// Layout information for a control in a form.
typedef struct FormLayoutType {
	int32_t		structSize;			// sizeof(FormLayoutType); 0 ends array
	uint16_t	id;					// which object this entry is for
	uint16_t	reserved1;			// always set to 0
	uint32_t	rule;				// layout rule (i.e., frmFollowBottom)
} FormLayoutType;

// Set FormLayoutType::id to this to include the Graffiti Shift Indicator in
// the Form's layout (in cases where the GSI is not drawn in the AIA instead):
#define frmLayoutGraffitiShiftID	(frmInvalidObjectId - 1)

//--------------------------------------------------------------------
//
// Form Layout Functions
//
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the layout rules for this form.  You should call this
// "soon" after FrmInitForm() -- after you have added any controls
// to the form or otherwise adjusted it, but before going back to
// the event loop or calling WinSetBounds() or WinSetConstraints()
// on the form's window.
//
// Upon calling this function, the array of FormLayoutType structures
// (terminated by a structure with 0 length) will be attached to the
// form.  THIS STRUCTURE MUST EXIST FOR THE ENTIRE LIFETIME OF THE
// FORM.  Calling FrmPerformLayout() will use these rules to move
// objects from their current position to the new bounds specified there.
extern status_t FrmInitLayout (FormType *formP, const FormLayoutType *formLayoutP);

// Move the objects in the form according to the rules supplied in
// FormInitLayout() to fill a window of size "newBounds".
//
// You do not normally need to call this function -- the default event
// handler of the form will call it in response to a winResizedEvent.
// You can intercept this event and handle it yourself if you would
// like to do additional or completely custom layout of the form.
extern void FrmPerformLayout(FormType *formP, const RectangleType *newBounds);

// Return the bounds that the form was originally created at.
// These are NOT the current window bounds, but the initial bounds that
// were specified in the form resource.
extern void FrmGetFormInitialBounds (const FormType *formP, RectangleType *rP);

// Return the original bounds of an object.
// This will only work if you have previously called FrmInitLayout().
// These are the exact bounds that were specified in the form resource.
extern void FrmGetObjectInitialBounds (const FormType *formP, uint16_t objIndex, RectangleType *rP);

#ifdef __cplusplus 
}
#endif

#endif // _FORM_LAYOUT_H_
