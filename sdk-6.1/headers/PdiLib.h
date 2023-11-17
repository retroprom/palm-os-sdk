/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PdiLib.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *              Public API of versit lib
 *
 *****************************************************************************/

#ifndef __PDILIB_H__
#define __PDILIB_H__

#include <PalmTypes.h>
#include <SystemMgr.h>
#include <TextMgr.h>

/*******************************************************************
 *	Unified data access types and macros
 *******************************************************************/

#include <UDAMgr.h>

/*******************************************************************
 *	Pdi library built-in name constants (default dictionary)
 *******************************************************************/

// Constants for vObject Names id, (PR)operties (N)ames id
// for (PA)rameters (N)ames id and (PA)rameters (V)alues id

#include <PdiConst.h>
#include <PdiCustomConst.h>
 
/*******************************************************************
 * Internal library name which can be passed to SysLibFind()
 *******************************************************************/
 
#define		kPdiLibName						"Pdi.lib"	

/*******************************************************************
 * Pdi Library function trap ID's
 *******************************************************************/
 
#define PdiLibTrapReaderNew				(sysLibTrapCustom)
#define PdiLibTrapReaderDelete			(sysLibTrapCustom+1)
#define PdiLibTrapWriterNew				(sysLibTrapCustom+2)
#define PdiLibTrapWriterDelete			(sysLibTrapCustom+3)
#define PdiLibTrapReadProperty			(sysLibTrapCustom+4)
#define PdiLibTrapReadPropertyField		(sysLibTrapCustom+5)
#define PdiLibTrapReadPropertyName		(sysLibTrapCustom+6)
#define PdiLibTrapReadParameter			(sysLibTrapCustom+7)
#define PdiLibTrapDefineResizing		(sysLibTrapCustom+8)
#define PdiLibTrapEnterObject			(sysLibTrapCustom+9)
#define PdiLibTrapWriteBeginObject		(sysLibTrapCustom+10)
#define PdiLibTrapWriteProperty			(sysLibTrapCustom+11)
#define PdiLibTrapWriteParameter		(sysLibTrapCustom+12)
#define PdiLibTrapWritePropertyValue	(sysLibTrapCustom+13)
#define PdiLibTrapWritePropertyFields	(sysLibTrapCustom+14)
#define PdiLibTrapWritePropertyBinaryValue	(sysLibTrapCustom+15)
#define PdiLibTrapSetEncoding			(sysLibTrapCustom+16)
#define PdiLibTrapSetCharset			(sysLibTrapCustom+17)
#define PdiLibTrapWritePropertyStr		(sysLibTrapCustom+18)
#define PdiLibTrapWriteParameterStr		(sysLibTrapCustom+19)
#define PdiLibTrapDefineReaderDictionary	(sysLibTrapCustom+20)
#define PdiLibTrapDefineWriterDictionary	(sysLibTrapCustom+21)

/*******************************************************************
 * Pdi Library result codes
 *******************************************************************/

#define pdiErrRead 							(pdiErrorClass | 1)
#define pdiErrWrite 						(pdiErrorClass | 2)
#define pdiErrNoPropertyName 				(pdiErrorClass | 3)
#define pdiErrNoPropertyValue 				(pdiErrorClass | 4)
#define pdiErrMoreChars 					(pdiErrorClass | 5)
#define pdiErrNoMoreFields 					(pdiErrorClass | 6)
#define pdiErrOpenFailed					(pdiErrorClass | 7)
#define pdiErrCloseFailed					(pdiErrorClass | 8)

/*******************************************************************
 * Pdi library constants
 *******************************************************************/

#define kPdiASCIIEncoding 					0										// consider ascii value 
#define kPdiQPEncoding  					kPdiPAV_ENCODING_QUOTED_PRINTABLE		// value must be QP encoded (write) or is QP encoded (read)
#define kPdiB64Encoding 					kPdiPAV_ENCODING_BASE64					// value must be B64 encoded (write) or is B64 encoded (read)
#define kPdiBEncoding 						kPdiPAV_ENCODING_B						// same as above but ENCODING=B in place of ENCODING=BASE64
#define kPdiEscapeEncoding          		((uint16_t) (0x8000))						// special encoding where newline are backslashed
#define kPdiNoEncoding          			((uint16_t) (0x8001))						// value must not be encoded (write)

// Constants for structured property values
#define kPdiNoFields						((uint16_t) 0)		// Consider property value has just one field
#define kPdiCommaFields						((uint16_t) 1)		// Consider property value can have several fields comma separated
#define kPdiSemicolonFields					((uint16_t) 2)		// Consider property value can have several fields semicolon separated
#define kPdiDefaultFields					((uint16_t) 4)		// Accept default fields definition (dictionary information)
#define kPdiConvertComma					((uint16_t) 8)		// Consider property value has just one field, commas are converted to '\n'
#define kPdiConvertSemicolon				((uint16_t) 16)		// Consider property value has just one field, semicolons are converted to '\n'

// Constants to manage parser/generator behavior

// Generator behavior
#define kPdiEnableFolding					((uint16_t) 1)
#define kPdiEnableQuotedPrintable			((uint16_t) 2)
#define kPdiEscapeMultiFieldValues			((uint16_t) 4) 		// Earlier PalmOS compatiblity
#define kPdiEnableB							((uint16_t) 8) 		// New B encoding type (in place of base64)

// Parser behavior, currently the open parser is OK
// Maybe future evolution will declare new constants
#define kPdiOpenParser						((uint16_t) 16) 		// Generic parser

#define kPdiPalmCompatibility 				(kPdiEscapeMultiFieldValues | kPdiEnableQuotedPrintable | kPdiBypassLocaleCharEncoding)

// parser/generator behavior
#define kPdiBypassLocaleCharEncoding		((uint16_t) 32)		// bypass inbound/outbound default char encoding (determined via locale)

// Constants to manage writting of values
#define kPdiWriteData						((uint16_t) 0)		// No charset computation (non text values)
#define	kPdiWriteText						((uint16_t) 8)		// charset computation
#define kPdiWriteMultiline					((uint16_t) 16)		// if present: must encode else encoding is determinated by charset

// Constant to manage growing buffers
#define kPdiResizableBuffer					((uint16_t) 0xFFFF)	// Special value to indicate a resizable buffer (handle based)
#define kPdiDefaultBufferMaxSize 			((uint16_t) 0x3FFF)	// Maximum size of a resizable buffer non including terminal 0
#define kPdiDefaultBufferDeltaSize 			((uint16_t) 0x0010) 	// Delta (& minimum) size of resizable buffer

// event mask of automata
#define kPdiEOFEventMask 					((uint16_t) 1)
#define kPdiGroupNameEventMask				((uint16_t) 2)		// A group name is found
#define kPdiPropertyNameEventMask			((uint16_t) 4)		// A property name is found
#define kPdiParameterNameEventMask			((uint16_t) 8)		// A parameter name is found
#define kPdiParameterValueEventMask		 	((uint16_t) 16)		// A parameter value is found
#define kPdiPropertyDefinedEventMask		((uint16_t) 32)		// A property definition is found (the ':' separator is reached)
#define kPdiPropertyValueEventMask			((uint16_t) 64)		// An entire property value is found
#define kPdiPropertyValueFieldEventMask	 	((uint16_t) 128)		// A value field is found (';' separated)
#define kPdiPropertyValueItemEventMask		((uint16_t) 256)		// A value item is found (',' separated)
#define kPdiPropertyValueMoreCharsEventMask ((uint16_t) 512)		// The application didn't provide a large enought buffer: more chars must be read
#define kPdiBeginObjectEventMask 		 	((uint16_t) 1024)		// BEGIN reached
#define kPdiEndObjectEventMask 		 	 	((uint16_t) 2048)		// END reached
#define kPdiPropertyValueCRLFEventMask		((uint16_t) 4096)  	// A value item is found (',' separated)

/*******************************************************************
 * Public Data structures.
 *******************************************************************/

typedef uint8_t PdiDictionary;

// Typedef for the old 16-bit status_t
typedef uint16_t Err16;

typedef struct PdiReaderTag {
    Err16				errorLowWord;			// last error
    uint16_t			encoding;				// Type of encoding of the property value
    uint8_t				fieldNum;
    uint8_t             padding1;               // ARM port : padding   
    CharEncodingType	charset;				// Charset of property value
    uint16_t			written;				// Current number of chars already written in buffer
    uint16_t			property;				// ID of the current property
    uint16_t			propertyValueType;		// type of property value
    uint16_t			parameter;				// ID of the last parsed parameter name
    uint32_t			parameterPairs[8];		// set of bits of parsed parameter values
    uint16_t  			customFieldNumber;		// Value of X-PALM-CUSTOM (cutom fields)
    uint16_t			padding2;				// ARM port : padding	
    void*   			appData;				// General usage app dependent field
    uint16_t			pdiRefNum;				// The refNum of the Pdi library - kept for 68K compatibility purpose
	uint16_t			events;					// Mask of events (see kPdiXXXXEventMask constants)
    char*				groupName;
    char*				propertyName;
    char*				parameterName;
    char*				parameterValue;
    char*				propertyValue;
} PdiReaderType;

typedef struct _PdiWriter {
	Err16				errorLowWord;			// last error
	uint16_t			encoding;				// Type of encoding of the property value
	CharEncodingType	charset;				// Charset of property value (16 bits)
    uint8_t				padding1[2];			// ARM port : padding	
	uint32_t			reserved;				// reserved for future use
    void*   			appData;				// General usage app dependent field
	uint16_t			pdiRefNum;				// The refNum of the Pdi library - kept for 68K compatibility purpose
    uint16_t			padding2;				// ARM port : padding	
} PdiWriterType;

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************
 * Library Open & Close functions
 *******************************************************************/

extern status_t PdiLibOpen(void);
				
extern status_t PdiLibClose(void);

/*******************************************************************
 * Reader / Writer initialization & finalization functions
 *******************************************************************/

extern PdiReaderType* PdiReaderNew(UDAReaderType *input, uint16_t version);

extern void PdiReaderDelete(PdiReaderType** ioReader);

extern PdiWriterType* PdiWriterNew(UDAWriterType *output, uint16_t version);

extern void PdiWriterDelete(PdiWriterType** ioWriter);
				

/*******************************************************************
 * Read functions group. 
 *******************************************************************/

extern status_t PdiReadProperty(PdiReaderType* ioReader);

extern status_t PdiReadPropertyField(PdiReaderType* ioReader, char** bufferPP, uint16_t bufferSize, uint16_t readMode);

extern status_t PdiReadPropertyName(PdiReaderType* ioReader);

extern status_t PdiReadParameter(PdiReaderType* ioReader);
				
extern status_t PdiDefineResizing(PdiReaderType* ioReader, uint16_t deltaSize, uint16_t maxSize);

#define PdiParameterPairTest(reader, pair) \
	((reader->parameterPairs[(pair) & 7] & ((uint32_t) (1) << ((uint8_t) (pair) >> 3))) != 0)

/*******************************************************************
 * Recursive objects functions group.
 *******************************************************************/

extern status_t PdiEnterObject(PdiReaderType* ioReader);

/*******************************************************************
 * Write functions group. 
 *******************************************************************/

extern status_t PdiWriteBeginObject(PdiWriterType* ioWriter, uint16_t objectNameID);

#define PdiWriteEndObject PdiWriteBeginObject

extern status_t PdiWriteProperty(PdiWriterType* ioWriter, uint16_t propertyNameID);

extern status_t PdiWriteParameter(PdiWriterType* ioWriter, uint16_t parameter, Boolean parameterName);


extern status_t PdiWritePropertyValue(PdiWriterType* ioWriter, char* buffer, uint16_t options);
				
extern status_t PdiWritePropertyFields(PdiWriterType* ioWriter, char* fields[], uint16_t fieldNumber, uint16_t options);
				
extern status_t PdiWritePropertyBinaryValue(PdiWriterType* ioWriter, const char* buffer, uint16_t size, uint16_t options);

extern status_t PdiSetEncoding(PdiWriterType* ioWriter, uint16_t encoding);

extern status_t PdiSetCharset(PdiWriterType* ioWriter, CharEncodingType charset);

extern status_t PdiWritePropertyStr(PdiWriterType* ioWriter, const char* propertyName, uint8_t writeMode, uint8_t requiredFields);

extern status_t PdiWriteParameterStr(PdiWriterType* ioWriter , const char* parameterName, const char* parameterValue);

/*******************************************************************
 * Customisation functions group
 *******************************************************************/
 
extern PdiDictionary* PdiDefineReaderDictionary(PdiReaderType* ioReader, PdiDictionary* dictionary, Boolean disableMainDictionary);

extern PdiDictionary* PdiDefineWriterDictionary(PdiWriterType* ioWriter, PdiDictionary* dictionary, Boolean disableMainDictionary);
				
#ifdef __cplusplus 
}
#endif


#endif // __PDILIB_H__
