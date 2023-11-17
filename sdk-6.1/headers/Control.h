/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Control.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines check box routines.
 *
 *****************************************************************************/

#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <Event.h>

typedef struct ControlAttrTag ControlAttrType;


enum controlStyles {buttonCtl, pushButtonCtl, checkboxCtl, popupTriggerCtl,
						  selectorTriggerCtl, repeatingButtonCtl, sliderCtl,
						  feedbackSliderCtl };
typedef Enum8 ControlStyleType;

enum buttonFrames {noButtonFrame, standardButtonFrame, boldButtonFrame,
						 rectangleButtonFrame};
typedef Enum8 ButtonFrameType;

typedef struct ControlType ControlType;

typedef ControlType *ControlPtr;				// deprecated, use ControlType *


// GraphicControlType *'s can be cast to ControlType *'s and passed to all
// Control API functions (as long as the 'graphical' bit in the attrs is set)

typedef struct GraphicControlType GraphicControlType;


// SliderControlType *'s can be cast to ControlType *'s and passed to all
// Control API functions (as long as the control style is a slider)

typedef struct SliderControlType SliderControlType;

//----------------------------------------------------------
//	Control Functions
//----------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern void CtlDrawControl (ControlType *controlP);

extern void CtlEraseControl (ControlType *controlP);

extern void CtlHideControl (ControlType *controlP);

extern void CtlShowControl (ControlType *controlP);

extern Boolean CtlEnabled (const ControlType *controlP);

extern void CtlSetEnabled (ControlType *controlP, Boolean usable);

extern void CtlSetUsable (ControlType *controlP, Boolean usable);

extern int16_t CtlGetValue (const ControlType *controlP);

extern void CtlSetValue (ControlType *controlP, int16_t newValue);

extern const char *CtlGetLabel (const ControlType *controlP);

extern void CtlSetLabel (ControlType *controlP, const char *newLabel);

extern void CtlSetGraphics (ControlType *ctlP, DmOpenRef database,
	DmResourceID newBitmapID, DmResourceID newSelectedBitmapID);

extern void CtlSetSliderValues(ControlType *ctlP, const uint16_t *minValueP, const uint16_t *maxValueP,
					const uint16_t *pageSizeP, const uint16_t *valueP);

extern void CtlGetSliderValues(const ControlType *ctlP, uint16_t *minValueP, uint16_t *maxValueP,
					uint16_t *pageSizeP, uint16_t *valueP);

extern void CtlHitControl (const ControlType *controlP);

extern Boolean CtlHandleEvent (ControlType *controlP, EventType *pEvent);

extern Boolean CtlValidatePointer (const ControlType *controlP);

extern ControlType *CtlNewControl (void **formPP, uint16_t ID,
 	ControlStyleType style, const char *textP,
	Coord x, Coord y, Coord width, Coord height,
	FontID font, uint8_t group, Boolean leftAnchor);

extern GraphicControlType *CtlNewGraphicControl (void **formPP, uint16_t ID,
   ControlStyleType style, DmOpenRef database, DmResourceID bitmapID, DmResourceID selectedBitmapID,
   Coord x, Coord y, Coord width, Coord height,
   uint8_t group, Boolean leftAnchor);

extern SliderControlType *CtlNewSliderControl (void **formPP, uint16_t ID,
   ControlStyleType style, DmOpenRef database, DmResourceID thumbID, DmResourceID backgroundID,
   Coord x, Coord y, Coord width, Coord height, uint16_t minValue, uint16_t maxValue,
   uint16_t pageSize, uint16_t value);

extern ControlStyleType CtlGetControlStyle(const ControlType *ctlP);

extern FontID CtlGetFont (const ControlType *ctlP);

extern void CtlSetFont (ControlType *ctlP, FontID fontID);

extern void CtlGetGraphics (const ControlType *ctlP, DmResourceID *bitmapID, DmResourceID *selectedBitmapID);

extern void CtlSetLeftAnchor (ControlType *ctlP, Boolean leftAnchor);

extern Boolean CtlIsGraphicControl (ControlType *ctlP);

extern void CtlSetFrameStyle (ControlType *ctlP, ButtonFrameType frameStyle);

extern void CtlDrawCheckboxControl(fcoord_t left, fcoord_t top, Boolean selected);

#ifdef __cplusplus
}
#endif


#endif //_CONTROL_H_
