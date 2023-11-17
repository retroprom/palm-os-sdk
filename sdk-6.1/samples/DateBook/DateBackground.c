/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateBackground.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <UIResources.h>
#include <SysUtils.h>
#include <SystemResources.h>
#include <string.h>
#include <stdlib.h>

#include "DateBackground.h"

enum {
	cacheUninitalized = 0,
	cacheImageExists,
	cacheNoImage
};

static DmOpenRef gSkinDbP = 0;
static Boolean gSkinDBSearched = false;
static RGBColorType gDefaultBackColor;
static RGBColorType gMonthBackColorsCache[12];
static Boolean		gMonthBackCacheValid[12];
static uint8_t		gMonthImageExistsCache[12];

extern DmOpenRef gApplicationDbP;

/***********************************************************************
 *
 * FUNCTION:    DateBackgroundGadgetHandler
 *
 * DESCRIPTION: Gadget handler callback for the background gadget.  We
 *              draw the gadget here.
 *
 * PARAMETERS:  gadgetP - the gadget being drawn
 *              cmd - command for this call
 *              paramP - unused
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
Boolean DateBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP,
									uint16_t cmd, void *paramP)
{
	DateGadgetCallbackDataType	*cbdP;
	GcGradientType	grad;
	GcPointType		pnt[2];
	GcColorType		col[2];
	GcHandle		gc;
	GcBitmapHandle	headingImageH;
	RectanglePtr	bounds;
	Coord			width;
	Boolean			doWhiteRect = false;

	if(cmd != formGadgetDrawCmd)
		return false;

	cbdP = (DateGadgetCallbackDataType *)gadgetP->data;

	// If the color is black, it means don't draw a gradient
	if(cbdP->color.r == 0 && cbdP->color.g == 0 && cbdP->color.b == 0)
		return true;

	if(cbdP->whiteRect.extent.x != 0)
		doWhiteRect = true;

	// Draw the gradient background
	bounds = &gadgetP->rect;

	col[0].red = col[1].red = cbdP->color.r;
	col[0].green = col[1].green = cbdP->color.g;
	col[0].blue = col[1].blue = cbdP->color.b;
	col[0].alpha = 128;
	col[1].alpha = 255;

	pnt[0].x = (fcoord_t)(bounds->topLeft.x);
	pnt[0].y = (fcoord_t)(bounds->topLeft.y);
	pnt[1].x = (fcoord_t)(bounds->topLeft.x);
	pnt[1].y = (fcoord_t)(bounds->topLeft.y + bounds->extent.y);

	gc = GcGetCurrentContext();

	GcInitGradient(&grad, pnt, col, 2);
	GcSetGradient(gc, &grad);

	GcSetDithering(gc, true);

	if(doWhiteRect)
		GcSetWindingRule(gc, kOddWinding);

	GcRect(gc, pnt[0].x, pnt[0].y,
		   pnt[1].x + (fcoord_t)bounds->extent.x, pnt[1].y);

	if(doWhiteRect)
		GcRectI(gc, cbdP->whiteRect.topLeft.x, cbdP->whiteRect.topLeft.y,
				cbdP->whiteRect.topLeft.x + cbdP->whiteRect.extent.x,
				cbdP->whiteRect.topLeft.y + cbdP->whiteRect.extent.y);

	GcPaint(gc);

	GcReleaseContext(gc);

	// Draw background image if there is one
	if(cbdP && cbdP->bitmapID && gSkinDbP)
	{
		headingImageH = FrmGetBitmapHandle(cbdP->formP, gSkinDbP,
										   bitmapRsc, cbdP->bitmapID, 0);

		if(headingImageH)
		{
			width = (Coord)GcGetBitmapWidth(headingImageH);
			width = WinScaleCoordNativeToActive(width, false);
			bounds = &gadgetP->rect;
			WinDrawBitmapHandle(headingImageH, bounds->extent.x - width, 0);
		}
	}

	gc = GcGetCurrentContext();

	if(doWhiteRect)
	{
		GcSetColor(gc, 255, 255, 255, 255);
		GcRectI(gc, cbdP->whiteRect.topLeft.x, cbdP->whiteRect.topLeft.y,
				cbdP->whiteRect.topLeft.x + cbdP->whiteRect.extent.x,
				cbdP->whiteRect.topLeft.y + cbdP->whiteRect.extent.y);
		GcPaint(gc);
	}

	GcReleaseContext(gc);

	return true;
}

status_t PrvGetColorResource(DmOpenRef dbP, DmResourceID id, int16_t index,
							 RGBColorType *color)
{
	char		*strP;
	uint8_t	colorValue;
	char		colorStr[8];
	char		parseStr[4];

	if(!dbP)
	{
		return dmErrNoOpenDatabase;
	}

	if(gMonthBackCacheValid[index])
	{
		*color = gMonthBackColorsCache[index];
		return errNone;
	}
	
	strP = SysStringByIndex(dbP, id, index, colorStr, 8);

	if(!strP)
		return dmErrResourceNotFound;

	if(strlen(strP) != 7 )  // Has to be #123456 format
		return dmErrInvalidParam;

	parseStr[2] = '\0';

	parseStr[0] = strP[1];
	parseStr[1] = strP[2];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->r = colorValue;

	parseStr[0] = strP[3];
	parseStr[1] = strP[4];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->g = colorValue;

	parseStr[0] = strP[5];
	parseStr[1] = strP[6];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->b = colorValue;

	gMonthBackColorsCache[index] = *color;
	gMonthBackCacheValid[index] = true;

	return errNone;
}

void DateBackgroundGetHighlightColor(RGBColorType *color, uint16_t month)
{
	Boolean		gotColor = false;
	status_t	err;

	// If we don't have the skin database open
	if(gSkinDbP == 0)
	{
		if(!gSkinDBSearched)
		{
			memset(gMonthBackCacheValid, 0, 12 * sizeof(Boolean));
			memset(gMonthImageExistsCache, 0, 12 * sizeof(uint8_t));
			// Open the skin database
			gSkinDbP = DmOpenDatabaseByTypeCreator('bimg', sysFileCDatebook,
												   dmModeReadOnly);
			gSkinDBSearched = true;
		}
	}

	if(gSkinDbP != 0)
	{
		err = PrvGetColorResource(gSkinDbP, 1000, month - 1, color);
		if(err == errNone)
			gotColor = true;
	}

	// If we didn't find a skin database, then use the default from our
	// own database
	if(!gotColor)
	{
		err = PrvGetColorResource(gApplicationDbP, 500, month - 1, color);
		if(err != errNone)
		{	// Hard coded fallback
			color->index = 0;
			color->r = 160;
			color->g = 160;
			color->b = 160;
		}
	}
}

void DateBackgroundGetBitmapID(DmResourceID *bitmapID, uint16_t month)
{
	MemHandle bitmapH;

	// Open the skin database if we need to
	if(gSkinDbP == 0)
	{
		if(!gSkinDBSearched)
		{
			memset(gMonthBackCacheValid, 0, 12 * sizeof(Boolean));
			memset(gMonthImageExistsCache, 0, 12 * sizeof(uint8_t));
			// Open the skin database
			gSkinDbP = DmOpenDatabaseByTypeCreator('bimg', sysFileCDatebook,
												   dmModeReadOnly);
			gSkinDBSearched = true;
		}
	}

	// If we didn't find the skin db, or month is out of range, use
	// bitmap 1000 from the datebook.prc
	if(!gSkinDbP || month < 1 || month > 12)
	{
		*bitmapID = 1000;
		return;
	}

	// Make month 0 based
	month--;

	// If we have already verified that this image exits return the id
	if(gMonthImageExistsCache[month] == cacheImageExists)
	{
		*bitmapID = 1000 + month;  // 1000 - 1011 for the month
		return;
	}

	// If we know the image does not exist, return 1000 (the default id)
	if(gMonthImageExistsCache[month] == cacheNoImage)
	{
		*bitmapID = 1000;
		return;
	}

	// If we get here, then we need to go try to find the image
	bitmapH = DmGetResource(gSkinDbP, bitmapRsc, 1000 + month);

	if(bitmapH)
	{
		DmReleaseResource(bitmapH);
		gMonthImageExistsCache[month] = cacheImageExists;
		*bitmapID = 1000 + month;  // 1000 - 1011 for the month
		return;
	}
	else
	{
		gMonthImageExistsCache[month] = cacheNoImage;
		*bitmapID = 1000;
		return;
	}
}
