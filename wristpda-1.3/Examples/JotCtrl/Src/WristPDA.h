/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: WristPDA.h
 *
 * Description:
 *		Include file for 
 *
 * History:
 *   	Jan03,03	MT	 Initial revision based on Flextronic's include file
 *   	Jan07,03	MT	 First Publication
 *
 *****************************************************************************/

#ifndef	__WRISTPDA_H__
#define	__WRISTPDA_H__

#include <PalmOS.h>


/////////////////////////////////////////////////////////////////////////////////
// How to detect if this is a Fossil OS or not
//
#define	WPdaCreator					'Foss'
#define	WPdaFtrNumVersion			0
#define	WPdaGetVersion( verP )	\
			(Err) FtrGet( WPdaCreator, WPdaFtrNumVersion, verP )

#define	WPdaDialogsCreator		'FosD'

#define	WPdaExt2Creator 'FosX'

// Usefull to conditionally compile code when WristPDA.h is included or not
#define	WRISTPDA						1


/////////////////////////////////////////////////////////////////////////////////
// Fossil Key definitions
//
// Key bit masks
#define	keyBitRockerUp				0x1000		// Rocker switch up
#define	keyBitRockerDown			0x2000		// Rocker switch down
#define	keyBitEnter					0x4000		// Rocker switch in
#define	keyBitBack					0x8000		// Back key


/////////////////////////////////////////////////////////////////////////////////
// Virtual key character codes
//
#ifndef	vchrFossilMin	
#define	vchrFossilMin    	   	0x1900      // Assigned to Fossil by Palm
#define	vchrFossilMax   	    	0x190F 
#endif

// Palm OS 5.0 R2 Thumb Characters (also known as Jog)
// The following values exist to allow all hardware that has these
// (optional) control clusters to emit the same key codes.
#ifndef	vchrThumbWheelUp
#define	vchrThumbWheelUp			0x012E		// optional thumb-wheel up
#define	vchrThumbWheelDown		0x012F		// optional thumb-wheel down
#define	vchrThumbWheelPush		0x0130		// optional thumb-wheel press/center
#define	vchrThumbWheelBack		0x0131		// optional thumb-wheel cluster back
#endif

#define vchrHardRockerEnter		vchrThumbWheelPush	// Rocker switch in
#define vchrHardRockerUp			vchrThumbWheelUp		// Rocker switch up
#define vchrHardRockerDown			vchrThumbWheelDown	// Rocker switch down
#define vchrHardBack					vchrThumbWheelBack	// Back key

#define	vchrFossilWatchMode		(vchrFossilMin +4)	// Watch Mode selection (PDA off)
#define	vchrFossilPDAMode			(vchrFossilMin +5)	// PDA Mode selection (Watch off)
#define	vchrFossilLCDUpdate		(vchrFossilMin +6)	// LCD (i.e. display) update


/////////////////////////////////////////////////////////////////////////////////
// Fossil-specific Preferences Panels Creator Types
// 
#define	sysFileCWristPDAPnl		'wpda'
#define sysFileCScreenWritePnl	'scwr'


/////////////////////////////////////////////////////////////////////////////////
// Fossil notifications
//

// The fossilNotifyWatchModeWakeup notification is broadcast upon
// wakeup from Watch mode when a Watch app is the current app
#define fossilNotifyWatchModeWakeup		'wkup'

/////////////////////////////////////////////////////////////////////////////////
// Fossil defines for UI
// 
#define	WPDA_BUTTON_HEIGHT		16


/////////////////////////////////////////////////////////////////////////////////
// Fossil Fonts
// 
#define	FossilFontIDStart			0x10
#define	FossilStdFont				(FossilFontIDStart +0)
#define	FossilBoldFont				(FossilFontIDStart +1)
#define	FossilLargeFont				(FossilFontIDStart +2)
#define	FossilSymbolFont			(FossilFontIDStart +3)
#define	FossilSymbol11Font			(FossilFontIDStart +4)
#define	FossilSymbol7Font			(FossilFontIDStart +5)
#define	FossilLedFont				(FossilFontIDStart +6)
#define	FossilLargeBoldFont			(FossilFontIDStart +7)
#define	FossilFontIDEnd				(FossilFontIDStart +7)

/////////////////////////////////////////////////////////////////////////////////
// Extended Fossil Fonts
// 
#define	FossilExtendedFontIDStart	0x20
#define	FossilLarge8Font			(FossilExtendedFontIDStart +0)
#define	FossilLargeBold8Font		(FossilExtendedFontIDStart +1)
#define	FossilExtendedFontIDEnd		(FossilExtendedFontIDStart +1)

// Note: While FossilLarge8Font and FossilLargeBold8Font are 'Fossil' fonts,
//       they are NOT part of the Fossil font set with a one-to-one mapping
//       onto the standard font set.  Apps that use these two fonts are
//       directly responsible for managing their use, with no assistance from
//       the Fossil font macros below.

#define	FossilIsFossilFont( _fontId )		\
			((_fontId >= FossilFontIDStart) && (_fontId <= FossilFontIDEnd))
#define	FossilIsSystemFont( _fontId )		\
			((_fontId >= stdFont) && (_fontId <= largeBoldFont))

#define	FossilLargeFontID( _doIt, _fontId )			\
			(_fontId + (_doIt && FossilIsSystemFont(_fontId) ? FossilFontIDStart : 0))
#define	FossilNormalFontID( _doIt, _fontId )			\
			(_fontId - (_doIt && FossilIsFossilFont(_fontId) ? FossilFontIDStart : 0))

/////////////////////////////////////////////////////////////////////////////////
// Fossil Wrist PDA Watch app custom launch codes
// 

// Draw the time with the current face and immediately return
#define wpdaAppLaunchWatchDrawTime	( sysAppLaunchCmdCustomBase + 0 )

// Switch to the next face, draw the time, and immediately return
#define wpdaAppLaunchWatchFaceNext	( sysAppLaunchCmdCustomBase + 1 )

// Switch to the previous face, draw the time, and immediately return
#define wpdaAppLaunchWatchFacePrev	( sysAppLaunchCmdCustomBase + 2 )

/////////////////////////////////////////////////////////////////////////////////
// Fossil API
// 
#define	FossilAPITrap		sysTrapOEMDispatch
#define	FOSSIL_API(sel)	\
	_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE,FossilAPITrap,sel)

#define	FossilAPIUnused1						0
#define	FossilAPIUnused2						1
#define	FossilAPIBackKeyModeGet				2
#define	FossilAPIBackKeyModeSet				3
#define	FossilAPIMenuCmdBarIconsGet		4
#define	FossilAPIMenuCmdBarIconsSet		5
#define	FossilAPIDisplayRefreshRateGet	6
#define	FossilAPIDisplayRefreshRateSet	7
#define  FossilAPIResetStat					8
#define  FossilAPIFrmIsFossil					9
#define  FossilAPIMenuSetActiveMenuRscID	10
#define  FossilAPIMenuLargeFontGet			11
#define  FossilAPIMenuLargeFontSet			12
#define  FossilAPIFrmEnlargeObject			13
#define	FossilAPIMaxSelector					14


/***********************************************************************
 *
 *	FUNCTION:		FossilBackKeyModeGet
 *
 *	DESCRIPTION:	Gets the Back key interpretation mode.
 *						See FossilBackKeyModeSet() for valid values
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *
 ***********************************************************************/
#define	kFossilBackKeyNoAction	0x0000
#define	kFossilBackKeyLauncher	0x0001
#define	kFossilBackKeyStopEvent	0x0002	// Default set by FossilResetStat()
														// and by Launching a new app.
UInt16	FossilBackKeyModeGet(void)
			FOSSIL_API(FossilAPIBackKeyModeGet);


/***********************************************************************
 *
 *	FUNCTION:		FossilBackKeyModeSet
 *
 *	DESCRIPTION:	Sets the Back key interpretation mode.
 *						Valid values are
 *						kFossilBackKeyNoAction	// Ignore this key
 *						kFossilBackKeyLauncher	// Puts this key in the event queue,
 *															if not handled, insert an appStopEvent
 *					or kFossilBackKeyStopEvent // Insert an appStopEvent (without putting any key in the event queue)
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *		Mar10,03 MichT Add kFossilBackKeyStopEvent mode (new default mode)
 *
 ***********************************************************************/
void		FossilBackKeyModeSet( UInt16 newBackKeyMode )
			FOSSIL_API(FossilAPIBackKeyModeSet);



/***********************************************************************
 *
 *	FUNCTION:		FossilMenuCmdBarIconsGet
 *
 *	DESCRIPTION:	Gets the Menu Cmd Bar Fossil mode.
 *						Valid values are any combination of
 *							kFossilMenuCmdNone
 *							kFossilMenuCmdMenu
 *							kFossilMenuCmdFind
 *							kFossilMenuCmdFindMenu
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *
 ***********************************************************************/
#define	kFossilMenuCmdNone		0x0000
#define	kFossilMenuCmdMenu		0x0001
#define	kFossilMenuCmdFind		0x0002
#define	kFossilMenuCmdKeyboard	0x0004
#define	kFossilMenuCmdAll			\
			(kFossilMenuCmdMenu | kFossilMenuCmdFind | kFossilMenuCmdKeyboard)
//
UInt16	FossilMenuCmdBarIconsGet(void)
			FOSSIL_API(FossilAPIMenuCmdBarIconsGet);

/***********************************************************************
 *
 *	FUNCTION:		FossilMenuCmdBarIconsSet
 *
 *	DESCRIPTION:	Sets the Back key interpretation mode.
 *						Valid values are
 *						kFossilBackKeyNoAction and kFossilBackKeyLauncher
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *
 ***********************************************************************/
UInt16	FossilMenuCmdBarIconsSet( UInt16 newIconFlags )
			FOSSIL_API(FossilAPIMenuCmdBarIconsSet);


/***********************************************************************
 *
 *	FUNCTION:		FossilMenuSetActiveMenuRscID
 *
 *	DESCRIPTION:	Set the active menu from a Resouirce ID
 *						and indicates if this menu if to be enlarged.
 *
 *	HISTORY
 *		Jan18,03	MichT	Initial revision
 *
 ***********************************************************************/
void		FossilMenuSetActiveMenuRscID(UInt16 resourceId, Boolean largeMenu )
			FOSSIL_API(FossilAPIMenuSetActiveMenuRscID);

/***********************************************************************
 *
 *	FUNCTION:		FossilMenuLargeFontGet
 *
 *	DESCRIPTION:	Return if this menu is displayed in large fonts
 *
 *	HISTORY
 *		Jan18,03	MichT	Initial revision
 *
 ***********************************************************************/
Boolean	FossilMenuLargeFontGet( MenuBarType* menuP )
			FOSSIL_API(FossilAPIMenuLargeFontGet);

/***********************************************************************
 *
 *	FUNCTION:		FossilMenuLargeFontSet
 *
 *	DESCRIPTION:	Set if a menu is to be displayed in Large font
 *
 *	HISTORY
 *		Jan18,03	MichT	Initial revision
 *
 ***********************************************************************/
Boolean	FossilMenuLargeFontSet( MenuBarType* menuP, Boolean setLargeFont )
			FOSSIL_API(FossilAPIMenuLargeFontSet);
			

/***********************************************************************
 *
 *	FUNCTION:		FossilDisplayRefreshRateGet
 *
 *	DESCRIPTION:	Gets refresh rate in number of system ticks
 *						(1 refresh per rate/SysTicksPerSecond() seconds)
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *
 ***********************************************************************/
UInt16	FossilDisplayRefreshRateGet( void )
			FOSSIL_API(FossilAPIDisplayRefreshRateGet);

/***********************************************************************
 *
 *	FUNCTION:		FossilDisplayRefreshRateSet
 *
 *	DESCRIPTION:	Set refresh rate in number of system ticks
 *						(1 refresh per newRate/SysTicksPerSecond() seconds)
 *
 *	HISTORY
 *		Jan06,03	MichT	Inspired from ExpMgr
 *
 ***********************************************************************/
void		FossilDisplayRefreshRateSet( UInt16 newRefreshRate )
			FOSSIL_API(FossilAPIDisplayRefreshRateSet);
#define	kFossilRefreshAuto		0xFFFE
#define	kFossilRefreshDefault	0xFFFD


/***********************************************************************
 *
 *	FUNCTION:		FossilResetStat
 *
 *	DESCRIPTION:	Resets the Wrist PDA to compatibility mode
 *
 *	HISTORY
 *		Jan06,03	MichT	Initial revision
 *
 ***********************************************************************/
void		FossilResetStat( void )
			FOSSIL_API(FossilAPIResetStat);


// Form API
#define	FossilFrmGadgetLargeFontIDMod100		70
#define	FossilFrmGadgetLargeFontRectX			0
#define	FossilFrmGadgetLargeFontRectY			1
#define	FossilFrmGadgetLargeFontRectW			2
#define	FossilFrmGadgetLargeFontRectH			3
#define	FossilFrmGadgetLargeFontRect			\
			{FossilFrmGadgetLargeFontRectX,		\
			 FossilFrmGadgetLargeFontRectY,		\
			 FossilFrmGadgetLargeFontRectW,		\
			 FossilFrmGadgetLargeFontRectH}


/***********************************************************************
 *
 *	FUNCTION:		FossilFrmIsLargeFont
 *
 *	DESCRIPTION:	Return true if the form is a Fossil Friendly form
 *
 *	HISTORY
 *		Jan18,03	MichT	Initial revision
 *
 ***********************************************************************/
Boolean	FossilFrmIsLargeFont( const FormType* formP )
			FOSSIL_API(FossilAPIFrmIsFossil);


/***********************************************************************
 *
 *	FUNCTION:		FossilFrmEnlargeObject
 *
 *	DESCRIPTION:	Enlarge one or all objects in a form
 *						Must be called before a form is first displayed
 *
 *	HISTORY
 *		Jan18,03	MichT	Initial revision
 *
 ***********************************************************************/
void		FossilFrmEnlargeObject( FormType* formP, UInt16 objectIdx )
			FOSSIL_API(FossilAPIFrmEnlargeObject);
#define	kFossilFrmEnlargeObjectAll		0xFFFF


#include "HAL.h" // Needed by FossilExtendedFontSelectGet/Set APIs

/***********************************************************************
 *
 *	FUNCTION:		FossilExtendedFontSelectGet
 *
 *	DESCRIPTION:	Return FontSelect extended dialog state
 *
 *					Boolean * extendedP - Ptr to UInt32 to receive state
 *
 *	HISTORY
 *		apr14,03	DMC	Initial revision
 *
 ***********************************************************************/
#define FossilExtendedFontSelectGetOpCode	( WPdaExt2Creator + 1 )
#define FossilExtendedFontSelectGet( extendedP ) \
	HwrCustom( WPdaExt2Creator, FossilExtendedFontSelectGetOpCode, (void *) extendedP, NULL )

/***********************************************************************
 *
 *	FUNCTION:		FossilExtendedFontSelectSet
 *
 *	DESCRIPTION:	Control use of extended FontSelect dialog
 *
 *					Boolean extended: true  -> Use extended dialog
 *									  false -> Use original dialog
 *
 *	HISTORY
 *		apr14,03	DMC	Initial revision
 *
 ***********************************************************************/
#define FossilExtendedFontSelectSetOpCode	( WPdaExt2Creator + 2 )
#define FossilExtendedFontSelectSet( extended ) \
	HwrCustom( WPdaExt2Creator, FossilExtendedFontSelectSetOpCode, (void *) extended, NULL )

#endif//	__WRISTPDA_H__
