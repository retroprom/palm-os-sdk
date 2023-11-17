/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CmnBitmapTypes.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Bitmap types and definitions shared by DAL and Palm OS.
 *
 *****************************************************************************/

#ifndef _CMNBITMAPTYPES_H_
#define _CMNBITMAPTYPES_H_

#include <PalmTypes.h>

//-----------------------------------------------
// The Bitmap Structure.
//-----------------------------------------------

// bitmap version numbers
#define BitmapVersionZero			0
#define BitmapVersionOne			1
#define BitmapVersionTwo			2
#define BitmapVersionThree			3


// Compression Types for BitMap BitmapVersionTwo.
enum BitmapCompressionTag
{
	BitmapCompressionTypeScanLine	= 0,
	BitmapCompressionTypeRLE,
	BitmapCompressionTypePackBits,
	
	BitmapCompressionTypeEnd,		// must follow last compression algorithm
	
	BitmapCompressionTypeBest		= 0x64,			
	BitmapCompressionTypeNone		= 0xFF
} ;
typedef Enum8 BitmapCompressionType ;


// pixel format defined with BitmapVersionThree
enum PixelFormatTag
{
	pixelFormatIndexed,		// standard for Palm 68k;  standard for BMPs
	pixelFormat565,			// standard for Palm 68k
	pixelFormat565LE,		// standard for BMPs;  popular on ARM hardware
	pixelFormatIndexedLE,	// popular on ARM hardware
	pixelFormat5551,		// 15 bit color with an alpha channel, MUCH faster than 565 with a "transparent color".
							// Note that tools to generate 5551 format bitmaps may not be available yet, but they can be drawn.
	pixelFormat4444			// 4 bit Red, green blue and alpha. Provides 4096 colors and 16 levels of transparency for each color.
} ;
typedef Enum8 PixelFormatType;


// constants used by density field
enum DensityTag {
	kDensityLow						= 72,
	kDensityOneAndAHalf				= 108,
	kDensityDouble					= 144,
	kDensityTriple					= 216,
	kDensityQuadruple				= 288
};
typedef Enum16 DensityType;


#define bmpFlagsCompressed			0x8000
#define bmpFlagsHasColorTable		0x4000
#define bmpFlagsHasTransparency		0x2000
#define bmpFlagsIndirect			0x1000
#define bmpFlagsForScreen			0x0800
#define bmpFlagsDirectColor			0x0400
#define bmpFlagsIndirectColorTable	0x0200
#define bmpFlagsNoDither			0x0100

#define kTransparencyNone			((uint32_t)0xFFFFFFFF)

 // Base BitmapType structure
typedef struct BitmapType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
 	int16_t  			widthBE16;
	int16_t  			heightBE16;
	uint16_t  			rowBytesBE16;
	uint16_t			flagsBE16;
	uint8_t				pixelSize;
	uint8_t				version;					
}
#endif
BitmapType;
typedef BitmapType *BitmapPtr;



// This data structure is the version 3 BitmapType.
typedef struct BitmapTypeV3
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	// BitmapType
	int16_t  			widthBE16;
	int16_t  			heightBE16;
	uint16_t  			rowBytesBE16;
	uint16_t			flagsBE16;				// see bitmap flag defines above
	uint8_t				pixelSize;				// bits per pixel
	uint8_t				version;				// data structure version 3
	
	// version 3 fields
	uint8_t				size;					// size of this structure in bytes (0x18)
	uint8_t				pixelFormat;			// format of the pixel data, see pixelFormatType
	uint8_t				unused;
	uint8_t				compressionType;		// see BitmapCompressionType
	uint16_t			densityBE16;			// used by the blitter to scale bitmaps
	uint32_t			transparentValueBE32;	// the index or RGB value of the transparent color
	uint32_t			nextBitmapOffsetBE32;	// byte offset to next bitmap in bitmap family

	// if (flags.hasColorTable)
	//		{
	//		if (flags.indirectColorTable)
	//			ColorTableType* colorTableP;	// pointer to color table
	//		else
	//	  		ColorTableType	colorTable;		// color table, could have 0 entries (2 bytes long)
	//		}
	//
	// if (flags.indirect)
	//	  	void*	  bitsP;					// pointer to actual bits
	// else
	//   	uint8_t	  bits[];					// or actual bits
	//
}
#endif
BitmapTypeV3;
typedef BitmapTypeV3 *BitmapPtrV3;


 // Version 2 BitmapType structure (superset of Version 1 and Version 0)
typedef struct BitmapTypeV2
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	// BitmapType
 	int16_t  			widthBE16;
	int16_t  			heightBE16;
	uint16_t  			rowBytesBE16;
	uint16_t			flagsBE16;				// (bmpFlagsCompressed, bmpFlagsHasColorTable, bmpFlagsHasTransparency, bmpFlagsIndirect, bmpFlagsForScreen, bmpFlagsDirectColor)
	uint8_t				pixelSize;
	uint8_t				version;					
	
	// version 2 fields
	uint16_t	 		nextDepthOffsetBE16;	// offset in longwords
	uint8_t				transparentIndex;	
	uint8_t				compressionType;
	uint16_t	 		reservedBE16;
	
	// if (flags.hasColorTable)
	//	  ColorTableType	colorTable			// color table, could have 0 entries (2 bytes long)
	//
	// if (flags.directColor)
	//	  BitmapDirectInfoType	directInfo;
	//
	// if (flags.indirect)
	//	  void*	  bitsP;						// pointer to actual bits
	// else
	//    uint8_t	  bits[];						// or actual bits
	//
}
#endif
BitmapTypeV2;
typedef BitmapTypeV2* BitmapPtrV2;


 // Version 1 BitmapType structure
typedef struct BitmapTypeV1
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	// BitmapType
 	int16_t  			widthBE16;
	int16_t  			heightBE16;
	uint16_t  			rowBytesBE16;
	uint16_t			flagsBE16;				// (bmpFlagsCompressed, bmpFlagsHasColorTable)
	uint8_t				pixelSize;
	uint8_t				version;					
	
	// version 1 fields
	uint16_t	 		nextDepthOffsetBE16;	// offset in longwords
	uint16_t			reservedBE16[2];	
}
#endif
BitmapTypeV1;
typedef BitmapTypeV1* BitmapPtrV1;


 // Version 0 BitmapType structure
typedef struct BitmapTypeV0
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	// BitmapType
 	int16_t  			widthBE16;
	int16_t  			heightBE16;
	uint16_t  			rowBytesBE16;
	uint16_t			flagsBE16;				// (bmpFlagsCompressed)
	
	// version 0 fields
	uint16_t			reservedBE16[4];		// pixelSize and version fields do not exist, but the reserved array
												// was initialized to 0; the OS recognizes that pixelSize of 0 means
}												// the the bitmap's depth is 1
#endif
BitmapTypeV0;
typedef BitmapTypeV0* BitmapPtrV0;


// This is the structure of a color table. It maps pixel values into
// RGB colors. Each element in the table corresponds to the next
// index, starting at 0.
typedef struct RGBColorType
{
	uint8_t				index;					// index of color or best match to cur CLUT or unused.
	uint8_t				r;						// amount of red, 0->255
	uint8_t				g;						// amount of green, 0->255
	uint8_t				b;						// amount of blue, 0->255

#ifdef __cplusplus
	// Comparison operators -- note that the comparison does NOT
	// include the index field.
	inline bool operator==(const RGBColorType& o)
	{ // We cannot user a 32 bits compare here becase the object might not be aligned
		return ((r==o.r) && (g==o.g) && (b==o.b));
	}
	inline bool operator!=(const RGBColorType& o)
	{
		return !operator==(o);
	}
#endif

} RGBColorType;


// -----------------------------------------------
// For direct color bitmaps (flags.directColor set), this structure follows
// the color table if one is present, or immediately follows the BitmapType if a
// color table is not present.
// The only type of direct color bitmap that is currently supported in version 3
// of the Window Manager (feature: sysFtrCreator, #sysFtrNumWinVersion) are
// 16 bits/pixel with redBits=5, greenBits=6, blueBits=5.
// -----------------------------------------------
typedef struct BitmapDirectInfoType
{
	uint8_t		  		redBits;				// # of red bits in each pixel
	uint8_t		  		greenBits;				// # of green bits in each pixel
	uint8_t		  		blueBits;				// # of blue bits in each pixel
	uint8_t		  		reserved;				// must be zero
	RGBColorType  		transparentColor;		// transparent color (index field ignored)
}
BitmapDirectInfoType;	
	

typedef struct ColorTableType {
												// high bits (numEntries > 256) reserved
	uint16_t			entryCount;				// number of entries in table
	// RGBColorType		entry[];				// array 0..numEntries-1 of colors
												// starts immediately after numEntries
} ColorTableType;

// Use a define for the color table header size because the color table is in 68K format
// which is "packed" per ARM 32-bit alignment conventions.  Using the PACKED keyword is
// less optimal and is problematic.
#define colorTableHeaderSize	2

// get start of color table entries array given pointer to ColorTableType
#define ColorTableEntries(ctP)	((RGBColorType *)((char *)(ctP)+colorTableHeaderSize))


#endif //_CMNBITMAPTYPES_H_
