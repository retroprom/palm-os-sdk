/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GcFont.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	            	C font API.
 *
 *****************************************************************************/

#ifndef _GC_FONT_H_
#define _GC_FONT_H_

#include <PalmTypes.h>
#include <Font.h>

//! Font faces.
enum {
	kRegularFace		= 0x0001,
	kItalicFace			= 0x0002,
	kBoldFace			= 0x0004
};

/*! Generic constants for enabling/disabling features. */
enum GcFeatureValueTag {
	kGcDisable			= 0,			//!< disable the feature
	kGcEnable			= 0xffffffffLU	//!< enable the feature with default value
};

/*! Rendering quality and antialiasing */
enum GcAliasingTag {
	kDisableAntialiasing	= 0,
	kNormalAntialiasing		= 1,
};

enum {
	kInvalidFontHeight	= -1
};

/*! A point */
typedef struct
{
	fcoord_t	x;
	fcoord_t	y;
} GcPointType;

typedef struct FAbsRectType {
	fcoord_t		left;
	fcoord_t		top;
	fcoord_t		right;
	fcoord_t		bottom;
} FAbsRectType;


//! This structure describes the horizontal spacing of a font.
typedef struct FontHeightType
{
	fcoord_t				ascent;
	fcoord_t				descent;
	fcoord_t				leading;

#ifdef __cplusplus	
							FontHeightType();
							FontHeightType(fcoord_t a, fcoord_t d, fcoord_t l);
							FontHeightType(const FontHeightType& fh);

	void					set_to(	fcoord_t _ascent, fcoord_t _descent, fcoord_t _leading);

	const FontHeightType	invalid() const;
						
	FontHeightType &		operator=(const FontHeightType& fh);
	bool					operator==(const FontHeightType& fh) const;
	bool					operator!=(const FontHeightType& fh) const;
#endif
} FontHeightType; 

/*******************************************************************************
 Typedefs
 *******************************************************************************/

/*! Handle on a font. */
struct GcFontType;
typedef struct GcFontType* GcFontHandle;

#define kFontFamilyLength	63
typedef char FontFamily[kFontFamilyLength + 1];

#define kFontStyleLength	63
typedef char FontStyle[kFontStyleLength + 1];

/*******************************************************************************
 Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* get info about fonts available to the application */
extern int32_t		GcCountFontFamilies(void);

/* gets available family by index - returns -1 if index invalid */
extern status_t		GcGetFontFamily(int32_t index, FontFamily *family);

extern int32_t		GcCountFontStyles(const char* family);
extern status_t		GcGetFontStyle(	const char* family, int32_t index, FontStyle *style);

/* create a font handle from a font spec -- returns NULL if no matching fonts are found*/
extern GcFontHandle	GcCreateFont(const char* fontSpec);
/* create a font handle of the specified family - returns a NULL if not succesful in creating font handle*/
extern GcFontHandle	GcCreateFontFromFamily(const char* family);
/* create a font handle for on old font ID - returns a NULL if the ID is not defined*/
extern GcFontHandle	GcCreateFontFromID(FontID id);
/* returns errNone if this is a valid font */
extern status_t		GcCheckFont(GcFontHandle font);
/* release (free) a font handle */
extern void			GcReleaseFont(GcFontHandle font);

/* SPECIFY FONT INFO */
extern status_t		GcApplyFontSpec(GcFontHandle font, const char* fontSpec);

/* set font to the specified style */
extern status_t		GcSetFontStyle(GcFontHandle font, const char* style);
/* set font to the closest match to the face flags */
extern status_t		GcSetFontFace(GcFontHandle font, uint16_t face);
/* set font to specified size */
extern status_t		GcSetFontSize(GcFontHandle font, fcoord_t size);
/* apply transformation matrix to font, 4 items: kGcXX, kGcYX, kGcXY, kGcYY */
extern status_t		GcSetFontTransform(GcFontHandle font, const fcoord_t* matrix);
/* control hinting */
extern status_t		GcSetFontHinting(GcFontHandle font, Boolean enabled);
/* control antialising, use GcAliasingTag enum */
extern status_t		GcSetFontAntialiasing(GcFontHandle font, uint32_t value);

/* RETRIEVE FONT INFO */
/* get font's family and style */
extern status_t		GcGetFontFamilyAndStyle(	GcFontHandle font,
												FontFamily *family,
												FontStyle *style);
/* get font's size */
extern fcoord_t		GcGetFontSize(GcFontHandle font);
/* get font's face */
extern uint16_t		GcGetFontFace(GcFontHandle font);
/* get font's transform */
extern status_t		GcGetFontTransform(GcFontHandle font, fcoord_t* out_matrix);
/* get font's hinting */
extern Boolean		GcGetFontHinting(GcFontHandle font);
/* get font's antialiasing */
extern uint32_t		GcGetFontAntialiasing(GcFontHandle font);
/* get font's height at current size */
extern status_t		GcGetFontHeight(GcFontHandle font, struct FontHeightType *height);
/* get font's bounding box at current size */
extern status_t		GcGetFontBoundingBox(GcFontHandle font, FAbsRectType* bbox);

/* GET INFO ABOUT USING THE FONT */
/* return the x and y cursor movement for the given string */
extern void			GcFontStringEscapement(	GcFontHandle font,
											const char *string,
											size_t length,
											GcPointType* outEscapement);

/* get the width of up to length characters of string.  this is the same as
   calling GcFontStringEscapement() and just dropping the y component. */
extern fcoord_t		GcFontStringWidth(GcFontHandle font, const char *string, size_t length);

/* get the bounding box for the string */
extern status_t		GcFontStringBounds(	GcFontHandle font,
										const char *string,
										size_t length,
										FAbsRectType* outBounds);

/* return number of string bytes that fit in maxWidth using font */
extern size_t		GcFontStringBytesInWidth(	GcFontHandle font,
												const char *string,
												size_t maxWidth);

/* return number of characters of string that fit in
   maxWidth using font*/
extern size_t		GcFontStringCharsInWidth(	GcFontHandle font,
												const char *string,
												size_t maxWidth);

#ifdef __cplusplus 
}
#endif

#ifdef __cplusplus	
inline FontHeightType::FontHeightType()
	:	ascent(kInvalidFontHeight),
descent(kInvalidFontHeight),
leading(kInvalidFontHeight)
{
}

inline FontHeightType::FontHeightType(fcoord_t a, fcoord_t d, fcoord_t l)
	:	ascent(a), descent(d), leading(l)
{
}

inline FontHeightType::FontHeightType(const FontHeightType& fh)
	:	ascent(fh.ascent), descent(fh.descent), leading(fh.leading)
{
}

inline void FontHeightType::set_to(fcoord_t _asc, fcoord_t _desc, fcoord_t _ld)
{
	ascent = _asc; descent = _desc; leading = _ld;
}
					
inline const FontHeightType
FontHeightType::invalid() const
{
	return FontHeightType();
}

inline FontHeightType& FontHeightType::operator=(const FontHeightType& fh)
{
	ascent = fh.ascent; descent = fh.descent; leading = fh.leading;
	return *this;
}

inline bool FontHeightType::operator==(const FontHeightType& fh) const
{
	return (		(ascent == fh.ascent)
			&&	(descent == fh.descent)
			&&	(leading == fh.leading) );
}

inline bool FontHeightType::operator!=(const FontHeightType& fh) const
{
	return (		(ascent != fh.ascent)
			||	(descent != fh.descent)
			||	(leading != fh.leading) );
}
#endif



#endif	// _GC_FONT_H_
