/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GUIUtilities.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

 #ifndef _GUIUTILITIES_H_
 #define _GUIUTILITIES_H_

#include <PalmTypes.h>
#include <Form.h>
#include <Field.h>
#include <ScrollBar.h>

// Resources utilities
uint16_t 	UtilitiesMapToPositionByte (uint8_t *mappingArray, uint8_t value, uint16_t mappings, uint16_t defaultItem);
uint8_t 	UtilitiesGetValueFromByteListRsc(DmOpenRef dbRef, int16_t listRscID, int16_t selection);

uint16_t 	UtilitiesMapValueInByteListRsc(DmOpenRef dbRef, int16_t listRscID, uint8_t value);


// Text utilities
char* 		UtilitiesSkipLeadingSpace(char * strP);
char* 		UtilitiesRemoveControlChars(char * strP);
char* 		UtilitiesTruncateStringWithEllipses(const char* stringP, char* outputP, Coord desiredWidth);


// Forms' object utilities
void* 		UtilitiesGetFormObjectPtr (FormType* formP, uint16_t objectID);
void* 		UtilitiesGetObjectPtr (uint16_t formId, uint16_t objectId);
void		UtilitiesGetFormObjectPosition (FormType* formP, uint16_t objectID, Coord* x, Coord* y);
void 		UtilitiesGetFormObjectBound (FormType* formP, uint16_t objectID, RectangleType* bounds);

void 		UtilitiesFormHideObjectID (FormType* formP, uint16_t objectID);
void 		UtilitiesFormShowObjectID (FormType* formP, uint16_t objectID);
void 		UtilitiesSetFormObjectPosition (FormType* formP, uint16_t objectID, Coord x, Coord y);
void 		UtilitiesSetFormObjectBound (FormType* formP, uint16_t objectID, RectangleType* bounds);
void 		UtilitiesMoveFormObject(FormType* formP, uint16_t objectID, Coord deltaX, Coord deltaY, Boolean redraw);
void 		UtilitiesFormSetList(uint16_t listResID, uint16_t triggerResID, int16_t selection);


// Field utilities
void 		UtilitiesSetFieldPtrText(FieldType* fieldP, char * srcP, Boolean append);
void 		UtilitiesGetFieldPtrText(FieldType* fieldP, uint16_t bufSize, char * destP);
void 		UtilitiesGetFieldText(uint16_t fldID, uint16_t bufSize, char * destP);

void 		UtilitiesResizeEditField (FieldType* fieldP, uint16_t maxNumLines, uint16_t newHeight, uint16_t currentPos);


// Scrollbars utilities
void 		UtilitiesUpdateScrollBar(FieldType* fieldP, ScrollBarType* scrollP);
void 		UtilitiesScrollScrollBar(uint16_t scrollID, int16_t linesToScroll, Boolean updateScrollbar, uint16_t fldID);
Boolean 	UtilitiesHandleTextScrollWithKey(EventType* event, uint16_t fieldID, uint16_t scrollID) ;


// Event utilities
void 		UtilitiesSendSimpleEvent(eventsEnum eType);


// Graffiti Shift utilities
uint16_t 	UtilitiesGetGraffitiObjectIndex (uint16_t formId);


// Gadget Utilities
void 		UtilitiesGadgetFree(uint16_t formId, uint16_t gadgetId);
void 		UtilitiesGadgetChangeBitmap(DmOpenRef dbRef, uint16_t formId, uint16_t gadgetId, uint16_t bitmapId);
void 		UtilitiesGadgetSetup(DmOpenRef dbRef, uint16_t formId, uint16_t gadgetId, uint16_t bitmapId, FormGadgetHandlerType* gadgetCallback);
Boolean 	UtilitiesGadgetDrawBitmapCallback(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);


// Bitmap Utilities
void 		UtilitiesGetBitmapPtrBounds(FormType* formP, BitmapType*	bitmapP, RectangleType * bounds);
void 		UtilitiesGetBitmapHandleBounds(FormType* formP, MemHandle bitmapH, RectangleType * bounds);


// Drawing Utilities
void 		UtilitiesDrawBitmapPtr(BitmapType*	bitmapP, RectangleType * bounds);
void 		UtilitiesDrawBitmapHandle(MemHandle bitmapH, RectangleType * bounds);
void 		UtilitiesDrawBitmap(DmOpenRef dbRef, uint16_t bitmapId, RectangleType * bounds);



void 		UtilitiesDrawBitmapAt(DmOpenRef dbRef, uint16_t bitmapId, Coord *atX, Coord *atY, RectangleType* bounds);
void 		UtilitiesDrawBitmapHandleAt(MemHandle bitmapH, Coord* atX, Coord* atY, RectangleType* bounds);
void 		UtilitiesDrawBitmapAt(DmOpenRef dbRef, uint16_t bitmapId, Coord *atX, Coord *atY, RectangleType* bounds);


// Rectangle utilities
Boolean 	UtilitiesRctRectangleEquals(RectangleType* rect1, RectangleType* rect2);
 #endif
