/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UIControls.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *             	Contrast & brightness control for devices with
 *						software contrast.
 *
 *****************************************************************************/

#ifndef	__UICONTROLS_H__
#define	__UICONTROLS_H__

#include <Window.h>

// for UIPickColor
#define UIPickColorStartPalette	0
#define UIPickColorStartRGB		1

typedef uint16_t UIPickColorStartType;



#ifdef __cplusplus
extern "C" {
#endif

extern void UIContrastAdjust(void);

extern void UIBrightnessAdjust(void);

Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP,
					     UIPickColorStartType start, const char *titleP,
					     const char *tipP);


#ifdef __cplusplus 
}
#endif

#endif	// __UICONTROLS_H__
