/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDisplayLayouts.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSDISPLAYLAYOUTS_H_
#define _ADDRESSDISPLAYLAYOUTS_H_

#include <TextMgr.h>

#define kViewRowLayoutRscType		'VRLT'
#define kViewBlockLayoutRscType		'VBLT'

// Fix limits for simplicity of use and definition
#define	kNumMaxFieldsByRow			5
#define	kNumMaxRowByLayout			6

#define kLayoutInvalidResID			((uint16_t)0xFFFF)
#define kLayoutBlankLine			((uint16_t)0xFFFF)

#define kDefaultSeparatorString		" "
#define kDefaultSeparatorLength		1

// Base ID for resources used to build a layout depending
// of the country code (got from local manager)
// The country code will be converted using an ISO 3166 function
// to a int (value between 1101 to 1876)
// This will reserve a range of VBLT/u32l resource from 3101 to 6876

#define kAddrTemplateResIDIncrement					1000

#define kAddrDisplayNameByNameTemplateBaseID		2000
#define kAddrDisplayNameByCompanyTemplateBaseID		3000
#define kAddrFFNTemplateBaseID						4000
#define kAddrAddressTemplateBaseID					5000

/************************************************************
 *
 * ViewRowIndexLayoutType
 *
 * Contains a list of ViewRowLayoutType resource IDs
 * Each ViewRowLayoutType resource represents a layout line
 *
 ************************************************************/
typedef	struct ViewRowIdLayoutTag
{
	Boolean				doubleByteEncoding;
	uint8_t				padding;
	uint16_t			numRows;
	uint16_t			rowLayoutIDs[kNumMaxRowByLayout];	  // resource IDs of VRLT resources or special case IDs -1, -2, 0.
} ViewRowIdLayoutType;

/************************************************************
 *
 * ViewBlockLayoutType
 *
 * This resource contains a list of layouts represented
 * by a table of ViewRowIndexLayoutType
 *
 ************************************************************/
typedef	struct ViewBlockLayoutTag
{
	uint16_t			numLayouts;	// The number of layouts that could	be applied
	uint8_t				padding[2];
	ViewRowIdLayoutType	rowIndexLayouts[1]; // An ID of 0xFFFF indicates a blank line
} ViewBlockLayoutType;

/************************************************************
 *
 * ViewFieldLayoutType
 *
 * This is the base element of a layout.
 * - See AddressFields.h for the list of possible fieldType
 * - separatorStrId is the tSTR resource ID used as separator
 *   with the previous field
 * - preffix/SuffixStrID are string resource IDs used for the
 *   field prefix and suffix.
 * - if mandatory is true and the field is missing in the
 *   record, then the next layout must be used.
 *
 ************************************************************/
typedef	struct ViewFieldLayoutTag
{
	uint32_t	fieldType;			// Field type as defined by the AddressFields enum.
	uint16_t	separatorStrId;		// Id of a tSTR resource used as separator with the previous field.
	uint16_t	prefixStrId;		// Id of a tSTR	resource that needs	to be used as a	prefix.
	uint16_t	suffixStrId;		// Id of a tSTR	resource that needs	to be used as a	suffix.
	uint8_t		maxWidthPercent;	// Maximun width in percent
	Boolean		mandatory;			// If a mandatory fields is missing, the layout engine switch to the next layout
} ViewFieldLayoutType;

/************************************************************
 *
 * ViewRowLayoutType
 *
 * It represents a layout row and contains a list a
 * ViewFieldLayoutType structures
 *
 ************************************************************/
typedef	struct ViewRowLayoutTag
{
	uint16_t			numFields; // blank line if numElem == 0xFFFF (-1)
	uint8_t				padding[2];
	ViewFieldLayoutType	fields[kNumMaxFieldsByRow];
} ViewRowLayoutType;

/**************************************
 *
 * ViewDisplayLayoutType
 *
 * This the data structure used at the application level.
 * It contains a list of layouts represented by the
 * ViewRowLayoutType structures table.
 *
 **************************************/
typedef struct ViewDisplayLayoutTag
{
	Boolean				doubleByteEncoding;
	uint8_t				padding;
	uint16_t			numRows;
	ViewRowLayoutType	rows[kNumMaxRowByLayout];
} ViewDisplayLayoutType;

/**************************************
 *
 * ViewDisplayLayoutInfoType
 *
 * Used by the display layout engine
 *
 **************************************/
typedef struct ViewDisplayLayoutInfoTag
{
	DmOpenRef				dbP;
	uint32_t				rowID; // row ID or cursor ID positioned on a valid row
	uint32_t *				columnIDsP;
	uint32_t				numColumns;
	DbSchemaColumnValueType*columnValuesP;
	uint16_t				layoutRscID;
	FontID					fontID;
	uint8_t					padding;
	ViewDisplayLayoutType	layout;
} ViewDisplayLayoutInfoType;

/**************************************
 *
 * ViewDiplayLayoutRowTextType
 *
 * Used to get an entire line of text
 * from a layout template
 *
 **************************************/
typedef struct ViewDiplayLayoutRowTextTag
{
	char **		fieldTextPP;
	AddressTabColumnPropertiesType **	columnPropertiesPP;
	uint8_t *	fieldMaxWidthPercentP;
	uint8_t *	fieldClassP;
	uint16_t	numFields;
	size_t		numBytes;
	Coord		textWidth;
	uint16_t	padding;
} ViewDiplayLayoutRowTextType;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean		AddrDisplayLayoutsInit(ViewDisplayLayoutInfoType *layoutInfoP);
void		AddrDisplayLayoutsFree(ViewDisplayLayoutInfoType *layoutInfoP);

Boolean		AddrDisplayLayoutsGetRowText(ViewDisplayLayoutInfoType *layoutInfoP, uint16_t rowIndex, ViewDiplayLayoutRowTextType *rowTextP);
void		AddrDisplayLayoutsFreeRowText(ViewDiplayLayoutRowTextType *rowTextP);

Boolean		AddrDisplayLayoutsGetDefaultText(ViewDisplayLayoutInfoType *layoutInfoP, ViewDiplayLayoutRowTextType *rowTextP);

#ifdef __cplusplus
}
#endif

#endif /* _ADDRESSDISPLAYLAYOUTS_H_ */
