//***************************************************************
// *
// * Project:
// *    Overridable Resources for the Flash Updater application 
// *
// * Copyright info:
// *    Copyright Handspring, Inc. 1999.
// *
// * FileName:
// *    CardUpdaterOvrRsc.h
// * 
// * Description:
// *    This file contains application resource constant definitions
// *	  that can be defined to override those in the Flash Updater Core
// *	  Application. 
// *
// * ToDo:
// *
// * History:
// *    17-May-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
// *
// ****************************************************************/



// Version resource ID
#define resOvrVerID						1

//
// Text strings						
//
#define resOvrStrIDHelpText				5000	// text string to display when user taps Info/Help
#define resOvrStrIDMainFormTitleText	5001	// text for the main form's title
#define resOvrStrIDMainFormDescription	resOvrStrIDMainFormTitleText  // for backward compatibility
#define resOvrStrIDBuildDate			5002	// build date/time
#define resOvrStrIDStartUpdateAlertMsg	5003	// the text that appears in confirmation alert
												//  which is displayed when the use taps the
												//  "Update Now" button.
#define resOvrStrIDResetAlertMsg		5004	// the text that appears in the "reset" alert
												//  which is displayed after the image has been
												//  flashed
#define resOvrStrIDModuleProductName	5005	// This string specifies the name of
												//  the module product for using in
												//  the updater's user interface
												//  (alerts, prompts, etc.)

#define resOvrStrIDChipSelectAutoConfigScheme	5006  // 0 (zero) = leave Springboard chip
													  //     selects at their default values.
													  // 1 = auto-detect and configure flash
													  //     module configurations where the
													  //     flash chips' chip-enable inputs
													  //     are driven directly by the 1 or
													  //     both Springboard chip-select
													  //     signals (i.e. -- those that don't
													  //     rely on their own address decoding
													  //     logic.
													  // other values = reserved by Handspring, Inc.
													  //     for future enhancements.


//
// String lists (tSTL objects)
//

#define resOvrSTLCompatibilityList		5000	// list of of strings where each string
//  indicates a product with which this update is compatible. For most updaters, there
//  will be just one such string.  The Prefix field of the string list object MUST be
//  left empty.
//
//  Each string is formatted as: <manuf name>::<card name>::<firstVer-lastVer>
//
//  <card name> and <manuf name> are the card and manufacturer name strings as
//  would appear in the card header (each of these names can be a maximum of 31
//  ASCII characters); <firstVer-lastVer> is an inclusive range of card version
//  numbers (as would appear in the module's card header) where the version
//  number can be specified either in decimal or hex (if using hex, the number
//  must be prefixed with 0x -- the number zero followed by the letter 'x').
//  Keep in mind that the card version number is a two-byte integer. The
//  expected format for card version numbers is:
//
//   0xMMmm
//
//  where MM (the most significant byte) is the major version number, and mm (the
//  least significant byte) is the minor version number of the card.
//
//  As a special case, the string whose only content is the '*' (asterisk) character
//  ("*") matches any module (i.e., allows the update to be applied to any module,
//  regardless of its manufacturer, name, and version).
//
//  For example:
//
//  "My Shop, Inc.::My Cool Module::0x0100-0x0102"
//
//  this string indicates that this updater is valid for updating My Shop, Inc.'s
//  My Cool Module card image for versions 1.0 through 1.2, inclusively.  The
//  manufacturer name and card name must exactly match those used in the product's
//  card header.
