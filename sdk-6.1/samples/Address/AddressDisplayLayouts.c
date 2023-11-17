/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDisplayLayouts.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              Contains helper functions for AddressBook sample app.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <string.h>
#include <ErrorMgr.h>

#include "Address.h"
#include "AddressTab.h"
#include "AddressTools.h"
#include "AddressDisplayLayouts.h"

/***********************************************************************
 *
 * FUNCTION:    PrvViewLoadAddressLayouts
 *
 * DESCRIPTION: Load address display layouts from resources
 *
 * PARAMETERS:  ->	layoutRscID -	The ID of the resource containing the
 *									layout templates
 *				<-	layoutsPP - on return contains a list of
 *								valid layouts to be used for address display
 *								It's the caller responsability to freed this buffer
 *				<-	numLayoutsP - the number of layouts in layoutsPP
 *
 * RETURNED:    true if successful, false otherwise
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		03/24/03		Initial Revision
 *
 ***********************************************************************/
Boolean PrvLoadRscLayouts(uint16_t layoutsRscID, ViewDisplayLayoutType **layoutsPP, uint32_t *numLayoutsP)
{
	uint32_t				numLayouts;
	uint32_t				rowIndex;
	uint32_t				size;
	MemHandle				blockLayoutsH	= NULL;
	ViewBlockLayoutType *	blockLayoutsP	= NULL;
	ViewRowIdLayoutType *	rowIndexLayoutP	= NULL;
	MemHandle				rowLayoutH		= NULL;
	ViewRowLayoutType *		rowLayoutP		= NULL;
	ViewDisplayLayoutType *	layoutP			= NULL;

	// Load the block layout resource
	if ((blockLayoutsH = DmGetResource(gApplicationDbP, kViewBlockLayoutRscType, layoutsRscID)) == NULL)
		return false;

	blockLayoutsP = DmHandleLock(blockLayoutsH);

	*numLayoutsP = blockLayoutsP->numLayouts;

	if (!blockLayoutsP->numLayouts)
	{
		*layoutsPP = NULL;
		return false;
	}

	size = (uint32_t)(blockLayoutsP->numLayouts * sizeof(ViewDisplayLayoutType));

	*layoutsPP = MemPtrNew(size);
	ErrFatalDisplayIf((*layoutsPP) == NULL, "Out of memory!");

	memset(*layoutsPP, 0, size);

	for (numLayouts = 0; numLayouts < blockLayoutsP->numLayouts; numLayouts++)
	{
		rowIndexLayoutP = &blockLayoutsP->rowIndexLayouts[numLayouts];
		layoutP = &(*layoutsPP)[numLayouts];

		// char encoding
		layoutP->doubleByteEncoding = rowIndexLayoutP->doubleByteEncoding;
		layoutP->numRows = rowIndexLayoutP->numRows;

		// Loop on each row of the layout loaded from resource
		for (rowIndex = 0; rowIndex < layoutP->numRows; rowIndex++)
		{
			// the row layout ID is 0xFFFF which means that it's a blank line
			if (rowIndexLayoutP->rowLayoutIDs[rowIndex] == kLayoutInvalidResID)
			{
				layoutP->rows[rowIndex].numFields = kLayoutBlankLine;
			}
			else
			{
				// We found a valid resID. Try to load it
				if ((rowLayoutH = DmGetResource(gApplicationDbP, kViewRowLayoutRscType, rowIndexLayoutP->rowLayoutIDs[rowIndex])) == NULL)
					continue;

				rowLayoutP = DmHandleLock(rowLayoutH);

				memmove(&layoutP->rows[rowIndex], rowLayoutP, sizeof(ViewRowLayoutType));

				DmHandleUnlock(rowLayoutH);
				DmReleaseResource(rowLayoutH);
			}
		}
	}

	DmHandleUnlock(blockLayoutsH);
	DmReleaseResource(blockLayoutsH);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvGetColumnValue
 *
 * DESCRIPTION: Search for the passed columnID in the record data
 *
 * PARAMETERS:  ->	layoutInfoP: Struct containing info for the layouts
 *				->	columnID: the ID of the wanted column data
 *				<-	dataPP: the pointer to the column data is returned here
 *				<-	sizeP: the dataSize of the column
 *
 * RETURNED:    true if found. false otherwise
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		05/23/03		Initial Revision
 *
 ***********************************************************************/
static Boolean PrvGetColumnValue(ViewDisplayLayoutInfoType *layoutInfoP, uint32_t columnID, void **dataPP, uint32_t *sizeP)
{
	uint32_t	i;
	Boolean		result = false;

	if (dataPP)
		*dataPP = NULL;

	*sizeP = 0;

	for (i = 0; i < layoutInfoP->numColumns; i++)
	{
		if (layoutInfoP->columnValuesP[i].columnID == columnID)
		{
			if (layoutInfoP->columnValuesP[i].errCode == errNone)
			{
				if (dataPP)
					*dataPP = layoutInfoP->columnValuesP[i].data;

				*sizeP = layoutInfoP->columnValuesP[i].dataSize;
				result = true;
			}

			break;
		}
	}

	return result;
}

/***********************************************************************
 *
 * FUNCTION:    AddrDisplayLayoutsInit
 *
 * DESCRIPTION: Search for a suitable layout, based on the info contained
 *				in layoutInfoP. The found layout is returned in layoutInfoP->layout
 *
 * PARAMETERS:  ->	layoutInfoP: Struct containing info for the layouts
 *
 * RETURNED:    true if a suitable layout is found. false otherwise
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		04/11/03		Initial Revision
 *
 ***********************************************************************/
Boolean AddrDisplayLayoutsInit(ViewDisplayLayoutInfoType *layoutInfoP)
{
	uint32_t				numLayouts;
	ViewDisplayLayoutType *	layoutsP = NULL;
	uint32_t				layoutIndex = 0;
	ViewDisplayLayoutType *	foundLayoutP = NULL;
	uint32_t				rowIndex;
	CharEncodingType		charEncoding = 0;
	Boolean					doubleByteEncoding;
	ViewRowLayoutType *		rowP;
	uint32_t				fieldType;
	Boolean					found;
	uint32_t				fieldIndex;
	uint32_t				colIndex;
	Boolean					result = false;
	status_t				err;
	uint32_t				size;
	AddressTabColumnPropertiesType *	columnPropertiesP;

	// Determine char encoding
	doubleByteEncoding = false;

	layoutInfoP->columnValuesP = NULL;
	err = DbGetColumnValues(layoutInfoP->dbP, layoutInfoP->rowID, layoutInfoP->numColumns, layoutInfoP->columnIDsP, &layoutInfoP->columnValuesP);

	if (err < errNone && err != dmErrOneOrMoreFailed)
	{
		if (layoutInfoP->columnValuesP)
			DbReleaseStorage(layoutInfoP->dbP, layoutInfoP->columnValuesP);

		layoutInfoP->columnValuesP = NULL;
		return false;
	}

	for (colIndex = 0; colIndex < layoutInfoP->numColumns && !doubleByteEncoding; colIndex++)
	{
		if (layoutInfoP->columnValuesP[colIndex].errCode >= errNone)
		{
			charEncoding = TxtStrEncoding((char*)layoutInfoP->columnValuesP[colIndex].data);
			doubleByteEncoding = (Boolean) (TxtGetEncodingFlags(charEncoding) == charEncodingHasDoubleByte);
		}
	}

	// Choose a suitable layout
	if (!PrvLoadRscLayouts(layoutInfoP->layoutRscID, &layoutsP, &numLayouts))
		return false;

	layoutIndex = 0;

	while (foundLayoutP == NULL && layoutIndex < numLayouts)
	{
		foundLayoutP = &layoutsP[layoutIndex++];

		// Check char encoding
		// We assume that layouts with specific encoding are place at begining of the resource
		if (!doubleByteEncoding && foundLayoutP->doubleByteEncoding)
		{
			foundLayoutP = NULL;
			continue;
		}

		for (rowIndex = 0; foundLayoutP && rowIndex < foundLayoutP->numRows; rowIndex++)
		{
			rowP = &foundLayoutP->rows[rowIndex];

			// Skip blank line
			if (rowP->numFields == kLayoutBlankLine)
				continue;

			// Search for the field type in the passed address columns
			for (fieldIndex = 0; fieldIndex < rowP->numFields; fieldIndex++)
			{
				fieldType = rowP->fields[fieldIndex].fieldType;
				found = false;

				for (colIndex = 0; colIndex < layoutInfoP->numColumns; colIndex++)
				{
					columnPropertiesP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, layoutInfoP->columnIDsP[colIndex]);
					PrvGetColumnValue(layoutInfoP, layoutInfoP->columnIDsP[colIndex], NULL, &size);

					if (columnPropertiesP && (columnPropertiesP->fieldInfo & fieldType) == fieldType && size > 1)
					{
						found = true;
						break;
					}
				}

				// Reject this layout if the current field type can't be found
				// and if it's mandatory
				if (!found && rowP->fields[fieldIndex].mandatory)
				{
					foundLayoutP = NULL;
					break;
				}
			}
		}
	}

	if (foundLayoutP)
	{
		memmove(&layoutInfoP->layout, foundLayoutP, sizeof(ViewDisplayLayoutType));
		result = true;
	}

	if (layoutsP)
		MemPtrFree(layoutsP);

	return result;
}

/***********************************************************************
 *
 * FUNCTION:    AddrDisplayLayoutsFree
 *
 * DESCRIPTION: Freed the record column data
 *
 * PARAMETERS:  ->	layoutInfoP - layout info
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		05/27/03		Initial Revision
 *
 ***********************************************************************/
void AddrDisplayLayoutsFree(ViewDisplayLayoutInfoType *layoutInfoP)
{
	if (layoutInfoP->columnValuesP)
		DbReleaseStorage(layoutInfoP->dbP, layoutInfoP->columnValuesP);

	layoutInfoP->columnValuesP = NULL;
}

/***********************************************************************
 *
 * FUNCTION:    AddrDisplayLayoutsGetRowText
 *
 * DESCRIPTION: Filled rowTextP with the row content
 *
 * PARAMETERS:  ->	layoutInfoP - layout info
 *				->	rowIndex - The desire row in the layout
 *				<-	rowTextP - The fields text and info for the row
 *
 * RETURNED:    true if succesful, false otherwise
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		04/16/03		Initial Revision
 *
 ***********************************************************************/
Boolean AddrDisplayLayoutsGetRowText(ViewDisplayLayoutInfoType *layoutInfoP, uint16_t rowIndex, ViewDiplayLayoutRowTextType *rowTextP)
{
	uint16_t	fieldIndex;
	Boolean		first;
	uint32_t	size;
	uint16_t	colIndex;
	void *		colDataP;
	size_t		numBytes;
	char **		fieldTextPP;
	uint8_t *	fieldMaxWidthPercentP;
	uint8_t *	fieldClassP;
	uint16_t	numFields;
	char *		strP;
	FontID		currFont;
	Coord		textWidth;

	ViewRowLayoutType *		rowP;
	AddressTabColumnPropertiesType *colPropP = NULL;
	AddressTabColumnPropertiesType **colPropsPP;

	memset(rowTextP, 0, sizeof(ViewDiplayLayoutRowTextType));

	if (rowIndex >= layoutInfoP->layout.numRows)
		return false;

	rowP = &layoutInfoP->layout.rows[rowIndex];

	if (rowP->numFields == kLayoutBlankLine)
	{
		rowTextP->numFields = kLayoutBlankLine;
		return true;
	}

	size = sizeof(char*) * (uint32_t)rowP->numFields * 4;
	fieldTextPP = MemPtrNew(size);
	ErrFatalDisplayIf(fieldTextPP == NULL, "Out of memory!");
	memset(fieldTextPP, 0, size);

	size = sizeof(AddressTabColumnPropertiesType*) * (uint32_t)rowP->numFields * 4;
	colPropsPP = MemPtrNew(size);
	ErrFatalDisplayIf(colPropsPP == NULL, "Out of memory!");
	memset(colPropsPP, 0, size);

	size = sizeof(uint8_t) * (uint32_t)rowP->numFields * 4;
	fieldMaxWidthPercentP = MemPtrNew(size);
	ErrFatalDisplayIf(fieldMaxWidthPercentP == NULL, "Out of memory!");
	memset(fieldMaxWidthPercentP, 0, size);

	size = sizeof(uint8_t) * (uint32_t)rowP->numFields * 4;
	fieldClassP = MemPtrNew(size);
	ErrFatalDisplayIf(fieldClassP == NULL, "Out of memory!");
	memset(fieldClassP, 0, size);

	currFont	= FntSetFont(layoutInfoP->fontID);
	first		= true;
	numBytes	= 0;
	numFields	= 0;
	textWidth	= 0;

	for (fieldIndex = 0; fieldIndex < rowP->numFields; fieldIndex++)
	{
		for (colIndex = 0; colIndex < layoutInfoP->numColumns; colIndex++)
		{
			colPropP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, layoutInfoP->columnIDsP[colIndex]);

			if (colPropP && (colPropP->fieldInfo & rowP->fields[fieldIndex].fieldType) == rowP->fields[fieldIndex].fieldType)
				break;

			colPropP = NULL;
		}

		// Data in this column
		if (colPropP)
		{
			colDataP = NULL;

			if (!PrvGetColumnValue(layoutInfoP, colPropP->columnId, &colDataP, &size) || !colDataP)
				continue;

			// If not first column to be added, check for separator
			if (!first && rowP->fields[fieldIndex].separatorStrId)
			{
				strP = ToolsCopyStringResource(rowP->fields[fieldIndex].separatorStrId);

				if (strP)
				{
					fieldTextPP[numFields] = strP;
					colPropsPP[numFields] = NULL;
					fieldClassP[numFields] = kFieldClass_Separator;
					numBytes += strlen(strP);
					textWidth += FntCharsWidth(strP, strlen(strP));
					numFields++;
				}
			}

			first = false;

			// Check for prefix and display it if needed.
			if (rowP->fields[fieldIndex].prefixStrId)
			{
				strP = ToolsCopyStringResource(rowP->fields[fieldIndex].prefixStrId);

				if (strP)
				{
					fieldTextPP[numFields] = strP;
					colPropsPP[numFields] = NULL;
					fieldClassP[numFields] = kFieldClass_Prefix;
					numBytes += strlen(strP);
					textWidth += FntCharsWidth(strP, strlen(strP));
					numFields++;
				}
			}

			fieldTextPP[numFields] = ToolsStrDup(colDataP);

			ToolsStripNonPrintableChar(fieldTextPP[numFields]);
			size = strlen(colDataP);

			colPropsPP[numFields] = colPropP;
			fieldClassP[numFields] = kFieldClass_Standard;
			numBytes += size;
			textWidth += FntCharsWidth(fieldTextPP[numFields], size);
			fieldMaxWidthPercentP[numFields] = rowP->fields[fieldIndex].maxWidthPercent;
			numFields++;

			// Check for suffix and display it if needed.
			if (rowP->fields[fieldIndex].suffixStrId)
			{
				strP = ToolsCopyStringResource(rowP->fields[fieldIndex].suffixStrId);

				if (strP)
				{
					fieldTextPP[numFields] = strP;
					colPropsPP[numFields] = NULL;
					fieldClassP[numFields] = kFieldClass_Suffix;
					numBytes += strlen(strP);
					textWidth += FntCharsWidth(strP, strlen(strP));
					numFields++;
				}
			}
		}
	}

	rowTextP->fieldTextPP			= fieldTextPP;
	rowTextP->columnPropertiesPP	= colPropsPP;
	rowTextP->fieldMaxWidthPercentP = fieldMaxWidthPercentP;
	rowTextP->fieldClassP			= fieldClassP;
	rowTextP->numFields				= numFields;
	rowTextP->numBytes				= numBytes;
	rowTextP->textWidth				= textWidth;

	FntSetFont(currFont);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    AddrDisplayLayoutsGetDefaultText
 *
 * DESCRIPTION: In case of AddrDisplayLayoutsInit returns false (no layouts found),
 *				you can still call this function to get the text fields in the
 *				rowTextP structure.
 *
 * PARAMETERS:  ->	layoutInfoP - layout info (only user info are used. Not the layout)
 *				<-	rowTextP - The fields text and info for the row
 *
 * RETURNED:    true if succesful, false otherwise
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		04/16/03		Initial Revision
 *
 ***********************************************************************/
Boolean AddrDisplayLayoutsGetDefaultText(ViewDisplayLayoutInfoType *layoutInfoP, ViewDiplayLayoutRowTextType *rowTextP)
{
	size_t		size;
	uint16_t	colIndex;
	char **		fieldTextPP = NULL;
	uint16_t	numFields;
	size_t		numBytes;
	uint16_t	textWidth;
	size_t		textLen;
	FontID		currFont;

	AddressTabColumnPropertiesType *columnPropertiesP = NULL;
	AddressTabColumnPropertiesType **colPropsPP = NULL;

	memset(rowTextP, 0, sizeof(ViewDiplayLayoutRowTextType));
	numFields	= 0;
	numBytes	= 0;
	textWidth	= 0;

	if (!layoutInfoP->numColumns)
		return false;

	size = sizeof(char*) * layoutInfoP->numColumns * 2;

	fieldTextPP = MemPtrNew(size);
	ErrFatalDisplayIf(fieldTextPP == NULL, "Out of memory!");

	memset(fieldTextPP, 0, size);

	size = sizeof(AddressTabColumnPropertiesType*) * layoutInfoP->numColumns * 2;

	colPropsPP = MemPtrNew(size);
	ErrFatalDisplayIf(colPropsPP == NULL, "Out of memory!");

	memset(colPropsPP, 0, size);

	currFont = FntSetFont(layoutInfoP->fontID);

	if (!layoutInfoP->columnValuesP)
		goto ExitErr;

	for (colIndex = 0; colIndex < layoutInfoP->numColumns; colIndex++)
	{
		if (layoutInfoP->columnValuesP[colIndex].errCode >= errNone)
		{
			columnPropertiesP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, layoutInfoP->columnIDsP[colIndex]);

			if (numFields)
			{
				fieldTextPP[numFields] = ToolsStrDup(kDefaultSeparatorString);
				colPropsPP[numFields] = columnPropertiesP;
				numBytes += kDefaultSeparatorLength;
				textWidth += FntCharsWidth(kDefaultSeparatorString, kDefaultSeparatorLength);
				numFields++;
			}

			fieldTextPP[numFields] = ToolsStrDup((char*)layoutInfoP->columnValuesP[colIndex].data);

			ToolsStripNonPrintableChar(fieldTextPP[numFields]);

			colPropsPP[numFields] = columnPropertiesP;
			textLen = strlen(fieldTextPP[numFields]);
			numBytes += textLen;
			textWidth += FntCharsWidth(fieldTextPP[numFields], textLen);
			numFields++;
		}
	}

	rowTextP->fieldTextPP			= fieldTextPP;
	rowTextP->columnPropertiesPP	= colPropsPP;
	rowTextP->numFields				= numFields;
	rowTextP->numBytes				= numBytes;
	rowTextP->textWidth				= textWidth;

	FntSetFont(currFont);

	return true;

ExitErr:
	if (fieldTextPP)
		MemPtrFree(fieldTextPP);

	if (colPropsPP)
		MemPtrFree(colPropsPP);

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    AddrDisplayLayoutsFreeRowText
 *
 * DESCRIPTION: Freed a rowTextP structure
 *
 * PARAMETERS:  ->	rowTextP - The structure to be freed
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *        	Name   	Date      		Description
 *        	----   	----      		-----------
 *			TEs		04/16/03		Initial Revision
 *
 ***********************************************************************/
void AddrDisplayLayoutsFreeRowText(ViewDiplayLayoutRowTextType *rowTextP)
{
	if (rowTextP->fieldTextPP)
	{
		while (rowTextP->numFields--)
		{
			if (rowTextP->fieldTextPP[rowTextP->numFields])
				MemPtrFree(rowTextP->fieldTextPP[rowTextP->numFields]);
		}

		MemPtrFree(rowTextP->fieldTextPP);
		rowTextP->fieldTextPP = NULL;
	}

	if (rowTextP->columnPropertiesPP)
	{
		MemPtrFree(rowTextP->columnPropertiesPP);
		rowTextP->columnPropertiesPP = NULL;
	}

	if (rowTextP->fieldMaxWidthPercentP)
	{
		MemPtrFree(rowTextP->fieldMaxWidthPercentP);
		rowTextP->fieldMaxWidthPercentP = NULL;
	}

	if (rowTextP->fieldClassP)
	{
		MemPtrFree(rowTextP->fieldClassP);
		rowTextP->fieldClassP = NULL;
	}

	rowTextP->numBytes = 0;
	rowTextP->textWidth = 0;
}
