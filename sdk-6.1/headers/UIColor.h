/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: UIColor.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 * 	This file defines structs and functions for setting the "system
 *		colors" that the UI routines use.
 *
 *****************************************************************************/

#ifndef __UICOLOR_H__
#define __UICOLOR_H__

#include <PalmTypes.h>

#include <Window.h>

enum UIColorTableEntriesTag {
	UIObjectFrame = 0,
	UIObjectFill,
	UIObjectForeground,
	UIObjectSelectedFill,
	UIObjectSelectedForeground,

	UIMenuFrame,
	UIMenuFill,
	UIMenuForeground,
	UIMenuSelectedFill,
	UIMenuSelectedForeground,

	UIFieldBackground,
	UIFieldText,
	UIFieldTextLines,
	UIFieldCaret,
	UIFieldTextHighlightBackground,
	UIFieldTextHighlightForeground,
	UIFieldFepRawText,
	UIFieldFepRawBackground,
	UIFieldFepConvertedText,
	UIFieldFepConvertedBackground,
	UIFieldFepUnderline,

	UIFormFrame,
	UIFormFill,

	UIDialogFrame,
	UIDialogFill,

	UIAlertFrame,
	UIAlertFill,

	UIOK,
	UICaution,
	UIWarning,

	UIMenuBarFill,
	UIMenuBarSelectedFill,
	
	UILastColorTableEntry
} ;
typedef Enum8 UIColorTableEntries;



#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------
// UI Color Table Manipulation Routines 
//------------------------------------------------------------

IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which);

void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP);

status_t UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP);

#ifdef __cplusplus 
}
#endif

#endif //__UICOLOR_H__
