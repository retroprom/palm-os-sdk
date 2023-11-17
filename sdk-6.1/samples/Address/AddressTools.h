/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTools.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSTOOLS_H_
#define _ADDRESSTOOLS_H_

#include <DataMgr.h>

#include <UIColor.h>
#include <Form.h>
#include <ExgMgr.h>
#include <Bitmap.h>
#include <LocaleMgrTypes.h>
#include <DateTime.h>

#include "AddressTab.h"
#include "AddressDisplayLayouts.h"

// Abbreviations: String sizes
#define kMaxFamilySuffixSize		5	//	Ok for Double byte strings
#define kMaxAbbreviationSize		9	//  Ok for Double byte strings
#define kMaxPhoneColumnWidth		80 // (415)-000-0000x...

#define kBackgroundColorRed			234
#define kBackgroundColorGreen		200
#define kBackgroundColorBlue		128

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean		ToolsIsDialerPresent(void);

char *		ToolsGetRecordNameStr(DmOpenRef dbP, uint32_t rowID, FontID fontID, Coord maxWidth);

void 		ToolsComputePhoneLabelWidth(void);

char*		ToolsGetPhoneNumber(uint32_t rowID, uint32_t phoneColID, FontID fontID, Coord maxWidth, Coord* phoneWidth, char** phoneAbbrev);

uint16_t	ToolsDrawRecordNameAndPhoneNumber(uint32_t rowID, RectanglePtr bounds, FontID fontID, Boolean customDraw, Boolean phoneHighlight);

Coord		ToolsGetLabelColumnWidth(uint32_t rowID, AddressBookInfoType* bookInfo, uint32_t tabIndex, FontID labelFontID, Boolean onlyViewableLabel);

void		ToolsChangeCategory(CategoryID *categoriesP, uint32_t numCategories);

FontID		ToolsSelectFont(uint16_t formID, FontID currFontID);

Boolean		ToolsIsPhoneFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex, Boolean strictPhone);

Boolean		ToolsIsPhoneFieldByColId(uint32_t columnID, Boolean strictPhone);

Boolean		ToolsIsYomiFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex);

Boolean		ToolsIsYomiFieldByColId(uint32_t columnID);

Boolean		ToolsIsTextFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex);

Boolean		ToolsIsTextFieldByColId(uint32_t columnID);

status_t	ToolsCustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP);

uint16_t 	ToolsGetGraffitiObjectIndex(FormType* formP);

void *		ToolsGetFrmObjectPtr(FormType* frmP, DmResourceID objectID);

Boolean		ToolsAddrBeamBusinessCard(void);

void		ToolsAddrAttachRecord(DmOpenRef dbP, uint32_t rowIDToSend);

Boolean		ToolsIsPhoneIndexSupported(uint32_t rowID, uint32_t columnID);

void		ToolsGetAbbreviationFromProperty(char *abbrevStringP, uint32_t property);

char*		ToolsBuildDateAndTimeString(DateTimeType* dateTimeP);

void		ToolsFrmInvalidateWindow(uint16_t formID);

void		ToolsFrmInvalidateRect(uint16_t formID, RectangleType *rectP);

void		ToolsFrmInvalidateRectFunc(uint16_t formID, RectangleType *rectP, winInvalidateFunc callbackP, void *userParamP);

void 		ToolsDrawTextLabel(char* textLabel, size_t textLen, Coord x, Coord y, Boolean highlight);

char *		ToolsStrDup(char *srcP);

char*		ToolsCopyStringResource (DmResourceID stringResourceID);

void		ToolsDrawFullnamePopup(MemHandle fullnameH, RectangleType *popupBoundsP);

char*		ToolsStripNonPrintableChar(char* stringToStrip);

DmResourceID	ToolsCountryToResourceID(LmCountryType country);

DmResourceID	ToolsGetCountryBaseResID(DmResourceType resType, DmResourceID baseID, uint16_t defaultIncrement);

Boolean		ToolsCursorMoveNext(void);

Boolean		ToolsCursorMovePrevious(void);

void		ToolsSetBusinessCardIndicatorPosition(FormType *formP);

Boolean		ToolsBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

Boolean		ToolsConfirmDeletion(void);

void		ToolsCheckCategories(CategoryID *categoriesP, uint32_t *numCategoriesP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSTOOLS_H_
