/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PdiConst.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		PDI Library constants
 *
 *****************************************************************************/

#ifndef __PDICONST_H__
#define __PDICONST_H__

/******************************************************************************
 * Property fields access
 *****************************************************************************/

#define kPdiPVF_ADR_POST_OFFICE ((uint8_t) 0)
#define kPdiPVF_ADR_EXTENDED ((uint8_t) 1)
#define kPdiPVF_ADR_STREET ((uint8_t) 2)
#define kPdiPVF_ADR_LOCALITY ((uint8_t) 3)
#define kPdiPVF_ADR_REGION ((uint8_t) 4)
#define kPdiPVF_ADR_POSTAL_CODE ((uint8_t) 5)
#define kPdiPVF_ADR_COUNTRY ((uint8_t) 6)
#define kPdiPVF_GEO_LATITUDE ((uint8_t) 0)
#define kPdiPVF_GEO_LONGITUDE ((uint8_t) 1)
#define kPdiPVF_N_FAMILY ((uint8_t) 0)
#define kPdiPVF_N_GIVEN ((uint8_t) 1)
#define kPdiPVF_N_ADDITIONAL ((uint8_t) 2)
#define kPdiPVF_N_PREFIXES ((uint8_t) 3)
#define kPdiPVF_N_SUFFIXES ((uint8_t) 4)

/******************************************************************************
 * Properties constants
 *****************************************************************************/

#define kPdiPRN_FREEBUSY		((uint16_t) 988)
#define kPdiPRN_X_PALM_CUSTOM		((uint16_t) 1044)
#define kPdiPRN_METHOD		((uint16_t) 1108)
#define kPdiPRN_ORG		((uint16_t) 1236)
#define kPdiPRN_X_PALM_CATEGORY		((uint16_t) 1250)
#define kPdiPRN_TITLE		((uint16_t) 1488)
#define kPdiPRN_ORGANIZER		((uint16_t) 1528)
#define kPdiPRN_TZ		((uint16_t) 1566)
#define kPdiPRN_VERSION		((uint16_t) 1682)
#define kPdiPRN_TZID		((uint16_t) 1722)
#define kPdiPRN_CLASS		((uint16_t) 1814)
#define kPdiPRN_TZURL		((uint16_t) 1832)
#define kPdiPRN_EXDATE		((uint16_t) 1886)
#define kPdiPRN_EXRULE		((uint16_t) 1906)
#define kPdiPRN_PRODID		((uint16_t) 1926)
#define kPdiPRN_TZNAME		((uint16_t) 1946)
#define kPdiPRN_GEO		((uint16_t) 1966)
#define kPdiPRN_UID		((uint16_t) 1980)
#define kPdiPRN_PROFILE		((uint16_t) 1994)
#define kPdiPRN_PRIORITY		((uint16_t) 2032)
#define kPdiPRN_ROLE		((uint16_t) 2056)
#define kPdiPRN_TZOFFSET		((uint16_t) 2072)
#define kPdiPRN_AALARM		((uint16_t) 2160)
#define kPdiPRN_TZOFFSETTO		((uint16_t) 2180)
#define kPdiPRN_TZOFFSETFROM		((uint16_t) 2244)
#define kPdiPRN_SOUND		((uint16_t) 2312)
#define kPdiPRN_ACTION		((uint16_t) 2410)
#define kPdiPRN_SOURCE		((uint16_t) 2430)
#define kPdiPRN_ADR		((uint16_t) 2472)
#define kPdiPRN_COMMENT		((uint16_t) 2486)
#define kPdiPRN_CONTACT		((uint16_t) 2530)
#define kPdiPRN_NICKNAME		((uint16_t) 2568)
#define kPdiPRN_COMPLETED		((uint16_t) 2620)
#define kPdiPRN_RRULE		((uint16_t) 2646)
#define kPdiPRN_ATTACH		((uint16_t) 2692)
#define kPdiPRN_SORT_STRING		((uint16_t) 2728)
#define kPdiPRN_ATTENDEE		((uint16_t) 2776)
#define kPdiPRN_LOGO		((uint16_t) 2800)
#define kPdiPRN_EMAIL		((uint16_t) 2832)
#define kPdiPRN_END		((uint16_t) 2852)
#define kPdiPRN_BDAY		((uint16_t) 2866)
#define kPdiPRN_CALSCALE		((uint16_t) 2882)
#define kPdiPRN_LOCATION		((uint16_t) 2906)
#define kPdiPRN_PERCENT_COMPLETE		((uint16_t) 2930)
#define kPdiPRN_PHOTO		((uint16_t) 2970)
#define kPdiPRN_RDATE		((uint16_t) 2988)
#define kPdiPRN_CATEGORIES		((uint16_t) 3026)
#define kPdiPRN_CREATED		((uint16_t) 3074)
#define kPdiPRN_REV		((uint16_t) 3096)
#define kPdiPRN_LABEL		((uint16_t) 3226)
#define kPdiPRN_BEGIN		((uint16_t) 3244)
#define kPdiPRN_END_VCARD		((uint16_t) 3262)
#define kPdiPRN_END_VTODO		((uint16_t) 3288)
#define kPdiPRN_AGENT		((uint16_t) 3314)
#define kPdiPRN_DALARM		((uint16_t) 3372)
#define kPdiPRN_FN		((uint16_t) 3392)
#define kPdiPRN_REPEAT		((uint16_t) 3404)
#define kPdiPRN_END_VEVENT		((uint16_t) 3424)
#define kPdiPRN_END_VJOURNAL		((uint16_t) 3560)
#define kPdiPRN_END_VCALENDAR		((uint16_t) 3694)
#define kPdiPRN_RESOURCES		((uint16_t) 3728)
#define kPdiPRN_END_VFREEBUSY		((uint16_t) 3754)
#define kPdiPRN_END_VTIMEZONE		((uint16_t) 3788)
#define kPdiPRN_STATUS		((uint16_t) 3870)
#define kPdiPRN_RELATED_TO		((uint16_t) 3890)
#define kPdiPRN_TRANSP		((uint16_t) 3918)
#define kPdiPRN_KEY		((uint16_t) 3938)
#define kPdiPRN_BEGIN_VCARD		((uint16_t) 3952)
#define kPdiPRN_BEGIN_VTODO		((uint16_t) 3982)
#define kPdiPRN_TRIGGER		((uint16_t) 4012)
#define kPdiPRN_NOTE		((uint16_t) 4034)
#define kPdiPRN_BEGIN_VEVENT		((uint16_t) 4050)
#define kPdiPRN_N		((uint16_t) 4118)
#define kPdiPRN_LAST_MODIFIED		((uint16_t) 4128)
#define kPdiPRN_RECURRENCE_ID		((uint16_t) 4162)
#define kPdiPRN_MAILER		((uint16_t) 4216)
#define kPdiPRN_REQUEST_STATUS		((uint16_t) 4236)
#define kPdiPRN_BEGIN_VJOURNAL		((uint16_t) 4272)
#define kPdiPRN_SUMMARY		((uint16_t) 4366)
#define kPdiPRN_BEGIN_VCALENDAR		((uint16_t) 4388)
#define kPdiPRN_URL		((uint16_t) 4426)
#define kPdiPRN_BEGIN_VFREEBUSY		((uint16_t) 4440)
#define kPdiPRN_BEGIN_VTIMEZONE		((uint16_t) 4478)
#define kPdiPRN_SEQUENCE		((uint16_t) 4594)
#define kPdiPRN_DTEND		((uint16_t) 4660)
#define kPdiPRN_DTSTART		((uint16_t) 4678)
#define kPdiPRN_DUE		((uint16_t) 4700)
#define kPdiPRN_TEL		((uint16_t) 4714)
#define kPdiPRN_DTSTAMP		((uint16_t) 4744)
#define kPdiPRN_NAME		((uint16_t) 4800)
#define kPdiPRN_DURATION		((uint16_t) 4954)
#define kPdiPRN_DESCRIPTION		((uint16_t) 5270)

/******************************************************************************
 * Parameters constants
 *****************************************************************************/

#define kPdiPAN_DELEGATED_TO		((uint16_t) 1012)
#define kPdiPAN_X		((uint16_t) 1098)
#define kPdiPAN_DELEGATED_FROM		((uint16_t) 1128)
#define kPdiPAN_MEMBER		((uint16_t) 1164)
#define kPdiPAN_UTC_OFFSET		((uint16_t) 1186)
#define kPdiPAN_DIR		((uint16_t) 1350)
#define kPdiPAN_TYPE		((uint16_t) 1428)
#define kPdiPAN_TIME		((uint16_t) 1446)
#define kPdiPAN_PARTSTAT		((uint16_t) 1738)
#define kPdiPAN_ROLE		((uint16_t) 2056)
#define kPdiPAN_CN		((uint16_t) 2208)
#define kPdiPAN_SOUND		((uint16_t) 2312)
#define kPdiPAN_RANGE		((uint16_t) 2330)
#define kPdiPAN_CONTEXT		((uint16_t) 2508)
#define kPdiPAN_RSVP		((uint16_t) 2816)
#define kPdiPAN_ENCODE		((uint16_t) 3054)
#define kPdiPAN_ENCODING		((uint16_t) 3166)
#define kPdiPAN_FMTTYPE		((uint16_t) 3452)
#define kPdiPAN_RELATED		((uint16_t) 3474)
#define kPdiPAN_RELTYPE		((uint16_t) 3496)
#define kPdiPAN_LANGUAGE		((uint16_t) 3618)
#define kPdiPAN_STATUS		((uint16_t) 3870)
#define kPdiPAN_CUTYPE		((uint16_t) 4308)
#define kPdiPAN_SENT_BY		((uint16_t) 4556)
#define kPdiPAN_URI		((uint16_t) 4580)
#define kPdiPAN_VALUE		((uint16_t) 4852)
#define kPdiPAN_ALTREP		((uint16_t) 5200)
#define kPdiPAN_FBTYPE		((uint16_t) 5220)
#define kPdiPAN_CHARSET		((uint16_t) 5300)

/******************************************************************************
 * Parameter pairs constants
 *****************************************************************************/

#define kPdiPAV_TYPE_HOME		((uint16_t) 0)
#define kPdiPAV_VALUE_VCARD		((uint16_t) 2)
#define kPdiPAV_TYPE_VCARD		((uint16_t) 4)
#define kPdiPAV_VALUE_UTC_OFFSET		((uint16_t) 6)
#define kPdiPAV_TYPE_POSTAL		((uint16_t) 8)
#define kPdiPAV_RELTYPE_SIBLING		((uint16_t) 10)
#define kPdiPAV_TYPE_INTL		((uint16_t) 12)
#define kPdiPAV_CUTYPE_GROUP		((uint16_t) 14)
#define kPdiPAV_ROLE_OPT_PARTICIPANT		((uint16_t) 16)
#define kPdiPAV_VALUE_INTEGER		((uint16_t) 18)
#define kPdiPAV_VALUE_TIME		((uint16_t) 20)
#define kPdiPAV_TYPE_INTERNET		((uint16_t) 22)
#define kPdiPAV_TYPE_PAGER		((uint16_t) 24)
#define kPdiPAV_ROLE_ORGANIZER		((uint16_t) 26)
#define kPdiPAV_ENCODING_Q		((uint16_t) 28)
#define kPdiPAV_CUTYPE_INDIVIDUAL		((uint16_t) 30)
#define kPdiPAV_PARTSTAT_IN_PROCESS		((uint16_t) 32)
#define kPdiPAV_RELTYPE_PARENT		((uint16_t) 34)
#define kPdiPAV_TYPE_PARCEL		((uint16_t) 36)
#define kPdiPAV_TYPE_PREF		((uint16_t) 38)
#define kPdiPAV_RANGE_THISANDPRIOR		((uint16_t) 40)
#define kPdiPAV_ENCODING_8BIT		((uint16_t) 42)
#define kPdiPAV_RANGE_THISANDFUTURE		((uint16_t) 44)
#define kPdiPAV_TYPE_PCS		((uint16_t) 46)
#define kPdiPAV_CUTYPE_ROOM		((uint16_t) 48)
#define kPdiPAV_PARTSTAT_NEEDS_ACTION		((uint16_t) 50)
#define kPdiPAV_STATUS_NEEDS_ACTION		((uint16_t) 52)
#define kPdiPAV_ENCODING_B		((uint16_t) 54)
#define kPdiPAV_VALUE_BOOLEAN		((uint16_t) 56)
#define kPdiPAV_TYPE_X400		((uint16_t) 58)
#define kPdiPAV_TYPE_ISDN		((uint16_t) 60)
#define kPdiPAV_ROLE_OWNER		((uint16_t) 62)
#define kPdiPAV_TYPE_VIDEO		((uint16_t) 64)
#define kPdiPAV_ENCODING_BASE64		((uint16_t) 66)
#define kPdiPAV_VALUE_PERIOD		((uint16_t) 68)
#define kPdiPAV_TYPE_BBS		((uint16_t) 70)
#define kPdiPAV_PARTSTAT_ACCEPTED		((uint16_t) 72)
#define kPdiPAV_STATUS_ACCEPTED		((uint16_t) 74)
#define kPdiPAV_PARTSTAT_COMPLETED		((uint16_t) 76)
#define kPdiPAV_STATUS_COMPLETED		((uint16_t) 78)
#define kPdiPAV_STATUS_CONFIRMED		((uint16_t) 80)
#define kPdiPAV_TYPE_CAR		((uint16_t) 82)
#define kPdiPAV_TYPE_DOM		((uint16_t) 84)
#define kPdiPAV_ROLE_ATTENDEE		((uint16_t) 86)
#define kPdiPAV_RELATED_END		((uint16_t) 88)
#define kPdiPAV_VALUE_FLOAT		((uint16_t) 90)
#define kPdiPAV_CUTYPE_UNKNOWN		((uint16_t) 92)
#define kPdiPAV_VALUE_CAL_ADDRESS		((uint16_t) 94)
#define kPdiPAV_FBTYPE_BUSY		((uint16_t) 96)
#define kPdiPAV_VALUE_DATE		((uint16_t) 98)
#define kPdiPAV_VALUE_RECUR		((uint16_t) 100)
#define kPdiPAV_TYPE_MODEM		((uint16_t) 102)
#define kPdiPAV_ENCODING_QUOTED_PRINTABLE		((uint16_t) 104)
#define kPdiPAV_CUTYPE_RESOURCE		((uint16_t) 106)
#define kPdiPAV_RSVP_TRUE		((uint16_t) 108)
#define kPdiPAV_VALUE_PHONE_NUMBER		((uint16_t) 110)
#define kPdiPAV_RELATED_START		((uint16_t) 112)
#define kPdiPAV_VALUE_DATE_TIME		((uint16_t) 114)
#define kPdiPAV_TYPE_CELL		((uint16_t) 116)
#define kPdiPAV_STATUS_SENT		((uint16_t) 118)
#define kPdiPAV_TYPE_VOICE		((uint16_t) 120)
#define kPdiPAV_FBTYPE_BUSY_TENTATIVE		((uint16_t) 122)
#define kPdiPAV_ROLE_REQ_PARTICIPANT		((uint16_t) 124)
#define kPdiPAV_VALUE_URI		((uint16_t) 126)
#define kPdiPAV_FBTYPE_BUSY_UNAVAILABLE		((uint16_t) 128)
#define kPdiPAV_TYPE_FAX		((uint16_t) 130)
#define kPdiPAV_TYPE_MSG		((uint16_t) 132)
#define kPdiPAV_TYPE_WORK		((uint16_t) 134)
#define kPdiPAV_VALUE_TEXT		((uint16_t) 136)
#define kPdiPAV_CONTEXT_WORD		((uint16_t) 138)
#define kPdiPAV_RSVP_FALSE		((uint16_t) 140)
#define kPdiPAV_VALUE_BINARY		((uint16_t) 142)
#define kPdiPAV_ROLE_NON_PARTICIPANT		((uint16_t) 144)
#define kPdiPAV_VALUE_DURATION		((uint16_t) 146)
#define kPdiPAV_X_X_PALM_N		((uint16_t) 148)
#define kPdiPAV_X_X_IRMC_N		((uint16_t) 150)
#define kPdiPAV_FBTYPE_FREE		((uint16_t) 152)
#define kPdiPAV_PARTSTAT_DECLINED		((uint16_t) 154)
#define kPdiPAV_STATUS_DECLINED		((uint16_t) 156)
#define kPdiPAV_PARTSTAT_TENTATIVE		((uint16_t) 158)
#define kPdiPAV_STATUS_TENTATIVE		((uint16_t) 160)
#define kPdiPAV_PARTSTAT_DELEGATED		((uint16_t) 162)
#define kPdiPAV_STATUS_DELEGATED		((uint16_t) 164)
#define kPdiPAV_RELTYPE_CHILD		((uint16_t) 166)
#define kPdiPAV_ROLE_CHAIR		((uint16_t) 168)
#define kPdiPAV_X_X_PALM_ORG		((uint16_t) 170)
#define kPdiPAV_X_X_IRMC_ORG		((uint16_t) 172)
#define kPdiPAV_X_X_PALM_MAIN		((uint16_t) 174)

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

#endif /* __PDICONST_H__ */
