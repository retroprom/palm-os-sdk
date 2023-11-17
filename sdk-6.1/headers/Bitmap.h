/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Bitmap.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *        This file defines bitmap structures and routines.
 *
 *****************************************************************************/

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <PalmTypes.h>

// Type definitions used by PalmOS and the DAL
#include <CmnBitmapTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
// Routines relating to bitmap management
// -----------------------------------------------

extern BitmapType* BmpCreate (Coord width, Coord height, uint8_t depth, ColorTableType * colortableP, status_t * error);

extern status_t      BmpDelete (BitmapType * bitmapP);

extern status_t      BmpCompress (BitmapType * bitmapP, BitmapCompressionType compType);

extern void*    BmpGetBits (BitmapType * bitmapP);

extern ColorTableType* BmpGetColortable (BitmapType * bitmapP);

extern uint32_t   BmpSize (const BitmapType * bitmapP);

extern void		 BmpGetSizes (const BitmapType * bitmapP, uint32_t * dataSizeP, uint32_t * headerSizeP);

extern uint16_t   BmpColortableSize (const BitmapType * bitmapP);

extern void     BmpGetDimensions (const BitmapType * bitmapP, Coord * widthP, Coord * heightP, uint16_t * rowBytesP);

extern uint8_t    BmpGetBitDepth (const BitmapType * bitmapP);

extern PixelFormatType BmpGetPixelFormat(const BitmapType * bitmapP);

extern BitmapType* BmpGetNextBitmap (BitmapType * bitmapP);

//-----------------------------------------------
// High Density support functions
//-----------------------------------------------

extern BitmapType* BmpGetNextBitmapAnyDensity(BitmapType* bitmapP);

extern uint8_t	BmpGetVersion(const BitmapType* bitmapP);

extern BitmapCompressionType	BmpGetCompressionType(const BitmapType* bitmapP);

extern uint16_t	BmpGetDensity(const BitmapType* bitmapP);

extern status_t		BmpSetDensity(BitmapType* bitmapP, uint16_t density);
							
extern Boolean	BmpGetTransparentValue(const BitmapType* bitmapP, uint32_t* transparentValueP);
							
extern void		BmpSetTransparentValue(BitmapType* bitmapP, uint32_t transparentValue);
							
extern BitmapTypeV3* BmpCreateBitmapV3(const BitmapType* bitmapP, uint16_t density, const void* bitsP, const ColorTableType* colorTableP);

extern BitmapType* BmpCreateVersion3 (Coord width, Coord height, uint8_t depth,
				   ColorTableType * colortableP, uint16_t density,
				   uint32_t transparency, PixelFormatType pixelFormat,
				   status_t * error);

#ifdef __cplusplus
}
#endif


#endif							// _BITMAP_H_
