/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDayDetails.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Date Day View Event Details dialog source code.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <AppMgr.h>
#include <CatMgr.h>
#include <Control.h>
#include <ClipBoard.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>
#include <FontSelect.h>
#include <FormLayout.h>
#include <List.h>
#include <Loader.h>
#include <Menu.h>
#include <PenInputMgr.h>
#include <PrivateRecords.h>
#include <Preferences.h>
#include <SelDay.h>
#include <SelTimeZone.h>
#include <SchemaDatabases.h>
#include <ScrollBar.h>
#include <SoundMgr.h>
#include <string.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <SysEvtMgr.h>
#include <SystemResources.h>
#include <Table.h>
#include <TimeMgr.h>
#include <TextMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>

#include "Books.h"
#include "NoteViewLib.h"
#include "PhoneLookup.h"
#include "GUIUtilities.h"

#include "DateGlobals.h"
#include "DateAlarm.h"
#include "DateDB.h"
#include "DateDay.h"
#include "DateDayDetails.h"
#include "DateRepeat.h"
#include "DateUtils.h"
#include "DateU16List.h"
#include "DatebookRsc.h"



/***********************************************************************
 *
 *	Local constants
 *
 ***********************************************************************/
#define kFocusDetailsNoFocus noFocus


/***********************************************************************
 *
 *	Local definitions, shared with DateDay.c, declared in DateDayDetails.h
 *
 ***********************************************************************/
uint32_t		gDetailsPosition		= 0;
uint32_t		gDetailsSelectionLength	= 0;
int32_t			gLocationFieldMaxLength = 0;


/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
static FormType*		sFormP = NULL;
static char 			sDetailsTimeZoneString[TZNAME_MAX]; 	// Event details time zone string
static BookType*		sDetailsBook = NULL;
static uint16_t 		sDefaultEventDetailsTabId = DetailsBookEventInformationTabId;

static uint16_t 		sDetailsFocusState		= kFocusDetailsNoFocus;
static Boolean			sDetailsTabsFree 		= false;
static Boolean			sAlarmIsSet				= false;
static TabInfoType*		sDetailsTabs 			= NULL;
static TabInfoType		sDefaultDetailsTabs[2] 	=
{
	{ "Options",  },
	{ "Event Information", }
};


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:    	SplitRepeatingEvent
 *
 * DESCRIPTION:	 	This routine splits a repeating appointment into two
 *              	repeating appointment.  The orginal record is
 *              	copied and the end date of the new record is set to
 *              	yesterday or the day before the current date, which
 *					ever earlier.
 *
 * PARAMETERS:  	cursorID: CursorID set on plitted event.
 *
 * RETURNED:    	error code
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	4/25/96		Initial revision.
 *
 ***********************************************************************/
status_t SplitRepeatingEvent (uint32_t cursorID)
{
	status_t			err = errNone;
	uint32_t 			newRowID = dbInvalidRowID;
	uint32_t 			oldRowID;
	DateType			endDate;
	DateType			repeatDate;
 	RepeatInfoType		repeat;
	ApptDBRecordType	apptRec;
	uint32_t 			numCategories;
	CategoryID *		pCategoryIDs;
	uint16_t 			initialAttributes;
	uint16_t			attributes;

	if (!DbIsCursorID(cursorID))
	{
		ErrNonFatalDisplay ("CreateException must be given a cursor ID (not a rowID)");
		return dmErrInvalidParam;
	}

	// Save initial appointment rowID
	DbCursorGetCurrentRowID(cursorID, &oldRowID);

	// Get yesterday's date or the date of the day before the current date,
	// which ever earlier.
	endDate = Date;
	DateAdjust (&endDate, -1);

	ApptGetRecord (ApptDB, cursorID, &apptRec, DBK_SELECT_ALL_FIELDS);
	DbGetCategory (ApptDB, cursorID, &numCategories, &pCategoryIDs);

	// Check for past occurrences of the event.
	repeatDate = apptRec.when.date;
	ApptNextRepeat (&apptRec, &repeatDate, true);
	if (DateToDays (repeatDate) <= DateToDays (endDate))
	{
		DbGetRowAttr(ApptDB, cursorID, &initialAttributes);

		repeat = apptRec.repeat;
		repeat.repeatEndDate = endDate;
		apptRec.repeat = repeat;

		err = ApptDuplicateRecord(
				ApptDB,
				cursorID,
				DBK_SELECT_TIME | DBK_SELECT_REPEAT, 	// changed fields (field to be changed from clone)
				0, 										// unwanted field (field to be removed from clone)
				&apptRec,
				&newRowID);

		if (err >= errNone)
		{
			// update the cursor to have the created record in
			ApptDBRequery( ApptDB, cursorID, true);

			// set cursor on new record
			DbCursorMoveToRowID(cursorID, newRowID);

			DbSetCategory(ApptDB, cursorID, numCategories, pCategoryIDs);

			// Set the secret attribute
			if (initialAttributes & dmRecAttrSecret)
			{
				DbGetRowAttr(ApptDB, cursorID, &attributes);
				attributes |= dmRecAttrSecret;
				DbSetRowAttr (ApptDB, cursorID, &attributes);
			}

			// Reload the appointments from DB to reload the newly inserted event
			DayViewLoadApptsFromDB();
		}
	}

	ApptFreeRecordMemory (&apptRec);
	DbReleaseStorage(ApptDB, pCategoryIDs);

	// Restore initial appointment
	DbCursorMoveToRowID(cursorID, oldRowID);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    	DeleteRecord
 *
 * DESCRIPTION: 	This routine deletes the specified appointment record.
 *
 * PARAMETERS:  	rowID:	rowID / CursorID of record being deleted
 *
 * RETURNED:    	true if the record was deleted, false if the delete
 *              	operation was canceled.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	8/1/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
Boolean DeleteRecord (uint32_t rowID)
{
	status_t	 			err = errNone;
	uint16_t 				ctlIndex;
	size_t					scndLineStart;
	size_t					lineLength;
	size_t					descNewSize;
	size_t					apptTemplateLen;
	size_t					initialQuestionLen;
	uint16_t	 			alertButton = RangeAllButton;
	FormPtr 				alert;
	Boolean					archive;
	Boolean					exception = false;
	ApptDBRecordType	 	apptRec;
	RepeatInfoType 			repeatInfo;
	DateRangeType 			dateRange = dateRangeAll;
	Boolean					hasAlarm;
	char *					deleteEventQuestionStr;
	char *					apptQuestionTemplateStr;
	MemHandle				apptQuestionTemplateH;
	FieldPtr				fldP;
	char *					descP;
	MemHandle				unnamedApptStrH = NULL;
	RecordFilteringInfoType	changedFields;


	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if an exception is being created.
	ApptGetRecord (ApptDB, rowID, &apptRec,
		DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS | DBK_SELECT_DESCRIPTION);

	descP = apptRec.description;

	// If the description is empty, get default name from resources
	if (descP == NULL || descP[0] == '\0')
	{
		unnamedApptStrH = DmGetResource (gApplicationDbP, strRsc, unnamedAppointmentStrID);
		descP = MemHandleLock(unnamedApptStrH);
	}

	if ((apptRec.repeat.repeatType != repeatNone) && ApptHasMultipleOccurences(&apptRec))
	{
		alert = FrmInitForm(gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		// If the alert was canceled don't delete the record.
		if (alertButton == RangeCancelButton)
		{
			dateRange = dateRangeNone;
		}

		else if (alertButton == RangeCurrentButton)
		{
			dateRange = dateRangeCurrent;
			exception = true;
		}

		else if (alertButton == RangeFutureButton)
		{
			// When editing the first instance, deleting "this and future"
			// will cause the event to disappear. Modify the dialog result
			// to trigger the confirmation dialog.
			if (DateToDays(apptRec.when.date) != DateToDays(Date))
			{
				dateRange = dateRangeCurrentAndFuture;
				repeatInfo = apptRec.repeat;
			}
			else
			{
				dateRange = dateRangeAll;
			}
		}
	}

	hasAlarm = (Boolean) (apptRec.alarm.advance != apptNoAlarm);

	switch (dateRange)
	{
		// ignore dateRangeNone

		case dateRangeCurrent:
			// if the selected event had an alarm, be sure to remove it
			// from the posted alarm queue before the event is changed.
			if (hasAlarm)
				DeleteAlarmIfPosted(rowID);

			// Add an exception to the current record.
			err = ApptAddException (ApptDB, rowID, Date);

			if ((err >= errNone) && (ApptCountMultipleOccurences(&apptRec) - 1 == 1))
			{
				DateType	startDate;

				ApptFreeRecordMemory (&apptRec);
				ApptGetRecord (ApptDB, rowID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);
				startDate = apptRec.when.date;

				if (ApptNextRepeat (&apptRec, &startDate, true))
				{
					apptRec.when.date = startDate;
					ApptRemoveExceptions(&apptRec);
					apptRec.repeat.repeatType = repeatNone;
					err = ApptChangeRecord (ApptDB, rowID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);
				}
				else DbgOnlyFatalError("Could not find exception");
			}
			break;

		case dateRangeCurrentAndFuture:
			// if the selected event had an alarm, be sure to remove it
			// from the posted alarm queue before the event is changed.
			if (hasAlarm)
				DeleteAlarmIfPosted(rowID);

			// Clip the meeting's end date to just before the displayed date
			repeatInfo.repeatEndDate = Date;
			DateAdjust (&repeatInfo.repeatEndDate, -1);

			// Fill part of apptRec with the new repeat info and mark the field
			// as modified. Only marked fields will be used by ApptChangeRecord.
			apptRec.repeat = repeatInfo;

			changedFields = DBK_SELECT_REPEAT;

			if (ApptCountMultipleOccurences(&apptRec) == 1)
			{
				ApptRemoveExceptions(&apptRec);
				apptRec.repeat.repeatType = repeatNone;
				changedFields |= DBK_SELECT_EXCEPTIONS;
			}

			err = ApptChangeRecord (ApptDB, rowID, &apptRec, changedFields);
			break;

		case dateRangeAll:
			// Display an alert to comfirm the delete operation.
			alert = FrmInitForm(gApplicationDbP, DeleteApptDialog);

			ctlIndex = FrmGetObjectIndex (alert, DeleteApptSaveBackup);
			FrmSetControlValue (alert, ctlIndex, SaveBackup);

			// Get the template string from resources
			apptQuestionTemplateH = DmGetResource (gApplicationDbP, strRsc, DeleteApptQuestionStrID);
			apptQuestionTemplateStr = (char *) MemHandleLock (apptQuestionTemplateH);

			deleteEventQuestionStr = TxtParamString(apptQuestionTemplateStr, descP, NULL, NULL, NULL);
			initialQuestionLen = strlen(deleteEventQuestionStr);

			fldP = FrmGetObjectPtr (alert, FrmGetObjectIndex (alert, DeleteApptQuestionField));
			FldSetTextPtr(fldP, deleteEventQuestionStr);
			FldRecalculateField(fldP, false);

			if (FldGetLineInfo(fldP, 1, &scndLineStart, &lineLength) && (scndLineStart + lineLength < initialQuestionLen))
			{
				uint32_t		desc1Length;
				uint32_t		desc2Length;
				size_t			offset;
				RectangleType	rect;
				Coord			desc2Width;
				Coord			remainingSpace;
				Boolean			fitWithinWidth;
				FontID			saveFont;
				char			apptQuestionAbbreviatedDescStr[DeleteApptMaxQuestionLength+1];

				// Description is too long. Need to truncat it.

				saveFont = FntSetFont(FldGetFont(fldP));
				FldGetBounds(fldP, &rect);
				FldSetTextPtr(fldP, NULL);
				MemPtrFree(deleteEventQuestionStr);

				// Compute template length before and after the param str (^0)
				apptTemplateLen = strlen(apptQuestionTemplateStr);
				deleteEventQuestionStr = StrChr(apptQuestionTemplateStr, '^');
				desc1Length = deleteEventQuestionStr - apptQuestionTemplateStr;
				offset = desc1Length;
				offset += TxtGetNextChar(apptQuestionTemplateStr, offset, NULL);	// skip '^'
				offset += TxtGetNextChar(apptQuestionTemplateStr, offset, NULL);	// skip '0'
				desc2Length = apptTemplateLen - offset;

				// Compute the length needed by the end of the template string.
				desc2Width = FntCharsWidth(apptQuestionTemplateStr + offset, desc2Length);
				remainingSpace = rect.extent.x - desc2Width;

				// How many char could be displayed from the description in the remaining space.
				offset = scndLineStart - desc1Length;
				descNewSize = strlen(descP + offset);
				FntCharsInWidth(descP + offset, &remainingSpace, &descNewSize, &fitWithinWidth);
				TxtTruncateString(apptQuestionAbbreviatedDescStr, descP, offset + descNewSize, true);
				deleteEventQuestionStr = TxtParamString(apptQuestionTemplateStr, apptQuestionAbbreviatedDescStr, NULL, NULL, NULL);
				FldSetTextPtr(fldP, deleteEventQuestionStr);
				FntSetFont(saveFont);
			}

			alertButton = FrmDoDialog (alert);
			archive = (Boolean) FrmGetControlValue (alert, ctlIndex);
			FldSetTextPtr(fldP, NULL);

			// FrmDeleteForm will delete the question allocated by TxtParamString
			FrmDeleteForm (alert);
			MemHandleUnlock(apptQuestionTemplateH);
		   	DmReleaseResource(apptQuestionTemplateH);
			MemPtrFree(deleteEventQuestionStr);

			if (alertButton == DeleteApptCancel)
			{
				dateRange = dateRangeNone;
			}
			else
			{
				SaveBackup = archive;

				// if the event to be deleted had an alarm, be sure to remove it
				// from the posted alarm queue before the event is deleted.
				if (hasAlarm)
					DeleteAlarmIfPosted(rowID);

				// Delete or archive the record
				err = ApptDeleteRecord(ApptDB, rowID, archive);

				DbgOnlyFatalErrorIf(err < errNone, "Could not delete record");
			}
			break;
	}

	// Release apptRec structure
	ApptFreeRecordMemory (&apptRec);

	if (dateRange != dateRangeNone)
	{
		if (err < errNone)
		{
			FrmUIAlert(DeviceFullAlert);
			return false;
		}

		// If the event to be delete had an alarm, be sure to remove it
		// from the posted alarm queue before the event is deleted and
		// reschedule the next alarm.
		if (hasAlarm)
			RescheduleAlarmsAllRows();
	}

	if (unnamedApptStrH)
	{
		// Release the resource
		MemHandleUnlock(unnamedApptStrH);
		DmReleaseResource(unnamedApptStrH);
	}

	return (Boolean) (dateRange != dateRangeNone);
}


/***********************************************************************
 *
 * FUNCTION:    	CreateExceptionAndMove
 *
 * DESCRIPTION: 	This routine creates an exception record.
 *					It does not add the exception to the exceptions list
 *					of the original record.
 *
 * PARAMETERS:  	newRec:			new record.
 *              	cursorID:		CursorID of record being excepted.
 *					unwantedFilter:	unwanted fields, repeat and exceptions aside
 *					numCategories: 	how many categories.
 *					pCategoryIDs:	categories set.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	This Function is called as a result of splitting an
 *					occurence of a repeating event.
 *					two case:
 *					1 - called whithin the week view :
 *						the cursor date ranges is already set as the
 *						week view allow to move an event within the week
 *					2 - called from DateApply from Day View
 *						the cursor have to be set with correct date ranges
 *						in order to include the new recortd that is going
 *						to be created.
 *
 *
 * REVISION HISTORY:
 *	art	11/22/95	Initial Revision.
 *
 ***********************************************************************/
status_t CreateExceptionAndMove (
	ApptDBRecordPtr newRec,
	uint32_t 		cursorID,
	RecordFilteringInfoType unwantedFilter,
	uint32_t 		numCategories,
	CategoryID 		pCategoryIDs[])
{
	uint32_t		newRowID = dbInvalidRowID;
	status_t 		err;
	uint16_t 		attributes;

	if (!DbIsCursorID(cursorID))
	{
		ErrNonFatalDisplay ("CreateException must be given a cursor ID (not a rowID)");
		return dmErrInvalidParam;
	}

	// Get the secret setting of the record.
	err = DbGetRowAttr(ApptDB, cursorID, &attributes);

	err = ApptDuplicateRecord(
			ApptDB,
			cursorID,
			(DBK_SELECT_TIME | DBK_SELECT_ALARM),			// changed fields (field to be changed from clone)
			(DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS | unwantedFilter), 	// unwanted field (field to be removed from clone)
			newRec,
			&newRowID);

	if (err >= errNone)
	{
		// update the cursor to have the created record in
		ApptDBRequery(ApptDB,  cursorID, true);

		// set cursor on new record
		err = DbCursorMoveToRowID(cursorID, newRowID);

		// Assign the record category
		err = DbSetCategory(ApptDB, cursorID, numCategories, pCategoryIDs);

		if (attributes & dmRecAttrSecret)
		{
			err = DbGetRowAttr(ApptDB, cursorID, &attributes);
			attributes |= dmRecAttrSecret;
			err = DbSetRowAttr (ApptDB, cursorID, &attributes);
		}
	}
	return err;
}


/***********************************************************************
 *
 * FUNCTION:    	CreateNote
 *
 * DESCRIPTION: 	This routine adds an empty note to the current record.
 *
 * PARAMETERS:  	prompt:	if true, ask if an exception should be created.
 *
 * RETURNED:    	true if the note was added, false if it was not.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	8/4/95		Initial revision.
 *
 ***********************************************************************/
Boolean CreateNote (Boolean prompt)
{
	status_t 				err = errNone;
	FormPtr 				alert;
	uint16_t 				alertButton;
	Boolean 				exception = false;
	Boolean 				splitEvent = false;
	ApptDBRecordType 		apptRec;
	ApptDBRecordType 		newRec;
	RecordFilteringInfoType	changedFields;
	Boolean 				repeat;
	uint32_t 				numCategories;
	CategoryID *			pCategoryIDs;
	Boolean					result = false;

	if (gApptCursorID == dbInvalidCursorID)
		return false;

	DbGetCategory (ApptDB, gApptCursorID, &numCategories, &pCategoryIDs);

	// If the record already has a note, exit
	if (ApptEventHasNote(ApptDB, gApptCursorID))
	{
		// want to put the note view form in wiewing mode, aia condensed
		result = true;
		goto Exit;
	}

	// If we're changing a repeating appointment, check if all occurrences
	// are being changed, current & future occurrences are being changed,
	// or if and exception is being created.
	repeat = (Boolean) ApptEventRepeatIsSet(ApptDB, gApptCursorID);
	if (repeat && prompt)
	{
		alert = FrmInitForm(gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		if (alertButton == RangeCancelButton)
			goto Exit;

		else if (alertButton == RangeCurrentButton)
			exception = true;

		else if (alertButton == RangeFutureButton)
			splitEvent = true;
	}

	if (exception)
	{
		// Add an exception to the current record.
		err = ApptAddException (ApptDB, gApptCursorID, Date);
		if (err < errNone)
			goto Exit;

		// Create a new record on the current day that contains
		// the same description, time, and alarm setting as the
		// repeating event. Do not load the note
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec,
			DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_DESCRIPTION | DBK_SELECT_LOCATION);

		memset (&newRec, 0x00, sizeof (newRec));
		newRec.when = apptRec.when;
		newRec.when.date = Date;
		newRec.description = apptRec.description;
		newRec.location = apptRec.location;
		newRec.alarm = apptRec.alarm;
		newRec.note = (char *)"";

		err = CreateExceptionAndMove (&newRec, gApptCursorID, NULL, numCategories, pCategoryIDs);

		ApptFreeRecordMemory (&apptRec);
		if (err < errNone)
			goto Exit;
	}
	else
	{
		// Clear all changed fields flags.
		changedFields = 0;
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_TIME);

		if (splitEvent)
		{
			// Split off the previous occurrences of the event
			err = SplitRepeatingEvent (gApptCursorID);
			if (err < errNone)
			{
				ApptFreeRecordMemory (&apptRec);
				goto Exit;
			}

			// Set the new start date for the event
			memset (&newRec, 0x00, sizeof (newRec));
			newRec.when = apptRec.when;
			newRec.when.date = Date;
			changedFields |= DBK_SELECT_TIME;

			err = ApptChangeRecord (ApptDB, gApptCursorID, &newRec, changedFields);
		}

		ApptFreeRecordMemory (&apptRec);
		if (err < errNone)
			goto Exit;
	}

	result = true;

Exit:
	DbReleaseStorage(ApptDB, pCategoryIDs);
	return result;
}


/***********************************************************************
 *
 * FUNCTION:    	DeleteNote
 *
 * DESCRIPTION: 	This routine deletes the note field from an
 *              	appointment record.
 *
 * PARAMETERS:  	exception:	true if a new record, an exception,
 *								should be created.
 *					splitEvent:	true if only deleting note from
 *								current and future occurrences of
 *								the specified event.
 *
 * RETURNED:    	true if the note was deleted.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	10/13/95	Initial revision.
 *
 ***********************************************************************/
Boolean DeleteNote (Boolean exception, Boolean splitEvent)
{
	status_t 				err;
	ApptDBRecordType 		newRec;
	ApptDBRecordType 		apptRec;
	RecordFilteringInfoType	changedFields;
	uint32_t 				numCategories;
	CategoryID *			pCategoryIDs;


	if (exception)
	{
		// Add an exception to the current record.
		err = ApptAddException (ApptDB, gApptCursorID, Date);
		if (err < errNone)
			goto Exit;


		// Create a new record on the current day that contains
		// the same description, time, and alarm settings as the
		// repeating event, but not the note.
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec,
			DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_DESCRIPTION | DBK_SELECT_LOCATION);

		DbGetCategory (ApptDB, gApptCursorID, &numCategories, &pCategoryIDs);

		memset (&newRec, 0x00, sizeof (newRec));
		newRec.when = apptRec.when;
		newRec.when.date = Date;
		newRec.description = apptRec.description;
		newRec.location = apptRec.location;
		newRec.alarm = apptRec.alarm;

		err = CreateExceptionAndMove (&newRec, gApptCursorID, DBK_SELECT_NOTE, numCategories, pCategoryIDs);

		ApptFreeRecordMemory (&apptRec);
		DbReleaseStorage(ApptDB, pCategoryIDs);

		if (err < errNone)
			goto Exit;
	}

	else
	{
		// Clear all changed fields flags.
		changedFields = 0;
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_TIME);

		if (splitEvent)
		{
			// Split off the previous occurrences of the event
			err = SplitRepeatingEvent (gApptCursorID);
			if (err < errNone)
			{
				ApptFreeRecordMemory (&apptRec);
				goto Exit;
			}

			// Set the new start date for the event
			newRec.when = apptRec.when;
			newRec.when.date = Date;
			changedFields |= DBK_SELECT_TIME;
		}

		err = ApptChangeRecord (ApptDB, gApptCursorID, &newRec, changedFields);
		ApptFreeRecordMemory (&apptRec);

		// Delete the note
		ApptDeleteNote(ApptDB, gApptCursorID);

		if (err < errNone)
			goto Exit;
	}
	return true;

Exit:
	FrmUIAlert(DeviceFullAlert);
	return false;
}

/***********************************************************************
 *
 * FUNCTION:    	NoteViewDeleteNote
 *
 * DESCRIPTION: 	Called as a callback from the NoteViewLib if the user
 *					wants to delete an already existing note.
 *
 * PARAMETERS:  	dbRef: 		database  reference.
 *					rowID: 		rowID or cursorID pointing on the edited row.
 *					pos: 		cursor position for the edited row.
 *					columnId: 	the note column ID
 *
 * RETURNED:    	true if the note was deleted.
 *					false if the note view has to stay active.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	8/4/95		Initial revision.
 *
 ***********************************************************************/
Boolean NoteViewDeleteNote (DmOpenRef dbRef, uint32_t rowID, uint32_t pos, uint32_t columnID)
{
	FormPtr 			alert;
	UInt16 				alertButton;
	Boolean 			exception = false;
	Boolean 			splitEvent = false;
	Boolean 			noteDeleted = false;
	ApptDBRecordType 	apptRec;

	ErrNonFatalDisplayIf(columnID != EVENT_NOTE_ID, "Delete note: invalid column ID.");
	ErrNonFatalDisplayIf(DbIsCursorID(rowID) && (rowID != gApptCursorID), "Delete note: invalid cursorID / rowID.");

	if (DbIsCursorID(rowID))
		DbCursorSetAbsolutePosition(rowID, pos);

	ApptGetRecord (dbRef, rowID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT);

	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if and exception is being created.
	if (apptRec.repeat.repeatType != repeatNone)
	{
		alert = FrmInitForm (gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		if (alertButton == RangeCancelButton)
			goto Exit;

		else if (alertButton == RangeCurrentButton)
			exception = true;

		else if (alertButton == RangeFutureButton)
			splitEvent = true;
	}

	// Confirm the operation.
	else if (FrmAlert(gApplicationDbP, DeleteNoteAlert) != DeleteNoteYes)
		goto Exit;

	// Remove the note field form the record.
	if (DeleteNote (exception, splitEvent))
	{
		// The deleted note may create an empty record - We need to remove it if so
		ApptDeleteRecordIfEmpty(ApptDB, gApptCursorID);
		noteDeleted = true;
		goto Exit;
	}

	// If we were unable to delete the note, restore the state of the
	// note view.  Deletes can fails if the appointment repeats and
	// there not enough space to create an exception.
	DayViewRefreshDisplay (false, true);

Exit:
	// Free the appointment record links
	ApptFreeRecordMemory (&apptRec);
	return noteDeleted;
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsEditNote
 *
 * DESCRIPTION: 	This routine calls the shared lib NoteViewLib exported
 *					routine called EditNote
 *
 * PARAMETERS:  -> 	rowID : rowID / CursorID of the record to edit note.
 *
 * RETURNED:    	nothing
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	Ple	01/01/03	Initial revision.
 *
 ***********************************************************************/
void DetailsEditNote (uint32_t rowID)
 {
	ApptDBRecordType	apptRec;
	char * 				descP;
	MemHandle			unnamedApptStrH = 0;
	uint32_t			pos = 0;

	if (DbIsCursorID(rowID))
		DbCursorGetCurrentPosition(rowID, &pos);

	// Get current record and related info.
	ApptGetRecord (ApptDB, rowID, &apptRec, DBK_SELECT_DESCRIPTION);

	if (apptRec.description && strlen(apptRec.description) > 0)
	{
		// Get note title from description
		descP = apptRec.description;
	}
	else
	{
		// Lock the resource
		unnamedApptStrH = DmGetResource (gApplicationDbP, strRsc, unnamedAppointmentStrID);
		descP = MemHandleLock(unnamedApptStrH);
	}

	// Edit the note, give it the delete call back
	EditNote(descP, ApptDB, rowID, pos, EVENT_NOTE_ID, NoteViewDeleteNote);

	if (unnamedApptStrH)
	{
		// Release the resource
		MemHandleUnlock(unnamedApptStrH);
		DmReleaseResource(unnamedApptStrH);
	}

	// Free the appointment record links
	ApptFreeRecordMemory (&apptRec);
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsAlarmOnOff
 *
 * DESCRIPTION: 	This routine shows or hides the alarm advance ui object.
 *              	It is called when the alarm checkbox is turn on or off.
 *
 * PARAMETERS:  	on:	true to show object, false to hide
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	7/21/95		Initial revision.
 *
 ***********************************************************************/
static void DetailsAlarmOnOff (Boolean on)
 {
 	uint16_t 	advance;
 	uint16_t 	advanceUnit;
 	char* 		textP;
 	char* 		label;
 	MemHandle 	textH;
 	FormPtr 	frm;
 	ListPtr 	lst;
 	FieldPtr 	fld;
 	ControlPtr 	ctl;

	frm = FrmGetFormPtr (DetailsDialog);
	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DetailsAlarmAdvanceField));
	sAlarmIsSet = on;

 	if (on)
 	{
		if (AlarmPreset.advance != apptNoAlarm)
		{
			advance = AlarmPreset.advance;
			advanceUnit = AlarmPreset.advanceUnit;
		}
		else
		{
			advance = defaultAlarmAdvance;
			advanceUnit = defaultAdvanceUnit;
		}

		// Set the value of the alarm advance field.
		textH = FldGetTextHandle (fld);
		if (textH)
		{
			MemHandleFree (textH);
		}
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, advance);
		MemPtrUnlock (textP);

		FldSetTextHandle (fld, textH);

		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DetailsAlarmAdvanceList));
		LstSetSelection (lst, advanceUnit);
		label = LstGetSelectionText (lst, advanceUnit);

		ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DetailsAlarmAdvanceSelector));
		CtlSetLabel (ctl, label);
		ShowObject (frm, DetailsAlarmAdvanceSelector);

		// Show the alarm advance ui objects.
 		ShowObject (frm, DetailsAlarmAdvanceField);
 		ShowObject (frm, DetailsAlarmAdvanceSelector);
 	}
 	else
 	{
		FldFreeMemory (fld);

		CtlSetValue (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DetailsAlarmCheckbox)), false);

		// Hide the alarm advance ui objects.
 		HideObject (frm, DetailsAlarmAdvanceField);
 		HideObject (frm, DetailsAlarmAdvanceSelector);
 	}
 }

/***********************************************************************
 *
 * FUNCTION:    	DetailsSetDateTrigger
 *
 * DESCRIPTION: 	This routine sets the label of the trigger in a details
 *              	dialog that displays the appointment's date
 *
 * PARAMETERS:  	date:	appointment date.
 *
 * RETURNED:    	Nothing.
 *
 * NOTES:			None.
 *
 * REVISION HISTORY:
 *	PLe	06/21/02	Initial revision.
 *	PPL	01/09/03	Add the midnight spanning to the next day.
 *
 ***********************************************************************/
static void DetailsSetDateTrigger (DateType date, Boolean midnightWrapped)
{
	ControlPtr 	ctl;
	char*		label;
	char*		str;
	FormPtr 	frmP;

	// Set the label of the date selector.
	frmP = FrmGetFormPtr (DetailsDialog);
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsDateSelector));
	str = label = (char *)CtlGetLabel (ctl);	// OK to cast; we call CtlSetLabel

	if (midnightWrapped)
	{
		DateToAscii ((uint8_t) date.month, (uint8_t) date.day, (uint16_t) (date.year + firstYear),
			ShortDateFormat, str);

		str += strlen (str);
		*str++ = spaceChr;
		*str++ = '-';
		*str++ = spaceChr;

		DateAdjust(&date, 1);
		DateToAscii ((uint8_t) date.month, (uint8_t) date.day, (uint16_t) (date.year + firstYear),
			ShortDateFormat, str);
	}
	else
		DateToDOWDMFormat ((uint8_t) date.month, (uint8_t) date.day, (uint16_t) (date.year + firstYear),
			ShortDateFormat, label);

	CtlSetLabel (ctl, label);
}



/***********************************************************************
 *
 * FUNCTION:    	DetailsPlaceStartToEndItems
 *
 * DESCRIPTION: 	This function calculate horizontal spacing of:
 *					- StartTime text selector trigger,
 *					- To Label
 *					- EndTime text selector trigger.
 *
 *					Only to label and endTime text selector trigget will
 *					have to move.
 *
 *					To keep the look of the frame of text selector trigger
 *					the x position of "to" and "endTime" item is adjusted
 *					to be even.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTES:			None.
 *
 * REVISION HISTORY:
 *	ppl	30/04/03	Initial revision.
 *
 ***********************************************************************/
static void DetailsPlaceStartToEndItems(void)
{
	RectangleType	bound;
	Coord			startWidth;
	Coord			toWidth;
	Coord			startLeft;
	Coord			startTop;
	Coord			toLeft;
	Coord			toTop;
	Coord			endLeft;
	Coord			endTop;
	uint16_t		startIndex;
	uint16_t		toIndex;
	uint16_t		endIndex;
	FormPtr 		frmP;

	frmP = FrmGetFormPtr (DetailsDialog);

	startIndex = FrmGetObjectIndex (frmP, DetailsStartTimeSelector);

	// "start time" text selector trigger
	// retrieve "starttime" width and position
	FrmGetObjectBounds(frmP, startIndex, &bound);
	FrmGetObjectPosition(frmP, startIndex, &startLeft, &startTop);
	startWidth = bound.extent.x;

	// "to" label
	// retrieve "to" width and position
	toIndex = FrmGetObjectIndex (frmP, DetailsToEndTimeLabel);
	FrmGetObjectBounds(frmP, toIndex, &bound);
	FrmGetObjectPosition(frmP, toIndex, &toLeft, &toTop);
	toWidth = bound.extent.x;

	// "End time" text selector trigger
	// retrieve position and bounds
	endIndex = FrmGetObjectIndex (frmP, DetailsEndTimeSelector);
	FrmGetObjectPosition(frmP, endIndex, &endLeft, &endTop);

	// calculate "to" label Left potential position
	toLeft = startLeft + startWidth + DetailsTimeItemsSpacing;

	// calculate  "end time" text selector trigger left potential
	endLeft = toLeft + toWidth + DetailsTimeItemsSpacing;

	// adjust horizontal parity to havenice pattern in text selector trigger frame
	// "marching ants effect"
	toLeft += (endLeft % 2);
	endLeft += (endLeft % 2);

	// move "to" label
	FrmSetObjectPosition(frmP, toIndex, toLeft, toTop);

	// move end time text selector trigger
	FrmSetObjectPosition(frmP, endIndex, endLeft, endTop);
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsSetTimeTrigger
 *
 * DESCRIPTION: 	This routine sets the label of the trigger in a details
 *              	dialog that displays the appointment's start and end
 *              	time.
 *
 * PARAMETERS:  ->	end: start time or minus one if no-time event
 *              ->	noTimeEvent: true if no time event
 *
 * RETURNED:    	Nothing.
 *
 * NOTES:			This routine assumes that the memory allocated for
 *					the label of the time trigger is large enough to hold
 *					the largest posible label.  This label's memory is
 *					reserved by initializing the label in the resource file.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *	ppl	01/09/03	Split for end time selector trigger.
 *
 ***********************************************************************/
static void DetailsSetTimeTrigger (TimeType theTime, Boolean noTimeEvent, uint16_t selectorID)
{
	char* 		label;
	ControlPtr 	ctl;
	MemHandle 	rscH;
	char* 		rscP;
	FormPtr 	frmP;

	frmP = FrmGetFormPtr (DetailsDialog);
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, selectorID));
	label = (char *) CtlGetLabel (ctl);	// OK to cast; we call CtlSetLabel

	if (noTimeEvent)
	{
		rscH = DmGetResource (gApplicationDbP, strRsc, noTimeStrID);
		rscP = MemHandleLock (rscH);
		strcpy (label, rscP);
		MemPtrUnlock (rscP);
	   	DmReleaseResource(rscH);
	}

	else
	{
		TimeToAscii ((uint8_t)theTime.hours, (uint8_t)theTime.minutes, TimeFormat, label);
	}

	CtlSetLabel (ctl, label);
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsSetTimeTriggers
 *
 * DESCRIPTION: 	This routine sets the label of the trigger in a details
 *              	dialog that displays the appointment's start and end
 *              	time.
 *
 * PARAMETERS:  <->	detailsP: 	current details.
 *				->	end: 		start time or minus one if no-time
 *								event.
 *              ->	noTimeEvent: true if no time event.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			This routine assumes that the memory allocated for the
 *					label of the time trigger is large enough to hold the
 *					largest posible label.  This label's memory is reserved
 *					by initializing the label in the resource file.
 *
 *					no time event do not have time zone assigned:
 *					detailsP->when.timeZoneName[0] == chrNull;
 *
 *					normal timed event are created with the current device
 *					time zone (gDeviceTimeZone)
 *
 *
 * REVISION HISTORY:
 *	PLe	07/30/03	Initial revision.
 *	PPL	09/02/03	Add detailsP, and set /reset of time zone
 *
 ***********************************************************************/
static void DetailsSetTimeTriggers (DetailsPtr detailsP, TimeType start, TimeType end, Boolean noTimeEvent)
{
	FormPtr 	frmP;

	frmP = FrmGetFormPtr (DetailsDialog);
	DetailsSetTimeTrigger(start, noTimeEvent, DetailsStartTimeSelector);

	// Do not show end time trigger if untimed
	if (noTimeEvent)
	{
		UtilitiesFormHideObjectID (frmP, DetailsToEndTimeLabel);
		UtilitiesFormHideObjectID (frmP, DetailsEndTimeSelector);

		detailsP->when.timeZoneName[0] = chrNull;
	}
	else
	{
		UtilitiesFormShowObjectID (frmP, DetailsToEndTimeLabel);
		UtilitiesFormShowObjectID (frmP, DetailsEndTimeSelector);
		DetailsPlaceStartToEndItems();

		DetailsSetTimeTrigger(end, noTimeEvent, DetailsEndTimeSelector);

		if (detailsP->when.timeZoneName[0] == chrNull)
		{
			strncpy(detailsP->when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsSelectDate
 *
 * DESCRIPTION: 	This routine selects the start date of an appointment.
 *
 * PARAMETERS:  	dateP:
 *					passed:   current start date
 *                  returned: selected start date, if selection was
 *                            made
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *  art	4/2/96		Adjusted end of if repeating event.
 *
 ***********************************************************************/
static void DetailsSelectDate (DetailsPtr details)
{
	int16_t 		month;
	int16_t			day;
	int16_t			year;
	MemHandle 		titleH;
	char* 			title;

	year = details->when.date.year + firstYear;
	month = details->when.date.month;
	day = details->when.date.day;

	titleH = DmGetResource (gApplicationDbP, strRsc, startDateTitleStrID);
	title = MemHandleLock (titleH);

	if (SelectDay (selectDayByDay, &month, &day, &year, title))
	{
		// Return the date selected.
		details->when.date.day = day;
		details->when.date.month = month;
		details->when.date.year = year - firstYear;

		DetailsSetDateTrigger(details->when.date, details->when.midnightWrapped);

		// If the event repeats monthly-by-day update the day the
		// month the event repeats on.
		if (details->repeat.repeatType == repeatMonthlyByDay)
		{
			details->repeat.repeatOn = (uint8_t) DayOfMonth (month, day, year);
		}
	}

	MemHandleUnlock (titleH);
   	DmReleaseResource (titleH);
 }


/***********************************************************************
 *
 * FUNCTION:    	DetailsSelectTime
 *
 * DESCRIPTION: 	This routine selects the start date of an appointment.
 *
 * PARAMETERS:  	detailsP: 		current details.
 *
 *					startP:
 *						passed:   	current start time
 *                      returned: 	selected start time
 *              	endtP:
 *						passed:   	current end time
 *                      returned: 	elected end time
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *
 ***********************************************************************/
static void DetailsSelectTime (
	DetailsPtr 			detailsP,
	TimePtr 			startP,
	TimePtr 			endP,
	Boolean* 			midnightWrappingP,
	Boolean* 			noTimeEventP,
	uint16_t 			controlId)
{
	TimeSelectionType 	selectType;

	if (*noTimeEventP)
		selectType = tSelectNoTime;
	else
	{
		switch(controlId)
		{
			case DetailsStartTimeSelector:
				selectType = tSelectStart;
				break;
			case DetailsEndTimeSelector:
				selectType = tSelectEnd;
				break;
		}
	}

	if (GetTime (startP, endP, &Date, midnightWrappingP, &selectType, setTimeTitleStrID))
	{
		*noTimeEventP = (Boolean) (selectType == tSelectNoTime);

		DetailsSetTimeTriggers(detailsP, *startP, *endP, *noTimeEventP);
	}
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsSetRepeatingTrigger
 *
 * DESCRIPTION:		Update the trigger for the repeating frequency.
 *
 * PARAMETERS:  	details:	pointer to a structure that hold the info about
 *                       		 the current record.
 *
 * RETURNED:    	nothing
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	08/27/03	Initial revision.
 *
 ***********************************************************************/
void DetailsSetRepeatingTrigger(DetailsPtr details)
{
	FormType*			detailsFrmP;
	ControlPtr 			ctl;
	MemHandle 			rscH;
	char* 				rscP;
	uint16_t 			i;

	detailsFrmP = FrmGetFormPtr (DetailsDialog);

	// Set the repeat type selector label.  The label will point
	// to a locked resouce string,  we'll need to unlock this
	// resource when the dialog is dismissed.
	ctl = FrmGetObjectPtr (detailsFrmP, FrmGetObjectIndex (detailsFrmP, DetailsRepeatSelector));

	// to have an array of string null terminated in a buffer with the new XML resource compiler
	rscH = DmGetResource (gApplicationDbP, appInfoStringsRsc, repeatTypesStrID);
	rscP = MemHandleLock (rscH);

	for (i = 0; i < details->repeat.repeatType; i++)
	{
		rscP += strlen (rscP) + 1;
	}

	CtlSetLabel (ctl, rscP);
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsSetCategoryTrigger
 *
 * DESCRIPTION:		Update the trigger for the category
 *
 * PARAMETERS:  	details:	pointer to a structure that hold the info
 *								aboutb the current record.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	07/30/02	Introduced category support.
 *
 ***********************************************************************/
static void DetailsSetCategoryTrigger(DetailsPtr detailsP)
{
	FormPtr 		detailsFrmP;
	uint16_t		maxCatWidth;
	uint16_t		catTriggerIndex;
	Coord			catPosx;
	Coord			catPosy;
	RectangleType 	detailsRect;

	// Get the details form ptr
	detailsFrmP = FrmGetFormPtr (DetailsDialog);

	catTriggerIndex = FrmGetObjectIndex (detailsFrmP, DetailsCategoryPopSelector);
	FrmGetObjectPosition(detailsFrmP, catTriggerIndex, &catPosx, &catPosy);
	FrmGetFormInitialBounds(detailsFrmP, &detailsRect);
	maxCatWidth = detailsRect.extent.x - catPosx - detailsTriggerSelectorsSpaceOnRight;

	CatMgrTruncateName(ApptDB, detailsP->pCategoryIDs, detailsP->numCategories,
		maxCatWidth, detailsP->categoriesName);
	CtlSetLabel (FrmGetObjectPtr (detailsFrmP, catTriggerIndex), detailsP->categoriesName);

}

/***********************************************************************
 *
 * FUNCTION:    	DetailsSelectCategory
 *
 * DESCRIPTION: 	This routine displays a pop-up list for updating the
 *					appointment category.
 *
 * PARAMETERS:  	details: pointer to a structure that hold the info
 *							 about the current record.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	07/30/02	Introduced category support.
 *
 ***********************************************************************/
static void DetailsSelectCategory (DetailsPtr detailsP)
{
	FormType* 		frmP;
	uint32_t 		numSelectedCategories = 0;
	CategoryID *	selectedCategoriesP = NULL;
	status_t		error;

	// Process the category popup list.
	frmP = FrmGetFormPtr (DetailsDialog);

	detailsP->categoryChanged = CatMgrSelectEdit(
									ApptDB,
									frmP,
									DetailsCategoryPopSelector,
									detailsP->categoriesName,
									DetailsCategoryList,
									true /* allowMultiple */,
									detailsP->pCategoryIDs,
									detailsP->numCategories,
									&selectedCategoriesP,
									&numSelectedCategories,
									true /* showEditingStr */,
									NULL);

	// Add the new categories
	if (detailsP->categoryChanged)
	{
		// Free previous list
		if (detailsP->pCategoryIDs != NULL)
			MemPtrFree(detailsP->pCategoryIDs);

		error = DbSetCategory(ApptDB, gApptCursorID, numSelectedCategories, selectedCategoriesP);
		TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : DetailsSelectCategory, DbSetCategory, err = %lx",error));

		// Free list allocated by CatMgrSelectEdit
		CatMgrFreeSelectedCategories(ApptDB, &selectedCategoriesP);

		error = DbGetCategory(ApptDB, gApptCursorID, &numSelectedCategories, &selectedCategoriesP);
		TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : DetailsSelectCategory, DbGetCategory, err = %lx",error));

		// Assign new list
		detailsP->numCategories = numSelectedCategories;
		if (selectedCategoriesP != NULL)
		{
			detailsP->pCategoryIDs = MemPtrNew(sizeof(CategoryID) * numSelectedCategories);
			memmove(detailsP->pCategoryIDs, selectedCategoriesP, sizeof(CategoryID) * numSelectedCategories);
		}
		else
			detailsP->pCategoryIDs = NULL;

		DbReleaseStorage(ApptDB, selectedCategoriesP);

		// Update display
		DetailsSetCategoryTrigger(detailsP);
	}
	else if (selectedCategoriesP)
	{
		// Free list allocated by CatMgrSelectEdit
		CatMgrFreeSelectedCategories(ApptDB, &selectedCategoriesP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsDeleteAppointment
 *
 * DESCRIPTION: 	This routine deletes an appointment record. It is called
 *             	 	when the delete button in the details dialog is pressed.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	true if the record was delete or archived.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *
 ***********************************************************************/
static Boolean DetailsDeleteAppointment (void)
{
	// Clear the edit state, this will delete the current record if it is empty
	if (DayViewClearEditState ())
		return true;

	// Delete the record,  this routine will display an appropriate
	// dialog to confirm the action.  If the dialog is canceled
	// don't update the display.
	if (! DeleteRecord (gApptCursorID))
		return false;

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsGet
 *
 * DESCRIPTION: 	This routine get the current setting of the details
 *              	dialog.
 *
 * PARAMETERS:  	details: pointer to a structure that hold the info about
 *                        	 the current record.
 *
 * RETURNED:    	true if the form info was all valid, otherwise false.
 *
 * NOTE:       	 	Not all the members of the structure passed are filled
 *              	in by this routine.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *
 ***********************************************************************/
static Boolean DetailsGet (DetailsPtr details)
{
	ListPtr		lst;
	FieldPtr 	fld;
	ControlPtr 	ctl;
	FormPtr 	frmP;
	char* 		alarmStr;
	wchar32_t 	curChar;

	// Get the alarm settings.
	frmP = FrmGetFormPtr (DetailsDialog);
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmCheckbox));

	if (CtlGetValue (ctl))
	{
		fld = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmAdvanceField));
		alarmStr = FldGetTextPtr (fld);
		while (*alarmStr)
		{

			alarmStr += TxtGetNextChar(alarmStr, 0, &curChar);

			if (!TxtCharIsDigit(curChar))
			{
				FrmAlert(gApplicationDbP, AlarmInvalidAlert);
				return false;
			}
		}

		details->alarm.advance = (uint8_t) StrAToI (FldGetTextPtr (fld));

		lst = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmAdvanceList));

		details->alarm.advanceUnit = (AlarmUnitType) LstGetSelection (lst);
	}
	else
		details->alarm.advance = apptNoAlarm;		// no alarm is set

	// Get the private setting.
	details->secret = (Boolean) CtlGetValue (
									FrmGetObjectPtr (frmP,
										FrmGetObjectIndex (frmP, DetailsPrivateCheckbox)));

	// Get the location pointer, this pointer is valid util
	// FrmDeleteFrom is called (actually the FrmReturnToForm routine)
	details->locationH = FldGetTextHandle (
								FrmGetObjectPtr (frmP,
									FrmGetObjectIndex (frmP, DetailsLocationField)));

	return true;
}




/***********************************************************************
 *
 * FUNCTION:    	DetailsCheckRecordPrivacy
 *
 * DESCRIPTION: 	This routine is called when the details dialog is
 *              	applied or canceled. It checks the privacy status
 *					and unselect the record if the security is Hide.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	01/01/03	Initial Revision.
 *
 ***********************************************************************/
static Boolean DetailsCheckRecordPrivacy(Boolean * recordHiddenP)
{
	Boolean 	mustMaskRecord = false;
	uint16_t	attr;

	if (recordHiddenP)
		*recordHiddenP = false;

	DbGetRowAttr (ApptDB, gApptCursorID, &attr);
	if (attr & dmRecAttrSecret)
	{
		switch (PrivateRecordVisualStatus)
		{
			case showPrivateRecords:
				// Nothing to do
				break;

			case maskPrivateRecords:
				mustMaskRecord = true;
				break;

			case hidePrivateRecords:
				// The record disappeared
				if (recordHiddenP)
					*recordHiddenP = true;
				gItemSelected = false;
				break;
		}
	}
	return mustMaskRecord;
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsUpdateTimeZoneDisplayed
 *
 * DESCRIPTION:		Update the trigger for time zone.
 *
 * PARAMETERS:  	details: pointer to a structure that hold the info about
 *                        	 the current record.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	06/13/02	Introduced timezone support
 *
 ***********************************************************************/
static void DetailsUpdateTimeZoneDisplayed(DetailsPtr detailsP)
{
	FormPtr 		detailsFrmP;
	MemHandle		defaultTZStrH;
	Coord			triggerWidth;
	uint16_t		tzTriggerIndex;
	Coord			tzPosx;
	Coord			tzPosy;
	RectangleType 	detailsRect;

	// Get the details form ptr
	detailsFrmP = FrmGetFormPtr (DetailsDialog);
	tzTriggerIndex = FrmGetObjectIndex (detailsFrmP, DetailsTimeZoneSelector);

	// No time event : do not display any timezone
	if (detailsP->when.noTimeEvent)
	{
		defaultTZStrH = DmGetResource (gApplicationDbP, strRsc, noTimeTimezoneStrID);
		StrNCopy(sDetailsTimeZoneString, MemHandleLock(defaultTZStrH), TZNAME_MAX);
		// Release the resources
		MemHandleUnlock(defaultTZStrH);
		DmReleaseResource(defaultTZStrH);
	}
	// If the timezone is not assigned, get the text to display from resources
	else if (detailsP->when.timeZoneName[0] == chrNull)
	{
		defaultTZStrH = DmGetResource (gApplicationDbP, strRsc, unassignedTimezoneStrID);
		StrNCopy(sDetailsTimeZoneString, MemHandleLock(defaultTZStrH), TZNAME_MAX);
		// Release the resources
		MemHandleUnlock(defaultTZStrH);
		DmReleaseResource(defaultTZStrH);
	}
	else
	{
		// Done to accelerate first invocation of Event Details Dialog
		// after date book is launched.
		if (strcmp(gDeviceTimeZone, detailsP->when.timeZoneName))
		{
			TimeZoneToAscii(detailsP->when.timeZoneName, sDetailsTimeZoneString);
		}
		else
		{
			// we already get the localized time zone, we use it
			strcpy(sDetailsTimeZoneString, gLocalizedTimeZomeName);
		}

		FrmGetObjectPosition(detailsFrmP, tzTriggerIndex, &tzPosx, &tzPosy);
		FrmGetFormInitialBounds(detailsFrmP, &detailsRect);
		triggerWidth = detailsRect.extent.x - tzPosx - detailsTriggerSelectorsSpaceOnRight;
		FntTruncateString(sDetailsTimeZoneString, sDetailsTimeZoneString, stdFont,
			triggerWidth, true);
	}

	CtlSetLabel(FrmGetObjectPtr (detailsFrmP, tzTriggerIndex), sDetailsTimeZoneString);
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsCheckTimeZoneValidity
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	details: pointer to a structure that hold the info about
 *                        	the current record.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	06/13/02	Introduced timezone support.
 *
 ***********************************************************************/
static Boolean DetailsCheckTimeZoneValidity (DetailsPtr detailsP)
{
	ApptDateTimeType 	when;
	uint16_t			initYear = detailsP->when.date.year;

	// No problem if we didn't reach the bounds
	if ((initYear != 0) && (initYear != numberOfYears-1))
		return true;

	memset (&when, 0x00, sizeof (ApptDateTimeType));
	CnvApptTZ2LocalTime(&(detailsP->when), NULL, &when, NULL);

	// The date crossed the very first date 1/1/1904
	if ((initYear == 0) && (when.date.year == numberOfYears-1))
		return false;

	// The date crossed the very last date 12/31/2031
	if ((initYear == numberOfYears-1) && (when.date.year == 0))
		return false;

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsSelectTimeZone
 *
 * DESCRIPTION: 	This routine displays a pop-up list for updating the
 *					time zone.
 *
 * PARAMETERS:  	details: pointer to a structure that hold the info about
 *                       	 the current record.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	06/13/02	Introduced timezone support.
 *
 ***********************************************************************/
static void DetailsSelectTimeZone (DetailsPtr detailsP)
{
	MemHandle 	timeZoneSelectorTitleH;
	char		*timeZoneSelectorTitleP;
	char		initialTimeZoneID[TZNAME_MAX];

	if (detailsP->when.noTimeEvent)
	{
		FrmAlert(gApplicationDbP, UntimedTimezoneAlert);
		return;
	}

	strncpy(initialTimeZoneID, detailsP->when.timeZoneName, TZNAME_MAX);
	timeZoneSelectorTitleH = DmGetResource (gApplicationDbP, strRsc, DetailsTimeZoneSelectTitle);
	timeZoneSelectorTitleP = (char*)MemHandleLock(timeZoneSelectorTitleH);

	if (SelectTimeZone(
			detailsP->when.timeZoneName,
			timeZoneSelectorTitleP,
			showDeviceTimeZone))
	{
		// Check if the new timezone selected doesn't make appt to exit PalmOS dates range
		if (! DetailsCheckTimeZoneValidity(detailsP))
		{
			// Display alert and switch back to previous timezone
			FrmAlert(gApplicationDbP, OutOfRangeApptAlert);
			strncpy(detailsP->when.timeZoneName, initialTimeZoneID, TZNAME_MAX);
		}
		else
		{
			// Update the display for date, time and time zone
			DetailsUpdateTimeZoneDisplayed(detailsP);
		}
	}

	MemHandleUnlock(timeZoneSelectorTitleH);
   	DmReleaseResource(timeZoneSelectorTitleH);
}


/***********************************************************************
 *
 * FUNCTION:   		DetailsSetTitle
 *
 * DESCRIPTION: 	This routine updates the dialog title
 *
 * PARAMETERS:  	frm: pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void DetailsSetTitle(FormPtr frmP)
{
	ApptDBRecordType 	apptRec;
	char *				tempP;
	int32_t				tempCount = 0;
	Coord				titleWidth;
	char *				titleTextP;
	RectangleType		dialogRect;
	MemHandle			unnamedApptStrH;

	// Get the appointment description
	ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_DESCRIPTION);

	// If the appointment doesn't have name, get string from resources
	if (apptRec.description == NULL || strlen(apptRec.description) == 0)
	{
		// Lock the resource
		unnamedApptStrH = DmGetResource (gApplicationDbP, strRsc, unnamedAppointmentStrID);
		tempP = MemHandleLock(unnamedApptStrH);
		tempCount = strlen(tempP);
		titleTextP = MemPtrNew(tempCount+1);
		strcpy(titleTextP, tempP);
		// Release the resources
		MemHandleUnlock(unnamedApptStrH);
		DmReleaseResource(unnamedApptStrH);
	}
	else
	{
		// Get the title, if it's multi-line, retrieve only the first line
		tempP = apptRec.description;
		while (*tempP && *tempP != linefeedChr)
		{
			++tempCount;
			++tempP;
		}
		titleTextP = MemPtrNew(tempCount + 2); // Add 2 for chrNull and ellipsis if needed
		StrNCopy(titleTextP, apptRec.description, tempCount);
		if (*tempP == linefeedChr)
			titleTextP[tempCount++] = chrEllipsis;
	}
	titleTextP[tempCount] = chrNull;

	// Get the width available : substract the size of the help icon
	// (Do not dynamically compute help icon width to save time)
	FrmGetFormInitialBounds(frmP, &dialogRect);
	titleWidth = dialogRect.extent.y - DetailsDialogHelpIconWidth;

	// Truncate the title if it's too large
	FntTruncateString(titleTextP, titleTextP, boldFont, titleWidth, true);

	// Release record
	ApptFreeRecordMemory (&apptRec);

	// Assign the title
	FrmSetTitle(frmP, titleTextP);
}

/***********************************************************************
 *
 * FUNCTION:   		DetailsFreeTitle
 *
 * DESCRIPTION: 	This routine free the dynamically allocated dialog title
 *
 * PARAMETERS:  	frm - pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void DetailsFreeTitle(FormPtr frmP)
{
	char * titleTextP;

	titleTextP = (char*) FrmGetTitle(frmP);
	MemPtrFree(titleTextP);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvArrangeTabsLabels
 *
 * DESCRIPTION: 	This routine initializes the Book's Gadget labels
 *					Array.
 *
 * PARAMETERS:  ->	stlId:			string list resource id.
 * 				->	tabIdsListId:	uint16	list resource id.
 *
 * RETURNED:    	a pointer holding an array of TabInfoType.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static MemPtr PrvArrangeTabsLabels(uint16_t stlId, uint16_t tabIdsListId)
{
	char			label[DayBookLabelCharsNum];
	MemHandle		tabIdsListH;
	TabInfoType*	tabsP;
	uint32_t		index;
	uint32_t		tabsSize;
	uint32_t		howManyTabs;


	tabIdsListH = U16ListGetList(gApplicationDbP, tabIdsListId);

	if (!tabIdsListH)
		return NULL;

	howManyTabs = U16ListGetItemNum(gApplicationDbP, tabIdsListId, tabIdsListH);

	tabsSize = howManyTabs * sizeof(TabInfoType);
	tabsP = (TabInfoType*) MemPtrNew(tabsSize);

	if (!tabsP)
		return NULL;


	for (index = 0 ; index < howManyTabs ; ++index)
	{
		SysStringByIndex(gApplicationDbP, stlId,  (uint16_t) index, label, DayBookLabelCharsNum - 1);
		label[DayBookLabelCharsNum-1] = 0x00;

		strcpy(tabsP[index].name, label );
		tabsP[index].id = U16ListGetItem(gApplicationDbP, tabIdsListId, tabIdsListH, index);
	}

	U16ListReleaseList(tabIdsListH);

	return tabsP;
}

/***********************************************************************
 *
 * FUNCTION:    	PrvFreeTabsLabels
 *
 * DESCRIPTION: 	This routine frees the Book's Gadget labels array.
 *
 * PARAMETERS:  ->	stlId:			string list resource id.
 * 				->	tabIdsListId:	uint16	list resource id.
 *
 * RETURNED:   	 	a pointer holding an array of TabInfoType.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void PrvFreeTabsLabels(TabInfoType* tabsP)
{
	if (tabsP)
	{
		MemPtrFree((MemPtr) tabsP);
	}
}

/***********************************************************************
 *
 * FUNCTION:    	PrvShowHideTabItems
 *
 * DESCRIPTION: 	This routine initializes the Book's Gadget
 *
 * PARAMETERS:  -> 	formP:			form Ptr
 *				->	stlId:			string list resource id.
 * 				->	tabIdsListId:	uint16	list resource id.
 *				-> 	tabId:			id of the tab to show or hide depending
 *									on the show boolean parameter
 *				->	show:			Show items if true,
 *									Hide items if false.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void PrvShowHideTabItems(FormType* formP, uint16_t stlId, uint16_t tabIdsListId, uint16_t tabId, Boolean show)
{
	MemHandle	tabIdsListH;
	uint32_t	index;
	uint32_t	howManyItems;
	uint16_t	itemId;

	tabIdsListH = U16ListGetList(gApplicationDbP, tabId);

	if (!tabIdsListH)
		return;

	howManyItems = U16ListGetItemNum(gApplicationDbP, tabId, tabIdsListH);

	for (index = 0 ; index < howManyItems ; ++index)
	{
		itemId = U16ListGetItem(gApplicationDbP, tabId, tabIdsListH, index);

		if (show)
			UtilitiesFormShowObjectID (formP, itemId);
		else
			UtilitiesFormHideObjectID (formP, itemId);
	}

	U16ListReleaseList(tabIdsListH);
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsSetDefaultEventDetailsTab
 *
 * DESCRIPTION: 	This routine shows or hides the alarm advance ui
 *					object.
 *
 *              	It is called when the alarm checkbox is turn on
 *					or off.
 *
 * PARAMETERS:  ->	tabId: 	ID of the Tab to open on Details Dialog
 *							Opening.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	7/21/02		Initial revision.
 *
 ***********************************************************************/
void DetailsSetDefaultEventDetailsTab (uint16_t tabId)
 {
 	if (tabId == DetailsBookOptionsTabId || tabId == DetailsBookEventInformationTabId)
 	{
 		sDefaultEventDetailsTabId = tabId;
 	}
 }


/***********************************************************************
 *
 * FUNCTION:    	DetailsDrawCallBack
 *
 * DESCRIPTION: 	Callback for WinRectangleInvalidateFunc() - see Window.h
 *
 * PARAMETERS:  	cmd
 *					window
 *					diryRect
 *					state
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	06/16/03	Initial Revision.
 *
 ***********************************************************************/
static Boolean DetailsDrawCallBack (int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	if (cmd == winInvalidateDestroy)
		return true;

	FrmDrawForm(FrmGetFormPtr(DetailsDialog));

	return true;
}


/***********************************************************************
 *
 * FUNCTION:   		DetailsDeactivateBookTab
 *
 * DESCRIPTION: 	This routine is a callback for BooksLib Module.
 *					Called when a tab is sent in bakground.
 *					The Tab is deactivated.
 *
 * PARAMETERS:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DetailsDeactivateBookTab(BookType* bookP, uint16_t tabId,
	uint16_t tabIndex, uint32_t userData)
{
	ControlPtr	alarmCheckBoxP;

	PrvShowHideTabItems(FrmGetActiveForm(), DetailsDialogTabLabelsSTL, DetailsDialogTabIdsList, tabId, false);

	switch(tabId)
	{
		case DetailsBookOptionsTabId:
			alarmCheckBoxP = UtilitiesGetFormObjectPtr (bookP->formP, DetailsAlarmCheckbox);
			if ((Boolean) CtlGetValue (alarmCheckBoxP))
			{
				UtilitiesFormHideObjectID (bookP->formP, DetailsAlarmAdvanceField);
				UtilitiesFormHideObjectID (bookP->formP, DetailsAlarmAdvanceSelector);
			}
			break;

		case DetailsBookEventInformationTabId:
			UtilitiesFormHideObjectID (bookP->formP, DetailsAlarmAdvanceField);
			UtilitiesFormHideObjectID (bookP->formP, DetailsAlarmAdvanceSelector);
			break;
	}

	return true;
}

 /***********************************************************************
 *
 * FUNCTION:   		DetailsActiveBookTab
 *
 * DESCRIPTION: 	This routine is a callback for BooksLib Module.
 *					Called when a tab is sent in foreground.
 *					The Tab is activated.
 *
 * PARAMETERS:  	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void DetailsActivateBookTab(BookType* bookP, uint16_t tabId,
	uint16_t tabIndex, uint32_t userData)
{
	FormPtr 	frmP;
	ControlPtr	alarmCheckBoxP;
	uint16_t 	alrmAdvIndex;
	//uint16_t	locationIndex;

	frmP = bookP->formP;

	PrvShowHideTabItems(frmP, DetailsDialogTabLabelsSTL, DetailsDialogTabIdsList, tabId, true);
	DetailsSetDefaultEventDetailsTab(tabId);


	// Select default focus according to active tab
	switch(tabId)
	{
		case DetailsBookOptionsTabId:
			alrmAdvIndex = FrmGetObjectIndex (frmP, DetailsAlarmAdvanceField);
			FrmSetFocus (frmP, alrmAdvIndex);
			// FldSetInsertionPoint (FrmGetObjectPtr (frmP, alrmAdvIndex), mostRightFieldPosition);

			alarmCheckBoxP = UtilitiesGetFormObjectPtr (frmP, DetailsAlarmCheckbox);
			if ((Boolean) CtlGetValue (alarmCheckBoxP))
			{
				UtilitiesFormShowObjectID (frmP, DetailsAlarmAdvanceField);
				UtilitiesFormShowObjectID (frmP, DetailsAlarmAdvanceSelector);
			}
			else
			{
				UtilitiesFormHideObjectID (frmP, DetailsAlarmAdvanceField);
				UtilitiesFormHideObjectID (frmP, DetailsAlarmAdvanceSelector);
			}
			break;

		case DetailsBookEventInformationTabId:
			// remove as it it interfering with one handed navigation
			//locationIndex = FrmGetObjectIndex (frmP, DetailsLocationField);
			//FrmSetFocus (frmP, FrmGetObjectIndex (frmP, DetailsLocationField));
			//FldSetInsertionPoint (FrmGetObjectPtr (frmP, locationIndex), mostRightFieldPosition);

			UtilitiesFormHideObjectID (frmP, DetailsAlarmAdvanceField);
			UtilitiesFormHideObjectID (frmP, DetailsAlarmAdvanceSelector);
			break;
	}
}


 /***********************************************************************
 *
 * FUNCTION:   		DetailsActiveCycleTab
 *
 * DESCRIPTION: 	This routine activate the next tab.
 *
 * PARAMETERS:   ->	bookP: tabs group data.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	06/14/04	Initial revision.
 *
 ***********************************************************************/
static void DetailsActiveCycleTab(BookType* bookP)
{
	uint16_t currentTabIndex;

	currentTabIndex = BookGetActiveTabIndex(bookP);

	currentTabIndex++;

	if (currentTabIndex >= BookGetNumberOfTab(bookP))
		currentTabIndex = 0;

	BookActiveTabIndex(bookP, currentTabIndex);

}


/***********************************************************************
 *
 * FUNCTION:   		DetailsInitBook
 *
 * DESCRIPTION: 	This routine initializes the event details Book's Gadget
 *
 * PARAMETERS:  	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	PPL	01/08/03	Initial revision.

 *
 ***********************************************************************/
static void DetailsInitBook(FormPtr frmP)
 {
 	uint16_t	index;
 	uint32_t	howManyTabs;
 	status_t	err;

	sDetailsTabs = PrvArrangeTabsLabels(DetailsDialogTabLabelsSTL, DetailsDialogTabIdsList);
	if (!sDetailsTabs)
	{
		sDetailsTabsFree = false;
		sDetailsTabs = sDefaultDetailsTabs;
	}

	howManyTabs = U16ListGetItemNum(gApplicationDbP, DetailsDialogTabIdsList, NULL);

 	sDetailsBook = BookCreate(frmP, DetailsBookTabs,  DetailsBookBody , DayBookMinTabWidth, DayBookMaxTabWidth, (uint16_t) howManyTabs, kBookStandardLookAndFeelNoScroll);

	for (index = 0 ;  index < (uint16_t) howManyTabs ; ++index)
		err  = BookAddTab(sDetailsBook,  sDetailsTabs[index].name, NULL, 0, 0,  sDetailsTabs[index].id , kBookTabMaxIndex, 0, DetailsActivateBookTab, DetailsDeactivateBookTab, DetailsDrawCallBack);
 }


/***********************************************************************
 *
 * FUNCTION:    	DetailsInit
 *
 * DESCRIPTION: 	This routine initializes the Details Dialog,
 *					and allocates and initialize a memory block
 *					that hold the record info to be edited by
 *					the details dialog.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	details:	memory block that hold info to be
 *								edited by the details dialog.
 *
 * NOTE:			This routine assumes that the memory allocated for
 *					the label of the time trigger is large enough to
 *					hold the largest posible label. This label's memory
 * 					is reserved by initializing the label
 *      			in the resource file.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *
 ***********************************************************************/
static void DetailsInit (FormPtr frmP)
{
	FieldAttrType 		fldAttr;
	uint16_t 			attr;
	char* 				label;
	char* 				textP;
	ListPtr 			lst;
	FieldPtr 			fld;
	ControlPtr 			ctl;
	DetailsPtr 			details;
	MemHandle 			textH;
	ApptDBRecordType 	apptRec;
	MemPtr 				locationTxtP;
	uint32_t 			numCategories = 0;
	CategoryID *		pCategoryIDs;
	status_t			err;


	PIMAppProfilingBegin("DetailsInit");

	
	FrmSetTransparentObjects(frmP, true);

	// Allocate and initialize a block to hold a temporary copy of the
	// info from the appointment record, we don't want to apply any changes
	// to the record until the "Details Dialog" is confirmed.
	if (! ApptDetailsP)
	{
		ApptDetailsP = MemPtrNew (sizeof (DetailsType));

		details = ApptDetailsP;

		memset (details, 0x00, sizeof (DetailsType));

		// Get a pointer to the appointment record.
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec,
			DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT | DBK_SELECT_LOCATION);

		DbGetRowAttr (ApptDB, gApptCursorID, &attr);

		details->secret = (Boolean) (attr & dmRecAttrSecret) == dmRecAttrSecret;
		details->alarm = apptRec.alarm;

		// If the event is repeated, set its date to current
		if (apptRec.repeat.repeatType != repeatNone)
			apptRec.when.date = Date;

		// Convert from local time to appointment TZ
		CnvLocalTime2ApptTZ(&apptRec.when, &apptRec.repeat, &details->when, &details->repeat);

		if (apptRec.location)
		{
			// Allocate the text handle
			details->locationH = MemHandleNew((uint32_t)(strlen(apptRec.location) + 1));

			// Set the location from saved record
			locationTxtP = MemHandleLock(details->locationH);
			memmove(locationTxtP, apptRec.location, (int32_t)(strlen(apptRec.location) + 1));
			MemHandleUnlock(details->locationH);
		}

		// Free the appointment record links
		ApptFreeRecordMemory (&apptRec);

		// Get the category assigned to current record
		err = DbGetCategory (ApptDB, gApptCursorID, &numCategories, &pCategoryIDs);

		details->numCategories = numCategories;

		if (pCategoryIDs != NULL && err >= errNone)
		{
			// current categories
			details->pCategoryIDs = MemPtrNew(sizeof(CategoryID) * numCategories);
			if (details->pCategoryIDs)
				memmove(details->pCategoryIDs, pCategoryIDs, sizeof(CategoryID) * numCategories);

			// saved categories
			details->savedNumCategories = numCategories;
			details->savedCategoryIDs =  MemPtrNew(sizeof(CategoryID) * numCategories);
			if (details->savedCategoryIDs)
				memmove(details->savedCategoryIDs, pCategoryIDs, sizeof(CategoryID) * numCategories);
		}
		else
			details->pCategoryIDs = NULL;

		details->categoryChanged = false;

		DbReleaseStorage(ApptDB, pCategoryIDs);
	}
	else
	{
		details = ApptDetailsP;
	}

	// Init Books Tab
	DetailsInitBook(frmP);

	// Set the alarm check box. (Need to have the checkBox set before to activate the tab)
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmCheckbox));
	CtlSetValue (ctl, (int16_t)(details->alarm.advance != apptNoAlarm));

	// Set "options" tab
	BookActiveTabId(sDetailsBook, sDefaultEventDetailsTabId);
	BookReActiveCurrentTab(sDetailsBook, false, false);

	// Set the timezone
	DetailsUpdateTimeZoneDisplayed(details);

	// Set the event category
	DetailsSetCategoryTrigger(details);

	// Set the start & end time selectors labels.
	DetailsSetTimeTriggers(details, details->when.startTime, details->when.endTime, details->when.noTimeEvent);

	// Set the start date selector label.
	DetailsSetDateTrigger(details->when.date, details->when.midnightWrapped);


	if (details->alarm.advance != apptNoAlarm )
	{
		// Set the alarm advance value.
		fld = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmAdvanceField));
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, details->alarm.advance);
		MemPtrUnlock (textP);
		FldSetTextHandle (fld, textH);
		//if (sDefaultEventDetailsTabId == DetailsBookOptionsTabId)
			//ShowObject (frmP, DetailsAlarmAdvanceField);

		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmAdvanceList));
		LstSetSelection (lst, details->alarm.advanceUnit);
		label = LstGetSelectionText (lst, details->alarm.advanceUnit);

		ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsAlarmAdvanceSelector));
		CtlSetLabel (ctl, label);

		//if (sDefaultEventDetailsTabId == DetailsBookOptionsTabId)
			//ShowObject (frmP, DetailsAlarmAdvanceSelector);

		sAlarmIsSet = true;
	}

	// Get the Fld Ptr.
	fld = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsLocationField));

	// Set the maximum length
	FldSetMaxChars(fld, gLocationFieldMaxLength);

	// Set the field to support auto-shift.
	FldGetAttributes (fld, &fldAttr);
	fldAttr.autoShift = true;
	FldSetAttributes (fld, &fldAttr);

	// Set the location
	if (details->locationH)
	{
		// Update the field
		FldSetTextHandle(fld, details->locationH);

		// Set the selection if needed
		if (gDetailsSelectionLength)
			FldSetSelection(fld, gDetailsPosition, gDetailsPosition + gDetailsSelectionLength);
	}

	// Set the event category
	DetailsSetRepeatingTrigger(details);

	// If the record is mark secret, turn on the secret checkbox.
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DetailsPrivateCheckbox));
	CtlSetValue (ctl, details->secret);

	PIMAppProfilingEnd();
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsExit
 *
 * DESCRIPTION: 	This routine is called when the details dialog is
 *              	applied or canceled.  It unlocks any memory locked
 *              	and frees any memory allocated by the Details
 *					Dialog routine.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	art	7/21/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DetailsExit(void)
{
	MemHandle 	rscH;
	FieldPtr 	fld;
	FormPtr 	detailsFrmP;

	// Get the details form ptr
	detailsFrmP = FrmGetFormPtr (DetailsDialog);

	// The label of the repeat trigger is locked - unlock it.
	// Note that we can't get the pointer to be unlocked by using
	// CtlGetLabel on the repeat trigger because it may actually be
	// pointing into the middle of the resource.
	// to have an array of string null terminated in a buffer with the new XML resource compiler
	rscH = DmGetResource (gApplicationDbP, appInfoStringsRsc, repeatTypesStrID);
	MemHandleUnlock(rscH);
   	DmReleaseResource(rscH);

	if (ApptDetailsP)
	{
		// Reset selection
		gDetailsPosition = 0;
		gDetailsSelectionLength = 0;

		// Free the categories list
		if (((DetailsType*)ApptDetailsP)->pCategoryIDs != NULL)
		{
			MemPtrFree(((DetailsType*)ApptDetailsP)->pCategoryIDs);
			((DetailsType*)ApptDetailsP)->pCategoryIDs = NULL;
		}


		// Free the saved categories list
		if (((DetailsType*)ApptDetailsP)->savedCategoryIDs != NULL)
		{
			MemPtrFree(((DetailsType*)ApptDetailsP)->savedCategoryIDs);
			((DetailsType*)ApptDetailsP)->savedCategoryIDs = NULL;
		}

		// Free the block that holds the changes to the appointment record.
		MemPtrFree (ApptDetailsP);
		ApptDetailsP = NULL;
	}
	else
	{
		// The location handle must not be deleted (must be kept to
		// be available when going back to details form after repeat one)
		// Thus, we have to put NULL to the field handle.

		// Get the location Fld Ptr.
		fld = FrmGetObjectPtr (detailsFrmP, FrmGetObjectIndex (detailsFrmP, DetailsLocationField));

		// Update the field
		FldSetTextHandle(fld, NULL);
	}

	PrvFreeTabsLabels(sDetailsTabs);
	sDetailsTabs = NULL;
}

/***********************************************************************
 *
 * FUNCTION:    	DetailsApply
 *
 * DESCRIPTION: 	This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  	details:		appointment details.
 *             		attachNote:		true if pressing the note button being
 *									caused the changes to be applied.
 *
 * RETURNED:    	true if the changes were applied, false if the changes
 *              	were not applied.
 *
 * NOTE:			If a note is being attached to a repeating event the
 *              	user is prompted to determine if an exception should be
 *              	created,  the exception is create be this routine,  the
 *              	note is attached elsewhere.
 *
 * REVISION HISTORY:
 *	art	03/10/95	Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static Boolean DetailsApply (DetailsPtr details, Boolean attachNote)
{
	status_t				err = errNone;
	int32_t					adjust;
	uint16_t				attributes;
	uint16_t				id;
	uint16_t				repeatOn;
	uint16_t				alertButton;
	uint16_t 				newDayOfWeek;
	uint8_t					newDayMask;
	Boolean					dirty 		= false;
	Boolean					attrDirty 	= false;
	Boolean					exception 	= false;
	Boolean					pastRepeats	= false;
	Boolean					switchDate 	= false;
	Boolean					switchCategories = false;
	FormPtr					alert;
	ApptDateTimeType		when;
	ApptDBRecordType		apptRec;
	ApptDBRecordType		newRec;
	RecordFilteringInfoType	changedFields;
	char*					detailsLocP;
	char*					locationP = NULL;
	FormPtr 				detailsFrmP;
	DateType				dateToSwitchOnExit = Date;

	// Get the details form ptr
	detailsFrmP = FrmGetFormPtr (DetailsDialog);

	// Get the ui setting for the appointment. If the alarm setting contains
	// invalid characters, bail out.
	if (!DetailsGet (details))
		return false;

	// Convert the appointment time zone to the device time zone
	CnvApptTZ2LocalTime(&details->when, &details->repeat, &when, &details->repeat);

	// Compare the start date setting in the dialog with the date in the
	// current record.  Update the record if necessary.
	ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_ALL_FIELDS);

	changedFields = 0;
	memset (&newRec, 0x00, sizeof (ApptDBRecordType));

	// Has the start time or end time of the appointment been changed?
	// Has the time zone or country of the appointment been changed?
	if ((TimeToInt(apptRec.when.startTime) != TimeToInt(when.startTime)) ||
		(TimeToInt(apptRec.when.endTime) != TimeToInt(when.endTime)) ||
		(apptRec.when.noTimeEvent != when.noTimeEvent) ||
		(StrNCompareAscii(apptRec.when.timeZoneName, when.timeZoneName, TZNAME_MAX)))
	{
		newRec.when = when;
		changedFields |= DBK_SELECT_TIME;
		dirty = true;
	}

	// If this is a repeat appointment, when.date was pointing on current date
	if (apptRec.repeat.repeatType == repeatNone)
	{
		if (DateToInt(apptRec.when.date) != DateToInt(when.date))
		{
			newRec.when = when;
			changedFields |= DBK_SELECT_TIME;
			dirty = true;

			// Update the current Date in order to switch accurately when exiting from Details
			dateToSwitchOnExit = when.date;
		}
	}
	else // != repeatNone
	{
		if (DateToInt(when.date) != DateToInt(Date))
		{
			// The date has been modified in details dialog
			newRec.when = when;
			changedFields |= DBK_SELECT_TIME;
			dirty = true;

			// Update the current Date in order to switch accurately when exiting from Details
			dateToSwitchOnExit = when.date;
		}
		else
		{
			// The date wasn't modified in details dialog, keep record date and do not set
			// dirty to true
			newRec.when.date = apptRec.when.date;
		}
	}

	if (((apptRec.repeat.repeatType != repeatNone) && (DateToInt(when.date) != DateToInt(Date))) ||
		((apptRec.repeat.repeatType == repeatNone) && (DateToInt(apptRec.when.date) != DateToInt(when.date))))
	{
		newRec.when = when;
		changedFields |= DBK_SELECT_TIME;
		dirty = true;

		// Update the current Date in order to switch accurately when exiting from Details
		dateToSwitchOnExit = when.date;
	}

	// Has the category of the appointment been changed?

	if (!CategoriesTestEquality(
			details->numCategories,
			details->pCategoryIDs,
			details->savedNumCategories,
			details->savedCategoryIDs))
	{
		// we need to have the database cursor updated
		switchCategories = true;

		// Set the current displaying category to the record one,
		// Otherwise, the user may not see its record after tapping
		// on the Ok button.
		ChangeCurrentCategories(details->numCategories, details->pCategoryIDs);
	}

	// Has an alarm been set?
	if ((apptRec.alarm.advance == apptNoAlarm) && (details->alarm.advance != apptNoAlarm))
	{
		newRec.alarm = details->alarm;
		changedFields |= DBK_SELECT_ALARM;
		dirty = true;
	}

	// Has an alarm been canceled?
	else if ((apptRec.alarm.advance != apptNoAlarm) && (details->alarm.advance == apptNoAlarm))
	{
		newRec.alarm.advance = apptNoAlarm;
		changedFields |= DBK_SELECT_ALARM;
		dirty = true;
	}

	// Has the alarm advance-notice-time been changed?
	else if ( (apptRec.alarm.advance != apptNoAlarm) &&
				((apptRec.alarm.advance != details->alarm.advance) ||
				 (apptRec.alarm.advanceUnit != details->alarm.advanceUnit)))
	{
		newRec.alarm = details->alarm;
		changedFields |= DBK_SELECT_ALARM;
		dirty = true;
	}

	// Has a non-repeating event been changed to a repeating event?
	if ((apptRec.repeat.repeatType == repeatNone) && (details->repeat.repeatType != repeatNone))
	{
		newRec.repeat = details->repeat;
		changedFields |= DBK_SELECT_REPEAT;
		dirty = true;
	}

	// Has a repeating event been changed to a non-repeating event?
	else if ((apptRec.repeat.repeatType != repeatNone) && (details->repeat.repeatType == repeatNone))
	{
		newRec.repeat.repeatType = repeatNone;
		changedFields |= DBK_SELECT_REPEAT;
		dirty = true;
	}

	// Has any of the repeat info been changed?
	else if ((apptRec.repeat.repeatType != repeatNone) && (apptRec.repeat.repeatType != details->repeat.repeatType ||
	   	DateToInt (apptRec.repeat.repeatEndDate) != DateToInt(details->repeat.repeatEndDate) ||
	   	apptRec.repeat.repeatFrequency != details->repeat.repeatFrequency ||
		(details->repeat.repeatType == repeatWeekly &&
		apptRec.repeat.repeatOn != details->repeat.repeatOn)))
	{
		newRec.repeat = details->repeat;
		changedFields |= DBK_SELECT_REPEAT;
		dirty = true;
	}

	// Has the location been changed?
	// If details->locationH is NULL, the location has never been set and has
	// not been updated, so do nothing !
	if (details->locationH != NULL)
	{
		detailsLocP = MemHandleLock(details->locationH);
		if (((apptRec.location == NULL) && (detailsLocP[0] != '\0'))
			|| ((apptRec.location != NULL) && (StrCompareAscii(apptRec.location, detailsLocP) != 0)))
		{
			ApptChangeLocation(&newRec, detailsLocP, &changedFields);
			dirty = true;
		}
		MemHandleUnlock(details->locationH);
	}

	// Get the current setting of the secret checkbox and compare it the
	// the setting of the record.
	DbGetRowAttr(ApptDB, gApptCursorID, &attributes);
	if (((attributes & dmRecAttrSecret) == dmRecAttrSecret) != details->secret)
	{
		if (details->secret && (PrivateRecordVisualStatus == showPrivateRecords))
		{
			FrmUIAlert(privateRecordInfoAlert);
		}
		attrDirty = true;
	}


	// If the record is a repeating appointment and the repeat info
	// has not been modified,  find out if we should create an exception.
	if ( (apptRec.repeat.repeatType != repeatNone) && (!(changedFields & DBK_SELECT_REPEAT)) &&
	     (dirty || attrDirty || (attachNote && (!apptRec.note))) )
	{

		alert = FrmInitForm(gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);


		// If the alert was canceled don't apply any of the changes.
		if (alertButton == RangeCancelButton)
			goto cancelExit;

		else if (alertButton == RangeCurrentButton)
		{
			exception = true;

			// If the date of the appointment has not be changed then set the
			// date of the exception to the current date.
			if (! (changedFields & DBK_SELECT_TIME))
				when.date = Date;
		}
		else if (alertButton == RangeFutureButton)
			// Split the repeating event into two event, one of past occurrences
			// and one for future occurreneces.
		{
			pastRepeats = true;
			if (! (changedFields & DBK_SELECT_TIME))
				when.date = Date;
			newRec.when = when;
			changedFields |= DBK_SELECT_TIME;
			dirty = true;
		}
	}

	// If the record is a repeating appointment, and the repeat info has been
	// changed, and there are past occurrences of the event then we need
	// to duplicate the event so the past occurrence will continue to exist.
	if ((apptRec.repeat.repeatType != repeatNone) && (changedFields & DBK_SELECT_REPEAT))
	{
		// If any of the repeat info other than the end date has changed
		// then spawn a repeating event for all past occurrences.
		if (apptRec.repeat.repeatType != details->repeat.repeatType ||
			 apptRec.repeat.repeatFrequency != details->repeat.repeatFrequency ||
			 apptRec.repeat.repeatOn != details->repeat.repeatOn)
		{
			pastRepeats = true;

			// If the date of the appointment has not been changed then set the
			// start date of the repeating appointment to the current date.
			if (! (changedFields & DBK_SELECT_TIME))
			{
				when.date = Date;
				newRec.when = when;
				changedFields |= DBK_SELECT_TIME;
			}
		}
	}

	// If the record is a repeating appointment, and the repeat info has not
	// been changed but the start date has, then we need to adjust the
	// end date and possibly split the event into two events.
	if ((! exception) && (apptRec.repeat.repeatType != repeatNone) &&
		(!(changedFields & DBK_SELECT_REPEAT)) && (changedFields & DBK_SELECT_TIME))
	{
		// If the event is a repeating event and it has an end date, move
		// the end date such that the duration of the event is maintained.
		if ((DateToInt (apptRec.repeat.repeatEndDate) != apptNoEndDate))
		{
			adjust = DateToDays (apptRec.repeat.repeatEndDate) -
		         DateToDays (apptRec.when.date);
			details->repeat.repeatEndDate = when.date;
			DateAdjust (&details->repeat.repeatEndDate, adjust);
		}

		if (DateToInt (details->repeat.repeatEndDate) !=
			 (DateToInt(apptRec.repeat.repeatEndDate)))
		{
			newRec.repeat = details->repeat;
			changedFields |= DBK_SELECT_REPEAT;
			dirty = true;
		}
	}


	// If the record is a repeating appointment, and the start date has,
	// then delete the exceptions list.
	if ((! exception) && (apptRec.repeat.repeatType != repeatNone) &&
		(changedFields & DBK_SELECT_TIME))
	{
		ApptRemoveExceptions(&newRec);
		changedFields |= DBK_SELECT_EXCEPTIONS;
		dirty = true;
	}


	// If the repeat type is weekly and the start date of the event
	// has changed, but the user has not explicitly changed
	// the days the event repeats on, determine if the 'repeat on'
	// setting needs to be updated
	if ((! exception) &&
		 (details->repeat.repeatType == repeatWeekly) &&
		 (changedFields & DBK_SELECT_TIME)  &&
		 ((apptRec.repeat.repeatType != repeatNone) &&
		 (apptRec.repeat.repeatOn == details->repeat.repeatOn)))
	{
		// does the event only repeat once per week?
		ErrNonFatalDisplayIf(details->repeat.repeatOn == 0, "weekly appt. repeats on no days");

		newDayOfWeek = DayOfWeek (when.date.month, when.date.day, (int16_t)(when.date.year + firstYear));
		newDayMask = (uint8_t) (1 << newDayOfWeek);

		if (!(details->repeat.repeatOn & newDayMask))
		{
			// If the event only repeats on one day per week
			// update the 'repeat on' field, which contains
			// the days of the week the event repeats on, such that the
			// event occurs on the start date.
			if (OnlyRepeatsOncePerWeek(details->repeat))
				details->repeat.repeatOn = newDayMask;

			// If the event repeats on more than one day per week
			// and in the day of the newly selected date so that
			// it will be selected in the repeat on settings in the
			// event the user selected a date that a occurs on a day
			// not already set.
			else
				details->repeat.repeatOn |= newDayMask;
	
		}

		// anytime statrtime is change, the repeat on store as UTC data
		// might change in case the hours shift in th elocal time zone is 
		// related to a day change in the UTC (or GMT) time zone.
		
		changedFields |= DBK_SELECT_REPEAT;
		newRec.repeat = details->repeat;
		
	}


	// If the repeat type is monthly by day and the start date of the event
	// has been changed, update the repeat on field which contains
	// the date of the month the event repeating on (ex: first friday).
	if ((! exception) && (details->repeat.repeatType == repeatMonthlyByDay) &&
		 ( (changedFields & DBK_SELECT_TIME) || (apptRec.repeat.repeatType == repeatNone) ))
	{
		repeatOn = DayOfMonth (
						when.date.month,
						when.date.day,
						(int16_t)(when.date.year + firstYear));

		// If we're in the fourth week, and the fourth week is also the last
		// week,  ask the user which week the event repeats in (fourth or last).
		if ( ((repeatOn / daysInWeek) == 3) &&
		      (when.date.day + daysInWeek > DaysInMonth (
		      	when.date.month, (int16_t)(when.date.year + firstYear))))
		{
			alert = FrmInitForm(gApplicationDbP, MonthlyRepeatDialog);

			// Set the 4th / last push button.
			if ((apptRec.repeat.repeatType != repeatNone)
				&& apptRec.repeat.repeatType == repeatMonthlyByDay
 				&& apptRec.repeat.repeatOn > dom4thSat)
			{
				id = MonthlyRepeatLastButton;
			}
			else
			{
				id = MonthlyRepeatFourthButton;
			}

			FrmSetControlGroupSelection (alert, MonthlyRepeatWeekGroup,id);

			alertButton = FrmDoDialog (alert);

			if (FrmGetObjectIndex (alert, MonthlyRepeatLastButton) ==
			    FrmGetControlGroupSelection (alert, MonthlyRepeatWeekGroup))
			{
			  	 repeatOn += daysInWeek;
			}

			FrmDeleteForm (alert);

			if (alertButton == MonthlyRepeatCancel)
				goto cancelExit;
		}

		details->repeat.repeatOn = (uint8_t) repeatOn;
		newRec.repeat = details->repeat;
		changedFields |= DBK_SELECT_REPEAT;
	}

	// If the alarm info has been changed, or if the appointment has
	// an alarm and the date, time, or repeat info has been changed,
	// delete the item from the attention manager queue if present.
	if ((changedFields & DBK_SELECT_ALARM) ||
		((apptRec.alarm.advance != apptNoAlarm) && ((changedFields & DBK_SELECT_TIME) ||
		(changedFields & DBK_SELECT_REPEAT))) )
		DeleteAlarmIfPosted(gApptCursorID);

	// Release the appointment record.
	ApptFreeRecordMemory (&apptRec);

	// Erase the details dialog before the changes are applied to the
	// database.  It is necessary to do thing in this order because
	// erasing the details dialog may cause a frmUpdate event to be
	// sent to list view, and the list view may not be able to redraw
	// properly once the database has been changed.
	FrmEraseForm (FrmGetFormPtr (DetailsDialog));

	// Write the changes to the database.
	if (exception)
	{
		// Release the field before updating record (to unlock the record in DB)
		err = ApptAddException (ApptDB, gApptCursorID, Date);

		if (err >= errNone)
		{
			ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_ALL_FIELDS);

			memset (&newRec, 0x00, sizeof (newRec));
			newRec.description = apptRec.description;

			if (details->locationH)
			{
				locationP = MemHandleLock(details->locationH);
				newRec.location = locationP;
			}
			else
				newRec.location = apptRec.location;

			newRec.note = apptRec.note;
			newRec.when = when;

			if (details->alarm.advance != apptNoAlarm)
				newRec.alarm = details->alarm;
			else
				newRec.alarm.advance = apptNoAlarm;

			if (DateToInt(Date) != DateToInt(dateToSwitchOnExit))
			{
				Date = dateToSwitchOnExit;
				ApptDBRequeryOnDate(ApptDB, &gApptCursorID, Date);
			}

			err = CreateExceptionAndMove (&newRec, gApptCursorID, NULL, details->numCategories,
				details->pCategoryIDs);

			if (locationP)
				MemHandleUnlock(details->locationH);

			ApptFreeRecordMemory (&apptRec);
		}
	}
	else if (dirty)
	{
		// Create a copy of the repeating event and set its end date to yesterday.
		if (pastRepeats)
			err = SplitRepeatingEvent (gApptCursorID);

		if (err >= errNone)
		{
			// Release the field before updating record (to unlock the record in DB)
			err = ApptChangeRecord (ApptDB, gApptCursorID, &newRec, changedFields);
		}
	}

	if (err < errNone)
	{
		FrmUIAlert(DeviceFullAlert);
		DayViewClearEditState ();
	}

	// Save the new secret status if it's been changed.
	else if (attrDirty)
	{
		DbGetRowAttr (ApptDB, gApptCursorID, &attributes);

		if (details->secret)
			attributes |= dmRecAttrSecret;
		else
			attributes &= ~dmRecAttrSecret;

		err = DbSetRowAttr (ApptDB, gApptCursorID, &attributes);

		ErrNonFatalDisplayIf(err < errNone, "Setting attribute failed.");
	}

	// If the alarm info has been changed, or if the appointment has
	// an alarm and the date, time, or repeat info has been changed,
	// reschedule the next alarm.
	if ((changedFields & DBK_SELECT_ALARM) ||
		((apptRec.alarm.advance != apptNoAlarm) && ((changedFields & DBK_SELECT_TIME) ||
			(changedFields & DBK_SELECT_REPEAT))))
		RescheduleAlarmsAllRows();

	// Update date in order to switch on details exit
	if (changedFields & DBK_SELECT_REPEAT)
	{
		// Reload the newly updated record, including new exceptions...
		ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);

		switchDate = ApptRepeatsOnDate(&apptRec, dateToSwitchOnExit);
		if (!switchDate)
		{
			switchDate = ApptNextRepeat(&apptRec, &dateToSwitchOnExit, true);
			if (!switchDate)
			{
				// No match, clear the current edit state...
				ApptFreeRecordMemory (&apptRec);
				DayViewClearEditState();
			}
		}
	}
	else
		switchDate = true;


	// we need to have the database cursor updated
	// when the date is changing  as well as the categories set
	// we have only to ask for cursor update once as either
	// ApptDBRequeryOnDate or ApptDBRequery will
	// update the categories set

	if ( !switchDate && switchCategories)
	{
		ApptFreeRecordMemory (&apptRec);
		ApptDBRequeryOnCategories(ApptDB, &gApptCursorID);
	}
	else if (switchDate ||switchCategories )
	{
		// The event or the exception created moved to a new day, we have
		// to switch to this new day and therefore requery the cursor with
		// a new range.

		ApptFreeRecordMemory (&apptRec);
		if (DateToInt(Date) != DateToInt(dateToSwitchOnExit))
		{
			Date = dateToSwitchOnExit;
			ApptDBRequeryOnDate(ApptDB, &gApptCursorID, Date);
		}
	}


	return true;


cancelExit:
	// We're here if the cancel button one of the confirmation dialogs
	// was pressed.

	ApptFreeRecordMemory (&apptRec);
	return false;
}




/***********************************************************************
 *
 * FUNCTION:		DetailsFocusUpDayCell
 *
 * DESCRIPTION: 	Handle what to do on up key when navigation is enabled.
 *					Depending on the item Id focused.
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static Boolean DetailsFocusUpDayCell(EventType* eventP)
{
	Boolean handled = false;

	switch(sDetailsFocusState)
	{
		case kFocusDetailsNoFocus:
			// tapping down does same thing than to start time
			// (upper dialog part)
			//FrmNavObjectTakeFocus(sFormP, DetailsStartTimeSelector);
			//handled = true;

			// does nothing per UX spec
			break;

		/*
		case DetailsOkButton:
		case DetailsCancelButton:
		case DetailsDeleteButton:
		case DetailsNoteButton:
		case DetailsStartTimeSelector:
		case DetailsEndTimeSelector:
		case DetailsDateSelector:
		case DetailsBookTabs:
		case DetailsRepeatSelector:
		case DetailsAlarmCheckbox:
		case DetailsAlarmAdvanceField:
		case DetailsAlarmAdvanceSelector:
		case DetailsPrivateCheckbox:
		case DetailsCategoryPopSelector:
		case DetailsLocationField:
		case DetailsTimeZoneSelector:
		*/
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusDownDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static Boolean DetailsFocusDownDayCell(EventType* eventP)
{
	Boolean handled = false;

	switch(sDetailsFocusState)
	{
		case kFocusDetailsNoFocus:
			// tapping down does same thing than tothe first item in tabs
			// (middle dialog part)
			//FrmNavObjectTakeFocus(sFormP, DetailsLocationField);

			// tapping down does same thing than center select
			// per UX spec
			FrmNavObjectTakeFocus(sFormP, DetailsOkButton);
			handled = true;
			break;


		/*
		case DetailsOkButton:
		case DetailsCancelButton:
		case DetailsDeleteButton:
		case DetailsNoteButton:
		case DetailsStartTimeSelector:
		case DetailsEndTimeSelector:
		case DetailsDateSelector:
		case DetailsBookTabs:
		case DetailsRepeatSelector:
		case DetailsAlarmCheckbox:
		case DetailsAlarmAdvanceField:
		case DetailsAlarmAdvanceSelector:
		case DetailsPrivateCheckbox:
		case DetailsCategoryPopSelector:
		case DetailsLocationField:
		case DetailsTimeZoneSelector:
		*/
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusRightDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static Boolean DetailsFocusRightDayCell(EventType* eventP)
{
	uint16_t	modifiers 	= eventP->data.keyDown.modifiers;
	Boolean 	handled = false;

	switch(sDetailsFocusState)
	{
		case kFocusDetailsNoFocus:
			DetailsActiveCycleTab(sDetailsBook);
			handled = true;
			break;

		//case DetailsOkButton:
		//case DetailsCancelButton:
		//case DetailsDeleteButton:
		//case DetailsNoteButton:
		//case DetailsStartTimeSelector:
		case DetailsEndTimeSelector:
		case DetailsDateSelector:
		case DetailsBookTabs:
			// this handle pins block whithin bookslib
			handled = BookFocusNextTab(sDetailsBook, &modifiers);
			if (handled)
				break;

		case DetailsRepeatSelector:
		case DetailsPrivateCheckbox:
		case DetailsCategoryPopSelector:
		case DetailsLocationField:
		case DetailsTimeZoneSelector:
			FrmNavObjectTakeFocus(sFormP, DetailsOkButton);
			handled = true;
			break;

		case DetailsAlarmCheckbox:
		case DetailsAlarmAdvanceField:
		case DetailsAlarmAdvanceSelector:
			if (sAlarmIsSet)
				break;

			FrmNavObjectTakeFocus(sFormP, DetailsOkButton);
			handled = true;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusLeftDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static Boolean DetailsFocusLeftDayCell(EventType* eventP)
{
	uint16_t	modifiers	= eventP->data.keyDown.modifiers;
	Boolean handled = false;

	switch(sDetailsFocusState)
	{
		case kFocusDetailsNoFocus:
			// when nothing is focused we change tab tapping left key
			DetailsActiveCycleTab(sDetailsBook);
			handled = true;
			break;
		/*
		case DetailsOkButton:
		case DetailsCancelButton:
		case DetailsDeleteButton:
		case DetailsNoteButton:
		case DetailsStartTimeSelector:
		case DetailsEndTimeSelector:
		case DetailsDateSelector:
		case DetailsBookTabs:
		case DetailsRepeatSelector:
		case DetailsAlarmCheckbox:
		case DetailsAlarmAdvanceField:
		case DetailsAlarmAdvanceSelector:
		case DetailsPrivateCheckbox:
		case DetailsCategoryPopSelector:
		case DetailsLocationField:
		case DetailsTimeZoneSelector:
		*/
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusEnter
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static Boolean DetailsFocusEnter(EventType* eventP)
{
	// nothing special si done when focus center is pressed
	// so the default behavior is correct.
	return false;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusTake
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	eventP: event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static void DetailsFocusTake(EventType* eventP)
{
	sDetailsFocusState = eventP->data.frmObjectFocusTake.objectID;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsFocusLost
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	eventP:	event data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/14/04	Creation.
 *
 ***********************************************************************/
static void DetailsFocusLost(EventType* eventP)
{
	if (sDetailsFocusState == eventP->data.frmObjectFocusTake.objectID)
		sDetailsFocusState = kFocusDetailsNoFocus;
}


/***********************************************************************
 *
 * FUNCTION:		DetailsHandleNavigation
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->  modifiers: 	 	Command mofifiers.
 * 			 	->  chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
Boolean DetailsHandleNavigation(EventType* eventP, uint16_t	modifiers, wchar32_t chr )
{
	Boolean handled = false;

	TraceOutput(TL(appErrorClass, "DetailsView, DetailsHandleNavigation start"));

	// we do not check here details view is already focused
	// as some action has to be performed when nothing is focused.

	switch(chr)
	{
		case vchrRockerUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerUp, Above Cell is focused"));
			handled = DetailsFocusUpDayCell(eventP);
			break;


		case vchrRockerDown:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerDown, Below Cell is focused"));
			handled = DetailsFocusDownDayCell(eventP);
			break;


		case vchrThumbWheelBack:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelBack, next/below item, left not handled"));
			break;


		case vchrThumbWheelDown:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelDown as vchrRockerLeft"));

		case vchrRockerLeft:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerLeft, Previous tab is focused"));
			// 5-way rocker left
			// move focused tab to left
			handled = DetailsFocusLeftDayCell(eventP);
			break;


		case vchrThumbWheelUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelUp Next tab is focused"));


		case vchrRockerRight:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerRight, Next tab is focused"));
			// 5-way rocker right
			// move focused tab to right
			handled = DetailsFocusRightDayCell(eventP);
			break;


		case vchrThumbWheelPush:
			TraceOutput(TL(appErrorClass, "-------------> keyDownEvent, vchrThumbWheelPush as vchrRockerCenter"));


		case vchrRockerCenter:
			TraceOutput(TL(appErrorClass, "-------------> keyDownEvent, vchrRockerCenter"));
			// 5-way rocker center/press
			// active focused tab
			handled = DetailsFocusEnter(eventP);
			break;


		default:
			TraceOutput(TL( appErrorClass, "-------------> Not a 5-way or Wheel Navigation vchr"));
			break;
	}

	TraceOutput(TL(appErrorClass, "DetailsView, DetailsHandleNavigation exit"));
	return handled;
}



/***********************************************************************
 *
 * FUNCTION:    	DetailsHandleEvent
 *
 * DESCRIPTION: 	This routine is the event handler for the "Details
 *              	Dialog Box".
 *
 * PARAMETERS:  	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:    	true if the event has handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	7/20/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
Boolean DetailsHandleEvent (EventType* eventP)
{
	FormPtr 		frm;
	TablePtr		tableP;
	FieldPtr		fieldP;
	DetailsPtr 		detailsP = (DetailsPtr) ApptDetailsP;
	wchar32_t		chr;
	uint16_t		modifiers;
	Boolean			recordMasked;
	Boolean			recordHidden;
	Boolean			changeCategory = false;
	Boolean 		handled = false;

	// Trace the event in debug mode
	DBGTraceEvent("DetailsDialog", eventP);

	switch (eventP->eType)
	{
		case keyDownEvent:
			chr 		= eventP->data.keyDown.chr;
			modifiers	= eventP->data.keyDown.modifiers;

			if (TxtCharIsRockerKey(modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
			{
				handled = DetailsHandleNavigation(eventP, modifiers, chr);
			}
			break;


		case ctlSelectEvent:
		{
			switch (eventP->data.ctlSelect.controlID)
			{
				case DetailsOkButton:
					if (DetailsApply (detailsP, false))
					{
						recordMasked = DetailsCheckRecordPrivacy(&recordHidden);
						DetailsExit();

						FrmReturnToForm (DayView);


						//note: Day View is already loaded

						if (recordMasked)
						{
							// Clear the edit state so the current record will be masked
							DayViewClearEditState();
						}
						else
						{
							// Release the focus because the record row may have changed
							tableP = UtilitiesGetObjectPtr(DayView, DayTable);
							TblReleaseFocus(tableP);
							fieldP = TblGetCurrentField(tableP);
							if (fieldP)
								FldReleaseFocus(fieldP);
						}

						// Force the cursor to be refreshed if the record has to be hidden
						if (recordHidden)
							ApptDBRequery(ApptDB, gApptCursorID, true);


						DayViewDrawDate (Date, true);

					}

					handled = true;
					break;


				case DetailsCancelButton:
					// If the categories have changed restore the original ones.
					// We need to check that the original categories have not
					// been deleted or merged
					if (detailsP->categoryChanged)
					{
						if (!CategoriesTestEquality(
								detailsP->numCategories,
								detailsP->pCategoryIDs,
								detailsP->savedNumCategories,
								detailsP->savedCategoryIDs))
						{
							// Set the current displaying category to the record one,
							// Otherwise, the user may not see its record after tapping
							// on the Ok button.
							ChangeCurrentCategories(detailsP->numCategories, detailsP->pCategoryIDs);

							// we need to have the database cursor updated
							ApptDBRequeryOnCategories(ApptDB, &gApptCursorID);

							changeCategory = true;
						}
					}

					DetailsExit();
					FrmReturnToForm (DayView);

					if (DetailsCheckRecordPrivacy(NULL))
						DayViewClearEditState();		// The record will be masked

					// Release the focus because the record row may have changed because of AIA update.
					FrmSetFocus(FrmGetFormPtr (DayView), noFocus);

					if (changeCategory)
						DayViewDrawDate (Date, true);
					else DayViewRefreshDisplay(false, true);


					// Set up Day View item selection
					gItemSelected = true;

					// Set the cursor at the end of the line in Day View
					DayEditPosition = mostRightFieldPosition;
					handled = true;
					break;


				case DetailsDeleteButton:
					DetailsExit();
					FrmReturnToForm (DayView);

					if ( DetailsDeleteAppointment ())
						DayViewRefreshDisplay(true, true);

					handled = true;
					break;


				case DetailsNoteButton:
					if (DetailsApply (detailsP, true))
					{
						if (CreateNote (false))
							DetailsEditNote(gApptCursorID);
					}

					DetailsExit();
					FrmReturnToForm (DayView);

					if (DetailsCheckRecordPrivacy(&recordHidden))
					{
						CurrentRecordVisualStatus = ~PrivateRecordVisualStatus;
						DayViewClearEditState();		// The record will be masked
					}
					else
					{
						// Release the focus because the record row may have changed
						FrmSetFocus(FrmGetFormPtr (DayView), noFocus);

						if (recordHidden)
							ApptDBRequery(ApptDB, gApptCursorID, true);

						DayViewRefreshDisplay(true, true);
					}
					handled = true;
					break;


				case DetailsStartTimeSelector:
				case DetailsEndTimeSelector:
					DetailsSelectTime (
							detailsP,
							&detailsP->when.startTime,
							&detailsP->when.endTime,
							&detailsP->when.midnightWrapped,
							&detailsP->when.noTimeEvent,
							eventP->data.ctlSelect.controlID);

					// After DetailsSelectTime() we need to call DetailsSelectDate()
					DetailsSetDateTrigger(detailsP->when.date, detailsP->when.midnightWrapped);

					// Update the timezone trigger because we might have switched between timed
					// and untimed events.
					DetailsUpdateTimeZoneDisplayed(detailsP);
					handled = true;
					break;

				case DetailsDateSelector:
					DetailsSelectDate (detailsP);
					handled = true;
					break;


				case DetailsAlarmCheckbox:
					DetailsAlarmOnOff (eventP->data.ctlSelect.on);
					handled = true;
					break;


				case DetailsRepeatSelector:
					if (DetailsGet (detailsP))
					{
						// Give control of the details info block to the Repeat form
						FrmPopupForm(gApplicationDbP, RepeatDialog);
					}
					handled = true;
					break;


				case DetailsTimeZoneSelector:
					DetailsSelectTimeZone(detailsP);
					handled = true;
					break;


				case DetailsCategoryPopSelector:
					DetailsSelectCategory(detailsP);
					handled = true;
					break;
			}
			break;
		}


		case frmOpenEvent:
			sFormP = frm = FrmGetActiveForm ();
			DetailsSetTitle (frm);
			DetailsInit (frm);
			handled = true;
			break;


		case frmCloseEvent:
			frm = FrmGetActiveForm ();
			DetailsExit();
			DetailsFreeTitle (frm);
			sFormP = NULL;
			break;


		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.formID == DetailsDialog)
			{
				DetailsFocusTake(eventP);
			}
			break;


		case frmObjectFocusLostEvent:
 			if (eventP->data.frmObjectFocusLost.formID == DetailsDialog)
			{
				DetailsFocusLost(eventP);
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DoEventDetailsDialog
 *
 * DESCRIPTION: 	This routine Handles a tap on the "details" button.
 *					It opens the details dialog.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	01/08/03	Initial revision.
 * 	PPL 07/05/04	Move to DateDayDetails.
 *					Add PalmSource One Hande Spcification.
 *
 ***********************************************************************/
void DoEventDetailsDialog(void)
{
	if (gItemSelected || gItemWasSelected)
	{
		FrmPopupForm(gApplicationDbP, DetailsDialog);
	}
	else
		FrmAlert(gApplicationDbP, SelectItemAlert);
}
