/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateBackground.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef __DATEBACKGROUND_H__
#define __DATEBACKGROUND_H__

#include <Form.h>
#include <DataMgr.h>

typedef struct {
	RGBColorType		color;
	FormPtr				formP;
	DmResourceID		bitmapID;
	RectangleType		whiteRect;
} DateGadgetCallbackDataType;

extern Boolean DateBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);
extern void DateBackgroundGetHighlightColor(RGBColorType *color, uint16_t month);
extern void DateBackgroundGetBitmapID(DmResourceID *bitmapID, uint16_t month);

#endif
