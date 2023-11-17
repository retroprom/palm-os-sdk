/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Menu.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines menu routines.
 *
 *****************************************************************************/

#ifndef __MENU_H__
#define __MENU_H__

#include <PalmTypes.h>

#include <Font.h>
#include <Window.h>
#include <Event.h>
#include <DataMgr.h>

// Errors returned by Menu routines

#define menuErrNoMenu				(menuErrorClass | 1)
#define menuErrNotFound				(menuErrorClass | 2)
#define menuErrSameId				(menuErrorClass | 3)
#define menuErrTooManyItems			(menuErrorClass | 4)
#define menuErrOutOfMemory			(menuErrorClass | 5)


// Command bar structures

enum MenuCmdBarResultTag {
	menuCmdBarResultNone,			// send nothing (this'd be quite unusual but is allowed)
	menuCmdBarResultChar,			// char to send (with commandKeyMask bit set)
	menuCmdBarResultMenuItem,		// id of the menu item
	menuCmdBarResultNotify			// Nofication Manager notification type
} ;
typedef Enum8 MenuCmdBarResultType;
	
// maximum length of the prompt string to display in the command bar
#define menuCmdBarMaxTextLength 20

// to tell MenuCmdBarAddButton where to add the button: on right or left.
#define menuCmdBarOnRight 0
#define menuCmdBarOnLeft  0xff


////Menu-specific

#define noMenuSelection				-1
#define noMenuItemSelection			-1
#define separatorItemSelection		-2

// cause codes for menuOpen Event
#define menuButtonCause				0
#define menuCommandCause			1

// To match Apple's ResEdit the first byte of a menu item's text can
// be a special char indicating a special menu item.
#define MenuSeparatorChar			'-'

typedef struct MenuBarType MenuBarType;
typedef MenuBarType *MenuBarPtr;

	

#ifdef __cplusplus
extern "C" {
#endif

extern MenuBarType *MenuInit (DmOpenRef database, uint16_t resourceId);

extern MenuBarType *MenuGetActiveMenu (void);

extern MenuBarType *MenuSetActiveMenu (MenuBarType *menuP);

extern void MenuDispose (MenuBarType *menuP);

extern Boolean MenuHandleEvent (MenuBarType *menuP, EventType *event, status_t *error);

extern void MenuDrawMenu (MenuBarType *menuP);

extern void MenuEraseStatus (MenuBarType *menuP);

extern void MenuSetActiveMenuRscID (DmOpenRef database, uint16_t resourceId);
							
extern status_t MenuCmdBarAddButton(uint8_t where, DmOpenRef database, uint16_t bitmapId,
						MenuCmdBarResultType resultType, uint32_t result, char *nameP);

extern Boolean MenuCmdBarGetButtonData(int16_t buttonIndex, DmOpenRef *databaseP, uint16_t *bitmapIdP, 
						MenuCmdBarResultType *resultTypeP, uint32_t *resultP, char *nameP);
			
extern void MenuCmdBarDisplay (void);
							
extern Boolean MenuShowItem(uint16_t id);

extern Boolean MenuHideItem(uint16_t id);
						
extern status_t MenuAddItem(uint16_t positionId, uint16_t id, char cmd, const char *textP);

#ifdef __cplusplus 
}
#endif

#endif //__MENU_H__
