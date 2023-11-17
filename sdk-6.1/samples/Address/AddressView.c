/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressView.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *   This is the Address Book application's view form module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#ifdef PIM_APPS_PROFILING
#include <PIMAppsProfiling.h>
#else
#define PIMAppProfilingBegin(name)
#define PIMAppProfilingEnd()
#define PIMAppProfilingReturnVoid()	return
#define PIMAppProfilingReturnValue(val)	(return (val))
#endif

#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include <AboutBox.h>
#include <CatMgr.h>
#include <DateTime.h>
#include <Menu.h>
#include <UIResources.h>
#include <PenInputMgr.h>
#include <TraceMgr.h>
#include <TextMgr.h>
#include <Loader.h>
#include <string.h>
#include <FormLayout.h>
#include <PalmTypesCompatibility.h>
#include <TimeMgr.h>
#include <Find.h>
#include <GcRender.h>
#include <SysUtils.h>

// posix includes
#include <stdio.h>

#include "AddressView.h"
#include "AddressDialList.h"
#include "AddressEdit.h"
#include "AddressDetails.h"
#include "AddressNote.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressRsc.h"
//#include "AddressDefines.h"
#include "AddressTransfer.h"
#include "AddressTab.h"
#include "AddressDBSchema.h"
#include "AddressFreeFormName.h"
#include "AddressExportAsk.h"
#include "AddressDisplayLayouts.h"
#include "AddressU32List.h"

/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/

// Info on how to draw the record view
typedef struct DisplayLineTag
{
	uint32_t	columnID;
	uint32_t	fieldInfo;
	char *		labelP;
	char *		textP;
	Coord		textWidth;
	Coord		yOffset;
	Coord		lineHeight;
	Boolean		dynamicAlloc;
	Boolean		isCheckbox;
	FontID		labelFontId;
	FontID		dataFontId;
	size_t		offset;
	size_t		length;
	Coord		labelWidth;
	Coord		x;
	Boolean		lineFeed;
	uint8_t		fieldClass;
} DisplayLineType;


/***********************************************************************
 *
 *   Define
 *
 ***********************************************************************/

#define kAppendNone					0
#define kAppendWithoutSpace			1
#define kAppendWithSpace			2

#define kTabDefaultNumLines			100
#define kTabNumLinesIncrement		kTabDefaultNumLines

#define kInvalidGroupIndex			((uint16_t) 0xFFFF)
#define kInvalidLineIndex			((uint16_t) 0xFFFF)

#define kTabNameUnderliningOffset	2

/***********************************************************************
 *
 *   Static variables
 *
 ***********************************************************************/

static MemHandle				sTabItemsH = NULL;
static DisplayLineType*			sTabItemsP = NULL;
static uint16_t					sTabNumItems;

static uint16_t					sCurrentTabNumLines;
static uint16_t					sCurrentTabTopLineIndex;

static Coord					sDisplayWidth;

static Boolean					sGotoRecord = false;
static Boolean					sGotoRecordColIDInName = false;
static uint32_t					sGotoRecordColID;
static uint32_t					sGotoRecordMatchPos;
static uint32_t					sGotoRecordMatchLen;

static BookType*				sBookP = NULL; // Book data type - BooksLib
static MemHandle				sTabNameAllH = NULL;
static char*					sTabNameAllStr = NULL;

static MemHandle				sFullnameH = NULL;
static Coord					sFullnameWidth = 0;
static FontID					sFullnameFont = largeBoldFont;

static Boolean					sNamePopupDisplayed = false;
static uint64_t					sNamePopupDisplayTime = 0;

static uint32_t					sRecordNumColumns = 0;
static DbSchemaColumnValueType*	sRecordColumnValuesP = NULL;
static RectangleType			sNamePopupBounds;

static Boolean					sPhoneSelection;
static Boolean					sPhoneSelected;
static RectangleType			sSelectedPhoneBounds;
static uint16_t					sSelectedPhoneNumLines;
static uint16_t					sSelectedPhoneLineIndex;

static RectangleType			sGadgetWhiteArea = { 0 };

static Boolean					sOnehandedBookBodyItemSelected = false;
static uint16_t					sOnehandedSelectedLineIndex = kInvalidLineIndex;

// This flag is used when going from RecordView to EditView. There is no need to re-open
// the cursor at RecordView closing and close it immediatly after at EditView opening.
static Boolean					sOpenCursorOnClose = true;

/***********************************************************************
 *
 * FUNCTION:    PrvViewDisplayNamePopup
 *
 * DESCRIPTION: Check if the name doesn't fit screen width and display
 *				the FFN popup if needed.
 *
 * PARAMETERS:	->	formP
 *
 * RETURNED:	true if the popup is needed, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		07/29/04	Initial revision
 *
 ***********************************************************************/
static Boolean PrvViewDisplayNamePopup(FormType *formP)
{
	RectangleType	bounds;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewName), &sNamePopupBounds);

	if (sFullnameWidth > sNamePopupBounds.extent.x)
	{
		sNamePopupBounds.extent.y = bounds.topLeft.y + bounds.extent.y - sNamePopupBounds.topLeft.y;

		sNamePopupDisplayed = true;
		sNamePopupDisplayTime = TimGetTicks();
		gEvtGetEventTimeout = kNamePopupEventTimeOut;

		ToolsFrmInvalidateRect(RecordView, &sNamePopupBounds);

		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewDiscardNamePopup
 *
 * DESCRIPTION: Discard the FFN popup
 *
 * PARAMETERS:	->	update: if true, invalidate the window
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		07/01/03	Initial revision
 *
 ***********************************************************************/
static void PrvViewDiscardNamePopup(Boolean update)
{
	sNamePopupDisplayed = false;
	gEvtGetEventTimeout = evtWaitForever;

	if (update)
		ToolsFrmInvalidateWindow(RecordView);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewReleaseRecordDataFromDB
 *
 * DESCRIPTION: Release the record from memory to allow record update
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static void PrvViewReleaseRecordDataFromDB(void)
{
	status_t	error;

	if (sRecordColumnValuesP)
	{
		if ((error = DbReleaseStorage(gAddrDB, sRecordColumnValuesP)) < errNone)
			ErrNonFatalDisplay("Can't release record");

		sRecordColumnValuesP = NULL;
	}

	sRecordNumColumns = 0;
}


/***********************************************************************
 *
 * FUNCTION:		PrvViewFreeResources
 *
 * DESCRIPTION:		Freed all dynamically allocated resources
 *
 * PARAMETERS:		None
 *
 * RETURNED:		Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		07/17/02	Initial revision
 *
 ***********************************************************************/
static void PrvViewFreeResources(Boolean freeTabItems)
{
	int16_t	i;

	for (i = 0; i < sCurrentTabNumLines; i++)
	{
		if (sTabItemsP[i].dynamicAlloc && sTabItemsP[i].textP &&sTabItemsP[i].offset == 0)
		{
			MemPtrFree(sTabItemsP[i].textP);
			sTabItemsP[i].textP = NULL;
		}
	}

	if (freeTabItems)
	{
		MemHandleUnlock(sTabItemsH);
		MemHandleFree(sTabItemsH);
		sTabItemsH = NULL;
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewClose
 *
 * DESCRIPTION: This routine is called when frmCloseEvent is received
 *
 * PARAMETERS:  None
 *
 * RETURNED:    None
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * FPa		11/28/00	Initial Revision
 * FPa		12/05/00	Added MemHandleUnlock
 *
 ***********************************************************************/
void PrvViewClose(void)
{
	uint32_t	size;
	uint16_t	tabID;
	status_t	err;

	PrvViewFreeResources(true);

	if (sTabNameAllH)
	{
		DmHandleUnlock(sTabNameAllH);
		DmReleaseResource(sTabNameAllH);
		sTabNameAllH = NULL;
		sTabNameAllStr = NULL;
	}

	PrvViewReleaseRecordDataFromDB();

	// Save the current tabID for this row only if we don't exit on deletion
	if (gCurrentRowID != dbInvalidRowID)
	{
		size = sizeof(tabID);
		tabID = (sBookP ? BookGetActiveTabId(sBookP) : kTabAllId);
		err = DbWriteColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDLastVisibleTabID,	0, -1, (void*)&tabID, size);
		ErrNonFatalDisplayIf(err < errNone, "Can't write column");
	}

	if (sBookP)
	{
		BookFree(sBookP);
		sBookP = NULL;
	}

	AddrFreeFormNameFree(sFullnameH);
	sFullnameH = NULL;

	sGotoRecord = false;

	PrvViewDiscardNamePopup(false);

	FrmSetNavState(FrmGetFormPtr(RecordView), kFrmNavStateFlagsInteractionMode);

	// Re-open the cursor if needed
	if (sOpenCursorOnClose)
		AddressDBOpenCursor();
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewGetColumnValue
 *
 * DESCRIPTION: Gets the column data from the pre-loaded record column values
 *
 * PARAMETERS:  ->	columnID: the columnID to get
 *				<-	dataPP: Receiving buffer
 *				<-	dataLenP: size of get data
 *
 * RETURNED:    true if column found, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		23/05/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvViewGetColumnValue(uint32_t columnID, void **dataPP, uint32_t *dataLenP)
{
	uint32_t	index;
	Boolean		result = false;

	if (dataPP)
		*dataPP = NULL;

	*dataLenP = 0;

	for (index = 0; index < sRecordNumColumns; index++)
	{
		if (sRecordColumnValuesP[index].columnID == columnID)
		{
			if (sRecordColumnValuesP[index].errCode == errNone)
			{
				if (dataPP)
					*dataPP = sRecordColumnValuesP[index].data;

				*dataLenP = sRecordColumnValuesP[index].dataSize;
				result = true;
			}

			break;
		}
	}

	return result;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewJustUsedAnotherLine
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * MJPG		2003/11/22	Initial Revision
 *
 ***********************************************************************/
static void PrvViewJustUsedAnotherLine(void)
{
	sCurrentTabNumLines++;

	// Reallocate if needed
	if (sCurrentTabNumLines >= sTabNumItems)
	{
		status_t	err;

		MemHandleUnlock(sTabItemsH);
		err = MemHandleResize(sTabItemsH, (uint32_t)(sTabNumItems + kTabNumLinesIncrement) * sizeof(DisplayLineType));
		ErrFatalDisplayIf(err < errNone, "Out of memory!");

		sTabItemsP = MemHandleLock(sTabItemsH);
		memset(sTabItemsP + sTabNumItems, 0, kTabNumLinesIncrement * sizeof(DisplayLineType));
		sTabNumItems += kTabNumLinesIncrement;
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewAddBlankLine
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		08/02/02	Initial Revision
 *
 ***********************************************************************/
static void PrvViewAddBlankLine(void)
{
	// Don't add multiple blank lines and/or at first line
	if (!sCurrentTabNumLines || sTabItemsP[sCurrentTabNumLines - 1].fieldClass == kFieldClass_BlankLine)
		return;

	memset(&sTabItemsP[sCurrentTabNumLines], 0, sizeof(DisplayLineType));

	sTabItemsP[sCurrentTabNumLines].columnID = kInvalidColumnID;
	sTabItemsP[sCurrentTabNumLines].fieldClass = kFieldClass_BlankLine;
	sTabItemsP[sCurrentTabNumLines].lineFeed = true;
	FntSetFont(gAddrRecordFont);
	sTabItemsP[sCurrentTabNumLines].lineHeight = FntLineHeight() >> 1;
	sTabItemsP[sCurrentTabNumLines].yOffset = sTabItemsP[sCurrentTabNumLines - 1].yOffset + sTabItemsP[sCurrentTabNumLines - 1].lineHeight;

	PrvViewJustUsedAnotherLine();
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewAddTextField
 *
 * DESCRIPTION: Adds a field to the sTabsDisplayedLinesPP info.
 *
 * PARAMETERS:	->	colPropP - the column properties structure
 *				->	textP - the text to be added
 *				->	appendMethod -	kAppendNone to add a new line
 *									kAppendWithoutSpace	to append without space after the previous entry
 *									kAppendWithSpace to append with a space
 *				->	recordLabelColumnWidth - Width of labels in this tab
 *				->	labelFontID, dataFontID - fontID for labels and data
 *
 * RETURNED:    width is set to the width of the last line added
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * rsf		06/21/95	Created by Roger Flores
 * kwk		10/25/99	Fix sign extension w/calling TxtCharIsSpace
 * TEs		07/20/02	Rewrote for tabs support
 * PPL		11/27/02	rename the function into PrvVIewAddTextField
 * PPL 		01/08/02	Add oneChar Parameter to support drawing the
 *						check box Symbol 11 char which code ASCII is 0 !!!
 *
 ***********************************************************************/
static void PrvViewAddTextField(AddressTabColumnPropertiesType *columnPropertiesP, char *textP, Boolean dynamicAlloc, uint8_t appendMethod, FontID fontId, Coord maxLabelWidth, uint8_t fieldClass)
{
	Coord				textPosX;
	size_t				offset;
	size_t				length;
	uint16_t			lastLineIndex;
	FontID				currentFontId = 0;
	Coord				recordLabelColumnWidth;
	Boolean				textOnNextLine;
	char *				labelP = NULL;
	Coord				halfDisplayWidth = sDisplayWidth >> 1;
	wchar32_t			prevChar = 0;
	size_t				charSize;
	DisplayLineType*	lineP;
	Coord				newTextPosX;
	Coord				lineHeight;

	if (textP == NULL)
		return;

	currentFontId = FntSetFont(fontId);
	lineHeight = FntLineHeight();

	textPosX = recordLabelColumnWidth = 0;

	// Get column label
	if (maxLabelWidth && columnPropertiesP && !columnPropertiesP->noLabel)
	{
		// Check for renamable label
		if (columnPropertiesP->renameAllowed && columnPropertiesP->renamedLabelP && *columnPropertiesP->renamedLabelP)
			labelP = columnPropertiesP->renamedLabelP;
		else
			labelP = columnPropertiesP->labelP;

		textPosX = recordLabelColumnWidth = maxLabelWidth;

		// Handle special case for labels wider than a half display width
		if (textPosX > halfDisplayWidth)
		{
			// If current label width is less than a half display, the text will start at half display width
			// Otherwise it will be displayed right after the label
			if (FntCharsWidth(labelP, strlen(labelP)) <= halfDisplayWidth)
				textPosX = halfDisplayWidth;
			else
				textPosX = sDisplayWidth; // The text will start on the next line.

			recordLabelColumnWidth = halfDisplayWidth;
		}
	}

	// The new text field must be append to previous one.
	if (appendMethod != kAppendNone && sCurrentTabNumLines > 0)
	{
        // Modify the previous item to handle the new one one its same line
		lineP = &sTabItemsP[sCurrentTabNumLines - 1];
		newTextPosX = lineP->x;
		newTextPosX += FntCharsWidth(lineP->textP + lineP->offset, lineP->length);

		if (appendMethod == kAppendWithSpace)
			newTextPosX += FntCharWidth(chrSpace);

		if (newTextPosX < sDisplayWidth)
		{
			textPosX = newTextPosX;
			lineP->lineFeed = false;
		}
	}

	offset = 0;

	while(true)
	{
		textOnNextLine = false;
		lastLineIndex = sCurrentTabNumLines;

		if (columnPropertiesP)
		{
			sTabItemsP[lastLineIndex].columnID		= columnPropertiesP->columnId;
			sTabItemsP[lastLineIndex].fieldInfo		= columnPropertiesP->fieldInfo;
			sTabItemsP[lastLineIndex].isCheckbox	= (columnPropertiesP->columnDataType == dbBoolean);
		}
		else
		{
			sTabItemsP[lastLineIndex].columnID		= kInvalidColumnID;
			sTabItemsP[lastLineIndex].fieldInfo		= 0;
			sTabItemsP[lastLineIndex].isCheckbox	= false;
		}

		sTabItemsP[lastLineIndex].offset		= offset;
		sTabItemsP[lastLineIndex].dynamicAlloc	= dynamicAlloc;
		sTabItemsP[lastLineIndex].labelP		= labelP;
		sTabItemsP[lastLineIndex].labelFontId	= fontId;
		sTabItemsP[lastLineIndex].dataFontId	= fontId;
		sTabItemsP[lastLineIndex].lineFeed		= true;
		sTabItemsP[lastLineIndex].labelWidth	= recordLabelColumnWidth;
		sTabItemsP[lastLineIndex].fieldClass	= fieldClass;
		sTabItemsP[lastLineIndex].lineHeight	= lineHeight;
		if (sTabItemsP[lastLineIndex].fieldClass == kFieldClass_TabName)
			sTabItemsP[lastLineIndex].lineHeight += kTabNameUnderliningOffset;

		charSize = 0;

		// In case of a checkbox, specific management.
		if (sTabItemsP[lastLineIndex].isCheckbox)
		{
			sTabItemsP[lastLineIndex].textP = textP;
			length = 0;
		}
		else
		{
			if ((length = FntWordWrap(textP + offset, (uint16_t)(sDisplayWidth - textPosX))) > 0)
				charSize = TxtGetPreviousChar(textP, offset + length, &prevChar);

			// If the last char is a control, remove it from display
			// If it's not, reset charSize as it will be added to the offset at the end of the loop
			if (length && TxtCharIsCntrl(prevChar))
				length -= charSize;
			else charSize = 0;

			if (!length || (*(textP + offset + length) && !TxtCharIsSpace(prevChar)))
			{
				// We are displaying a large label and there is not enough room after it
				// to display the text
				if (labelP && !offset && textPosX > halfDisplayWidth)
				{
					textOnNextLine = true;
					textPosX = 0;
					length = 0;
				}
				// Simply happen to previous line
				else if (appendMethod != kAppendNone && lastLineIndex > 0)
				{
					sTabItemsP[lastLineIndex - 1].lineFeed = true;
					textPosX = recordLabelColumnWidth;
					length = FntWordWrap(textP + offset, sDisplayWidth - textPosX);
				}
			}

			if (textOnNextLine)
				sTabItemsP[lastLineIndex].textP	 = NULL;
			else if (!offset)
				sTabItemsP[lastLineIndex].textP	 = textP;
			else
				sTabItemsP[lastLineIndex].textP	 = sTabItemsP[lastLineIndex - 1].textP;
		}

		sTabItemsP[lastLineIndex].x 		= textPosX;
		sTabItemsP[lastLineIndex].length	= (uint16_t)length;

		if (sTabItemsP[lastLineIndex].textP != NULL)
			sTabItemsP[lastLineIndex].textWidth = FntCharsWidth(sTabItemsP[lastLineIndex].textP + offset, length);

		PrvViewJustUsedAnotherLine();

		if (lastLineIndex > 0)
		{
			sTabItemsP[lastLineIndex].yOffset = sTabItemsP[lastLineIndex - 1].yOffset;
			if (sTabItemsP[lastLineIndex - 1].lineFeed)
				sTabItemsP[lastLineIndex].yOffset += sTabItemsP[lastLineIndex - 1].lineHeight;
		}
		else
		{
			sTabItemsP[lastLineIndex].yOffset = 0;
		}

		if (sTabItemsP[lastLineIndex].isCheckbox)
			break;

		// Entire text in the line
		if (!textOnNextLine && TxtGetChar(textP, length + offset) == chrNull)
			break;

		labelP = NULL;

		offset += length + charSize;
		textPosX = recordLabelColumnWidth;
	}

	FntSetFont(currentFontId);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewAddItem
 *
 * DESCRIPTION:
 *			Add an UI Item depending on column data type.
 *
 * PARAMETERS:
 * 			->	columnPropertiesP - the column properties structure
 *			->	dataP - the data to be added
 *     		->	appendMethod -	kAppendNone to add a new line
 *								kAppendWithoutSpace	to append without space after the previous entry
 *								kAppendWithSpace to append with a space
 *			->	fontId - fontID
 *
 * RETURNED:	Nothing
 *
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		11/27/02	Reimplement PrvViewAddItem as a wrapper to
 *						other fucntions that actually install UI Item depending on
 *						the data type.
 *
 ***********************************************************************/
static void PrvViewAddItem(AddressTabColumnPropertiesType *columnPropertiesP, MemPtr dataP, Boolean dynamicAlloc, uint8_t appendMethod, FontID fontId, Coord maxLabelWidth)
{
	char *			textP = NULL;
	DateTimeType* 	dateTimeP = NULL;
	DateType*		dateP = NULL;
	TimeType*		timeP = NULL;
	DateFormatType 	dateFormat;
	TimeFormatType 	timeFormat;
	Boolean			dynamic = false;
	char*			resultP;

	if (!dataP)
		return;

	if (columnPropertiesP->columnDataType == dbVarChar)
	{
		textP = (char*)dataP;
		dynamic = dynamicAlloc;
	}
	else
	{
		textP = MemPtrNew(kStringMaxSize);
		ErrFatalDisplayIf(!textP, "Out of memory!");
		dynamic = true;

		switch (columnPropertiesP->columnDataType)
		{
			case dbUInt8:	sprintf(textP, "%u", *((uint8_t*) dataP));		break;
			case dbUInt16:	sprintf(textP, "%u", *((uint16_t*) dataP));		break;
			case dbUInt32:	sprintf(textP, "%lu", *((uint32_t*) dataP));	break;
			case dbUInt64:	sprintf(textP, "%llu", *((uint64_t*) dataP));	break;
			case dbInt8:	sprintf(textP, "%d", *((int8_t*) dataP));		break;
			case dbInt16:	sprintf(textP, "%d", *((int16_t*) dataP));		break;
			case dbInt32:	sprintf(textP, "%ld", *((int32_t*) dataP));		break;
			case dbInt64:	sprintf(textP, "%lld", *((int64_t*) dataP));	break;
			case dbFloat:	sprintf(textP, "%f", *((float*) dataP));		break;
			case dbDouble:	sprintf(textP, "%f", *((float*) dataP));		break;
			case dbChar:	sprintf(textP, "%c", (char*) dataP);			break;

			case dbBoolean:	sprintf(textP, "%c", *((Boolean*) dataP) ? symbolCheckboxOn : symbolCheckboxOff);	break;


			case dbDateTime:
				dateTimeP = (DateTimeType*) dataP;

				if ((resultP = ToolsBuildDateAndTimeString(dateTimeP)) != NULL)
				{
					strcpy(textP, resultP);
					MemPtrFree(resultP);
				}
				break;


			case dbDate:
				dateP = (DateType*) dataP;
				dateFormat = (DateFormatType) PrefGetPreference(prefLongDateFormat);
				DateToAscii( (uint8_t) dateP->month, (uint8_t) dateP->day, (uint16_t) (dateP->year + firstYear),  dateFormat, textP);
				break;


			case dbTime:
				timeP = (TimeType*) dataP;
				timeFormat = (TimeFormatType) PrefGetPreference(prefTimeFormat);
				TimeToAscii((uint8_t)timeP->hours, (uint8_t)timeP->minutes,  timeFormat, textP);
				break;

			default:
				MemPtrFree(textP);
				return;
		}
	}

	PrvViewAddTextField(columnPropertiesP, textP, dynamic, appendMethod, fontId, maxLabelWidth, kFieldClass_Standard);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewLoadAdressFromTemplate
 *
 * DESCRIPTION: Load an address in the internal struture used
 *				for display by using a template loaded from resources
 *
 * PARAMETERS:  ->	numColumns - the number of columns in this list
 *				->	colPropP - List of column properties of the address fields
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		03/24/03	Initial Revision
 *
 ***********************************************************************/
static void PrvViewLoadAdressFromTemplate(uint32_t numColumns, uint32_t *columnIDs)
{
	uint8_t						appendMethod;
	uint16_t					rowIndex;
	uint16_t					fieldIndex;
	char *						textP;
	ViewDisplayLayoutInfoType	layoutInfo;
	ViewDiplayLayoutRowTextType	rowText;

	if (!gAddressLayoutResID)
		gAddressLayoutResID = ToolsGetCountryBaseResID(kViewBlockLayoutRscType, kAddrAddressTemplateBaseID, kAddrTemplateResIDIncrement);

	layoutInfo.dbP			= gAddrDB;
	layoutInfo.rowID		= gCurrentRowID;
	layoutInfo.columnIDsP	= columnIDs;
	layoutInfo.numColumns	= numColumns;
	layoutInfo.layoutRscID	= gAddressLayoutResID;

	// Parse all layouts to find a suitable one. Layouts are rejected only if
	// they contain mandatory fields which are missing in the current record.
	if (AddrDisplayLayoutsInit(&layoutInfo))
	{
		// Now we have a valid layout. Display the passed address fields using it.
		for (rowIndex = 0; rowIndex < layoutInfo.layout.numRows; rowIndex++)
		{
			if (!AddrDisplayLayoutsGetRowText(&layoutInfo, rowIndex, &rowText))
				break;

			if (rowText.numFields == kLayoutBlankLine)
			{
				PrvViewAddBlankLine();
				continue;
			}

			appendMethod = kAppendNone;

			for (fieldIndex = 0; fieldIndex < rowText.numFields; fieldIndex++)
			{
				textP = ToolsStrDup(rowText.fieldTextPP[fieldIndex]);
				PrvViewAddTextField(rowText.columnPropertiesPP[fieldIndex], textP, true, appendMethod, gAddrRecordFont, 0, rowText.fieldClassP[fieldIndex]);
				appendMethod = kAppendWithoutSpace;
			}

			// Freed memory
			AddrDisplayLayoutsFreeRowText(&rowText);
		}

		// Add a blank line right after the address
		PrvViewAddBlankLine();
	}
	// Can't find a suitable layout. This can't happened but who knows...
	// In that case we add all fields in the view.
	else if (AddrDisplayLayoutsGetDefaultText(&layoutInfo, &rowText))
	{
		appendMethod = kAppendNone;

		for (fieldIndex = 0; fieldIndex < rowText.numFields; fieldIndex++)
		{
			textP = ToolsStrDup(rowText.fieldTextPP[fieldIndex]);
			PrvViewAddTextField(rowText.columnPropertiesPP[fieldIndex], textP, true, appendMethod, gAddrRecordFont, 0, rowText.fieldClassP[fieldIndex]);
			appendMethod = kAppendWithSpace;
		}

		// Add a blank line right after the address
		PrvViewAddBlankLine();

		// Freed memory
		AddrDisplayLayoutsFreeRowText(&rowText);
	}

	AddrDisplayLayoutsFree(&layoutInfo);
}

/***********************************************************************
 *
 * FUNCTION:    PrvGetCurrentTabDisplayInfo
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/14/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvGetCurrentTabDisplayInfo(uint16_t topItemIndex, uint16_t displayHeight, uint16_t *lastItemIndexP, uint16_t *heightP)
{
	uint16_t	lastItemIndex;
	uint16_t	height;

	lastItemIndex = topItemIndex;
	height = 0;

	while (lastItemIndex < sCurrentTabNumLines)
	{
		height += sTabItemsP[lastItemIndex].lineHeight;

		// Break if we filled the whole page
		if (height >= displayHeight)
		{
			if (height > displayHeight)
			{
				lastItemIndex--;

				while (lastItemIndex > 0 && !sTabItemsP[lastItemIndex - 1].lineFeed)
					lastItemIndex--;
			}
			break;
		}

		while (!sTabItemsP[lastItemIndex].lineFeed)
			lastItemIndex++;

		lastItemIndex++;
	}

	if (lastItemIndex >= sCurrentTabNumLines)
		lastItemIndex = sCurrentTabNumLines - 1;

	if (lastItemIndexP)
		*lastItemIndexP = lastItemIndex;

	if (heightP)
		*heightP = height;

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewUpdateScrollers
 *
 * DESCRIPTION: Draw the scroll buttons
 *
 * PARAMETERS:  ->	formP: pointer to the form
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		06/03/03	Initial revision
 *
 ***********************************************************************/
static void PrvViewUpdateScrollers(FormType *formP)
{
	uint16_t		upIndex;
	uint16_t		downIndex;
	Boolean			scrollableUp;
	Boolean			scrollableDown;
	uint16_t		lastLineIndex = 0;
	RectangleType	bounds;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
	PrvGetCurrentTabDisplayInfo(sCurrentTabTopLineIndex, bounds.extent.y, &lastLineIndex, NULL);

	// Now show/hide the scroll arrows
	scrollableUp = (Boolean) (sCurrentTabTopLineIndex != 0);
	scrollableDown = (Boolean) (lastLineIndex < sCurrentTabNumLines - 1);

	// Update the scroll button.
	upIndex = FrmGetObjectIndex(formP, RecordUpButton);
	downIndex = FrmGetObjectIndex(formP, RecordDownButton);
	FrmUpdateScrollers(formP, upIndex, downIndex, scrollableUp, scrollableDown);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewLoadTab
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		11/19/02	Initial Revision
 *
 ***********************************************************************/
static void PrvViewLoadTab(uint16_t tabId)
{
	uint32_t		displayIndex;
	MemPtr			dataP = NULL;
	uint32_t		dataLen = 0;
	uint32_t		textLen;
	uint32_t		columnID;
	uint32_t		tabIndex;
	uint32_t		startTabIndex;
	uint32_t		endTabIndex;
	char *			tabNameP = NULL;
	AddressTabColumnPropertiesType	*columnPropertiesP;
	Boolean			addBlankLine;
	Coord			labelWidth;
	FontID			currFont;
	FontID			tabNameFontID = boldFont;
	uint8_t *		addressGroupsListP = NULL;

	TraceOutput(TL(appErrorClass, "PrvViewLoadTab()"));

	PIMAppProfilingBegin("LoadTab");

	// Here we construct the recordViewLines info by laying out the record
	tabIndex = BookFindTabIndex(sBookP, tabId);

	if (tabId == kTabAllId)
	{
		if (gAddrRecordFont != stdFont)
			tabNameFontID = largeBoldFont;

		startTabIndex = 0;
		endTabIndex = gBookInfo.numTabs;
	}
	else
	{
		startTabIndex = tabIndex - 1; // We start one tab before because of the 'All' tab which is placed first
		endTabIndex = tabIndex;
	}

	if (sTabItemsH == NULL)
	{
		sTabNumItems = kTabDefaultNumLines;
		sTabItemsH = MemHandleNew(sizeof(DisplayLineType) * (uint32_t)sTabNumItems);
		ErrFatalDisplayIf(!sTabItemsH, "Out of memory!");

		sTabItemsP = MemHandleLock(sTabItemsH);
	}

	memset(sTabItemsP, 0, sizeof(DisplayLineType) * sTabNumItems);
	sCurrentTabNumLines = 0;
	sCurrentTabTopLineIndex = 0;

	currFont = FntSetFont(gAddrRecordFont);

	// Build the list of address groups
	if (gBookInfo.numAddressGroups)
	{
		size_t		size;

		size = sizeof(uint8_t) * gBookInfo.numAddressGroups;
		addressGroupsListP = MemPtrNew(size);
		ErrFatalDisplayIf(!addressGroupsListP, "Out of memory!");
		memset(addressGroupsListP, 0, size);
	}

	for (tabIndex = startTabIndex; tabIndex < endTabIndex; tabIndex++)
	{
		if (!TabIsVisible(gBookInfo.tabs[tabIndex].tabInfo.tabId))
			continue;

		labelWidth = ToolsGetLabelColumnWidth(gCurrentRowID, &gBookInfo, tabIndex, gAddrRecordFont, true);

		// Add a blank line between all tabs in the 'All' tab
		if (tabId == kTabAllId && sCurrentTabNumLines > 0)
			PrvViewAddBlankLine();

		if (tabId == kTabAllId)
		{
			// Get the tab name. It will be displayed only if
			// there is something to display in this tab
			tabNameP = gBookInfo.tabs[tabIndex].tabInfo.tabNameP;
		}

		displayIndex = 0;

		while (displayIndex < gBookInfo.tabs[tabIndex].numElement)
		{
			// columns properties are sorted by displayIndex, so it's ok to access them directly
			columnPropertiesP = &gBookInfo.tabs[tabIndex].columnsPropertiesP[displayIndex];
			addBlankLine = columnPropertiesP->blankLineAfter;

			// Don't display yomi fields and name fields but nickname
			if (columnPropertiesP == NULL || columnPropertiesP->hiddenOnRecordView)
			{
				displayIndex++;
				continue;
			}

			// We start displaying an address. Pass to the template handling function
			if ((columnPropertiesP->fieldInfo & kFieldTypeMask) == kFieldType_Address)
			{
				uint16_t	i;
				uint16_t	nextFreeSlot = kInvalidGroupIndex;
				uint32_t *	colsListP = NULL;
				uint16_t	numColumns = 0;
				uint8_t		groupID;

				groupID = columnPropertiesP->addressGroupID;

				// Check if these address fields have already been displayed
				for (i = 0; i < gBookInfo.numAddressGroups; i++)
				{
					// Already displayed.
					if (addressGroupsListP[i] == groupID)
						goto Next;

					// New group. Keep track of it.
					if (addressGroupsListP[i] == 0)
					{
						nextFreeSlot = i;
						break;
					}
				}

				if (nextFreeSlot == kInvalidGroupIndex || !AddressTabGetAddressGroupColumns(&gBookInfo, groupID, &numColumns, &colsListP))
				{
					ErrNonFatalDisplay("Can't get columns list for current groupID");
					goto Next;
				}

				if (AddressDBColumnsContainData(gCurrentRowID, numColumns, colsListP))
				{
					if (tabNameP)
					{
						// For the 'All' tab, display the tab name.
						// It's only displayed if there is something to display
						// for this tab
						PrvViewAddTextField(NULL, tabNameP, false, kAppendNone, tabNameFontID, 0, kFieldClass_TabName);
						tabNameP = NULL;
					}

					PrvViewLoadAdressFromTemplate(numColumns, colsListP);

					addressGroupsListP[nextFreeSlot] = groupID;
				}

Next:
				displayIndex++;
				continue;
			}

			// Non address fields
			columnID = columnPropertiesP->columnId;

			if (PrvViewGetColumnValue(columnID, &dataP, &dataLen) && dataP != NULL)
			{
				if (tabNameP)
				{
					PrvViewAddTextField(NULL, tabNameP, false, kAppendNone, tabNameFontID, 0, kFieldClass_TabName);
					tabNameP = NULL;
				}

				PrvViewAddItem(columnPropertiesP, dataP, false, kAppendNone, gAddrRecordFont, labelWidth);
			}

			if (sCurrentTabNumLines > 0 && addBlankLine)
				PrvViewAddBlankLine();

			displayIndex++;
		}

		// Remove trailling blank lines
		while (sCurrentTabNumLines)
		{
			if (sTabItemsP[sCurrentTabNumLines - 1].fieldClass != kFieldClass_BlankLine)
				break;

			sCurrentTabNumLines--;
		}
	}

	// Add the note at bottom of the All tab
	if (tabId == kTabAllId)
	{
		char *		noteP = NULL;
		uint32_t	noteLen = 0;

		// Get the note column
		PrvViewGetColumnValue(kAddrColumnIDNote, (void**)&noteP, &noteLen);

		if (noteP && *noteP != 0)
		{
			if (sCurrentTabNumLines > 0)
				PrvViewAddBlankLine();

			// Get note column name
			dataP = NULL;
			DbGetColumnPropertyValue(gAddrDB, kAddressDefaultTableName, kAddrColumnIDNote, kAddrColumnPropertyName,	&textLen, &dataP);

			if (dataP)
			{
				PrvViewAddTextField(NULL, ToolsStrDup(dataP), true, kAppendNone, tabNameFontID, 0, kFieldClass_TabName);
				DbReleaseStorage(gAddrDB, dataP);
			}

			columnPropertiesP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, kAddrColumnIDNote);
			PrvViewAddTextField(columnPropertiesP, noteP, false, kAppendNone, gAddrRecordFont, 0, kFieldClass_Standard);
		}
	}

	PrvViewUpdateScrollers(FrmGetFormPtr(RecordView));

	FntSetFont(currFont);

	if (addressGroupsListP)
		MemPtrFree(addressGroupsListP);

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewLoadRecordDataFromDB
 *
 * DESCRIPTION: This routine loads columns values from DB
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static Boolean PrvViewLoadRecordDataFromDB(void)
{
	status_t	error;

	sRecordNumColumns = 0;
	sRecordColumnValuesP = NULL;

	if ((error = DbGetAllColumnValues(gAddrDB, gCurrentRowID, &sRecordNumColumns, &sRecordColumnValuesP)) < errNone)
	{
		ErrNonFatalDisplay("Can't load record");
		return false;
	}

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewErase
 *
 * DESCRIPTION: Erases the record view
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	6/30/95		Initial Revision
 *
 ***********************************************************************/
void PrvViewErase(FormType* formP)
{
	RectangleType r;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &r);
	WinEraseRectangle(&r, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewBookActivation
 *
 * DESCRIPTION: Book activation callback
 *
 * PARAMETERS:
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static void PrvViewBookActivation(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	gBookInfo.currentTabId = tabId;
	gBookInfo.currentTabIndex = tabIndex;

	// Onehanded stuff reset after tab switching
	sOnehandedBookBodyItemSelected = false;
	sOnehandedSelectedLineIndex = kInvalidLineIndex;

	PrvViewLoadTab(tabId);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewBookDeactivation
 *
 * DESCRIPTION: Book deactivation callback
 *
 * PARAMETERS:
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static Boolean PrvViewBookDeactivation(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	PrvViewFreeResources(false);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvDrawGradientRect
 *
 * DESCRIPTION: Draw a gradient line (rect) as used for heading underline
 *
 * PARAMETERS:  colorP - color to draw in
 *              bounds - rectangel to draw in
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvDrawGradientRect(RGBColorType *colorP, RectanglePtr bounds)
{
		GcHandle		gc;
		GcGradientType	grad;
		GcPointType		pnt[2];
		GcColorType		col[2];

		gc = GcGetCurrentContext();

		pnt[0].x = (fcoord_t)(bounds->topLeft.x);
		pnt[0].y = (fcoord_t)(bounds->topLeft.y);
		pnt[1].x = (fcoord_t)(bounds->topLeft.x + bounds->extent.x);
		pnt[1].y = (fcoord_t)(bounds->topLeft.y);

		col[0].red = col[1].red = colorP->r;
		col[0].green = col[1].green = colorP->g;
		col[0].blue = col[1].blue = colorP->b;
		col[0].alpha = 255;
		col[1].alpha = 0;

		GcInitGradient(&grad, pnt, col, 2);
		GcSetGradient(gc, &grad);

		GcRect(gc, pnt[0].x, pnt[0].y, pnt[1].x, pnt[1].y + bounds->extent.y);
		GcPaint(gc);

		GcReleaseContext(gc);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawTab
 *
 * DESCRIPTION: Draw the current active tab content
 *
 * PARAMETERS:  ->	formP: the current form pointer
 *
 * RETURNED:    The index of the last drawn line
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		11/20/02	Initial Revision
 * PPL		01/08/02 	Add support to draw a One char data
 *						Symbol 11 non checked check box has code ASCII 0
 *						so we cannot use WinDrawChars...
 *
 ***********************************************************************/
static void PrvViewDrawTab(FormType *formP)
{
	FontID			currentFontId;
	int16_t			plainLineHeight;
	int16_t			labelFontLineHeight = 0;
	int16_t			bottomOfRecordViewDisplay;
	RectangleType	tabBounds;
	uint16_t		y;
	uint16_t		lineIndex;
	uint32_t		matchPos = sGotoRecordMatchPos;
	uint32_t		matchLen = sGotoRecordMatchLen;
	uint16_t		maxLabelWidth;
	MemHandle		colonResourceH;
	char*			colonStrP;

	if ((colonResourceH = DmGetResource(gApplicationDbP, strRsc, LabelSuffixColonStr)) != NULL)
		colonStrP = DmHandleLock(colonResourceH);
	else colonStrP = ":";

	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));

	PrvViewErase(formP);

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &tabBounds);
	bottomOfRecordViewDisplay = tabBounds.topLeft.y +  tabBounds.extent.y;

	// Fill the entire record view display with the background color.
	WinEraseRectangle(&tabBounds, 0);

	currentFontId = FntSetFont(gAddrRecordFont);
	plainLineHeight = FntLineHeight();

	y = tabBounds.topLeft.y;

	lineIndex = sCurrentTabTopLineIndex;

	while (lineIndex < sCurrentTabNumLines)
	{
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

		y = tabBounds.topLeft.y + sTabItemsP[lineIndex].yOffset - sTabItemsP[sCurrentTabTopLineIndex].yOffset;

		// If we are past the bottom stop drawing
		if (y + sTabItemsP[lineIndex].lineHeight > bottomOfRecordViewDisplay)
			break;

		ErrNonFatalDisplayIf(y < tabBounds.topLeft.y, "Drawing record out of gadget");

		// Draw the label if needed
		if (sTabItemsP[lineIndex].labelP)
		{
			size_t	textLen;
			size_t	colonSuffixLen;
			Coord	descFitWidth;
			Coord	colonWidth;
			Coord	spaceWidth;
			Coord	labelWidth;
			size_t	descFitLen;
			Boolean fit;

			FntSetFont(sTabItemsP[lineIndex].labelFontId);
			labelFontLineHeight = FntLineHeight();
			textLen = strlen(sTabItemsP[lineIndex].labelP);
			colonSuffixLen = strlen(colonStrP);

			spaceWidth = FntCharWidth(chrSpace);
			labelWidth = sTabItemsP[lineIndex].labelWidth - spaceWidth;
			colonWidth = FntCharsWidth(colonStrP, colonSuffixLen);
			maxLabelWidth = sDisplayWidth - colonWidth;
			descFitWidth = maxLabelWidth;
			descFitLen = textLen;

			// Calculate how many characters will fit in the window bounds
			FntCharsInWidth (sTabItemsP[lineIndex].labelP, &descFitWidth, &descFitLen, &fit);
			WinDrawTruncChars(sTabItemsP[lineIndex].labelP, (int16_t)textLen, labelWidth - descFitWidth - colonWidth, y, maxLabelWidth);
			WinDrawChars(colonStrP, colonSuffixLen, (int16_t) (labelWidth - colonWidth), y);
		}

		FntSetFont(sTabItemsP[lineIndex].dataFontId);

		if (sTabItemsP[lineIndex].fieldClass == kFieldClass_TabName)
		{
			RGBColorType	color;
			RectangleType	bounds;

			color.index = 0;
			color.r = kBackgroundColorRed;
			color.g = kBackgroundColorGreen;
			color.b = kBackgroundColorBlue;
			WinSetTextColorRGB(&color, NULL);

			bounds.topLeft.x = tabBounds.topLeft.x;
			bounds.extent.x = tabBounds.extent.x;
			bounds.topLeft.y = y + sTabItemsP[lineIndex].lineHeight - kTabNameUnderliningOffset;
			bounds.extent.y = 1;

			PrvDrawGradientRect(&color, &bounds);
		}
		else
			WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

		if (sTabItemsP[lineIndex].isCheckbox)
		{
			FontID savedFont = FntSetFont(symbol11Font);

			CtlDrawCheckboxControl((fcoord_t) sTabItemsP[lineIndex].x, (fcoord_t) y, (sTabItemsP[lineIndex].textP[0]));
			FntSetFont(savedFont);
		}

		// We have some text to highlight
		else if (sGotoRecord && sGotoRecordColID == sTabItemsP[lineIndex].columnID)
		{
			uint32_t	textOffset = sTabItemsP[lineIndex].offset;
			uint32_t	textLength = sTabItemsP[lineIndex].length;
			uint32_t	newLength;
			Coord		x = sTabItemsP[lineIndex].x;

			// Draw normal text before selection
			if (matchPos > textOffset)
			{
				newLength = matchPos - textOffset;

				if (newLength > textLength)
					newLength = textLength;

				WinDrawChars(sTabItemsP[lineIndex].textP + textOffset, (int16_t)newLength, x, y);

				x += FntCharsWidth(sTabItemsP[lineIndex].textP + textOffset, (int16_t)newLength);
				textOffset += newLength;
				textLength -= newLength;
			}

			// Draw the selected text inverted
			if (matchPos >= textOffset && matchPos < textOffset + textLength)
			{
				newLength = matchLen > textLength ? textLength : matchLen;

				WinPushDrawState();

				WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
				WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
				WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));

				WinDrawChars(sTabItemsP[lineIndex].textP + textOffset, (int16_t)newLength, x, y);

				WinPopDrawState();

				x += FntCharsWidth(sTabItemsP[lineIndex].textP + textOffset, (int16_t)newLength);
				textOffset += newLength;
				textLength -= newLength;

				// matchPos and matchlen need to be updated if the selection
				// continues on next line
				matchPos += newLength;
				matchLen -= newLength;
			}

			// Draw text normally after the selection
			if (textOffset < (uint32_t)(sTabItemsP[lineIndex].offset + sTabItemsP[lineIndex].length))
				WinDrawChars(sTabItemsP[lineIndex].textP + textOffset, (int16_t)textLength, x, y);
		}

		// No selection to do. Draw normal text
		else if (sTabItemsP[lineIndex].textP)
		{
			WinDrawChars(	sTabItemsP[lineIndex].textP + sTabItemsP[lineIndex].offset,
							(int16_t)sTabItemsP[lineIndex].length,
							sTabItemsP[lineIndex].x, y);
		}

		lineIndex++;
	}

	if (sOnehandedBookBodyItemSelected)
	{
		RectangleType	ringBounds;

		FrmClearFocusHighlight();

		RctSetRectangle(&ringBounds,
						sTabItemsP[sOnehandedSelectedLineIndex].x,
						tabBounds.topLeft.y + (sTabItemsP[sOnehandedSelectedLineIndex].yOffset - sTabItemsP[sCurrentTabTopLineIndex].yOffset),
						sTabItemsP[sOnehandedSelectedLineIndex].textWidth,
						sTabItemsP[sOnehandedSelectedLineIndex].lineHeight);

		FrmSetFocusHighlight(FrmGetWindowHandle(formP), &ringBounds, 0);
	}

	WinPopDrawState();
	FntSetFont(currentFontId);

	if (colonResourceH)
	{
		DmHandleUnlock(colonResourceH);
		DmReleaseResource(colonResourceH);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawTabCallback
 *
 * DESCRIPTION: Callback passed to WinInvalidateRectFunc to draw the tab
 *
 * PARAMETERS:  ->	cmd: winInvalidateDestroy or winInvalidateExecute
 *				->	window: win handle
 *				->	dirtyRect: the clipping region
 *				->	state: user param (unused)
 *
 * RETURNED:    true if draw succesfull, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		06/03/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvViewDrawTabCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	FormType *	formP;

	if (cmd == winInvalidateDestroy)
		return true;

	formP = FrmGetFormPtr(RecordView);
	PrvViewDrawTab(formP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawBusinessCardIndicator
 *
 * DESCRIPTION: Draw the business card indicator if the current record is
 * the business card.
 *
 * PARAMETERS:  formP - the form containing the business card indicator
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	10/22/97	Initial Revision
 *
 ***********************************************************************/
void PrvViewDrawBusinessCardIndicator(FormType *formP)
{
	TraceOutput(TL(appErrorClass, "PrvViewDrawBusinessCardIndicator()"));

	if (gBusinessCardRowID == gCurrentRowID && gBusinessCardRowID != dbInvalidRowID)
		FrmShowObject(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp));
	else
		FrmHideObject(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp));

}

/***********************************************************************
 *
 * FUNCTION:    PrvViewMakeVisible
 *
 * DESCRIPTION: Make a selection range visible
 *
 * PARAMETERS:  selectFieldNum - field to show selected text
 *                selectPos - offset into field for start of selected text
 *                selectLen - length of selected text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	08/03/95	Initial Revision
 *
 ***********************************************************************/
static uint16_t PrvViewMakeVisible(uint16_t lineIndex, Boolean atTop)
{
	uint16_t		height;
	uint16_t		displayHeight;
	FormType *		formP;
	RectangleType	bounds;
	Coord			recordViewFontHeight;
	FontID			savedFont;
	uint16_t		topLineIndex;
	uint16_t		lastItemIndex;

	formP = FrmGetFormPtr(RecordView);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
	displayHeight = bounds.extent.y;

	if (atTop)
	{
		topLineIndex = lineIndex;
	}
	else
	{
		topLineIndex = (lineIndex < sCurrentTabTopLineIndex) ? lineIndex : sCurrentTabTopLineIndex;

		PrvGetCurrentTabDisplayInfo(topLineIndex, displayHeight, &lastItemIndex, NULL);

		if (lineIndex < lastItemIndex)
			return topLineIndex;

		while ((sTabItemsP[lineIndex].yOffset + sTabItemsP[lineIndex].lineHeight - sTabItemsP[topLineIndex].yOffset > displayHeight ||
			sTabItemsP[topLineIndex].columnID == kInvalidColumnID) &&
			topLineIndex < sCurrentTabNumLines - 1)
		{
			topLineIndex++;
		}
	}

	PrvGetCurrentTabDisplayInfo(topLineIndex, displayHeight, NULL, &height);

	savedFont = FntSetFont(gAddrRecordFont);
	recordViewFontHeight = FntLineHeight();

	// If the table is not full and the first visible field is
	// not the first field	in the record, displays enough fields
	// to fill out the table by adding fields to the top of the table.
	while (height < displayHeight)
	{
		if (topLineIndex == 0)
			break;

		topLineIndex--;

		while (topLineIndex > 0 && !sTabItemsP[topLineIndex].lineFeed)
			topLineIndex--;

		height += sTabItemsP[topLineIndex].lineHeight;

		// Break if we filled the whole page
		if (height >= displayHeight)
		{
			if (height > displayHeight)
			{
				while (!sTabItemsP[topLineIndex].lineFeed)
					topLineIndex++;

				topLineIndex++;
			}

			break;
		}
	}

	FntSetFont(savedFont);

	return topLineIndex;
}


/***********************************************************************
 *
 * FUNCTION:    PrvDrawCategoryLabelCallback
 *
 * DESCRIPTION: Callback passed to WinInvalidateRectFunc to draw the category label
 *
 * PARAMETERS:  ->	cmd: winInvalidateDestroy or winInvalidateExecute
 *				->	window: win handle
 *				->	dirtyRect: the clipping region
 *				->	state: user param (unused)
 *
 * RETURNED:    true if draw succesfull, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		06/03/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvDrawCategoryLabelCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *userParamP)
{
	Coord			labelWidth = (Coord)((uint32_t)userParamP);
	FormType *		formP;
	RectangleType	bounds;

	if (cmd == winInvalidateExecute)
	{
		formP = FrmGetFormPtr(RecordView);
		FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordCategoryLabel), &bounds);
		WinEraseRectangle(&bounds, 0);
		bounds.topLeft.x = sDisplayWidth - labelWidth;
		bounds.extent.x = labelWidth;
		FrmSetObjectBounds(formP, FrmGetObjectIndex(formP, RecordCategoryLabel), &bounds);
		FrmCopyLabel(formP, RecordCategoryLabel, gCategoryName);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewSetCategoryLabel
 *
 * DESCRIPTION: Resizes and moves the category label as needed
 *
 * PARAMETERS:  formP - pointer to the view form.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		03/09/2004	Initial revision
 *
 ***********************************************************************/
static void PrvViewSetCategoryLabel(FormType *formP)
{
	CategoryID*		categoriesP;
	uint32_t		numCategories;
	RectangleType	r;
	Coord			labelWidth;
	FontID			currFont;

	if (DbGetCategory(gAddrDB, gCurrentRowID, &numCategories, &categoriesP) < errNone)
		return;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp), &r);
	labelWidth = sDisplayWidth - r.topLeft.x - 1;

	if (gBusinessCardRowID == gCurrentRowID && gBusinessCardRowID != dbInvalidRowID)
		labelWidth -= r.extent.x;

	CatMgrTruncateName(gAddrDB, categoriesP, numCategories, labelWidth, gCategoryName);
	currFont = FntSetFont(stdFont);
	labelWidth = FntCharsWidth(gCategoryName, StrLen(gCategoryName));
	FntSetFont(currFont);

	DbReleaseStorage(gAddrDB, categoriesP);

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordCategoryLabel), &r);

	if ((WinGetWindowFlags(FrmGetWindowHandle(formP)) & winFlagBackBuffer) == winFlagBackBuffer)
	{
		ToolsFrmInvalidateRectFunc(RecordView, &r, PrvDrawCategoryLabelCallback, (void*)labelWidth);
	}
	else
	{
		// ### TEs: this code should be removed once the bug
		// in WinInvalidateRectFunc is fixed in update-based mode.
		FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordCategoryLabel), &r);
		r.topLeft.x = sDisplayWidth - labelWidth;
		r.extent.x = labelWidth;
		FrmSetObjectBounds(formP, FrmGetObjectIndex(formP, RecordCategoryLabel), &r);
		FrmCopyLabel(formP, RecordCategoryLabel, gCategoryName);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewInit
 *
 * DESCRIPTION: This routine initializes the "Record View" of the
 *              Address application.  Most importantly it lays out the
 *                record and decides how the record is drawn.
 *
 * PARAMETERS:  formP - pointer to the view form.
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger  	6/21/95   	Initial Revision
 * aro		09/25/00	Add fieldMapH to release the resource properly
 * FPa		11/27/00	Do not add company field if this field is blank
 * ppl		02/06/02	Add Active Input Graffiti Area
 * TEs		07/17/02	Add books support
 *
 ***********************************************************************/
static void PrvViewInit(void)
{
	uint16_t 		index;
	RectangleType 	r;
	uint32_t		size;
	FormType*		formP;
	uint16_t		gadgetIndex;

	// Close the  cursor so we'll use gCurrentRowID
	if (gCursorID != dbInvalidCursorID)
	{
		DbCursorClose(gCursorID);
		gCursorID = dbInvalidCursorID;
	}

	// Re-open cursor upon form close.
	sOpenCursorOnClose = true;

	if (!AddressDBRecordContainsData(gCurrentRowID))
	{
		AddressDBDeleteRecord(false);
		FrmGotoForm(gApplicationDbP, ListView);
		return;
	}

	formP = FrmGetFormPtr(RecordView);

	if (!sBookP)
	{
		sBookP = BookCreate(formP, RecordBook, RecordBookBody, kTabMinWidth, kTabMaxWidth, gBookInfo.numTabs, kBookStandardLookAndFeel);

		if (sBookP)
		{
			// Place the "All" tab first
			if (!sTabNameAllH)
			{
				if ((sTabNameAllH = DmGetResource(gApplicationDbP, strRsc, TabNameAllStr)) != NULL)
					sTabNameAllStr = (char*)DmHandleLock(sTabNameAllH);
			}

			BookAddTab(sBookP, sTabNameAllStr, NULL, 0, 0, kTabAllId, kBookTabMaxIndex, 0, PrvViewBookActivation, PrvViewBookDeactivation, PrvViewDrawTabCallback);

			// Followed by the other tabs
			for (index = 0; index < gBookInfo.numTabs; index++)
			{
				if (TabIsVisible(gBookInfo.tabs[index].tabInfo.tabId))
				{
					BookAddTab(	sBookP, gBookInfo.tabs[index].tabInfo.tabNameP, NULL, 0, 0,
								gBookInfo.tabs[index].tabInfo.tabId, kBookTabMaxIndex, 0,
								PrvViewBookActivation, PrvViewBookDeactivation, PrvViewDrawTabCallback);
				}
			}
		}
	}

	// Get the form width from resources
	FrmGetFormInitialBounds(formP, &r);
	sDisplayWidth = r.extent.x;

	// Set the gadget draw function
	gadgetIndex = FrmGetObjectIndex(formP, RecordBackgroundGadget);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &sGadgetWhiteArea);
	FrmSetGadgetData(formP, gadgetIndex, &sGadgetWhiteArea);
	FrmSetGadgetHandler(formP, gadgetIndex, ToolsBackgroundGadgetHandler);

	FrmSetTransparentObjects(formP, true);

	// Move the Business Indicator bitmap right after the form title
	ToolsSetBusinessCardIndicatorPosition(formP);

	// Set the category label manually since FrmSetCategory doesn't work very well
	PrvViewSetCategoryLabel(formP);

	// Business card indicator
	PrvViewDrawBusinessCardIndicator(formP);

	gTappedColumnID = kInvalidColumnID;
	size = sizeof(gBookInfo.currentTabId);

	if (!sGotoRecord && // Display the 'All' tab if 'goto' (from search)
		DbCopyColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDLastVisibleTabID, 0, &gBookInfo.currentTabId, &size) < errNone)
		gBookInfo.currentTabId = gBookInfo.defaultViewTabId;

	gBookInfo.currentTabIndex = BookFindTabIndex(sBookP, gBookInfo.currentTabId);

	if (gBookInfo.currentTabIndex == kBookInvalidTabIndex)
	{
		// Reset to default values
		gBookInfo.currentTabId = gBookInfo.defaultViewTabId;
		gBookInfo.currentTabIndex = BookFindTabIndex(sBookP, gBookInfo.currentTabId);
	}

	BookSetActiveTabId(sBookP, gBookInfo.currentTabId, false);
	BookMakeTabVisible(sBookP, gBookInfo.currentTabId);

	// Load the Fullname from templates
	sFullnameH = AddrFreeFormNameBuild(gAddrDB, gCurrentRowID);

	if (sFullnameH)
	{
		RectangleType	nameBounds;
		Char *			fullnameP;
		FontID			font;

		sFullnameFont = largeBoldFont;
		font = FntSetFont(sFullnameFont);
        fullnameP = MemHandleLock(sFullnameH);
		sFullnameWidth = FntCharsWidth(fullnameP, strlen(fullnameP));
		FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewName), &nameBounds);

		if (sFullnameWidth > nameBounds.extent.x)
		{
			sFullnameFont = stdFont;
			FntSetFont(sFullnameFont);
			sFullnameWidth = FntCharsWidth(fullnameP, strlen(fullnameP));
		}

		MemHandleUnlock(sFullnameH);
		FntSetFont(font);
	}
	else
	{
		sFullnameWidth = 0;
	}

	if (gAttachRequest)
		FrmShowObject(formP, FrmGetObjectIndex(formP, RecordAttachButton));

	sPhoneSelection = false;

	if (!PrvViewLoadRecordDataFromDB())
	{
		FrmGotoForm(gApplicationDbP, ListView);
		return;
	}

	PrvViewLoadTab(gBookInfo.currentTabId);

	if (sGotoRecord)
	{
		uint16_t	lineIndex = 0;

		while (lineIndex < sCurrentTabNumLines)
		{
			if (sTabItemsP[lineIndex].columnID == sGotoRecordColID)
				break;

			lineIndex++;
		}

		if (lineIndex >= sCurrentTabNumLines)
		{
			sGotoRecord = false;
		}
		else
		{
			sCurrentTabTopLineIndex = PrvViewMakeVisible(lineIndex, false);
			PrvViewUpdateScrollers(formP);
		}
	}

	// Onehanded navigation stuff
	sOnehandedBookBodyItemSelected = false;
	sOnehandedSelectedLineIndex = kInvalidLineIndex;
}

/***********************************************************************
 *
 * FUNCTION:	PrvViewSelectColumn
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		08/06/02	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvViewDrawSelectionCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	DisplayLineType*	lineP;
	Coord				lineY;
	FontID				currentFont;
	uint16_t			i;

	if (! sPhoneSelection || cmd == winInvalidateDestroy)
		return true;

	// Set the background color in control-style colors since the text is selectable.
	WinPushDrawState();

	if (sPhoneSelected)
	{
		WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
	}
	else
	{
		WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
	}

	currentFont = FntSetFont(sTabItemsP[sSelectedPhoneLineIndex].dataFontId);
	lineY = sSelectedPhoneBounds.topLeft.y;

	for(i = 0; i < sSelectedPhoneNumLines; i++)
	{
		lineP = &sTabItemsP[sSelectedPhoneLineIndex + i];
		WinDrawChars(lineP->textP + lineP->offset, (int16_t)lineP->length, lineP->x, lineY);
		lineY += lineP->lineHeight;
	}

	FntSetFont(currentFont);
	WinPopDrawState();

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawName
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		07/16/02	Initial Revision
 * PPL		11/21/02	rewritten using address free form name function.
 *
 ***********************************************************************/
static void PrvViewDrawName(FormType *formP)
{
	FontID			currFont;
	RectangleType	r;
	uint16_t		x = 0;
	uint16_t		y;
	char *			fullnameP = NULL;
	char *			selectedTextP = NULL;
	char *			selPositionP = NULL;
    uint16_t		charsWidth;
	int16_t			selStart;

	TraceOutput(TL(appErrorClass, "PrvViewDrawName()"));

	if (sFullnameH == NULL)
		return;

	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

	currFont = FntSetFont(sFullnameFont);

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewName), &r);
	//WinEraseRectangle(&r, 0);
	y = r.topLeft.y + (r.extent.y >> 1) - (FntCharHeight() >> 1);

	fullnameP = (char*)MemHandleLock(sFullnameH);

	WinSetDrawMode(winOverlay);

	// We have some text to highlight
	if (sGotoRecord && sGotoRecordColIDInName)
	{
		// Load the searched text form record
		selectedTextP = MemPtrNew(sGotoRecordMatchLen + 1);
		ErrFatalDisplayIf(selectedTextP == NULL, "Out of memory!");
		DbCopyColumnValue(gAddrDB, gCurrentRowID, sGotoRecordColID, sGotoRecordMatchPos, selectedTextP, &sGotoRecordMatchLen);
		selectedTextP[sGotoRecordMatchLen] = 0;

		selPositionP = strstr(fullnameP, selectedTextP);

		if (selPositionP)
		{
			// Get the selection start point
			selStart = (int16_t) (selPositionP - fullnameP);

			// Draw text before Highlighted selection
			if (selStart)
			{
				WinPaintTruncChars(fullnameP, selStart, x, y, r.extent.x);

				charsWidth = FntCharsWidth(fullnameP, selStart);
				x += charsWidth;
				r.extent.x -= charsWidth;
				fullnameP += selStart;
			}

			// Highlighted text
			if (r.extent.x > 0)
			{
				WinPushDrawState();

				WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
				WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
				WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));

				WinPaintTruncChars(fullnameP, (int16_t)sGotoRecordMatchLen, x, y, r.extent.x);
				charsWidth = FntCharsWidth(fullnameP, (int16_t)sGotoRecordMatchLen);
				x += charsWidth;
				r.extent.x -= charsWidth;
				fullnameP += sGotoRecordMatchLen;

				WinPopDrawState();
			}
		}

		MemPtrFree(selectedTextP);
	}

	// Draw remaining unselected text if space remains
	if (r.extent.x > 0)
		WinPaintTruncChars(fullnameP, (int16_t)strlen(fullnameP), x, y, r.extent.x);

	MemHandleUnlock(sFullnameH);
	WinPopDrawState();
	FntSetFont(currFont);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewFormResize
 *
 * DESCRIPTION: Update the display and handles resizing
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		02/06/02	Initial release - Active InputArea Support
 *
 ***********************************************************************/
void PrvViewFormResize(FormType * formP, RectangleType *newBoundsP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	int16_t			xOffset;
	int16_t			yOffset;
	uint32_t		savedColID = dbInvalidRowID;
	uint16_t		lineIndex;

	xOffset = newBoundsP->extent.x - gCurrentWinBounds.extent.x;
	yOffset = newBoundsP->extent.y - gCurrentWinBounds.extent.y;

	if (!xOffset && !yOffset)
		return; // No need to resize

	sDisplayWidth = newBoundsP->extent.x;

	// Book
	BookResizeBody(sBookP, xOffset, yOffset);

	// The following objects moves only vertircally
	if (yOffset)
	{
		// RecordNewButton
		objIndex =  FrmGetObjectIndex (formP, RecordNewButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (formP, objIndex, x, y);

		// RecordEditButton
		objIndex =  FrmGetObjectIndex (formP, RecordEditButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (formP, objIndex, x, y);

		// RecordDoneButton
		objIndex =  FrmGetObjectIndex (formP, RecordDoneButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (formP, objIndex, x, y);

		// RecordAttachButton
		objIndex =  FrmGetObjectIndex (formP, RecordAttachButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (formP, objIndex, x, y);
	}

	if (xOffset)
	{
		// RecordViewName
		objIndex =  FrmGetObjectIndex (formP, RecordViewName);
		FrmGetObjectBounds(formP, objIndex, &objBounds);
		objBounds.extent.x += xOffset;
		FrmSetObjectBounds(formP, objIndex, &objBounds);

		// RecordCategoryLabel
		objIndex =  FrmGetObjectIndex (formP, RecordCategoryLabel);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += xOffset;
		FrmSetObjectPosition (formP, objIndex, x, y);
		PrvViewSetCategoryLabel(formP);
	}

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (formP, RecordBackgroundGadget);
	FrmGetObjectBounds(formP, objIndex, &objBounds);
	objBounds.extent.x +=   xOffset;
	objBounds.extent.y +=   yOffset;
	FrmSetObjectBounds(formP, objIndex, &objBounds);

	sGadgetWhiteArea.extent.x += xOffset;
	sGadgetWhiteArea.extent.y += yOffset;

	// EditTable
	objIndex =  FrmGetObjectIndex (formP, RecordViewDisplay);
	FrmGetObjectBounds(formP, objIndex, &objBounds);
	objBounds.extent.y += yOffset;
	objBounds.extent.x += xOffset;
	FrmSetObjectBounds(formP, objIndex, &objBounds);

	// RecordUpButton
	objIndex =  FrmGetObjectIndex (formP, RecordUpButton);
	FrmGetObjectPosition (formP, objIndex, &x, &y);
	y += yOffset;
	x += xOffset;
	FrmSetObjectPosition (formP, objIndex, x, y);

	// RecordDownButton
	objIndex =  FrmGetObjectIndex (formP, RecordDownButton);
	FrmGetObjectPosition (formP, objIndex, &x, &y);
	y += yOffset;
	x += xOffset;
	FrmSetObjectPosition (formP, objIndex, x, y);

	gCurrentWinBounds = *newBoundsP;

	if (sOnehandedBookBodyItemSelected && sOnehandedSelectedLineIndex != kInvalidLineIndex)
		savedColID = sTabItemsP[sOnehandedSelectedLineIndex].columnID;

	// Reload tab if needed
	if (xOffset)
		PrvViewLoadTab(gBookInfo.currentTabId);

	if (sOnehandedBookBodyItemSelected)
	{
		sOnehandedBookBodyItemSelected = false;
		sOnehandedSelectedLineIndex = sCurrentTabTopLineIndex;

		// Search for savedColID in the reloaded tab
		for (lineIndex = 0; lineIndex < sTabNumItems; lineIndex++)
		{
			if (sTabItemsP[lineIndex].columnID == savedColID)
			{
				sOnehandedBookBodyItemSelected = true;
				sOnehandedSelectedLineIndex = lineIndex;
				break;
			}
		}

		// This function returns a new line index if there is not
		// a full page of data with lineIndex as top item
		sCurrentTabTopLineIndex = PrvViewMakeVisible(sOnehandedSelectedLineIndex, false);
	}
	else
		sCurrentTabTopLineIndex = PrvViewMakeVisible(sCurrentTabTopLineIndex, true);

	PrvViewUpdateScrollers(formP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawForm
 *
 * DESCRIPTION: Update the display and handles resizing
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		02/06/02	Initial release - Active InputArea Support
 *
 ***********************************************************************/
void PrvViewDrawForm(void)
{
	FormType *	formP;

	TraceOutput(TL(appErrorClass, "PrvViewDrawForm()"));

	PIMAppProfilingBegin("PrvViewDrawForm");

	formP = FrmGetFormPtr(RecordView);

	FrmDrawForm(formP);
	PrvViewDrawName(formP);
	PrvViewDrawTab(formP);

	PIMAppProfilingEnd();
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewNavigate
 *
 * DESCRIPTION: Move to next/prev tab/record
 *
 * PARAMETERS:  direction - up or down
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		12/11/02	Initial revision
 *
 ***********************************************************************/
static Boolean PrvViewNavigate(WinDirectionType direction)
{
	Boolean	handled = false;

	if (direction == winUp)
	{
		if (gBookInfo.currentTabIndex > 0)
		{
			// Go to prev tab
			handled = BookActiveTabIndex(sBookP, gBookInfo.currentTabIndex - 1);
			BookMakeTabVisibleByIndex(sBookP, gBookInfo.currentTabIndex);
		}
	}
	else
	{
		if (gBookInfo.currentTabIndex < BookGetNumberOfTab(sBookP) - 1)
		{
			// Go to next tab
			handled = BookActiveTabIndex(sBookP, gBookInfo.currentTabIndex + 1);
			BookMakeTabVisibleByIndex(sBookP, gBookInfo.currentTabIndex);
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewScroll
 *
 * DESCRIPTION: Scrolls the record view
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	06/22/95	Initial Revision
 * roger	08/02/95	Reworked to handle half height blank lines.
 * roger	10/30/95	Reworked to obey FntLineHeight
 * gap		10/12/99	Close command bar before processing scroll
 * FPa		11/15/00	Fixed bug #44838
 *
 ***********************************************************************/
static Boolean PrvViewScroll(WinDirectionType direction, Boolean navigate)
{
	FormType *		formP;
	RectangleType	bounds;
	FontID			currentFont;
	uint16_t		displayHeight;
	uint16_t		height;
	uint16_t		lineIndex;
	Boolean			handled = false;

	currentFont = FntGetFont();

	formP = FrmGetFormPtr(RecordView);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
	displayHeight = bounds.extent.y;

	lineIndex = sCurrentTabTopLineIndex;
	height = 0;

	// Calculate the new top item
	if (direction == winDown)
	{
		// Get the new top item index
		PrvGetCurrentTabDisplayInfo(lineIndex, displayHeight, &lineIndex, &height);

		// Already at bottom. Go to next tab/record
		if (height < displayHeight || lineIndex == sCurrentTabNumLines - 1)
		{
			if (navigate)
				handled = PrvViewNavigate(winDown);

			FntSetFont(currentFont);
			return handled;
		}

		// Don't start with a blank line
		if (lineIndex && sTabItemsP[lineIndex].fieldClass == kFieldClass_BlankLine)
			lineIndex--;

		// This function returns a new line index if there is not
		// a full page of data with lineIndex as top item
		// Otherwise we'll get the same index
		lineIndex = PrvViewMakeVisible(lineIndex, true);
	}
	else // direction == winUp
	{
		if (lineIndex == 0)
		{
			if (navigate)
				handled = PrvViewNavigate(winUp);

			FntSetFont(currentFont);
			return handled;
		}

		while (height < displayHeight)
		{
			height += sTabItemsP[lineIndex].lineHeight;

			// Break if we filled the whole page
			if (height >= displayHeight)
			{
				if (height > displayHeight)
				{
					while (!sTabItemsP[lineIndex].lineFeed)
						lineIndex++;

					lineIndex++;
				}
				break;
			}

			if (lineIndex == 0)
				break;

			lineIndex--;

			while (lineIndex && !sTabItemsP[lineIndex].lineFeed)
				lineIndex--;
		}

		// Don't start with a blank line
		if (lineIndex < sCurrentTabNumLines - 1 && sTabItemsP[lineIndex].fieldClass == kFieldClass_BlankLine)
			lineIndex++;
	}

	while (lineIndex && !sTabItemsP[lineIndex - 1].lineFeed)
		lineIndex--;

	sCurrentTabTopLineIndex = lineIndex;

	FntSetFont(currentFont);

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
	ToolsFrmInvalidateRectFunc(RecordView, &bounds, PrvViewDrawTabCallback, NULL);

	PrvViewUpdateScrollers(formP);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewPhoneNumberAt
 *
 * DESCRIPTION: Given a point on the screen in the RecordViewDisplay,
 *					 determine whether that point is in a phone number, and if
 *					 so, which one. Phone numbers are defined as linefeed
 *					 separated.
 *
 * PARAMETERS:	x				- x coordinate of point to look at
 *					y				- y coordinate of point to look at
 *					fieldNumP	- result: which field the phone number is in
 *					offsetP		- result: where phone number starts in field
 *					lengthP		- result: how long phone number is
 *
 * RETURNED:	whether there is a phone number at the given point
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * peter	05/05/00	Initial Revision
 * peter	05/26/00	Fix bug: Restore font.
 * aro		06/27/00	Fix bug for non phone field
 *
 ***********************************************************************/
Boolean PrvViewPhoneNumberAt(int16_t x, int16_t y, uint16_t *lineIndexP, Coord *lineYP)
{
	FormPtr			formP;
	RectangleType	rect;
	RectangleType	tabBounds;
	int16_t			lineY = 0;
	uint16_t		lineIndex;

	formP = FrmGetFormPtr(RecordView);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &tabBounds);

	if (!RctPtInRectangle(x, y, &tabBounds))
		return false;

	lineIndex = sCurrentTabTopLineIndex;
	while (lineIndex < sCurrentTabNumLines)
	{
		lineY = tabBounds.topLeft.y + sTabItemsP[lineIndex].yOffset - sTabItemsP[sCurrentTabTopLineIndex].yOffset;
		if (y < lineY + sTabItemsP[lineIndex].lineHeight)
			break;

		lineIndex++;
	}

	// This should never happen
	if (lineIndex >= sCurrentTabNumLines)
		return false;

	// Check if this is a dialable phone
	if (ToolsIsPhoneIndexSupported(gCurrentRowID, sTabItemsP[lineIndex].columnID))
	{
		// Check tap position
		RctSetRectangle(&rect, sTabItemsP[lineIndex].x, lineY, sTabItemsP[lineIndex].textWidth, sTabItemsP[lineIndex].lineHeight);

		if (RctPtInRectangle(x, y, &rect))
		{
			uint32_t	columnID;

			// Give the first line for this columnID
			columnID = sTabItemsP[lineIndex].columnID;

			while (lineIndex > sCurrentTabTopLineIndex && sTabItemsP[lineIndex - 1].columnID == columnID)
			{
				lineIndex--;
				lineY -= sTabItemsP[lineIndex].lineHeight;
			}

			if (lineIndexP)
				*lineIndexP = lineIndex;

			if (lineYP)
				*lineYP = lineY;

			return true;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewGetColumnIDAt
 *
 * DESCRIPTION: Given a point coordinates, returns the columnID of the
 *				data located at this point
 *
 * PARAMETERS:  ->	formP
 *				->	x
 *				->	y
 *
 * RETURNED:    The columnID, or kInvalidColumnID
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		12/05/02	Initial revision
 *
 ***********************************************************************/
static uint32_t PrvViewGetColumnIDAt(FormType *formP, Coord x, Coord y)
{
	Coord			height;
	uint16_t		lineIndex;
	RectangleType	bounds;
	uint32_t		columnID = kInvalidColumnID;
	FontID			currentFont;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);

	if (!RctPtInRectangle(x, y, &bounds))
		return kInvalidColumnID;

	currentFont = FntGetFont();

	height = bounds.topLeft.y;
	lineIndex = sCurrentTabTopLineIndex;

	while (lineIndex < sCurrentTabNumLines)
	{
		if (height > bounds.topLeft.y + bounds.extent.y)
			break;

		FntSetFont(sTabItemsP[lineIndex].labelFontId);
		height += (sTabItemsP[lineIndex].fieldClass == kFieldClass_BlankLine) ? FntLineHeight() >> 1 : FntLineHeight();

		if (height > y)
		{
			if (sTabItemsP[lineIndex].columnID == kInvalidColumnID)
			{
				if (lineIndex < sCurrentTabNumLines - 1)
					lineIndex++;
				else if (lineIndex > 0)
					lineIndex--;
				else
					break;
			}

			// Check the right column if there are more than one column on one line
			while (lineIndex < sCurrentTabNumLines && !sTabItemsP[lineIndex].lineFeed)
			{
				if (x >= sTabItemsP[lineIndex].x && x < sTabItemsP[lineIndex + 1].x)
					break;

				lineIndex++;
			}

			columnID = sTabItemsP[lineIndex].columnID;

			break;
		}

		while (lineIndex < sCurrentTabNumLines && !sTabItemsP[lineIndex].lineFeed)
			lineIndex++;

		lineIndex++;
	}

	FntSetFont(currentFont);

	return columnID;
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewGotoEdit
 *
 * DESCRIPTION: Edit the current record or note
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		06/23/2004	Extracted from PrvViewHandlePen
 *
 ***********************************************************************/
static void PrvViewGotoEdit(void)
{
	// Edit the note
	if (gTappedColumnID == kAddrColumnIDNote)
	{
		uint16_t savedTopLine;

		// Release current row for the NoteViewLib
		PrvViewReleaseRecordDataFromDB();

		AddressEditNote(gCurrentRowID);

		// The note has been removed, check if we have to remove an empty record...
		// If so, switch to the list view
		if (! AddressDBRecordContainsData(gCurrentRowID))
		{
			AddressDBDeleteRecord(false);
			FrmGotoForm(gApplicationDbP, ListView);
			return;
		}

		// Reload the tab
		PrvViewLoadRecordDataFromDB();

		// Save current top item
		savedTopLine = sCurrentTabTopLineIndex;
		// Reload the table
		PrvViewLoadTab(kTabAllId);
		// Restore top item
		sCurrentTabTopLineIndex = savedTopLine;

		// Redraw
		ToolsFrmInvalidateWindow(RecordView);
		return;
	}

	// Set the tab to be displayed and switch to EditView
	if (gTappedColumnID == kInvalidColumnID)
	{
		gBookInfo.currentTabId = gBookInfo.defaultNewTabId;
		gTappedColumnID = gBookInfo.defaultNewColumnId;
		gBookInfo.currentTabIndex = kBookInvalidTabIndex;
	}
	else if (gBookInfo.currentTabId == kTabAllId)
	{
		AddressTabFindColIDTab(&gBookInfo, gTappedColumnID, &gBookInfo.currentTabIndex, &gBookInfo.currentTabId);
	}

	sOpenCursorOnClose = false;
	FrmGotoForm(gApplicationDbP, EditView);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewHandlePen
 *
 * DESCRIPTION: Handle pen movement in the RecordViewDisplay. If the user
 *					 taps in the RecordViewDisplay take them to the Edit View
 *					 unless they tap on a phone number. In that case, arrange
 *					 to dial the selected number.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if handled.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	11/27/95	Cut from RecordViewHandleEvent
 * peter	05/03/00	Add support for tapping on phone numbers to dial
 * aro		06/27/00    Check for dialing abilities
 *
 ***********************************************************************/
Boolean PrvViewHandlePen(EventType *eventP)
{
	FormType *		formP;
	RectangleType	tabBounds;
	RectangleType	nameBounds;
	int16_t			x, y;
	uint16_t		lineIndex;
	Coord			lineY;
	Boolean			handled = false;
	Boolean			selected;

	formP = FrmGetFormPtr(RecordView);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &tabBounds);

	switch (eventP->eType)
	{
		case penDownEvent:
			if (sNamePopupDisplayed)
			{
				handled = RctPtInRectangle(eventP->screenX, eventP->screenY, &sNamePopupBounds);
				break;
			}

	        // Handle TapDialing
			if (gEnableTapDialing && PrvViewPhoneNumberAt(eventP->screenX, eventP->screenY, &lineIndex, &lineY))
			{
				DisplayLineType *	lineP;
				FontID				savedFont;
				uint32_t			columnID;
				Coord				plainLineHeight;

				savedFont = FntSetFont(gAddrRecordFont);
				plainLineHeight = FntLineHeight();
				FntSetFont(savedFont);

				sPhoneSelection = true;
				sPhoneSelected = true;

				sSelectedPhoneBounds.topLeft.y = lineY;
				sSelectedPhoneBounds.extent.y = 0;
				sSelectedPhoneBounds.topLeft.x = sTabItemsP[lineIndex].x;
				sSelectedPhoneBounds.extent.x = tabBounds.topLeft.x + tabBounds.extent.x - sTabItemsP[lineIndex].x;
				sSelectedPhoneNumLines = 0;
				sSelectedPhoneLineIndex = lineIndex;

				columnID = sTabItemsP[lineIndex].columnID;
				// Compute phone number bounds
				while (true)
				{
					if (lineIndex >= sCurrentTabNumLines ||
						sSelectedPhoneBounds.topLeft.y + sSelectedPhoneBounds.extent.y + plainLineHeight > tabBounds.topLeft.y + tabBounds.extent.y)
						break;

					lineP = &sTabItemsP[lineIndex];

					if (!lineP || lineP->columnID != columnID)
						break;

					sSelectedPhoneBounds.extent.y += plainLineHeight;
					sSelectedPhoneNumLines++;
					lineIndex++;
				}

	            // Draw the phone number inverted
				ToolsFrmInvalidateRectFunc(RecordView, &sSelectedPhoneBounds, PrvViewDrawSelectionCallback, NULL);
				handled = true;
			}
			break;

		case penMoveEvent:
			if (! sPhoneSelection)
				break;

			// Check if the stylus is still over the phone number
			selected = RctPtInRectangle(eventP->screenX, eventP->screenY, &sSelectedPhoneBounds);

			if (selected != sPhoneSelected)
			{
				sPhoneSelected = selected;
				ToolsFrmInvalidateRectFunc(RecordView, &sSelectedPhoneBounds, PrvViewDrawSelectionCallback, NULL);
			}

			handled = true;
			break;

		case penUpEvent:
			// Discard the name popup if displayed
			if (sNamePopupDisplayed)
			{
				PrvViewDiscardNamePopup(true);
				handled = !RctPtInRectangle(eventP->screenX, eventP->screenY, &tabBounds);
				break;
			}

			// Handle TapDialing
			if (sPhoneSelection)
			{
				if (sPhoneSelected)
				{
					sPhoneSelected = false;
					ToolsFrmInvalidateRectFunc(RecordView, &sSelectedPhoneBounds, PrvViewDrawSelectionCallback, NULL);

					DialListShowDialog(gCurrentRowID, sTabItemsP[sSelectedPhoneLineIndex].columnID);
				}

				sPhoneSelection = false;

				handled = true;
				break;
			}

			// Display the popup if pen events occured within the name bounds.
			FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewName), &nameBounds);

			// Tap occurs with name bounds and the popup will be displayed
			if (RctPtInRectangle(eventP->data.penUp.start.x, eventP->data.penUp.start.y, &nameBounds) &&
				RctPtInRectangle(eventP->data.penUp.end.x, eventP->data.penUp.end.y, &nameBounds) &&
				PrvViewDisplayNamePopup(formP))
				break;

			// Occurs within tab bounds
			if (RctPtInRectangle(eventP->data.penUp.start.x, eventP->data.penUp.start.y, &tabBounds) &&
				RctPtInRectangle(eventP->data.penUp.end.x, eventP->data.penUp.end.y, &tabBounds))
			{
				x = eventP->data.penUp.end.x;
				y = eventP->data.penUp.end.y;

				gTappedColumnID = PrvViewGetColumnIDAt(formP, x, y);

				PrvViewGotoEdit();
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	06/27/95	Initial Revision
 * jmp		09/17/99	Use NewNoteView instead of NoteView.
 * FPa		11/20/00	Fixed a memory leak when deleting a note
 * FPa		01/26/00	Fixed bug #51545
 *
 ***********************************************************************/
Boolean PrvViewDoCommand (uint16_t command)
{
	uint32_t	newRowID;
	uint16_t	numCharsToHilite;
	uint32_t	columnIDToHilite;
	FontID		oldFont;
	Boolean		handled = false;
	uint32_t	size;
	uint32_t	colID;
	FormType *	formP;

	switch (command)
	{
		case RecordRecordDeleteRecordCmd:
			MenuEraseStatus (0);
			PrvViewReleaseRecordDataFromDB();

			if (ToolsConfirmDeletion())
			{
				AddressDBDeleteRecord(gSaveBackup);
				FrmGotoForm (gApplicationDbP, ListView);
			}
			else
			{
				// Get the data back if the user canceled
				PrvViewLoadRecordDataFromDB();
			}

			handled = true;
			break;

		case RecordRecordDuplicateAddressCmd:
			MenuEraseStatus (0);
			AddressDBDuplicateRecord(gCurrentRowID, &newRowID, &numCharsToHilite, &columnIDToHilite);

			// If we have a new record take the user to be able to edit it automatically.
			if (newRowID != dbInvalidRowID)
			{
				PrvViewReleaseRecordDataFromDB();

				gCurrentRowID = newRowID;
				gNumCharsToHilite = numCharsToHilite;
				gTappedColumnID = columnIDToHilite;
				AddressTabFindColIDTab(&gBookInfo, gTappedColumnID, &gBookInfo.currentTabIndex, &gBookInfo.currentTabId);
				sOpenCursorOnClose = false;
				FrmGotoForm (gApplicationDbP, EditView);
			}

			handled = true;
			break;

		case RecordRecordDialCmd:
			MenuEraseStatus (0);
			// Get the columnID for the displayed phone
			size = sizeof(colID);

			if (DbCopyColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDDisplayedPhone, 0, &colID, &size) < errNone)
			{
				SndPlaySystemSound (sndError);
				handled = true;
				break;
			}

			// Check if the Displayed Phone as a value and is a phone number, if the phone is not part of supported
			// dial phone number, let the table handle selection.
			if (!ToolsIsPhoneIndexSupported(gCurrentRowID, colID))
			{
				SndPlaySystemSound (sndError);
				handled = true;
				break;
			}

			// CGu
			DialListShowDialog(gCurrentRowID, colID);
			handled = true;
			break;

		case RecordRecordAttachNoteCmd:
			MenuEraseStatus (0);
			PrvViewReleaseRecordDataFromDB();

			if (AddressEditNote(gCurrentRowID))
				PrvViewInit();
			else PrvViewLoadRecordDataFromDB();

			handled = true;
			break;

		case RecordRecordDeleteNoteCmd:
			MenuEraseStatus (0);

			if (AddressExistingNote(gCurrentRowID) &&
				FrmAlert(gApplicationDbP, DeleteNoteAlert) == DeleteNoteYes)
			{
				PrvViewReleaseRecordDataFromDB();

				// Delete the column data
				AddressDeleteNote(gCurrentRowID);

				// Initialize sTabsDisplayedLinesPP, sRecordViewLastLine... so that the Note won't be drawn
				PrvViewFreeResources(false);
			  	PrvViewInit();
				ToolsFrmInvalidateWindow(RecordView);
			}

			handled = true;
			break;

		case RecordRecordSelectBusinessCardCmd:
			MenuEraseStatus (0);

			if (FrmAlert(gApplicationDbP, SelectBusinessCardAlert) == SelectBusinessCardYes)
			{
				gBusinessCardRowID = gCurrentRowID;
				formP = FrmGetFormPtr(RecordView);
				PrvViewSetCategoryLabel(formP);
				PrvViewDrawBusinessCardIndicator(formP);
			}

			handled = true;
			break;

		case RecordRecordBeamBusinessCardCmd:
			MenuEraseStatus (0);
			ToolsAddrBeamBusinessCard();
			handled = true;
			break;

		case RecordRecordAttachRecordCmd:
			MenuEraseStatus (0);
			ToolsAddrAttachRecord(gAddrDB, gCurrentRowID);
			handled = true;
			break;

		case RecordRecordBeamRecordCmd:
			MenuEraseStatus (0);
			TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(gBookInfo.currentTabId));
			handled = true;
			break;

		case RecordRecordSendRecordCmd:
			MenuEraseStatus (0);
			TransferSendRecord(gAddrDB, gCurrentRowID, exgSendPrefix, NoDataToSendAlert, AddressExportGetFamily(gBookInfo.currentTabId));
			handled = true;
			break;

		case RecordOptionsFontCmd:
			MenuEraseStatus (0);
			oldFont = gAddrRecordFont;
			gAddrRecordFont = ToolsSelectFont (RecordView, gAddrRecordFont);

			if (oldFont != gAddrRecordFont)
				PrvViewLoadTab(gBookInfo.currentTabId);

			handled = true;
			break;

		case RecordOptionsEditCustomFldsCmd:
			MenuEraseStatus (0);
			FrmPopupForm (gApplicationDbP, CustomEditDialog);
			handled = true;
			break;

		case RecordOptionsAboutCmd:
			MenuEraseStatus (0);
			AbtShowAbout (sysFileCAddress);
			handled = true;
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    RecordViewGotoItem
 *
 * DESCRIPTION: Sets goto item variables and switch to RecordView
 *
 * PARAMETERS:  ->	goToParamsP: Pointer to the structuture
 *						containing the goto info.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		10/29/2003	Initial revision
 *
 ***********************************************************************/
void RecordViewGotoItem(GoToParamsType *goToParamsP)
{
	sGotoRecord				= true;

	// Check for the presence of the found columnID in the list name columns list
	if (!gFFNColumnListResID)
		gFFNColumnListResID = ToolsGetCountryBaseResID(kU32ListResType, kAddrFFNTemplateBaseID, kAddrTemplateResIDIncrement);

	sGotoRecordColIDInName	= U32ListContainsItem(gApplicationDbP, gFFNColumnListResID, NULL, goToParamsP->matchFieldNum);
	sGotoRecordColID		= goToParamsP->matchFieldNum;
	sGotoRecordMatchPos		= goToParamsP->matchPos;
	sGotoRecordMatchLen		= goToParamsP->matchLen;
	gCurrentRowID			= goToParamsP->recordID;

	FrmGotoForm(gApplicationDbP, RecordView);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewHandleOnehandNavigation
 *
 * DESCRIPTION: Handle onehanded navigation events for the Booktab body object
 *
 * PARAMETERS:  ->	eventP: the event containing the keyDown structure
 *
 * RETURNED:    true if handled, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name   	 Date      		Description
 * ----		------      	-----------
 * TEs   	03/23/2004     	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvViewHandleOnehandNavigation(EventType *eventP)
{
	Boolean				handled = false;
	FormType *			formP;
	RectangleType		bounds;
	uint16_t			lastLineIndex;
	uint16_t			newLineIndex;
	uint16_t			lineIndex;
	Coord				delta;
	Coord				deltaMin;
	int16_t				increment;
	wchar32_t			keyChr;
	WinDirectionType	direction;
	FrmNavStateFlagsType	navState = 0;
	Boolean				objectFocusMode;
	uint16_t			objID;
	Boolean				noRepeat;
	uint16_t			focusedObjIndex;
	uint16_t			focusedObjID = frmInvalidObjectId;

	formP = FrmGetFormPtr(RecordView);

	if ((focusedObjIndex = FrmGetFocus(formP)) != noFocus)
		focusedObjID = FrmGetObjectId(formP, focusedObjIndex);

	objectFocusMode = FrmGetNavState(formP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0;
	keyChr = eventP->data.keyDown.chr;

	switch (keyChr)
	{
	case vchrRockerUp:
	case vchrRockerDown:
		if (!objectFocusMode)
		{
			direction = (keyChr == vchrRockerUp) ? winUp : winDown;
			noRepeat = (Boolean) ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0);

			if (!PrvViewScroll(direction, noRepeat) && noRepeat)
			{
				objID = (direction == winUp) ? RecordViewName : RecordDoneButton;
				FrmSetFocus(formP, FrmGetObjectIndex(formP, objID));
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, objID);
			}

			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case RecordViewDisplay:
			if (sOnehandedBookBodyItemSelected)
			{
				FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);

				newLineIndex = sOnehandedSelectedLineIndex;
				increment = (keyChr == vchrRockerDown) ? +1 : -1;

				while (keyChr == vchrRockerUp && newLineIndex > 0 ||
						keyChr == vchrRockerDown && newLineIndex < sCurrentTabNumLines - 1)
				{
					newLineIndex += increment;

					if (sTabItemsP[newLineIndex].columnID != kInvalidColumnID &&
						sTabItemsP[newLineIndex].yOffset != sTabItemsP[sOnehandedSelectedLineIndex].yOffset &&
						sTabItemsP[newLineIndex].textWidth > 0)
						break;
				}

				// Select the most appropriate in the same line
				lineIndex = newLineIndex;
				deltaMin = INT16_MAX;
				while (keyChr == vchrRockerUp && newLineIndex > 0 ||
						keyChr == vchrRockerDown && newLineIndex < sCurrentTabNumLines - 1)
				{
					if (sTabItemsP[lineIndex].yOffset != sTabItemsP[newLineIndex].yOffset)
						break;

					delta = Abs(sTabItemsP[lineIndex].x - sTabItemsP[sOnehandedSelectedLineIndex].x);
					if (sTabItemsP[lineIndex].columnID != kInvalidColumnID && delta < deltaMin)
					{
						deltaMin = delta;
						newLineIndex = lineIndex;
					}

					lineIndex += increment;
				}

				// If it's still an invalid columnID, then sOnehandedSelectedLineIndex was
				// already the topmost ot the bottommost selectable item
				if (sTabItemsP[newLineIndex].columnID == kInvalidColumnID ||
					newLineIndex == sOnehandedSelectedLineIndex)
				{
					if ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == autoRepeatKeyMask)
						handled = true;
					break;
				}

				sOnehandedSelectedLineIndex = newLineIndex;
				sCurrentTabTopLineIndex = PrvViewMakeVisible(sOnehandedSelectedLineIndex, false);
				PrvViewUpdateScrollers(formP);
				ToolsFrmInvalidateRectFunc(RecordView, &bounds, PrvViewDrawTabCallback, NULL);
				handled = true;
			}
			else
			{
				handled = PrvViewScroll((keyChr == vchrRockerDown) ? winDown : winUp, false) ||
						(eventP->data.keyDown.modifiers & autoRepeatKeyMask) != 0;
			}
			break;

		case RecordViewName:
			if (sNamePopupDisplayed)
				PrvViewDiscardNamePopup(true);
			break;
		}
		break;

	case vchrRockerLeft:
	case vchrRockerRight:
		if (!objectFocusMode)
		{
			direction = (keyChr == vchrRockerLeft) ? winUp : winDown;
			noRepeat = (Boolean) ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0);

			if (!PrvViewNavigate(direction) && noRepeat)
			{
				objID = (direction == winUp) ? RecordViewName : RecordDoneButton;
				FrmSetFocus(formP, FrmGetObjectIndex(formP, objID));
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, objID);
			}

			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case RecordViewDisplay:
			if (sOnehandedBookBodyItemSelected)
			{
				increment = (keyChr == vchrRockerRight) ? +1 : -1;
				newLineIndex = sOnehandedSelectedLineIndex;

				if (sTabItemsP[newLineIndex].yOffset != sTabItemsP[newLineIndex + increment].yOffset)
				{
					if (keyChr == vchrRockerLeft)
					{
						// Reselect the entire book body
						FrmNavObjectTakeFocus(formP, RecordViewDisplay);
						sOnehandedBookBodyItemSelected = false;
						handled = true;
					}

					break;
				}

				if (keyChr == vchrRockerLeft && newLineIndex > 0 ||
					keyChr == vchrRockerRight && newLineIndex < sCurrentTabNumLines)
				{
					// Skip non-text items
					do {
						newLineIndex += increment;
					} while ((keyChr == vchrRockerLeft && newLineIndex > 0 ||
								keyChr == vchrRockerRight && newLineIndex < sCurrentTabNumLines) &&
							sTabItemsP[newLineIndex].columnID == kInvalidColumnID);

					// Exit from the table if by skiping non text items we reach another line.
					if (sTabItemsP[newLineIndex].yOffset != sTabItemsP[sOnehandedSelectedLineIndex].yOffset)
					{
						if (keyChr == vchrRockerLeft)
						{
							// Reselect the entire book body
							FrmNavObjectTakeFocus(formP, RecordViewDisplay);
							sOnehandedBookBodyItemSelected = false;
							handled = true;
						}

						break;
					}

					sOnehandedSelectedLineIndex = newLineIndex;
					FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);
					ToolsFrmInvalidateRectFunc(RecordView, &bounds, PrvViewDrawTabCallback, NULL);
					handled = true;
				}
			}
			break;

		case RecordViewName:
			if (sNamePopupDisplayed)
				PrvViewDiscardNamePopup(true);
			break;
		}
		break;

	case vchrRockerCenter:
		if (!objectFocusMode && sNamePopupDisplayed)
		{
			PrvViewDiscardNamePopup(true);
			break;
		}

		switch (focusedObjID)
		{
		case RecordViewDisplay:
			if (sOnehandedBookBodyItemSelected)
			{
				gTappedColumnID = sTabItemsP[sOnehandedSelectedLineIndex].columnID;

				// If tap dialing disabled, go to EditView.
				if (!gEnableTapDialing ||
					!ToolsIsPhoneIndexSupported(gCurrentRowID, gTappedColumnID) ||
					!DialListShowDialog(gCurrentRowID, gTappedColumnID))
					PrvViewGotoEdit();
			}
			else if (sCurrentTabNumLines > 0)
			{
				FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, RecordViewDisplay), &bounds);

				PrvGetCurrentTabDisplayInfo(sCurrentTabTopLineIndex, bounds.extent.y, &lastLineIndex, NULL);
				if (sOnehandedSelectedLineIndex < sCurrentTabTopLineIndex || sOnehandedSelectedLineIndex > lastLineIndex)
				{
					sOnehandedSelectedLineIndex = sCurrentTabTopLineIndex;

					// Skip non-data lines.
					while (sOnehandedSelectedLineIndex < sCurrentTabNumLines - 1 && sTabItemsP[sOnehandedSelectedLineIndex].columnID == kInvalidColumnID)
						sOnehandedSelectedLineIndex++;
				}

				if (sTabItemsP[sOnehandedSelectedLineIndex].columnID != kInvalidColumnID)
				{
					ToolsFrmInvalidateRectFunc(RecordView, &bounds, PrvViewDrawTabCallback, NULL);
					sOnehandedBookBodyItemSelected = true;
				}
			}
			break;

		case RecordViewName:
			// Discard the name popup if currently displayed
			if (sNamePopupDisplayed)
			{
				PrvViewDiscardNamePopup(true);

				FrmGetObjectBounds(formP, focusedObjIndex, &bounds);
				FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
			}
			// Check the name length and display the popup if needed
			else if (sFullnameH)
			{
				if (PrvViewDisplayNamePopup(formP))
					FrmClearFocusHighlight();
			}
			break;
		}
		break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    ViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Address View"
 *              of the Address Book application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		6/5/95		Initial Revision
 * jmp		10/01/99	Fix frmUpdateEvent so that it redraws the form
 *						and updated the RecordView now that we can get
 *						into a situation where the bits might not necessarily
 *						be restored except through and update event itself.
 * FPa		11/28/00	kFrmCustomUpdateEvent handling
 * ppl		02/06/02	Add active input Area support (AIA)
 *
 ***********************************************************************/
Boolean ViewHandleEvent(EventType *eventP)
{
	uint32_t	numLibs;
	uint16_t	i;
	Boolean		handled = false;
	FormType *	formP;
	uint16_t	lineIndex;
	RectangleType	bounds;
	uint16_t		objIndex;
	FrmNavStateFlagsType	navState = 0;

	switch (eventP->eType)
	{
		case frmOpenEvent:
			PrvViewInit();
			handled = true;
			break;

		case frmCloseEvent:
            PrvViewClose();
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case RecordDoneButton:
					// When we return to the ListView highlight this record.
					gListViewSelectThisRowID = gCurrentRowID;
					FrmGotoForm (gApplicationDbP, ListView);
					handled = true;
					break;

				case RecordEditButton:
					// If the current Tab is All, open the edit form as form New
					if (gBookInfo.currentTabId == kTabAllId)
					{
						gBookInfo.currentTabId	= gBookInfo.defaultNewTabId;
						gTappedColumnID			= gBookInfo.defaultNewColumnId;
					}
					else
					{
						// currentTabId is already set. Find the first column to Edit
						for (i = 0; i < gBookInfo.tabs[gBookInfo.currentTabIndex].numElement; i++)
						{
							if (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[i].visible && !ToolsIsYomiFieldByIndex(gBookInfo.currentTabIndex, i))
							{
								gTappedColumnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[i].columnId;
								break;
							}
						}
					}

					gBookInfo.currentTabIndex	= kBookInvalidTabIndex;
					sOpenCursorOnClose = false;
					FrmGotoForm(gApplicationDbP, EditView);
					handled = true;
					break;

				case RecordNewButton:
					// Release current record so the EditView can requery the cursor.
					PrvViewReleaseRecordDataFromDB();
					EditNewRecord();
					handled = true;
					break;

				case RecordAttachButton:
					ToolsAddrAttachRecord(gAddrDB, gCurrentRowID);
					handled = true;
					break;
			}
			break;

		case penDownEvent:
		case penMoveEvent:
		case penUpEvent:
			handled = PrvViewHandlePen(eventP);
			break;

		case ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case RecordUpButton:
					PrvViewScroll(winUp, false);
					// leave unhandled so the buttons can repeat
					break;

				case RecordDownButton:
					PrvViewScroll(winDown, false);
					// leave unhandled so the buttons can repeat
					break;
			}
			break;

		case keyDownEvent:
			if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr))
			{
				FrmGotoForm (gApplicationDbP, ListView);
				handled = true;
			}

			else if (EvtKeydownIsVirtual(eventP))
			{
				handled = PrvViewHandleOnehandNavigation(eventP);
				if (handled)
					break;

				switch (eventP->data.keyDown.chr)
				{
					case vchrPageUp:
						PrvViewScroll(winUp, (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0);
						handled = true;
						break;

					case vchrPageDown:
						PrvViewScroll(winDown, (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0);
						handled = true;
						break;

					case vchrSendData:
						TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(gBookInfo.currentTabId));
						handled = true;
						break;
				}
			}
			break;

		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.objectID == RecordViewDisplay)
			{
				// Highlight the RecordViewBody
				formP = FrmGetFormPtr(RecordView);
				objIndex = FrmGetObjectIndex(formP, RecordViewDisplay);
				FrmSetFocus(formP, objIndex);

				if (!sOnehandedBookBodyItemSelected)
				{
					FrmGetObjectBounds(formP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
				}

				handled = true;
			}
			else if (eventP->data.frmObjectFocusTake.objectID == RecordViewName)
			{
				// Highlight the RecordViewBody
				formP = FrmGetFormPtr(RecordView);
				objIndex = FrmGetObjectIndex(formP, RecordViewName);
				FrmSetFocus(formP, objIndex);

				FrmClearFocusHighlight();

				// If the FFN popup doesn't need to be displayed, draw the focus ring around name
				if (!PrvViewDisplayNamePopup(formP))
				{
					FrmGetObjectBounds(formP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
				}

				handled = true;
			}
			break;

		case frmObjectFocusLostEvent:
			if (eventP->data.frmObjectFocusLost.objectID == RecordViewDisplay)
				sOnehandedBookBodyItemSelected = false;
			break;

		case menuEvent:
			handled = PrvViewDoCommand (eventP->data.menu.itemID);
			break;

		case menuCmdBarOpenEvent:
		{
			DmOpenRef	uilibdbP = NULL;
			uint32_t	uiLibRefNum = 0;
			status_t	err;

			err = SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
			if (err >= errNone)
				err = SysGetModuleDatabase(uiLibRefNum, NULL, &uilibdbP);
			ErrNonFatalDisplayIf(err < errNone, "Can't get UILibRefNum");

			MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarDeleteBitmap, menuCmdBarResultMenuItem, RecordRecordDeleteRecordCmd, 0);
			MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarBeamBitmap, menuCmdBarResultMenuItem, RecordRecordBeamRecordCmd, 0);

			if (err >= errNone)
				SysUnloadModule(uiLibRefNum);

			// tell the field package to not add cut/copy/paste buttons automatically
			eventP->data.menuCmdBarOpen.preventFieldButtons = true;

			// don't set handled to true; this eventP must fall through to the system.
			break;
		}

		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if(!ToolsIsDialerPresent())
					MenuHideItem(RecordRecordDialCmd);

				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
					MenuHideItem(RecordRecordSendRecordCmd);
				else
					MenuShowItem(RecordRecordSendRecordCmd);
			}
			else
			{
				// Hide send, beam & business card menu items
				MenuHideItem(RecordRecordDialCmd);
				MenuHideItem(RecordRecordBeamRecordCmd);
				MenuHideItem(RecordRecordSendRecordCmd);
				MenuHideItem(RecordRecordSelectBusinessCardCmd);
				MenuHideItem(RecordRecordBeamBusinessCardCmd);

				// Show attach record menu
				MenuShowItem(RecordRecordAttachRecordCmd);
			}
			// don't set handled = true
			break;

		case winUpdateEvent:
			// Discard name popup
			formP = FrmGetFormPtr(RecordView);

			if (eventP->data.winUpdate.window != FrmGetWindowHandle(formP))
				break;

			PrvViewDrawForm();

			if (sNamePopupDisplayed)
			{
				ToolsDrawFullnamePopup((void*)sFullnameH, &sNamePopupBounds);
			}
			handled = true;
			break;

		case kFrmCustomUpdateEvent:
			// Event sent by Custom view because when custom fields are renamed,
			// it can be necessary to recalculate view screen display: if the width
			// of the custom field is enlarged (and if its content can only be
			// displayed using 2 lines), its content can be displayed on the next line
			lineIndex = sCurrentTabTopLineIndex;
			PrvViewLoadTab(gBookInfo.currentTabId);
			sCurrentTabTopLineIndex = lineIndex;
			ToolsFrmInvalidateWindow(RecordView);
			handled = true;
			break;

		case winResizedEvent:
			formP = FrmGetFormPtr(RecordView);

			if (eventP->data.winResized.window != FrmGetWindowHandle(formP))
				break;

			PrvViewFormResize(formP, &eventP->data.winResized.newBounds);
			handled = true;
			break;

		case nilEvent:
			if (sNamePopupDisplayed && TimGetTicks() >= sNamePopupDisplayTime + SysTimeInMilliSecs(kNamePopupTimeout))
			{
				PrvViewDiscardNamePopup(true);

				formP = FrmGetFormPtr(RecordView);

				// If object focus mode, then draw the focus ring
				if (FrmGetNavState(formP, &navState) >= errNone &&
					(navState & kFrmNavStateFlagsObjectFocusMode) != 0 &&
					FrmGetFocus(formP) == (objIndex = FrmGetObjectIndex(formP, RecordViewName)))
				{
					FrmGetObjectBounds(formP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
				}
			}
			break;
	}

	return (handled);
}
