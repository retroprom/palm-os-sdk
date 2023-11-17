/*
 *  HandspringPanel.h - 'Panel' Drop-In Preferences
 *
 *  Copyright © 2001 metrowerks inc.  All rights reserved.
 *
 */

#ifndef _H_Panel
#define _H_Panel

#ifndef __TYPES__
#	include <Types.h>
#endif


#pragma options align=mac68k


/* this is the name of the panel */
#define kHandspringPanelName	"Handspring Panel"


// PREF PANEL TYPES *********************
#ifdef WIN32
	typedef unsigned char Str255[256];
	typedef unsigned char Str63[64];
#endif

//MakeROM pref panel structure
#pragma options align=mac68k
typedef struct HandspringPref
{
	short	dataversion;		 // version # of prefs data
	Str63	name;                // rom name
	Str63   manuf;               // rom manufacturer
	unsigned long version;       // rom version number
	Str63	outfile;			 // rom output file name
	unsigned long hdr_offset;    // rom hdr offset
	unsigned long initial_stack; // rom initial stack pointer
	int cmdline;                 // using command line ?
	Str255  command_line;        // command line
	
} HandspringPref, **HandspringPrefHandle;
#pragma options align=reset


#endif	/* _H_Panel */
