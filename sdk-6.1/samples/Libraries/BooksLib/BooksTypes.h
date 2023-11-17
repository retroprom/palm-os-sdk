/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BooksTypes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             Contains constants and structures for AddressBook sample app.
 *
 *****************************************************************************/

#ifndef _BOOKSTYPE_H_
#define _BOOKSTYPE_H_

#include <PalmTypes.h>
#include <Window.h>
#include <Form.h>


#include "BooksRenderingTypes.h"


/*	------------------------------------------------------------------- */

// flags for book
#define kBookUseFormTabs					0x0001
#define kBookCanHaveScrollsFlag				0x0002
#define kBookHasScrollFlag 					0x0004
#define kBookLeftSideScrollsFlag			0x0008
#define kBookBothSideScrollsFlag			0x0010
#define kBookHilightLeftArrow				0x0020
#define kBookHilightRightArrow				0x0040
#define kBookUsePalmOSFont					0x0080
#define kBookRestoreLockedTabsVisibility	0x0100
#define kBookEnableUpdatePosting			0x0200
#define kBookCanSelectCurrentTab			0x0400
#define kBookCanTrackCurrentTab				0x0800
#define kBookOwnsFocusRing					0x1000
#define kBookIsFocused						0x2000
#define kBookDoNotActiveFocusedTab			0x4000






// reserved flags
#define kBookScaledMakerFlag				0x8000

#define kBookStandardLookAndFeel			(kBookCanHaveScrollsFlag | kBookBothSideScrollsFlag | kBookUsePalmOSFont | kBookRestoreLockedTabsVisibility)
#define kBookStandardLookAndFeelNoScroll	(kBookUsePalmOSFont)

// flags for book's Tabs
#define kBookTabInvisibleFlag				0x0001
#define kBookTabHiddenFlag					0x0002
#define kBookTabIsActiveFlag				0x0004
#define kBookTabCenterLabelHorizontally		0x0008
#define kBookTabHightligthed				0x0010
#define kBookTabHasFocus					0x0020



#define kBookTabMaxIndex					((uint16_t) (-1))
#define kBookTabRightArrowIndex				((uint16_t) (0xFFFE))
#define kBookTabLeftArrowIndex				((uint16_t) (0xFFFD))

#define kBookInvalidTabIndex 				((uint16_t)(-1))
#define kBookInvalidTabId					((uint16_t)(-1))
#define kBookTabIndexWidth 					((uint16_t)(-1))

#define kBookTabsSlopWidth					0
#define kBookLabelSurroundWidth				3
#define kBookLabelVPositionQuo				1
#define kBookLabelVPositionDiv				2

#define kBookTabIndexTopOffset				0
#define kBookTabDrawLineWidth				1

#define kBookTabIndexTopOffset				0


#define	kBookArrowHeigth					7
#define kBookArrowWidth						5
#define kBookArrowLeftSpace					1
#define kBookArrowMiddleSpace				2
#define kBookArrowRightSpace				1
#define kBookArrowTopSpace					2

#define kBookGrafftiStrokeWidth				40  /* specified as single density */
#define kBookShowActiveTapsNum				((uint32_t) 2)

#define kBookScrollWidth 					(kBookArrowWidth + kBookArrowWidth + kBookArrowLeftSpace + kBookArrowMiddleSpace + kBookArrowRightSpace)

#define kBookTabKind						0x0001
#define kBookScrollKind						0x0002

#define kBookActiveTabFlag					0x0100
#define kBookRightTabFlag					0x0200
#define kBookLeftFlag						0x0400
#define kFirstScrollTimer					200	
#define kNextScrollTimer					100	


/*	------------------------------------------------------------------- */

struct BookTag; // forward declaration

typedef void (BookActiveCallbackProcType)(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData);
typedef Boolean (BookDeactiveCallbackProcType)(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData);


/*	------------------------------------------------------------------- */

typedef struct TabTag
{
	uint16_t						tabId;				// User Defined ID to identify a Tab
	uint16_t						flags;				// tab relative flags
	uint32_t						userData;			// info set by user and pass into the callback
	DmOpenRef						bitmapDmOpenRef;	// DmOpenRef of Database resource that holds both Bitmaps
	DmResourceID					bgBitmapId;			// Tab bitmap icon to use in place of name if non 0
	DmResourceID					fgBitmapId;			// Tab bitmap icon to use in place of name if non 0
	char*							nameP;				// pointer to Name to show as Tab Tab label
	Coord							tx;					// last text x used to draw
	Coord							ty;					// last text y used to draw
	Coord							xOffset;			// last tab x offset
	Coord							yOffset;			// last tab y offset
	Coord							labelWidth;			// label width for tab index in foreground (the selected index)
	Coord							labelHeight;		// height for graphics label in foreground (the selected index)
	Coord							horizCentering;		// Horizontal Offset to center label in foreground
	uint16_t						padding1;			// Arm Padding
	BookActiveCallbackProcType*		activationProc;		// Callback for tab Activation
	BookDeactiveCallbackProcType*	deactivationProc;	// CallBack for tab Deactivation
	winInvalidateFunc				invalidateBodyProc;	// Callback for tab drawing with WinInvalidateRectFunc()
	GcBitmapHandle					bgGcBitmapH;		// Loaded  back Bitmap (bgBitmapId)
	GcBitmapHandle					fgGcBitmapH;		// Loaded  fore Bitmap (fgBitmapId)
	void*							tabSkin;			// skin for the tab
} TabType, *TabPtr;


typedef struct BookTag
{
	FormType*						formP;				// Owner form Ptr
	RectangleType					bookTabZoneSd;		// rectangle zone for the book tab  part from resource (single density)
	RectangleType					bookBodyZoneSd;		// rectangle zone for the book body part from resource (single density)
	RectangleType					bookTabZoneDd;		// rectangle zone for the book tab  part (double density)
	RectangleType					bookBodyZoneDd;		// rectangle zone for the book body part (double density)
	RectangleType					bookTabZone;		// rectangle zone for the book tab  part
	RectangleType					bookBodyZone;		// rectangle zone for the book body part
	RectangleType					bookActiveZone;		// active tab's rectangle
	TabFrameType					frameInfo;			// hold information on how a book had to be drawn
	DmResourceID					formId;				// Owner form ID
	DmResourceID					bookTabGadgetId;	// Id of the gadget associated to the book in the owner form
	DmResourceID					bookBodyGadgetId;	// Id of gadget associated to the book body in the owner form
	uint16_t						allocatedTabs;		// Number of tabs for which we preallocate memory
	uint16_t						tabNum;				// Number of tabs in the book
	uint16_t						drawnTabsNum;		// Number of tabs that were drawn last time book was drawn
	uint16_t						firstVisibleTab;	// first visible Tab index
	uint16_t						currentTabIndex;	// active tab
	uint16_t						focusedTabIndex;	// focused tab for 5 way Navigation
	uint16_t						flags;				// flags
	uint16_t						maxTabWidth;		// maximun width for a tab
	uint16_t						minTabWidth;		// minimum width for a  tab
	Coord							bookWidth;			// full book width
	Coord							tabHeight;			// book's tab height
	Coord							tabSpacing;			// Tab spacing (slope)
	Coord							tabTopOffset;		// vertical put back for active tab frame (should never change)
	Coord							labelSurround;		// space to be left before (left) and after (right) label (icon or text)
	Coord							labelVPositionQuo;	// to center vertically text in the tab index
	Coord							labelVPositionDiv;	// use  as :  (height *labelVPositionQuo) / labelVPositionDiv
	FontID							labelFont;			// font for backgroung tabs labels (stdFont by default - can be overiden with GCFont)
	void*							leftScrollSkin;	// right scroll
	void*							rightScrollSkin;	// right scroll
	uint32_t						firstScrollTimer;	// first timer used within autoscroll
	uint32_t						nextScrollTimer;	// next timer used within autoscroll
	uint8_t							padding[3];			// Arm Padding
	TabType							tabs[1];			// Tabs array
} BookType, *BookPtr;


#define BookTabIndexSurrounds(bookP)	( ( (bookP)->slopeWidth) * 2 + ((bookP)->labelSurround) * 2)
#define BookSize(TabsToAllocate) 		((uint32_t) (sizeof (BookType)  + (((TabsToAllocate) - 1) * sizeof (TabType))) )


#endif
