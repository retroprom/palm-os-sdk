/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PdiConst.h
 *
 * Release: eclipse 5 SDK (68K) R4.
 *
 * Description:
 *		PDI Library constants
 *
 *****************************************************************************/

/******************************************************************************
 * Property fields access
 *****************************************************************************/

#define kPdiPVF_ADR_POST_OFFICE ((UInt8) 0)
#define kPdiPVF_ADR_EXTENDED ((UInt8) 1)
#define kPdiPVF_ADR_STREET ((UInt8) 2)
#define kPdiPVF_ADR_LOCALITY ((UInt8) 3)
#define kPdiPVF_ADR_REGION ((UInt8) 4)
#define kPdiPVF_ADR_POSTAL_CODE ((UInt8) 5)
#define kPdiPVF_ADR_COUNTRY ((UInt8) 6)
#define kPdiPVF_GEO_LATITUDE ((UInt8) 0)
#define kPdiPVF_GEO_LONGITUDE ((UInt8) 1)
#define kPdiPVF_N_FAMILY ((UInt8) 0)
#define kPdiPVF_N_GIVEN ((UInt8) 1)
#define kPdiPVF_N_ADDITIONAL ((UInt8) 2)
#define kPdiPVF_N_PREFIXES ((UInt8) 3)
#define kPdiPVF_N_SUFFIXES ((UInt8) 4)

/******************************************************************************
 * Properties constants
 *****************************************************************************/

#define kPdiPRN_FREEBUSY		((UInt16) 988)
#define kPdiPRN_X_PALM_CUSTOM		((UInt16) 1044)
#define kPdiPRN_METHOD		((UInt16) 1108)
#define kPdiPRN_ORG		((UInt16) 1236)
#define kPdiPRN_X_PALM_CATEGORY		((UInt16) 1250)
#define kPdiPRN_TITLE		((UInt16) 1488)
#define kPdiPRN_ORGANIZER		((UInt16) 1528)
#define kPdiPRN_TZ		((UInt16) 1566)
#define kPdiPRN_VERSION		((UInt16) 1682)
#define kPdiPRN_TZID		((UInt16) 1722)
#define kPdiPRN_CLASS		((UInt16) 1814)
#define kPdiPRN_TZURL		((UInt16) 1832)
#define kPdiPRN_EXDATE		((UInt16) 1886)
#define kPdiPRN_EXRULE		((UInt16) 1906)
#define kPdiPRN_PRODID		((UInt16) 1926)
#define kPdiPRN_TZNAME		((UInt16) 1946)
#define kPdiPRN_GEO		((UInt16) 1966)
#define kPdiPRN_UID		((UInt16) 1980)
#define kPdiPRN_PROFILE		((UInt16) 1994)
#define kPdiPRN_PRIORITY		((UInt16) 2032)
#define kPdiPRN_ROLE		((UInt16) 2056)
#define kPdiPRN_TZOFFSET		((UInt16) 2072)
#define kPdiPRN_AALARM		((UInt16) 2160)
#define kPdiPRN_TZOFFSETTO		((UInt16) 2180)
#define kPdiPRN_TZOFFSETFROM		((UInt16) 2244)
#define kPdiPRN_SOUND		((UInt16) 2312)
#define kPdiPRN_ACTION		((UInt16) 2410)
#define kPdiPRN_SOURCE		((UInt16) 2430)
#define kPdiPRN_ADR		((UInt16) 2472)
#define kPdiPRN_COMMENT		((UInt16) 2486)
#define kPdiPRN_CONTACT		((UInt16) 2530)
#define kPdiPRN_NICKNAME		((UInt16) 2568)
#define kPdiPRN_COMPLETED		((UInt16) 2620)
#define kPdiPRN_RRULE		((UInt16) 2646)
#define kPdiPRN_ATTACH		((UInt16) 2692)
#define kPdiPRN_SORT_STRING		((UInt16) 2728)
#define kPdiPRN_ATTENDEE		((UInt16) 2776)
#define kPdiPRN_LOGO		((UInt16) 2800)
#define kPdiPRN_EMAIL		((UInt16) 2832)
#define kPdiPRN_END		((UInt16) 2852)
#define kPdiPRN_BDAY		((UInt16) 2866)
#define kPdiPRN_CALSCALE		((UInt16) 2882)
#define kPdiPRN_LOCATION		((UInt16) 2906)
#define kPdiPRN_PERCENT_COMPLETE		((UInt16) 2930)
#define kPdiPRN_PHOTO		((UInt16) 2970)
#define kPdiPRN_RDATE		((UInt16) 2988)
#define kPdiPRN_CATEGORIES		((UInt16) 3026)
#define kPdiPRN_CREATED		((UInt16) 3074)
#define kPdiPRN_REV		((UInt16) 3096)
#define kPdiPRN_LABEL		((UInt16) 3226)
#define kPdiPRN_BEGIN		((UInt16) 3244)
#define kPdiPRN_END_VCARD		((UInt16) 3262)
#define kPdiPRN_END_VTODO		((UInt16) 3288)
#define kPdiPRN_AGENT		((UInt16) 3314)
#define kPdiPRN_DALARM		((UInt16) 3372)
#define kPdiPRN_FN		((UInt16) 3392)
#define kPdiPRN_REPEAT		((UInt16) 3404)
#define kPdiPRN_END_VEVENT		((UInt16) 3424)
#define kPdiPRN_END_VJOURNAL		((UInt16) 3560)
#define kPdiPRN_END_VCALENDAR		((UInt16) 3694)
#define kPdiPRN_RESOURCES		((UInt16) 3728)
#define kPdiPRN_END_VFREEBUSY		((UInt16) 3754)
#define kPdiPRN_END_VTIMEZONE		((UInt16) 3788)
#define kPdiPRN_STATUS		((UInt16) 3870)
#define kPdiPRN_RELATED_TO		((UInt16) 3890)
#define kPdiPRN_TRANSP		((UInt16) 3918)
#define kPdiPRN_KEY		((UInt16) 3938)
#define kPdiPRN_BEGIN_VCARD		((UInt16) 3952)
#define kPdiPRN_BEGIN_VTODO		((UInt16) 3982)
#define kPdiPRN_TRIGGER		((UInt16) 4012)
#define kPdiPRN_NOTE		((UInt16) 4034)
#define kPdiPRN_BEGIN_VEVENT		((UInt16) 4050)
#define kPdiPRN_N		((UInt16) 4118)
#define kPdiPRN_LAST_MODIFIED		((UInt16) 4128)
#define kPdiPRN_RECURRENCE_ID		((UInt16) 4162)
#define kPdiPRN_MAILER		((UInt16) 4216)
#define kPdiPRN_REQUEST_STATUS		((UInt16) 4236)
#define kPdiPRN_BEGIN_VJOURNAL		((UInt16) 4272)
#define kPdiPRN_SUMMARY		((UInt16) 4366)
#define kPdiPRN_BEGIN_VCALENDAR		((UInt16) 4388)
#define kPdiPRN_URL		((UInt16) 4426)
#define kPdiPRN_BEGIN_VFREEBUSY		((UInt16) 4440)
#define kPdiPRN_BEGIN_VTIMEZONE		((UInt16) 4478)
#define kPdiPRN_SEQUENCE		((UInt16) 4594)
#define kPdiPRN_DTEND		((UInt16) 4660)
#define kPdiPRN_DTSTART		((UInt16) 4678)
#define kPdiPRN_DUE		((UInt16) 4700)
#define kPdiPRN_TEL		((UInt16) 4714)
#define kPdiPRN_DTSTAMP		((UInt16) 4744)
#define kPdiPRN_NAME		((UInt16) 4800)
#define kPdiPRN_DURATION		((UInt16) 4954)
#define kPdiPRN_DESCRIPTION		((UInt16) 5270)

/******************************************************************************
 * Parameters constants
 *****************************************************************************/

#define kPdiPAN_DELEGATED_TO		((UInt16) 1012)
#define kPdiPAN_X		((UInt16) 1098)
#define kPdiPAN_DELEGATED_FROM		((UInt16) 1128)
#define kPdiPAN_MEMBER		((UInt16) 1164)
#define kPdiPAN_UTC_OFFSET		((UInt16) 1186)
#define kPdiPAN_DIR		((UInt16) 1350)
#define kPdiPAN_TYPE		((UInt16) 1428)
#define kPdiPAN_TIME		((UInt16) 1446)
#define kPdiPAN_PARTSTAT		((UInt16) 1738)
#define kPdiPAN_ROLE		((UInt16) 2056)
#define kPdiPAN_CN		((UInt16) 2208)
#define kPdiPAN_SOUND		((UInt16) 2312)
#define kPdiPAN_RANGE		((UInt16) 2330)
#define kPdiPAN_CONTEXT		((UInt16) 2508)
#define kPdiPAN_RSVP		((UInt16) 2816)
#define kPdiPAN_ENCODE		((UInt16) 3054)
#define kPdiPAN_ENCODING		((UInt16) 3166)
#define kPdiPAN_FMTTYPE		((UInt16) 3452)
#define kPdiPAN_RELATED		((UInt16) 3474)
#define kPdiPAN_RELTYPE		((UInt16) 3496)
#define kPdiPAN_LANGUAGE		((UInt16) 3618)
#define kPdiPAN_STATUS		((UInt16) 3870)
#define kPdiPAN_CUTYPE		((UInt16) 4308)
#define kPdiPAN_SENT_BY		((UInt16) 4556)
#define kPdiPAN_URI		((UInt16) 4580)
#define kPdiPAN_VALUE		((UInt16) 4852)
#define kPdiPAN_ALTREP		((UInt16) 5200)
#define kPdiPAN_FBTYPE		((UInt16) 5220)
#define kPdiPAN_CHARSET		((UInt16) 5300)

/******************************************************************************
 * Parameter pairs constants
 *****************************************************************************/

#define kPdiPAV_TYPE_HOME		((UInt16) 0)
#define kPdiPAV_VALUE_VCARD		((UInt16) 2)
#define kPdiPAV_TYPE_VCARD		((UInt16) 4)
#define kPdiPAV_VALUE_UTC_OFFSET		((UInt16) 6)
#define kPdiPAV_TYPE_POSTAL		((UInt16) 8)
#define kPdiPAV_RELTYPE_SIBLING		((UInt16) 10)
#define kPdiPAV_TYPE_INTL		((UInt16) 12)
#define kPdiPAV_CUTYPE_GROUP		((UInt16) 14)
#define kPdiPAV_ROLE_OPT_PARTICIPANT		((UInt16) 16)
#define kPdiPAV_VALUE_INTEGER		((UInt16) 18)
#define kPdiPAV_VALUE_TIME		((UInt16) 20)
#define kPdiPAV_TYPE_INTERNET		((UInt16) 22)
#define kPdiPAV_TYPE_PAGER		((UInt16) 24)
#define kPdiPAV_ROLE_ORGANIZER		((UInt16) 26)
#define kPdiPAV_ENCODING_Q		((UInt16) 28)
#define kPdiPAV_CUTYPE_INDIVIDUAL		((UInt16) 30)
#define kPdiPAV_PARTSTAT_IN_PROCESS		((UInt16) 32)
#define kPdiPAV_RELTYPE_PARENT		((UInt16) 34)
#define kPdiPAV_TYPE_PARCEL		((UInt16) 36)
#define kPdiPAV_TYPE_PREF		((UInt16) 38)
#define kPdiPAV_RANGE_THISANDPRIOR		((UInt16) 40)
#define kPdiPAV_ENCODING_8BIT		((UInt16) 42)
#define kPdiPAV_RANGE_THISANDFUTURE		((UInt16) 44)
#define kPdiPAV_TYPE_PCS		((UInt16) 46)
#define kPdiPAV_CUTYPE_ROOM		((UInt16) 48)
#define kPdiPAV_PARTSTAT_NEEDS_ACTION		((UInt16) 50)
#define kPdiPAV_STATUS_NEEDS_ACTION		((UInt16) 52)
#define kPdiPAV_ENCODING_B		((UInt16) 54)
#define kPdiPAV_VALUE_BOOLEAN		((UInt16) 56)
#define kPdiPAV_TYPE_X400		((UInt16) 58)
#define kPdiPAV_TYPE_ISDN		((UInt16) 60)
#define kPdiPAV_ROLE_OWNER		((UInt16) 62)
#define kPdiPAV_TYPE_VIDEO		((UInt16) 64)
#define kPdiPAV_ENCODING_BASE64		((UInt16) 66)
#define kPdiPAV_VALUE_PERIOD		((UInt16) 68)
#define kPdiPAV_TYPE_BBS		((UInt16) 70)
#define kPdiPAV_PARTSTAT_ACCEPTED		((UInt16) 72)
#define kPdiPAV_STATUS_ACCEPTED		((UInt16) 74)
#define kPdiPAV_PARTSTAT_COMPLETED		((UInt16) 76)
#define kPdiPAV_STATUS_COMPLETED		((UInt16) 78)
#define kPdiPAV_STATUS_CONFIRMED		((UInt16) 80)
#define kPdiPAV_TYPE_CAR		((UInt16) 82)
#define kPdiPAV_TYPE_DOM		((UInt16) 84)
#define kPdiPAV_ROLE_ATTENDEE		((UInt16) 86)
#define kPdiPAV_RELATED_END		((UInt16) 88)
#define kPdiPAV_VALUE_FLOAT		((UInt16) 90)
#define kPdiPAV_CUTYPE_UNKNOWN		((UInt16) 92)
#define kPdiPAV_VALUE_CAL_ADDRESS		((UInt16) 94)
#define kPdiPAV_FBTYPE_BUSY		((UInt16) 96)
#define kPdiPAV_VALUE_DATE		((UInt16) 98)
#define kPdiPAV_VALUE_RECUR		((UInt16) 100)
#define kPdiPAV_TYPE_MODEM		((UInt16) 102)
#define kPdiPAV_ENCODING_QUOTED_PRINTABLE		((UInt16) 104)
#define kPdiPAV_CUTYPE_RESOURCE		((UInt16) 106)
#define kPdiPAV_RSVP_TRUE		((UInt16) 108)
#define kPdiPAV_VALUE_PHONE_NUMBER		((UInt16) 110)
#define kPdiPAV_RELATED_START		((UInt16) 112)
#define kPdiPAV_VALUE_DATE_TIME		((UInt16) 114)
#define kPdiPAV_TYPE_CELL		((UInt16) 116)
#define kPdiPAV_STATUS_SENT		((UInt16) 118)
#define kPdiPAV_TYPE_VOICE		((UInt16) 120)
#define kPdiPAV_FBTYPE_BUSY_TENTATIVE		((UInt16) 122)
#define kPdiPAV_ROLE_REQ_PARTICIPANT		((UInt16) 124)
#define kPdiPAV_VALUE_URI		((UInt16) 126)
#define kPdiPAV_FBTYPE_BUSY_UNAVAILABLE		((UInt16) 128)
#define kPdiPAV_TYPE_FAX		((UInt16) 130)
#define kPdiPAV_TYPE_MSG		((UInt16) 132)
#define kPdiPAV_TYPE_WORK		((UInt16) 134)
#define kPdiPAV_VALUE_TEXT		((UInt16) 136)
#define kPdiPAV_CONTEXT_WORD		((UInt16) 138)
#define kPdiPAV_RSVP_FALSE		((UInt16) 140)
#define kPdiPAV_VALUE_BINARY		((UInt16) 142)
#define kPdiPAV_ROLE_NON_PARTICIPANT		((UInt16) 144)
#define kPdiPAV_VALUE_DURATION		((UInt16) 146)
#define kPdiPAV_X_X_PALM_N		((UInt16) 148)
#define kPdiPAV_X_X_IRMC_N		((UInt16) 150)
#define kPdiPAV_FBTYPE_FREE		((UInt16) 152)
#define kPdiPAV_PARTSTAT_DECLINED		((UInt16) 154)
#define kPdiPAV_STATUS_DECLINED		((UInt16) 156)
#define kPdiPAV_PARTSTAT_TENTATIVE		((UInt16) 158)
#define kPdiPAV_STATUS_TENTATIVE		((UInt16) 160)
#define kPdiPAV_PARTSTAT_DELEGATED		((UInt16) 162)
#define kPdiPAV_STATUS_DELEGATED		((UInt16) 164)
#define kPdiPAV_RELTYPE_CHILD		((UInt16) 166)
#define kPdiPAV_ROLE_CHAIR		((UInt16) 168)
#define kPdiPAV_X_X_PALM_ORG		((UInt16) 170)
#define kPdiPAV_X_X_IRMC_ORG		((UInt16) 172)
#define kPdiPAV_X_X_PALM_MAIN		((UInt16) 174)

/******************************************************************************
 * Properties types constants
 *****************************************************************************/

#define kPdiType_DATE_TIME		kPdiPAV_VALUE_DATE_TIME
#define kPdiType_TEXT		kPdiPAV_VALUE_TEXT
#define kPdiType_CAL_ADDRESS		kPdiPAV_VALUE_CAL_ADDRESS
#define kPdiType_DURATION		kPdiPAV_VALUE_DURATION
#define kPdiType_RECUR		kPdiPAV_VALUE_RECUR
#define kPdiType_PERIOD		kPdiPAV_VALUE_PERIOD
#define kPdiType_FLOAT		kPdiPAV_VALUE_FLOAT
#define kPdiType_BINARY		kPdiPAV_VALUE_BINARY
#define kPdiType_INTEGER		kPdiPAV_VALUE_INTEGER
#define kPdiType_UTC_OFFSET		kPdiPAV_VALUE_UTC_OFFSET
#define kPdiType_URI		kPdiPAV_VALUE_URI
#define kPdiType_BOOLEAN		kPdiPAV_VALUE_BOOLEAN
#define kPdiType_DATE		kPdiPAV_VALUE_DATE
#define kPdiType_TIME		kPdiPAV_VALUE_TIME
#define kPdiType_VCARD		kPdiPAV_VALUE_VCARD
#define kPdiType_PHONE_NUMBER		kPdiPAV_VALUE_PHONE_NUMBER
