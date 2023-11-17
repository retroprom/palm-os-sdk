/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: LocaleMgrCompatibility.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		This file contains deprecated types from LocaleMgr.h (actually,
 *	at least some of these types used to be in Localize.h, which no
 *	longer exists.)
 *
 *****************************************************************************/

#ifndef _PALM_OS__LOCALE_MGR_COMPATIBILITY_H
#define _PALM_OS__LOCALE_MGR_COMPATIBILITY_H

/* These values for NumberFormatType are locale-specific, and therefore
should not be used by applications.  Instead, use the format preference
selected by the user (prefNumberFormat).  If you need a locale-specific
format, pass lmChoiceNumberFormat to LmGetLocaleSetting.
*/
enum NumberFormatTag {
	nfCommaPeriod,
	nfPeriodComma,
	nfSpaceComma,
	nfApostrophePeriod,
	nfApostropheComma
	} ;

/* These are the old numeric values for country and language code. They
are still used in overlay resources, the silkscreen resource, and a few
other places.
*/

/* Language codes (ISO 639).  The first 8 preserve the old values for the
deprecated LanguageType; the rest are sorted by the 2-character language code.
*/

#define	lEnglishV50			((LanguageType)0)	// EN
#define	lFrenchV50			((LanguageType)1)	// FR
#define	lGermanV50			((LanguageType)2)	// DE
#define	lItalianV50			((LanguageType)3)	// IT
#define	lSpanishV50			((LanguageType)4)	// ES
#define	lUnusedV50			((LanguageType)5)	// Reserved

// New in 3.1
#define	lJapaneseV50		((LanguageType)6)	// JA (Palm calls this jp in pre-6.0)
#define	lDutchV50			((LanguageType)7)	// NL

// New in 4.0
#define	lAfarV50			((LanguageType)8)	// AA
#define	lAbkhazianV50		((LanguageType)9)	// AB
#define	lAfrikaansV50		((LanguageType)10)	// AF
#define	lAmharicV50			((LanguageType)11)	// AM
#define	lArabicV50			((LanguageType)12)	// AR
#define	lAssameseV50		((LanguageType)13)	// AS
#define	lAymaraV50			((LanguageType)14)	// AY
#define	lAzerbaijaniV50		((LanguageType)15)	// AZ
#define	lBashkirV50			((LanguageType)16)	// BA
#define	lByelorussianV50	((LanguageType)17)	// BE
#define	lBulgarianV50		((LanguageType)18)	// BG
#define	lBihariV50			((LanguageType)19)	// BH
#define	lBislamaV50			((LanguageType)20)	// BI
#define	lBengaliV50			((LanguageType)21)	// BN (Bangla)
#define	lTibetanV50			((LanguageType)22)	// BO
#define	lBretonV50			((LanguageType)23)	// BR
#define	lCatalanV50			((LanguageType)24)	// CA
#define	lCorsicanV50		((LanguageType)25)	// CO
#define	lCzechV50			((LanguageType)26)	// CS
#define	lWelshV50			((LanguageType)27)	// CY
#define	lDanishV50			((LanguageType)28)	// DA
#define	lBhutaniV50			((LanguageType)29)	// DZ
#define	lGreekV50			((LanguageType)30)	// EL
#define	lEsperantoV50		((LanguageType)31)	// EO
#define	lEstonianV50		((LanguageType)32)	// ET
#define	lBasqueV50			((LanguageType)33)	// EU
#define	lPersianV50			((LanguageType)34)	// FA (Farsi)
#define	lFinnishV50			((LanguageType)35)	// FI
#define	lFijiV50			((LanguageType)36)	// FJ
#define	lFaroeseV50			((LanguageType)37)	// FO
#define	lFrisianV50			((LanguageType)38)	// FY
#define	lIrishV50			((LanguageType)39)	// GA
#define	lScotsGaelicV50		((LanguageType)40)	// GD
#define	lGalicianV50		((LanguageType)41)	// GL
#define	lGuaraniV50			((LanguageType)42)	// GN
#define	lGujaratiV50		((LanguageType)43)	// GU
#define	lHausaV50			((LanguageType)44)	// HA
#define	lHindiV50			((LanguageType)45)	// HI
#define	lCroatianV50		((LanguageType)46)	// HR
#define	lHungarianV50		((LanguageType)47)	// HU
#define	lArmenianV50		((LanguageType)48)	// HY
#define	lInterlinguaV50		((LanguageType)49)	// IA
#define	lInterlingueV50		((LanguageType)50)	// IE
#define	lInupiakV50			((LanguageType)51)	// IK
#define	lIndonesianV50		((LanguageType)52)	// IN
#define	lIcelandicV50		((LanguageType)53)	// IS
#define	lHebrewV50			((LanguageType)54)	// IW
#define	lYiddishV50			((LanguageType)55)	// JI
#define	lJavaneseV50		((LanguageType)56)	// JW
#define	lGeorgianV50		((LanguageType)57)	// KA
#define	lKazakhV50			((LanguageType)58)	// KK
#define	lGreenlandicV50		((LanguageType)59)	// KL
#define	lCambodianV50		((LanguageType)60)	// KM
#define	lKannadaV50			((LanguageType)61)	// KN
#define	lKoreanV50			((LanguageType)62)	// KO
#define	lKashmiriV50		((LanguageType)63)	// KS
#define	lKurdishV50			((LanguageType)64)	// KU
#define	lKirghizV50			((LanguageType)65)	// KY
#define	lLatinV50			((LanguageType)66)	// LA
#define	lLingalaV50			((LanguageType)67)	// LN
#define	lLaothianV50		((LanguageType)68)	// LO
#define	lLithuanianV50		((LanguageType)69)	// LT
#define	lLatvianV50			((LanguageType)70)	// LV (Lettish)
#define	lMalagasyV50		((LanguageType)71)	// MG
#define	lMaoriV50			((LanguageType)72)	// MI
#define	lMacedonianV50		((LanguageType)73)	// MK
#define	lMalayalamV50		((LanguageType)74)	// ML
#define	lMongolianV50		((LanguageType)75)	// MN
#define	lMoldavianV50		((LanguageType)76)	// MO
#define	lMarathiV50			((LanguageType)77)	// MR
#define	lMalayV50			((LanguageType)78)	// MS
#define	lMalteseV50			((LanguageType)79)	// MT
#define	lBurmeseV50			((LanguageType)80)	// MY
#define	lNauruV50			((LanguageType)81)	// NA
#define	lNepaliV50			((LanguageType)82)	// NE
#define	lNorwegianV50		((LanguageType)83)	// NO
#define	lOccitanV50			((LanguageType)84)	// OC
#define	lAfanV50			((LanguageType)85)	// OM (Oromo)
#define	lOriyaV50			((LanguageType)86)	// OR
#define	lPunjabiV50			((LanguageType)87)	// PA
#define	lPolishV50			((LanguageType)88)	// PL
#define	lPashtoV50			((LanguageType)89)	// PS (Pushto)
#define	lPortugueseV50		((LanguageType)90)	// PT
#define	lQuechuaV50			((LanguageType)91)	// QU
#define	lRhaetoRomanceV50	((LanguageType)92)	// RM
#define	lKurundiV50			((LanguageType)93)	// RN
#define	lRomanianV50		((LanguageType)94)	// RO
#define	lRussianV50			((LanguageType)95)	// RU
#define	lKinyarwandaV50		((LanguageType)96)	// RW
#define	lSanskritV50		((LanguageType)97)	// SA
#define	lSindhiV50			((LanguageType)98)	// SD
#define	lSanghoV50			((LanguageType)99)	// SG
#define	lSerboCroatianV50	((LanguageType)100)	// SH
#define	lSinghaleseV50		((LanguageType)101)	// SI
#define	lSlovakV50			((LanguageType)102)	// SK
#define	lSlovenianV50		((LanguageType)103)	// SL
#define	lSamoanV50			((LanguageType)104)	// SM
#define	lShonaV50			((LanguageType)105)	// SN
#define	lSomaliV50			((LanguageType)106)	// SO
#define	lAlbanianV50		((LanguageType)107)	// SQ
#define	lSerbianV50			((LanguageType)108)	// SR
#define	lSiswatiV50			((LanguageType)109)	// SS
#define	lSesothoV50			((LanguageType)110)	// ST
#define	lSudaneseV50		((LanguageType)111)	// SU
#define	lSwedishV50			((LanguageType)112)	// SV
#define	lSwahiliV50			((LanguageType)113)	// SW
#define	lTamilV50			((LanguageType)114)	// TA
#define	lTeluguV50			((LanguageType)115)	// TE
#define	lTajikV50			((LanguageType)116)	// TG
#define	lThaiV50			((LanguageType)117)	// TH
#define	lTigrinyaV50		((LanguageType)118)	// TI
#define	lTurkmenV50			((LanguageType)119)	// TK
#define	lTagalogV50			((LanguageType)120)	// TL
#define	lSetswanaV50		((LanguageType)121)	// TN
#define	lTongaV50			((LanguageType)122)	// TO
#define	lTurkishV50			((LanguageType)123)	// TR
#define	lTsongaV50			((LanguageType)124)	// TS
#define	lTatarV50			((LanguageType)125)	// TT
#define	lTwiV50				((LanguageType)126)	// TW
#define	lUkrainianV50		((LanguageType)127)	// UK
#define	lUrduV50			((LanguageType)128)	// UR
#define	lUzbekV50			((LanguageType)129)	// UZ
#define	lVietnameseV50		((LanguageType)130)	// VI
#define	lVolapukV50			((LanguageType)131)	// VO
#define	lWolofV50			((LanguageType)132)	// WO
#define	lXhosaV50			((LanguageType)133)	// XH
#define	lYorubaV50			((LanguageType)134)	// YO
#define	lChineseV50			((LanguageType)135)	// ZH
#define	lZuluV50			((LanguageType)136)	// ZU
//
#define	lLanguageNumV50		((LanguageType)137)	// Number of Languages

/* Country codes (ISO 3166).  The first 33 preserve the old values for the
deprecated CountryType; the rest are sorted by the 2-character country code.
*/

#define	cAustraliaV50					((CountryType)0)		// AU
#define	cAustriaV50						((CountryType)1)		// AT
#define	cBelgiumV50						((CountryType)2)		// BE
#define	cBrazilV50						((CountryType)3)		// BR
#define	cCanadaV50						((CountryType)4)		// CA
#define	cDenmarkV50						((CountryType)5)		// DK
#define	cFinlandV50						((CountryType)6)		// FI
#define	cFranceV50						((CountryType)7)		// FR
#define	cGermanyV50						((CountryType)8)		// DE
#define	cHongKongV50					((CountryType)9)		// HK
#define	cIcelandV50						((CountryType)10)		// IS
#define	cIrelandV50						((CountryType)11)		// IE
#define	cItalyV50						((CountryType)12)		// IT
#define	cJapanV50						((CountryType)13)		// JP
#define	cLuxembourgV50					((CountryType)14)		// LU
#define	cMexicoV50						((CountryType)15)		// MX
#define	cNetherlandsV50					((CountryType)16)		// NL
#define	cNewZealandV50					((CountryType)17)		// NZ
#define	cNorwayV50						((CountryType)18)		// NO
#define	cSpainV50						((CountryType)19)		// ES
#define	cSwedenV50						((CountryType)20)		// SE
#define	cSwitzerlandV50					((CountryType)21)		// CH
#define	cUnitedKingdomV50				((CountryType)22)		// GB (UK)
#define	cUnitedStatesV50				((CountryType)23)		// US
#define	cIndiaV50						((CountryType)24)		// IN
#define	cIndonesiaV50					((CountryType)25)		// ID
#define	cRepublicOfKoreaV50				((CountryType)26)		// KR
#define	cMalaysiaV50					((CountryType)27)		// MY
#define	cChinaV50						((CountryType)28)		// CN
#define	cPhilippinesV50					((CountryType)29)		// PH
#define	cSingaporeV50					((CountryType)30)		// SG
#define	cThailandV50					((CountryType)31)		// TH
#define	cTaiwanV50						((CountryType)32)		// TW

// New in 4.0
#define	cAndorraV50						((CountryType)33)		// AD
#define	cUnitedArabEmiratesV50			((CountryType)34)		// AE
#define	cAfghanistanV50					((CountryType)35)		// AF
#define	cAntiguaAndBarbudaV50			((CountryType)36)		// AG
#define	cAnguillaV50					((CountryType)37)		// AI
#define	cAlbaniaV50						((CountryType)38)		// AL
#define	cArmeniaV50						((CountryType)39)		// AM
#define	cNetherlandsAntillesV50			((CountryType)40)		// AN
#define	cAngolaV50						((CountryType)41)		// AO
#define	cAntarcticaV50					((CountryType)42)		// AQ
#define	cArgentinaV50					((CountryType)43)		// AR
#define	cAmericanSamoaV50				((CountryType)44)		// AS
#define	cArubaV50						((CountryType)45)		// AW
#define	cAzerbaijanV50					((CountryType)46)		// AZ
#define	cBosniaAndHerzegovinaV50		((CountryType)47)		// BA
#define	cBarbadosV50					((CountryType)48)		// BB
#define	cBangladeshV50					((CountryType)49)		// BD
#define	cBurkinaFasoV50					((CountryType)50)		// BF
#define	cBulgariaV50					((CountryType)51)		// BG
#define	cBahrainV50						((CountryType)52)		// BH
#define	cBurundiV50						((CountryType)53)		// BI
#define	cBeninV50						((CountryType)54)		// BJ
#define	cBermudaV50						((CountryType)55)		// BM
#define	cBruneiDarussalamV50			((CountryType)56)		// BN
#define	cBoliviaV50						((CountryType)57)		// BO
#define	cBahamasV50						((CountryType)58)		// BS
#define	cBhutanV50						((CountryType)59)		// BT
#define	cBouvetIslandV50				((CountryType)60)		// BV
#define	cBotswanaV50					((CountryType)61)		// BW
#define	cBelarusV50						((CountryType)62)		// BY
#define	cBelizeV50						((CountryType)63)		// BZ
#define	cCocosIslandsV50				((CountryType)64)		// CC
#define	cDemocraticRepublicOfTheCongoV50	((CountryType)65)		// CD
#define	cCentralAfricanRepublicV50		((CountryType)66)		// CF
#define	cCongoV50						((CountryType)67)		// CG
#define	cIvoryCoastV50					((CountryType)68)		// CI
#define	cCookIslandsV50					((CountryType)69)		// CK
#define	cChileV50						((CountryType)70)		// CL
#define	cCameroonV50					((CountryType)71)		// CM
#define	cColumbiaV50					((CountryType)72)		// CO
#define	cCostaRicaV50					((CountryType)73)		// CR
#define	cCubaV50						((CountryType)74)		// CU
#define	cCapeVerdeV50					((CountryType)75)		// CV
#define	cChristmasIslandV50				((CountryType)76)		// CX
#define	cCyprusV50						((CountryType)77)		// CY
#define	cCzechRepublicV50				((CountryType)78)		// CZ
#define	cDjiboutiV50					((CountryType)79)		// DJ
#define	cDominicaV50					((CountryType)80)		// DM
#define	cDominicanRepublicV50			((CountryType)81)		// DO
#define	cAlgeriaV50						((CountryType)82)		// DZ
#define	cEcuadorV50						((CountryType)83)		// EC
#define	cEstoniaV50						((CountryType)84)		// EE
#define	cEgyptV50						((CountryType)85)		// EG
#define	cWesternSaharaV50				((CountryType)86)		// EH
#define	cEritreaV50						((CountryType)87)		// ER
#define	cEthiopiaV50					((CountryType)88)		// ET
#define	cFijiV50						((CountryType)89)		// FJ
#define	cFalklandIslandsV50				((CountryType)90)		// FK
#define	cMicronesiaV50					((CountryType)91)		// FM
#define	cFaeroeIslandsV50				((CountryType)92)		// FO
#define	cMetropolitanFranceV50			((CountryType)93)		// FX
#define	cGabonV50						((CountryType)94)		// GA
#define	cGrenadaV50						((CountryType)95)		// GD
#define	cGeorgiaV50						((CountryType)96)		// GE
#define	cFrenchGuianaV50				((CountryType)97)		// GF
#define	cGhanaV50						((CountryType)98)		// GH
#define	cGibraltarV50					((CountryType)99)		// GI
#define	cGreenlandV50					((CountryType)100)	// GL
#define	cGambiaV50						((CountryType)101)	// GM
#define	cGuineaV50						((CountryType)102)	// GN
#define	cGuadeloupeV50					((CountryType)103)	// GP
#define	cEquatorialGuineaV50			((CountryType)104)	// GQ
#define	cGreeceV50						((CountryType)105)	// GR
#define	cSouthGeorgiaAndTheSouthSandwichIslandsV50	((CountryType)106)	// GS
#define	cGuatemalaV50					((CountryType)107)	// GT
#define	cGuamV50						((CountryType)108)	// GU
#define	cGuineaBisseuV50				((CountryType)109)	// GW
#define	cGuyanaV50						((CountryType)110)	// GY
#define	cHeardAndMcDonaldIslandsV50		((CountryType)111)	// HM
#define	cHondurasV50					((CountryType)112)	// HN
#define	cCroatiaV50						((CountryType)113)	// HR
#define	cHaitiV50						((CountryType)114)	// HT
#define	cHungaryV50						((CountryType)115)	// HU
#define	cIsraelV50						((CountryType)116)	// IL
#define	cBritishIndianOceanTerritoryV50	((CountryType)117)	// IO
#define	cIraqV50						((CountryType)118)	// IQ
#define	cIranV50						((CountryType)119)	// IR
#define	cJamaicaV50						((CountryType)120)	// JM
#define	cJordanV50						((CountryType)121)	// JO
#define	cKenyaV50						((CountryType)122)	// KE
#define	cKyrgyzstanV50					((CountryType)123)	// KG (Kirgistan)
#define	cCambodiaV50					((CountryType)124)	// KH
#define	cKiribatiV50					((CountryType)125)	// KI
#define	cComorosV50						((CountryType)126)	// KM
#define	cStKittsAndNevisV50				((CountryType)127)	// KN
#define	cDemocraticPeoplesRepublicOfKoreaV50	((CountryType)128)	// KP
#define	cKuwaitV50						((CountryType)129)	// KW
#define	cCaymanIslandsV50				((CountryType)130)	// KY
#define	cKazakhstanV50					((CountryType)131)	// KK
#define	cLaosV50						((CountryType)132)	// LA
#define	cLebanonV50						((CountryType)133)	// LB
#define	cStLuciaV50						((CountryType)134)	// LC
#define	cLiechtensteinV50				((CountryType)135)	// LI
#define	cSriLankaV50					((CountryType)136)	// LK
#define	cLiberiaV50						((CountryType)137)	// LR
#define	cLesothoV50						((CountryType)138)	// LS
#define	cLithuaniaV50					((CountryType)139)	// LT
#define	cLatviaV50						((CountryType)140)	// LV
#define	cLibyaV50						((CountryType)141)	// LY
#define	cMorroccoV50					((CountryType)142)	// MA
#define	cMonacoV50						((CountryType)143)	// MC
#define	cMoldovaV50						((CountryType)144)	// MD
#define	cMadagascarV50					((CountryType)145)	// MG
#define	cMarshallIslandsV50				((CountryType)146)	// MH
#define	cMacedoniaV50					((CountryType)147)	// MK
#define	cMaliV50						((CountryType)148)	// ML
#define	cMyanmarV50						((CountryType)149)	// MM
#define	cMongoliaV50					((CountryType)150)	// MN
#define	cMacauV50						((CountryType)151)	// MO
#define	cNorthernMarianaIslandsV50		((CountryType)152)	// MP
#define	cMartiniqueV50					((CountryType)153)	// MQ
#define	cMauritaniaV50					((CountryType)154)	// MR
#define	cMontserratV50					((CountryType)155)	// MS
#define	cMaltaV50						((CountryType)156)	// MT
#define	cMauritiusV50					((CountryType)157)	// MU
#define	cMaldivesV50					((CountryType)158)	// MV
#define	cMalawiV50						((CountryType)159)	// MW
#define	cMozambiqueV50					((CountryType)160)	// MZ
#define	cNamibiaV50						((CountryType)161)	// NA
#define	cNewCaledoniaV50				((CountryType)162)	// NC
#define	cNigerV50						((CountryType)163)	// NE
#define	cNorfolkIslandV50				((CountryType)164)	// NF
#define	cNigeriaV50						((CountryType)165)	// NG
#define	cNicaraguaV50					((CountryType)166)	// NI
#define	cNepalV50						((CountryType)167)	// NP
#define	cNauruV50						((CountryType)168)	// NR
#define	cNiueV50						((CountryType)169)	// NU
#define	cOmanV50						((CountryType)170)	// OM
#define	cPanamaV50						((CountryType)171)	// PA
#define	cPeruV50						((CountryType)172)	// PE
#define	cFrenchPolynesiaV50				((CountryType)173)	// PF
#define	cPapuaNewGuineaV50				((CountryType)174)	// PG
#define	cPakistanV50					((CountryType)175)	// PK
#define	cPolandV50						((CountryType)176)	// PL
#define	cStPierreAndMiquelonV50			((CountryType)177)	// PM
#define	cPitcairnV50					((CountryType)178)	// PN
#define	cPuertoRicoV50					((CountryType)179)	// PR
#define	cPortugalV50					((CountryType)180)	// PT
#define	cPalauV50						((CountryType)181)	// PW
#define	cParaguayV50					((CountryType)182)	// PY
#define	cQatarV50						((CountryType)183)	// QA
#define	cReunionV50						((CountryType)184)	// RE
#define	cRomaniaV50						((CountryType)185)	// RO
#define	cRussianFederationV50			((CountryType)186)	// RU
#define	cRwandaV50						((CountryType)187)	// RW
#define	cSaudiArabiaV50					((CountryType)188)	// SA
#define	cSolomonIslandsV50				((CountryType)189)	// SB
#define	cSeychellesV50					((CountryType)190)	// SC
#define	cSudanV50						((CountryType)191)	// SD
#define	cStHelenaV50					((CountryType)192)	// SH
#define	cSloveniaV50					((CountryType)193)	// SI
#define	cSvalbardAndJanMayenIslandsV50	((CountryType)194)	// SJ
#define	cSlovakiaV50					((CountryType)195)	// SK
#define	cSierraLeoneV50					((CountryType)196)	// SL
#define	cSanMarinoV50					((CountryType)197)	// SM
#define	cSenegalV50						((CountryType)198)	// SN
#define	cSomaliaV50						((CountryType)199)	// SO
#define	cSurinameV50					((CountryType)200)	// SR
#define	cSaoTomeAndPrincipeV50			((CountryType)201)	// ST
#define	cElSalvadorV50					((CountryType)202)	// SV
#define	cSyranArabRepublicV50			((CountryType)203)	// SY
#define	cSwazilandV50					((CountryType)204)	// SZ
#define	cTurksAndCaicosIslandsV50		((CountryType)205)	// TC
#define	cChadV50						((CountryType)206)	// TD
#define	cFrenchSouthernTerritoriesV50	((CountryType)207)	// TF
#define	cTogoV50						((CountryType)208)	// TG
#define	cTajikistanV50					((CountryType)209)	// TJ
#define	cTokelauV50						((CountryType)210)	// TK
#define	cTurkmenistanV50				((CountryType)211)	// TM
#define	cTunisiaV50						((CountryType)212)	// TN
#define	cTongaV50						((CountryType)213)	// TO
#define	cEastTimorV50					((CountryType)214)	// TP
#define	cTurkeyV50						((CountryType)215)	// TR
#define	cTrinidadAndTobagoV50			((CountryType)216)	// TT
#define	cTuvaluV50						((CountryType)217)	// TV
#define	cTanzaniaV50					((CountryType)218)	// TZ
#define	cUkraineV50						((CountryType)219)	// UA
#define	cUgandaV50						((CountryType)220)	// UG
#define	cUnitedStatesMinorOutlyingIslandsV50	((CountryType)221)	// UM
#define	cUruguayV50						((CountryType)222)	// UY
#define	cUzbekistanV50					((CountryType)223)	// UZ
#define	cHolySeeV50						((CountryType)224)	// VA
#define	cStVincentAndTheGrenadinesV50	((CountryType)225)	// VC
#define	cVenezuelaV50					((CountryType)226)	// VE
#define	cBritishVirginIslandsV50		((CountryType)227)	// VG
#define	cUSVirginIslandsV50				((CountryType)228)	// VI
#define	cVietNamV50						((CountryType)229)	// VN
#define	cVanuatuV50						((CountryType)230)	// VU
#define	cWallisAndFutunaIslandsV50		((CountryType)231)	// WF
#define	cSamoaV50						((CountryType)232)	// WS
#define	cYemenV50						((CountryType)233)	// YE
#define	cMayotteV50						((CountryType)234)	// YT
#define	cYugoslaviaV50					((CountryType)235)	// YU
#define	cSouthAfricaV50					((CountryType)236)	// ZA
#define	cZambiaV50						((CountryType)237)	// ZM
#define	cZimbabweV50					((CountryType)238)	// ZW
//
#define	cCountryNumV50					((CountryType)239)	// Number of Countries


#endif // _PALM_OS__LOCALE_MGR_COMPATIBILITY_H
