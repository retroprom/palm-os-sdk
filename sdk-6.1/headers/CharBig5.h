/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. or its subsidiaries.
 *
 * File: CharBig5.h
 *
 * Release: 
 *
 * Description:
 *         	Header file for Big-5 (Traditional Chinese) coded character set.
 *
 * History:
 *	2003-09-22	CS	Created by Chris Schneider.
 *	2003-10-18	KKr	Filled out some of the Unicode double-byte names.
 *	2003-12-22	KKr	Filled out all of the names using MakeCharNames tool.
 *
 *****************************************************************************/

#ifndef _CHARBIG5_H_
#define _CHARBIG5_H_

/***********************************************************************
 * Public macros
 ***********************************************************************/

/***********************************************************************
 * Public constants
 ***********************************************************************/

// Transliteration operations that are not universal, but can be applied
// to Traditional Chinese (Big-5) text (none yet).

// Extended character attributes for the Big-5 character set.
// Note that these attributes have to be on an encoding basis, since
// they're shared across all languages which use this encoding. For
// Big-5 there's only one language, so we're OK to encode wrapping
// info here, which is often language-dependent.

#define	charXAttrBig5Mask									0x00FF
#define	charXAttrBig5Following								0x0001
#define	charXAttrBig5Leading								0x0002
#define	charXAttrBig5Break									0x0004

// Note that these values, as well as the first/last double byte
// constants defined below, are for a superset of Big-5 that's
// even larger than with just the Hong Kong Supplemental Character Set
// additions.
#define kBig5FirstHighByte									0x81
#define kBig5LastHighByte									0xFE
#define kBig5FirstLowByte									0x40
#define kBig5LastLowByte									0xFE

// Character codes that are specific to Big-5. These names
// are generated from the Unicode 3.1 data files.

#define	chrBig5ReverseSolidus								0x005c	// Is yen char in Japanese fonts.

#define	chrBig5FirstDoubleByte								0x8140

#define	chrBig5IdeographicSpace								0xa140
#define	chrBig5FullwidthComma								0xa141
#define	chrBig5IdeographicComma								0xa142
#define	chrBig5IdeographicFullStop							0xa143
#define	chrBig5FullwidthFullStop							0xa144
#define	chrBig5HyphenationPoint								0xa145
#define	chrBig5FullwidthSemicolon							0xa146
#define	chrBig5FullwidthColon								0xa147
#define	chrBig5FullwidthQuestionMark						0xa148
#define	chrBig5FullwidthExclamationMark						0xa149
#define	chrBig5VerticalFormTwoDotLeader						0xa14a
#define	chrBig5HorizontalEllipsis							0xa14b
#define	chrBig5TwoDotLeader									0xa14c
#define	chrBig5SmallComma									0xa14d
#define	chrBig5SmallIdeographicComma						0xa14e
#define	chrBig5SmallFullStop								0xa14f
#define	chrBig5MiddleDot									0xa150
#define	chrBig5SmallSemicolon								0xa151
#define	chrBig5SmallColon									0xa152
#define	chrBig5SmallQuestionMark							0xa153
#define	chrBig5SmallExclamationMark							0xa154
#define	chrBig5FullwidthVerticalLine						0xa155
#define	chrBig5EnDash										0xa156
#define	chrBig5VerticalFormEmDash							0xa157
#define	chrBig5EmDash										0xa158
#define	chrBig5VerticalFormLowLine							0xa159
#define	chrBig5BoxDrawingsLightLeft							0xa15a
#define	chrBig5VerticalFormWavyLowLine						0xa15b
#define	chrBig5WavyLowLine									0xa15c
#define	chrBig5FullwidthLeftParenthesis						0xa15d
#define	chrBig5FullwidthRightParenthesis					0xa15e
#define	chrBig5VerticalFormLeftParenthesis					0xa15f
#define	chrBig5VerticalFormRightParenthesis					0xa160
#define	chrBig5FullwidthLeftCurlyBracket					0xa161
#define	chrBig5FullwidthRightCurlyBracket					0xa162
#define	chrBig5VerticalFormLeftCurlyBracket					0xa163
#define	chrBig5VerticalFormRightCurlyBracket				0xa164
#define	chrBig5LeftTortoiseShellBracket						0xa165
#define	chrBig5RightTortoiseShellBracket					0xa166
#define	chrBig5VerticalFormLeftTortoiseShellBracket			0xa167
#define	chrBig5VerticalFormRightTortoiseShellBracket		0xa168
#define	chrBig5LeftBlackLenticularBracket					0xa169
#define	chrBig5RightBlackLenticularBracket					0xa16a
#define	chrBig5VerticalFormLeftBlackLenticularBracket		0xa16b
#define	chrBig5VerticalFormRightBlackLenticularBracket		0xa16c
#define	chrBig5LeftDoubleAngleBracket						0xa16d
#define	chrBig5RightDoubleAngleBracket						0xa16e
#define	chrBig5VerticalFormLeftDoubleAngleBracket			0xa16f
#define	chrBig5VerticalFormRightDoubleAngleBracket			0xa170
#define	chrBig5LeftAngleBracket								0xa171
#define	chrBig5RightAngleBracket							0xa172
#define	chrBig5VerticalFormLeftAngleBracket					0xa173
#define	chrBig5VerticalFormRightAngleBracket				0xa174
#define	chrBig5LeftCornerBracket							0xa175
#define	chrBig5RightCornerBracket							0xa176
#define	chrBig5VerticalFormLeftCornerBracket				0xa177
#define	chrBig5VerticalFormRightCornerBracket				0xa178
#define	chrBig5LeftWhiteCornerBracket						0xa179
#define	chrBig5RightWhiteCornerBracket						0xa17a
#define	chrBig5VerticalFormLeftWhiteCornerBracket			0xa17b
#define	chrBig5VerticalFormRightWhiteCornerBracket			0xa17c
#define	chrBig5SmallLeftParenthesis							0xa17d
#define	chrBig5SmallRightParenthesis						0xa17e
#define	chrBig5SmallLeftCurlyBracket						0xa1a1
#define	chrBig5SmallRightCurlyBracket						0xa1a2
#define	chrBig5SmallLeftTortoiseShellBracket				0xa1a3
#define	chrBig5SmallRightTortoiseShellBracket				0xa1a4
#define	chrBig5LeftSingleQuotationMark						0xa1a5
#define	chrBig5RightSingleQuotationMark						0xa1a6
#define	chrBig5LeftDoubleQuotationMark						0xa1a7
#define	chrBig5RightDoubleQuotationMark						0xa1a8
#define	chrBig5ReversedDoublePrimeQuotationMark				0xa1a9
#define	chrBig5DoublePrimeQuotationMark						0xa1aa
#define	chrBig5ReversedPrime								0xa1ab
#define	chrBig5Prime										0xa1ac
#define	chrBig5FullwidthNumberSign							0xa1ad
#define	chrBig5FullwidthAmpersand							0xa1ae
#define	chrBig5FullwidthAsterisk							0xa1af
#define	chrBig5ReferenceMark								0xa1b0
#define	chrBig5SectionSign									0xa1b1
#define	chrBig5DittoMark									0xa1b2
#define	chrBig5WhiteCircle									0xa1b3
#define	chrBig5BlackCircle									0xa1b4
#define	chrBig5WhiteUpPointingTriangle						0xa1b5
#define	chrBig5BlackUpPointingTriangle						0xa1b6
#define	chrBig5Bullseye										0xa1b7
#define	chrBig5WhiteStar									0xa1b8
#define	chrBig5BlackStar									0xa1b9
#define	chrBig5WhiteDiamond									0xa1ba
#define	chrBig5BlackDiamond									0xa1bb
#define	chrBig5WhiteSquare									0xa1bc
#define	chrBig5BlackSquare									0xa1bd
#define	chrBig5WhiteDownPointingTriangle					0xa1be
#define	chrBig5BlackDownPointingTriangle					0xa1bf
#define	chrBig5CircledIdeographCorrect						0xa1c0
#define	chrBig5CareOf										0xa1c1
#define	chrBig5Macron										0xa1c2
#define	chrBig5FullwidthMacron								0xa1c3
#define	chrBig5FullwidthLowLine								0xa1c4
#define	chrBig5ModifierLowMacron							0xa1c5
#define	chrBig5DashedOverline								0xa1c6
#define	chrBig5CentrelineOverline							0xa1c7
#define	chrBig5DashedLowLine								0xa1c8
#define	chrBig5CentrelineLowLine							0xa1c9
#define	chrBig5WavyOverline									0xa1ca
#define	chrBig5DoubleWavyOverline							0xa1cb
#define	chrBig5SmallNumberSign								0xa1cc
#define	chrBig5SmallAmpersand								0xa1cd
#define	chrBig5SmallAsterisk								0xa1ce
#define	chrBig5FullwidthPlusSign							0xa1cf
#define	chrBig5FullwidthHyphenMinus							0xa1d0
#define	chrBig5MultiplicationSign							0xa1d1
#define	chrBig5DivisionSign									0xa1d2
#define	chrBig5PlusMinusSign								0xa1d3
#define	chrBig5SquareRoot									0xa1d4
#define	chrBig5FullwidthLessThanSign						0xa1d5
#define	chrBig5FullwidthGreaterThanSign						0xa1d6
#define	chrBig5FullwidthEqualsSign							0xa1d7
#define	chrBig5LessThanOverEqualTo							0xa1d8
#define	chrBig5GreaterThanOverEqualTo						0xa1d9
#define	chrBig5NotEqualTo									0xa1da
#define	chrBig5Infinity										0xa1db
#define	chrBig5ApproximatelyEqualToOrTheImageOf				0xa1dc
#define	chrBig5IdenticalTo									0xa1dd
#define	chrBig5SmallPlusSign								0xa1de
#define	chrBig5SmallHyphenMinus								0xa1df
#define	chrBig5SmallLessThanSign							0xa1e0
#define	chrBig5SmallGreaterThanSign							0xa1e1
#define	chrBig5SmallEqualsSign								0xa1e2
#define	chrBig5FullwidthTilde								0xa1e3
#define	chrBig5Intersection									0xa1e4
#define	chrBig5Union										0xa1e5
#define	chrBig5UpTack										0xa1e6
#define	chrBig5Angle										0xa1e7
#define	chrBig5RightAngle									0xa1e8
#define	chrBig5RightTriangle								0xa1e9
#define	chrBig5SquareLog									0xa1ea
#define	chrBig5SquareLn										0xa1eb
#define	chrBig5Integral										0xa1ec
#define	chrBig5ContourIntegral								0xa1ed
#define	chrBig5Because										0xa1ee
#define	chrBig5Therefore									0xa1ef
#define	chrBig5FemaleSign									0xa1f0
#define	chrBig5MaleSign										0xa1f1
#define	chrBig5CircledPlus									0xa1f2
#define	chrBig5CircledDotOperator							0xa1f3
#define	chrBig5UpwardsArrow									0xa1f4
#define	chrBig5DownwardsArrow								0xa1f5
#define	chrBig5LeftwardsArrow								0xa1f6
#define	chrBig5RightwardsArrow								0xa1f7
#define	chrBig5NorthWestArrow								0xa1f8
#define	chrBig5NorthEastArrow								0xa1f9
#define	chrBig5SouthWestArrow								0xa1fa
#define	chrBig5SouthEastArrow								0xa1fb
#define	chrBig5ParallelTo									0xa1fc
#define	chrBig5Divides										0xa1fd
#define	chrBig5FullwidthSolidus								0xa1fe
#define	chrBig5FullwidthReverseSolidus						0xa240
#define	chrBig5DivisionSlash								0xa241
#define	chrBig5SmallReverseSolidus							0xa242
#define	chrBig5FullwidthDollarSign							0xa243
#define	chrBig5FullwidthYenSign								0xa244
#define	chrBig5PostalMark									0xa245
#define	chrBig5FullwidthCentSign							0xa246
#define	chrBig5FullwidthPoundSign							0xa247
#define	chrBig5FullwidthPercentSign							0xa248
#define	chrBig5FullwidthCommercialAt						0xa249
#define	chrBig5DegreeCelsius								0xa24a
#define	chrBig5DegreeFahrenheit								0xa24b
#define	chrBig5SmallDollarSign								0xa24c
#define	chrBig5SmallPercentSign								0xa24d
#define	chrBig5SmallCommercialAt							0xa24e
#define	chrBig5SquareMil									0xa24f
#define	chrBig5SquareMm										0xa250
#define	chrBig5SquareCm										0xa251
#define	chrBig5SquareKm										0xa252
#define	chrBig5SquareKmCapital								0xa253
#define	chrBig5SquareMSquared								0xa254
#define	chrBig5SquareMg										0xa255
#define	chrBig5SquareKg										0xa256
#define	chrBig5SquareCc										0xa257
#define	chrBig5DegreeSign									0xa258
#define	chrBig5LowerOneEighthBlock							0xa262
#define	chrBig5LowerOneQuarterBlock							0xa263
#define	chrBig5LowerThreeEighthsBlock						0xa264
#define	chrBig5LowerHalfBlock								0xa265
#define	chrBig5LowerFiveEighthsBlock						0xa266
#define	chrBig5LowerThreeQuartersBlock						0xa267
#define	chrBig5LowerSevenEighthsBlock						0xa268
#define	chrBig5FullBlock									0xa269
#define	chrBig5LeftOneEighthBlock							0xa26a
#define	chrBig5LeftOneQuarterBlock							0xa26b
#define	chrBig5LeftThreeEighthsBlock						0xa26c
#define	chrBig5LeftHalfBlock								0xa26d
#define	chrBig5LeftFiveEighthsBlock							0xa26e
#define	chrBig5LeftThreeQuartersBlock						0xa26f
#define	chrBig5LeftSevenEighthsBlock						0xa270
#define	chrBig5BoxDrawingsLightVerticalAndHorizontal		0xa271
#define	chrBig5BoxDrawingsLightUpAndHorizontal				0xa272
#define	chrBig5BoxDrawingsLightDownAndHorizontal			0xa273
#define	chrBig5BoxDrawingsLightVerticalAndLeft				0xa274
#define	chrBig5BoxDrawingsLightVerticalAndRight				0xa275
#define	chrBig5UpperOneEighthBlock							0xa276
#define	chrBig5BoxDrawingsLightHorizontal					0xa277
#define	chrBig5BoxDrawingsLightVertical						0xa278
#define	chrBig5RightOneEighthBlock							0xa279
#define	chrBig5BoxDrawingsLightDownAndRight					0xa27a
#define	chrBig5BoxDrawingsLightDownAndLeft					0xa27b
#define	chrBig5BoxDrawingsLightUpAndRight					0xa27c
#define	chrBig5BoxDrawingsLightUpAndLeft					0xa27d
#define	chrBig5BoxDrawingsLightArcDownAndRight				0xa27e
#define	chrBig5BoxDrawingsLightArcDownAndLeft				0xa2a1
#define	chrBig5BoxDrawingsLightArcUpAndRight				0xa2a2
#define	chrBig5BoxDrawingsLightArcUpAndLeft					0xa2a3
#define	chrBig5BoxDrawingsDoubleHorizontal					0xa2a4
#define	chrBig5BoxDrawingsVerticalSingleAndRightDouble		0xa2a5
#define	chrBig5BoxDrawingsVerticalSingleAndHorizontalDouble	0xa2a6
#define	chrBig5BoxDrawingsVerticalSingleAndLeftDouble		0xa2a7
#define	chrBig5BlackLowerRightTriangle						0xa2a8
#define	chrBig5BlackLowerLeftTriangle						0xa2a9
#define	chrBig5BlackUpperRightTriangle						0xa2aa
#define	chrBig5BlackUpperLeftTriangle						0xa2ab
#define	chrBig5BoxDrawingsLightDiagonalUpperRightToLowerLeft 0xa2ac
#define	chrBig5BoxDrawingsLightDiagonalUpperLeftToLowerRight 0xa2ad
#define	chrBig5BoxDrawingsLightDiagonalCross				0xa2ae
#define	chrBig5FullwidthDigitZero							0xa2af
#define	chrBig5FullwidthDigitOne							0xa2b0
#define	chrBig5FullwidthDigitTwo							0xa2b1
#define	chrBig5FullwidthDigitThree							0xa2b2
#define	chrBig5FullwidthDigitFour							0xa2b3
#define	chrBig5FullwidthDigitFive							0xa2b4
#define	chrBig5FullwidthDigitSix							0xa2b5
#define	chrBig5FullwidthDigitSeven							0xa2b6
#define	chrBig5FullwidthDigitEight							0xa2b7
#define	chrBig5FullwidthDigitNine							0xa2b8
#define	chrBig5RomanNumeralOne								0xa2b9
#define	chrBig5RomanNumeralTwo								0xa2ba
#define	chrBig5RomanNumeralThree							0xa2bb
#define	chrBig5RomanNumeralFour								0xa2bc
#define	chrBig5RomanNumeralFive								0xa2bd
#define	chrBig5RomanNumeralSix								0xa2be
#define	chrBig5RomanNumeralSeven							0xa2bf
#define	chrBig5RomanNumeralEight							0xa2c0
#define	chrBig5RomanNumeralNine								0xa2c1
#define	chrBig5RomanNumeralTen								0xa2c2
#define	chrBig5HangzhouNumeralOne							0xa2c3
#define	chrBig5HangzhouNumeralTwo							0xa2c4
#define	chrBig5HangzhouNumeralThree							0xa2c5
#define	chrBig5HangzhouNumeralFour							0xa2c6
#define	chrBig5HangzhouNumeralFive							0xa2c7
#define	chrBig5HangzhouNumeralSix							0xa2c8
#define	chrBig5HangzhouNumeralSeven							0xa2c9
#define	chrBig5HangzhouNumeralEight							0xa2ca
#define	chrBig5HangzhouNumeralNine							0xa2cb
#define	chrBig5FullwidthCapital_A							0xa2cf
#define	chrBig5FullwidthCapital_B							0xa2d0
#define	chrBig5FullwidthCapital_C							0xa2d1
#define	chrBig5FullwidthCapital_D							0xa2d2
#define	chrBig5FullwidthCapital_E							0xa2d3
#define	chrBig5FullwidthCapital_F							0xa2d4
#define	chrBig5FullwidthCapital_G							0xa2d5
#define	chrBig5FullwidthCapital_H							0xa2d6
#define	chrBig5FullwidthCapital_I							0xa2d7
#define	chrBig5FullwidthCapital_J							0xa2d8
#define	chrBig5FullwidthCapital_K							0xa2d9
#define	chrBig5FullwidthCapital_L							0xa2da
#define	chrBig5FullwidthCapital_M							0xa2db
#define	chrBig5FullwidthCapital_N							0xa2dc
#define	chrBig5FullwidthCapital_O							0xa2dd
#define	chrBig5FullwidthCapital_P							0xa2de
#define	chrBig5FullwidthCapital_Q							0xa2df
#define	chrBig5FullwidthCapital_R							0xa2e0
#define	chrBig5FullwidthCapital_S							0xa2e1
#define	chrBig5FullwidthCapital_T							0xa2e2
#define	chrBig5FullwidthCapital_U							0xa2e3
#define	chrBig5FullwidthCapital_V							0xa2e4
#define	chrBig5FullwidthCapital_W							0xa2e5
#define	chrBig5FullwidthCapital_X							0xa2e6
#define	chrBig5FullwidthCapital_Y							0xa2e7
#define	chrBig5FullwidthCapital_Z							0xa2e8
#define	chrBig5FullwidthSmall_A								0xa2e9
#define	chrBig5FullwidthSmall_B								0xa2ea
#define	chrBig5FullwidthSmall_C								0xa2eb
#define	chrBig5FullwidthSmall_D								0xa2ec
#define	chrBig5FullwidthSmall_E								0xa2ed
#define	chrBig5FullwidthSmall_F								0xa2ee
#define	chrBig5FullwidthSmall_G								0xa2ef
#define	chrBig5FullwidthSmall_H								0xa2f0
#define	chrBig5FullwidthSmall_I								0xa2f1
#define	chrBig5FullwidthSmall_J								0xa2f2
#define	chrBig5FullwidthSmall_K								0xa2f3
#define	chrBig5FullwidthSmall_L								0xa2f4
#define	chrBig5FullwidthSmall_M								0xa2f5
#define	chrBig5FullwidthSmall_N								0xa2f6
#define	chrBig5FullwidthSmall_O								0xa2f7
#define	chrBig5FullwidthSmall_P								0xa2f8
#define	chrBig5FullwidthSmall_Q								0xa2f9
#define	chrBig5FullwidthSmall_R								0xa2fa
#define	chrBig5FullwidthSmall_S								0xa2fb
#define	chrBig5FullwidthSmall_T								0xa2fc
#define	chrBig5FullwidthSmall_U								0xa2fd
#define	chrBig5FullwidthSmall_V								0xa2fe
#define	chrBig5FullwidthSmall_W								0xa340
#define	chrBig5FullwidthSmall_X								0xa341
#define	chrBig5FullwidthSmall_Y								0xa342
#define	chrBig5FullwidthSmall_Z								0xa343
#define	chrBig5GreekCapital_ALPHA							0xa344
#define	chrBig5GreekCapital_BETA							0xa345
#define	chrBig5GreekCapital_GAMMA							0xa346
#define	chrBig5GreekCapital_DELTA							0xa347
#define	chrBig5GreekCapital_EPSILON							0xa348
#define	chrBig5GreekCapital_ZETA							0xa349
#define	chrBig5GreekCapital_ETA								0xa34a
#define	chrBig5GreekCapital_THETA							0xa34b
#define	chrBig5GreekCapital_IOTA							0xa34c
#define	chrBig5GreekCapital_KAPPA							0xa34d
#define	chrBig5GreekCapital_LAMDA							0xa34e
#define	chrBig5GreekCapital_MU								0xa34f
#define	chrBig5GreekCapital_NU								0xa350
#define	chrBig5GreekCapital_XI								0xa351
#define	chrBig5GreekCapital_OMICRON							0xa352
#define	chrBig5GreekCapital_PI								0xa353
#define	chrBig5GreekCapital_RHO								0xa354
#define	chrBig5GreekCapital_SIGMA							0xa355
#define	chrBig5GreekCapital_TAU								0xa356
#define	chrBig5GreekCapital_UPSILON							0xa357
#define	chrBig5GreekCapital_PHI								0xa358
#define	chrBig5GreekCapital_CHI								0xa359
#define	chrBig5GreekCapital_PSI								0xa35a
#define	chrBig5GreekCapital_OMEGA							0xa35b
#define	chrBig5GreekSmall_ALPHA								0xa35c
#define	chrBig5GreekSmall_BETA								0xa35d
#define	chrBig5GreekSmall_GAMMA								0xa35e
#define	chrBig5GreekSmall_DELTA								0xa35f
#define	chrBig5GreekSmall_EPSILON							0xa360
#define	chrBig5GreekSmall_ZETA								0xa361
#define	chrBig5GreekSmall_ETA								0xa362
#define	chrBig5GreekSmall_THETA								0xa363
#define	chrBig5GreekSmall_IOTA								0xa364
#define	chrBig5GreekSmall_KAPPA								0xa365
#define	chrBig5GreekSmall_LAMDA								0xa366
#define	chrBig5GreekSmall_MU								0xa367
#define	chrBig5GreekSmall_NU								0xa368
#define	chrBig5GreekSmall_XI								0xa369
#define	chrBig5GreekSmall_OMICRON							0xa36a
#define	chrBig5GreekSmall_PI								0xa36b
#define	chrBig5GreekSmall_RHO								0xa36c
#define	chrBig5GreekSmall_SIGMA								0xa36d
#define	chrBig5GreekSmall_TAU								0xa36e
#define	chrBig5GreekSmall_UPSILON							0xa36f
#define	chrBig5GreekSmall_PHI								0xa370
#define	chrBig5GreekSmall_CHI								0xa371
#define	chrBig5GreekSmall_PSI								0xa372
#define	chrBig5GreekSmall_OMEGA								0xa373
#define	chrBig5Bopomofo_B									0xa374
#define	chrBig5Bopomofo_P									0xa375
#define	chrBig5Bopomofo_M									0xa376
#define	chrBig5Bopomofo_F									0xa377
#define	chrBig5Bopomofo_D									0xa378
#define	chrBig5Bopomofo_T									0xa379
#define	chrBig5Bopomofo_N									0xa37a
#define	chrBig5Bopomofo_L									0xa37b
#define	chrBig5Bopomofo_G									0xa37c
#define	chrBig5Bopomofo_K									0xa37d
#define	chrBig5Bopomofo_H									0xa37e
#define	chrBig5Bopomofo_J									0xa3a1
#define	chrBig5Bopomofo_Q									0xa3a2
#define	chrBig5Bopomofo_X									0xa3a3
#define	chrBig5Bopomofo_ZH									0xa3a4
#define	chrBig5Bopomofo_CH									0xa3a5
#define	chrBig5Bopomofo_SH									0xa3a6
#define	chrBig5Bopomofo_R									0xa3a7
#define	chrBig5Bopomofo_Z									0xa3a8
#define	chrBig5Bopomofo_C									0xa3a9
#define	chrBig5Bopomofo_S									0xa3aa
#define	chrBig5Bopomofo_A									0xa3ab
#define	chrBig5Bopomofo_O									0xa3ac
#define	chrBig5Bopomofo_E									0xa3ad
#define	chrBig5Bopomofo_EH									0xa3ae
#define	chrBig5Bopomofo_AI									0xa3af
#define	chrBig5Bopomofo_EI									0xa3b0
#define	chrBig5Bopomofo_AU									0xa3b1
#define	chrBig5Bopomofo_OU									0xa3b2
#define	chrBig5Bopomofo_AN									0xa3b3
#define	chrBig5Bopomofo_EN									0xa3b4
#define	chrBig5Bopomofo_ANG									0xa3b5
#define	chrBig5Bopomofo_ENG									0xa3b6
#define	chrBig5Bopomofo_ER									0xa3b7
#define	chrBig5Bopomofo_I									0xa3b8
#define	chrBig5Bopomofo_U									0xa3b9
#define	chrBig5Bopomofo_IU									0xa3ba
#define	chrBig5DotAbove										0xa3bb
#define	chrBig5ModifierMacron								0xa3bc
#define	chrBig5ModifierAcuteAccent							0xa3bd
#define	chrBig5Caron										0xa3be
#define	chrBig5ModifierGraveAccent							0xa3bf
#define	chrBig5EuroSign										0xa3e1
#define	chrBig5BoxDrawingsDoubleDownAndRight				0xf9dd
#define	chrBig5BoxDrawingsDoubleDownAndHorizontal			0xf9de
#define	chrBig5BoxDrawingsDoubleDownAndLeft					0xf9df
#define	chrBig5BoxDrawingsDoubleVerticalAndRight			0xf9e0
#define	chrBig5BoxDrawingsDoubleVerticalAndHorizontal		0xf9e1
#define	chrBig5BoxDrawingsDoubleVerticalAndLeft				0xf9e2
#define	chrBig5BoxDrawingsDoubleUpAndRight					0xf9e3
#define	chrBig5BoxDrawingsDoubleUpAndHorizontal				0xf9e4
#define	chrBig5BoxDrawingsDoubleUpAndLeft					0xf9e5
#define	chrBig5BoxDrawingsDownSingleAndRightDouble			0xf9e6
#define	chrBig5BoxDrawingsDownSingleAndHorizontalDouble		0xf9e7
#define	chrBig5BoxDrawingsDownSingleAndLeftDouble			0xf9e8
#define	chrBig5BoxDrawingsVerticalSingleAndRightDoubleDup	0xf9e9
#define	chrBig5BoxDrawingsVerticalSingleAndHorizontalDoubleDup	0xf9ea
#define	chrBig5BoxDrawingsVerticalSingleAndLeftDoubleDup	0xf9eb
#define	chrBig5BoxDrawingsUpSingleAndRightDouble			0xf9ec
#define	chrBig5BoxDrawingsUpSingleAndHorizontalDouble		0xf9ed
#define	chrBig5BoxDrawingsUpSingleAndLeftDouble				0xf9ee
#define	chrBig5BoxDrawingsDownDoubleAndRightSingle			0xf9ef
#define	chrBig5BoxDrawingsDownDoubleAndHorizontalSingle		0xf9f0
#define	chrBig5BoxDrawingsDownDoubleAndLeftSingle			0xf9f1
#define	chrBig5BoxDrawingsVerticalDoubleAndRightSingle		0xf9f2
#define	chrBig5BoxDrawingsVerticalDoubleAndHorizontalSingle	0xf9f3
#define	chrBig5BoxDrawingsVerticalDoubleAndLeftSingle		0xf9f4
#define	chrBig5BoxDrawingsUpDoubleAndRightSingle			0xf9f5
#define	chrBig5BoxDrawingsUpDoubleAndHorizontalSingle		0xf9f6
#define	chrBig5BoxDrawingsUpDoubleAndLeftSingle				0xf9f7
#define	chrBig5BoxDrawingsDoubleVertical					0xf9f8
#define	chrBig5BoxDrawingsDoubleHorizontalDup				0xf9f9
#define	chrBig5BoxDrawingsLightArcDownAndRightDup			0xf9fa
#define	chrBig5BoxDrawingsLightArcDownAndLeftDup			0xf9fb
#define	chrBig5BoxDrawingsLightArcUpAndRightDup				0xf9fc
#define	chrBig5BoxDrawingsLightArcUpAndLeftDup				0xf9fd
#define	chrBig5DarkShade									0xf9fe

// Aliases with common names for some characters.
#define	chrBig5FullwidthLeftSquareBracket					chrBig5LeftTortoiseShellBracket
#define	chrBig5FullwidthRightSquareBracket					chrBig5RightTortoiseShellBracket

#define chrBig5LastDoubleByte								0xFEFE

#endif // _CHARBIG5_H_
