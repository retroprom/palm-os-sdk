/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Font.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines font structures and routines.
 *
 *****************************************************************************/

#ifndef _FONT_H_
#define _FONT_H_

#include <PalmTypes.h>

// Pixel width of tab stops in fields
#define  fntTabChrWidth      20

typedef struct FontTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FONTS	// These fields will not be available in the next OS release!
{
	int16_t fontType; 		// font type
	int16_t firstChar; 		// ASCII code of first character
	int16_t lastChar; 		// ASCII code of last character
	int16_t maxWidth; 		// maximum character width
	int16_t kernMax; 			// negative of maximum character kern
	int16_t nDescent; 		// negative of descent
	int16_t fRectWidth; 		// width of font rectangle
	int16_t fRectHeight; 		// height of font rectangle
	int16_t owTLoc; 			// offset to offset/width table
	int16_t ascent; 			// ascent
	int16_t descent; 			// descent
	int16_t leading; 			// leading
	int16_t rowWords; 		// row width of bit image / 2
}
#endif
FontType;

typedef FontType *FontPtr;

typedef FontPtr *FontTablePtr;

// The FontDensityType contains information about a one of the font's
// glyph bitmaps. The value of FontDensityType.density is 72 for single
// density font data, and 144 for double density font data.
// The value of FontDensityType.glyphBitsOffset is the offset in bytes
// from the beginning of the FontTypeV2 structure to the first byte in
// the glyph bitmap.

typedef struct FontDensityTypeTag
{
	int16_t	density;
	int16_t	reserved;		// padding, must be set to 0
	uint32_t	glyphBitsOffset;
}
FontDensityTypeType;


typedef struct FontTypeV2Tag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FONTS	// These fields will not be available in the next OS release!
{
	// first part is basically the same as NFNT FontRec
	int16_t	fontType;
	int16_t	firstChar;		// character code of first character
	int16_t	lastChar;		// character code of last character
	int16_t	maxWidth;		// widMax = maximum character width
	int16_t	kernMax;		// negative of maximum character kern
	int16_t	nDescent;		// negative of descent
	int16_t	fRectWidth;		// width of font rectangle
	int16_t	fRectHeight;	// height of font rectangle
	int16_t	owTLoc;			// offset to offset/width table
	int16_t	ascent;			// ascent
	int16_t	descent;		// descent
	int16_t	leading;		// leading
	int16_t	rowWords;		// row width of bit image / 2
	
	// New fields (if fntExtendedFormatMask is set)
	int16_t	version;		// 1 = PalmNewFontVersion
	int16_t	densityCount;
	int16_t	reserved;		// padding, must be set to 0

	FontDensityTypeType	densities[1];	// array of 1 or more records
}
#endif
FontTypeV2Type;

enum fontID { 
	stdFont = 0x00,				// Small font used for the user's writing.  Shows a good amount
	boldFont, 					// Small font.  Bold for easier reading.  Used often for ui.
	largeFont,					// Larger font for easier reading.  Shows a lot less.
	symbolFont,					// Various ui images like check boxes and arrows
	symbol11Font, 				// Larger various ui images
	symbol7Font,				// Smaller various ui images
	ledFont,					// Calculator specific font
	largeBoldFont,				// A thicker version of the large font.  More readable.
	fntAppFontCustomBase = 0x80	// First available application-defined font ID
};

typedef Enum8 FontID;

#define checkboxFont symbol11Font

#define FntIsAppDefined(fnt) (fnt >= fntAppFontCustomBase)

//--------------------------------------------------------------------
// Font Functions
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

enum fontDefaults {
	defaultSmallFont = 0,
	defaultLargeFont,
	defaultBoldFont,
	defaultSystemFont
};
typedef enum fontDefaults FontDefaultType;

FontID FntGetDefaultFontID(FontDefaultType inFontType);

extern FontID FntGetFont(void);
extern FontID FntSetFont(FontID font);
extern const FontType* FntGetFontPtr(void);
extern status_t FntDefineFont(FontID font, const FontType* fontP);

extern Coord FntBaseLine(void);
extern Coord FntCharHeight(void);
extern Coord FntDescenderHeight(void);
extern Coord FntLineHeight(void);

extern Coord FntAverageCharWidth(void);
extern Coord FntCharWidth(wchar32_t ch);
// Deprecated routines, replaced by FntCharWidth w/new API.
extern Coord FntCharWidthV50(char ch);
extern Coord FntWCharWidthV50(wchar32_t iChar);
extern Coord FntCharsWidth(const char* chars, size_t len);

extern size_t FntWidthToOffset(	const char* chars,
								size_t length,
								Coord pixelWidth,
								Boolean *leadingEdge,
								Coord* truncWidth);

extern void FntCharsInWidth(const char* string, 
							Coord *stringWidthP,
							size_t *stringLengthP, 
							Boolean *fitWithinWidth);

extern Coord FntLineWidth(const char* chars, size_t length);
extern Boolean FntTruncateString(	char* iDstString,
									const char* iSrcString,
									FontID iFont,
									Coord iMaxWidth,
									Boolean iAddEllipsis);

extern size_t FntWordWrap(const char* string, Coord maxWidth);
extern void FntWordWrapReverseNLines(	const char* pChars, 
										Coord maxWidth,
										uint32_t* linesToScrollP,
										size_t* scrollPosP);

extern void FntGetScrollValues(	const char* chars,
								Coord maxWidth,
								size_t scrollPos,
								uint32_t* linesP,
								uint32_t* topLine);

#ifdef __cplusplus
}
#endif

#endif // _FONT_H_
