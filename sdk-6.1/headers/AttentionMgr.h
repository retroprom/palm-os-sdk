/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AttentionMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Include file for Attention Manager
 *
 *****************************************************************************/

#ifndef _ATTENTION_MGR_H_
#define _ATTENTION_MGR_H_

#include <PalmTypes.h>
#include <Rect.h>
#include <SysEventCompatibility.h>
#include <Event.h>

/************************************************************
 * Attention Manager result codes
 * (attnErrorClass is defined in ErrorBase)
 *************************************************************/
#define	attnErrMemory			(attnErrorClass | 1)	// ran out of memory
#define attnErrItemNotFound		(attnErrorClass | 2)    // An Item not found


/************************************************************
 * Attention Indicator bounds
 *************************************************************/
#define kAttnIndicatorLeft		0
#define kAttnIndicatorTop		0
#define kAttnIndicatorWidth		16
#define kAttnIndicatorHeight	15
	

/************************************************************
 * Constants used for list view drawing.
 *
 * Applications should use the following constants to format
 * the display of information in attention manager list view.
 *
 * The application's small icon should be drawn centered within
 * the first kAttnListMaxIconWidth pixels of the drawing bounds.
 *
 * Two lines of text information describing the attention should
 * then be drawn left justified starting at kAttnListTextOffset
 * from the left edge of the drawing bounds.
 *************************************************************/
#define	kAttnListMaxIconWidth	15
#define	kAttnListTextOffset		17
			
			
/********************************************************************
 * Attention Manager Structures
 ********************************************************************/

typedef uint32_t AttnFlagsType;

#define kAttnFlagsSoundBit				((AttnFlagsType)0x0001)
#define kAttnFlagsLEDBit				((AttnFlagsType)0x0002)
#define kAttnFlagsVibrateBit			((AttnFlagsType)0x0004)
#define kAttnFlagsCustomEffectBit		((AttnFlagsType)0x0008)
	// Note: More bits can be defined if/when hardware capability increases

#define kAttnFlagsAllBits				((AttnFlagsType)0xFFFF)


// The following are passed to AttnGetAttention() and AttnUpdate to specify
// overrides from the user settings for an attention request.
#define kAttnFlagsUseUserSettings		((AttnFlagsType)0x00000000)

#define kAttnFlagsAlwaysSound			(kAttnFlagsSoundBit)
#define kAttnFlagsAlwaysLED				(kAttnFlagsLEDBit)
#define kAttnFlagsAlwaysVibrate			(kAttnFlagsVibrateBit)
#define kAttnFlagsAlwaysCustomEffect 	(kAttnFlagsCustomEffectBit)
#define kAttnFlagsEverything			(kAttnFlagsAllBits)

#define kAttnFlagsNoSound				(kAttnFlagsSoundBit<<16)
#define kAttnFlagsNoLED					(kAttnFlagsLEDBit<<16)
#define kAttnFlagsNoVibrate				(kAttnFlagsVibrateBit<<16)
#define kAttnFlagsNoCustomEffect		(kAttnFlagsCustomEffectBit<<16)
#define kAttnFlagsNothing				(kAttnFlagsAllBits<<16)


// The following are used to interpret the feature.
#define kAttnFtrCreator					'attn'
#define kAttnFtrCapabilities			0			// Read to determine device capabilities and user settings.

#define kAttnFlagsUserWantsSound		(kAttnFlagsSoundBit)
#define kAttnFlagsUserWantsLED			(kAttnFlagsLEDBit)
#define kAttnFlagsUserWantsVibrate		(kAttnFlagsVibrateBit)
#define kAttnFlagsUserWantsCustomEffect (kAttnFlagsCustomEffectBit)	// Always false
#define kAttnFlagsUserSettingsMask		(kAttnFlagsAllBits)

#define kAttnFlagsHasSound				(kAttnFlagsSoundBit<<16)
#define kAttnFlagsHasLED				(kAttnFlagsLEDBit<<16)
#define kAttnFlagsHasVibrate			(kAttnFlagsVibrateBit<<16)
#define kAttnFlagsHasCustomEffect		(kAttnFlagsCustomEffectBit<<16)	// Always true
#define kAttnFlagsCapabilitiesMask		(kAttnFlagsAllBits<<16)


typedef uint16_t AttnLevelType;
	#define kAttnLevelInsistent			((AttnLevelType)0)
	#define kAttnLevelSubtle			((AttnLevelType)1)

typedef uint16_t AttnCommandType;
	#define kAttnCommandDrawDetail		((AttnCommandType)1)
	#define kAttnCommandDrawList		((AttnCommandType)2)
	#define kAttnCommandPlaySound		((AttnCommandType)3)
	#define kAttnCommandCustomEffect	((AttnCommandType)4)
	#define kAttnCommandGoThere			((AttnCommandType)5)
	#define kAttnCommandGotIt			((AttnCommandType)6)
	#define kAttnCommandSnooze			((AttnCommandType)7)
	#define kAttnCommandIterate			((AttnCommandType)8)

typedef union AttnCommandArgsTag {
	struct AttnCommandArgsDrawDetailTag {
		RectangleType bounds;
		Boolean firstTime;
		uint8_t padding1;
		uint16_t padding2;
		AttnFlagsType flags;
	} drawDetail;
	
	struct AttnCommandArgsDrawListTag {
		RectangleType bounds;
		Boolean firstTime;
		uint8_t padding1;
		uint16_t padding2;
		AttnFlagsType flags;
		Boolean selected;
		uint8_t padding3;
		uint16_t padding4;
	} drawList;
	
	struct AttnCommandArgsGotItTag {
		Boolean dismissedByUser;
	} gotIt;
	
	struct AttnCommandArgsIterateTag {
		uint32_t iterationData;
	} iterate;
} AttnCommandArgsType;

typedef struct {
	AttnCommandType command;
	uint16_t padding;
	uint32_t userData;
	AttnCommandArgsType *commandArgsP;
} AttnLaunchCodeArgsType;

// These details go with the sysNotifyGotUsersAttention notification.
typedef struct {
	AttnFlagsType flags;
} AttnNotifyDetailsType;


/********************************************************************
 * Public Attention Manager Routines
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

status_t	AttnGetAttention (DatabaseID dbID, uint32_t userData,
			AttnLevelType level, AttnFlagsType flags,
			uint16_t nagRateInSeconds, uint16_t nagRepeatLimit);

Boolean		AttnUpdate (DatabaseID dbID, uint32_t userData,
			AttnFlagsType *flagsP, uint16_t *nagRateInSecondsP,
			uint16_t *nagRepeatLimitP);

Boolean		AttnForgetIt (DatabaseID dbID, uint32_t userData);

uint16_t	AttnGetCounts (DatabaseID dbID, uint16_t *insistentCountP, uint16_t *subtleCountP);

void		AttnListOpen (void);

void		AttnIterate (DatabaseID dbID, uint32_t iterationData);

status_t	AttnDoSpecialEffects(AttnFlagsType flags);

void		AttnIndicatorEnable(Boolean enableIt);

Boolean		AttnIndicatorEnabled(void);

status_t	AttnGetAttentionV40(uint16_t cardNo, LocalID dbID, uint32_t userData,
			AttnLevelType level, AttnFlagsType flags,
			uint16_t nagRateInSeconds, uint16_t nagRepeatLimit);

Boolean		AttnUpdateV40(uint16_t cardNo, LocalID dbID, uint32_t userData,
			AttnFlagsType *flagsP, uint16_t *nagRateInSecondsP,
			uint16_t *nagRepeatLimitP);

Boolean		AttnForgetItV40(uint16_t cardNo, LocalID dbID, uint32_t userData);

uint16_t	AttnGetCountsV40(uint16_t cardNo, LocalID dbID, uint16_t *insistentCountP,
			uint16_t *subtleCountP);

void		AttnIterateV40(uint16_t cardNo, LocalID dbID, uint32_t iterationData);

#ifdef __cplusplus
}
#endif

#endif  // _ATTENTION_MGR_H_
