/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AudioTypes.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _AUDIOTYPES_H
#define _AUDIOTYPES_H

#include <PalmTypes.h>
#include <BuildDefaults.h>

typedef enum audio_type_
{
	sndInt8 		= 0x01,
	sndUInt8		= 0x11,

	sndInt16Big		= 0x02,
	sndInt16Little	= 0x12,
	sndInt32Big		= 0x04,
	sndInt32Little	= 0x14,
	sndFloatBig		= 0x24,
	sndFloatLittle	= 0x34,
	
#if CPU_ENDIAN == CPU_ENDIAN_BIG
	sndInt16		= sndInt16Big,			// native-endian
	sndInt16Opposite= sndInt16Little,		// opposite of native-endian 
	sndInt32		= sndInt32Big,		
	sndInt32Opposite= sndInt32Little,		
	sndFloat		= sndFloatBig,
	sndFloatOpposite= sndFloatLittle,
#else
	sndInt16		= sndInt16Little,
	sndInt16Opposite= sndInt16Big,
	sndInt32		= sndInt32Little,		
	sndInt32Opposite= sndInt32Big,
	sndFloat		= sndFloatLittle,
	sndFloatOpposite= sndFloatBig,
#endif
	
	sndAudioTypeEnd = (int32_t) 0x80000000
} audio_type_t;

#endif

