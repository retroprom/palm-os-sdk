/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: HWREngine.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This header file describes the API for HWRE (handwriting recognition
 *  engines), part of Pen Input Services.
 *
 *****************************************************************************/

#ifndef _HWRENGINE_H_
#define _HWRENGINE_H_

#include <CmnRectTypes.h>
#include <PenInputMgr.h>


// Error codes
#define hwreErrorClass            0
#define hwreErrBadParam           (hwreErrorClass | 1)
#define hwreErrPointBufferFull    (hwreErrorClass | 2)


// Ink hints
#define hwrInkHintNone            0
#define hwrInkHintEraseAll        1
#define hwrInkHintKeepAll         2
#define hwrInkHintKeepLastOnly    3


// HWR configuration parameters
#define kMaxHWRModeAreas  4

typedef struct
{
	uint16_t writingMode;
	uint16_t reserved;
	RectangleType modeBounds;
} HWRConfigModeArea;

typedef struct
{
	// Hardware info
	uint16_t hDotsPerInch;
	uint16_t vDotsPerInch;
  
	// Writing areas
	RectangleType writingBounds;
	uint32_t numModeAreas;
	HWRConfigModeArea modeArea[kMaxHWRModeAreas];
} HWRConfig;


// HWR stroke result info
#define kHWRMaxData  32

typedef struct
{
	wchar32_t chr;
	uint32_t flags;
} CharData;

typedef struct
{
	// Characters to send
	CharData chars[kHWRMaxData];
	uint16_t numChars;
	uint16_t uncertain;
	uint16_t deleteUncertain;

	// Display hints
	uint16_t inputMode;
	uint16_t inkHint;

	// Delayed processing
	Boolean timeout;     // Set to true to be notified later.

	uint8_t reserved;
} HWRResult;


typedef struct HWREContext_tag HWREContext;


// HWREngine - Generic Handwriting Recognition engine.

#ifdef __cplusplus
extern "C" {
#endif

// Initialize HWR engine
HWREContext *HWRInit(const HWRConfig *config);

// Release resources held by the HWR engine
status_t HWRShutdown(HWREContext *ctx);

// Process a stroke
status_t HWRProcessStroke(HWREContext *ctx,
						  const PointType *points,
						  uint32_t numPoints,
						  HWRResult *result);

// Finish processing after a timeout
status_t HWRTimeout(HWREContext *ctx, HWRResult *result);

// Input mode (shift vs capslock vs ...)
// Modes are defined in PenInputMgr.h
void HWRSetInputMode(HWREContext *ctx, uint16_t inputMode);
uint16_t HWRGetInputMode(HWREContext *ctx);

// Reset internal state of HWR
status_t HWRClearInputState(HWREContext *ctx);


// HWR reference/help dialog
void HWRShowReferenceDialog(void);


// The following functions access a shared global context.

// Initialize HWR engine
status_t HWRInitV60(const HWRConfig *config);

// Release resources held by the HWR engine
status_t HWRShutdownV60(void);

// Process a stroke
status_t HWRProcessStrokeV60(const PointType *points, uint32_t numPoints,
							 HWRResult *result);

// Finish processing after a timeout
status_t HWRTimeoutV60(HWRResult *result);

// Input mode (shift vs capslock vs ...)
// Modes are defined in PenInputMgr.h
void HWRSetInputModeV60(uint16_t inputMode);
uint16_t HWRGetInputModeV60(void);

// Reset internal state of HWR
status_t HWRClearInputStateV60(void);


#ifdef __cplusplus
}
#endif

#endif /* _HWRENGINE_H_ */
