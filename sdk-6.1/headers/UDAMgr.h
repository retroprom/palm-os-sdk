/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UDAMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *              Unified Data Manager header file
 *  			Define type and generic macro to access data
 *
 *****************************************************************************/

#ifndef __UDAMGR_H__
#define __UDAMGR_H__

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <ExgMgr.h>
#include <FileStream.h>
#include <stdarg.h>

 /***********************************************************************
 * Generic options flags
 ************************************************************************/

#define kUDAEndOfReader 		((uint16_t) 1)
#define kUDAMoreData    		((uint16_t) 2)


 /***********************************************************************
 * Generic control 
 ************************************************************************/

#define	kUDAReinitialize		((uint16_t) 1)

 /***********************************************************************
 * Generic error codes
 ************************************************************************/

#define udaErrControl			((status_t) udaErrorClass | 1)

 /***********************************************************************
 * General types
 ************************************************************************/

typedef uint16_t UDABufferSize;

#define kUDAZeroTerminatedBuffer	0xFFFF

struct UDAObjectTag;
struct UDAReaderTag;
struct UDAFilterTag;
struct UDAWriterTag;

 /***********************************************************************
 * Types of callback functions
 ************************************************************************/

typedef void (*UDADeleteFunction) (struct UDAObjectTag** ioObject);
typedef status_t (*UDAControlFunction) (struct UDAObjectTag* ioObject, uint16_t parameter, va_list args);

typedef UDABufferSize (*UDAReadFunction) (struct UDAReaderTag* ioReader, uint8_t* buffer, UDABufferSize bufferSize, status_t* error);

typedef status_t (*UDAWriteFunction)(struct UDAWriterTag* ioWriter);
typedef status_t (*UDAFlushFunction)(struct UDAWriterTag* ioWriter);

typedef struct UDAObjectTag {
	uint16_t			 	optionFlags;
	uint16_t				padding;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
} UDAObjectType;

typedef struct UDAReaderTag {
	// The Reader is a base object
	uint16_t			 	optionFlags;
	uint16_t				padding;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// Specific Reader fields
	UDAReadFunction  	readF;
} UDAReaderType;

typedef struct UDAFilterTag {
	// The Filter is a base Object
	uint16_t			 	optionFlags;
	uint16_t				padding;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// The Filter is a Reader
	UDAReadFunction  	readF;
	
	// Specific Filter fields
	UDAReaderType* 		upperReader;
} UDAFilterType;

typedef struct UDAWriterTag {
	// The Writer is a base Object
	uint16_t			 	optionFlags;
	uint16_t				padding;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// Specific Writer fields
	UDAWriteFunction 	initiateWriteF;
	UDAFlushFunction 	flushF;
	UDAReaderType* 		upperReader;
} UDAWriterType;

 /***********************************************************************
 * Generic macro to access generic functions
 ************************************************************************/

#define UDADelete(ioObject) \
	((*(ioObject->deleteF))((UDAObjectType**)(&(ioObject))))

#define UDARead(ioReader, bufferToFillP, bufferSizeInBytes, error) \
	((*(ioReader->readF))((UDAReaderType*)(ioReader), (bufferToFillP), (bufferSizeInBytes), (error)))
	
#define UDAEndOfReader(ioReader) \
	(((ioReader)->optionFlags & kUDAEndOfReader) != 0)

#define UDAMoreData(ioReader) \
	(((ioReader)->optionFlags & kUDAMoreData) != 0)
	
#define UDAFilterJoin(ioFilter, ioReader) \
	(((UDAFilterType*)(ioFilter))->upperReader = ioReader)

#define UDAWriterJoin(ioWriter, ioReader) \
	(ioWriter->upperReader = ioReader)

#define UDAInitiateWrite(ioWriter) \
	((*(ioWriter)->initiateWriteF))(ioWriter)

#define UDAWriterFlush(ioWriter) \
	((*(ioWriter)->flushF))(ioWriter)
	

/*****************************************************************
 * UDA API
 ****************************************************************/

// UDAMgr function prototypes

#ifdef __cplusplus
extern "C" {
#endif

extern status_t UDAControl(UDAObjectType* ioObject, uint16_t parameter, ...);
	
extern UDAReaderType* UDAExchangeReaderNew(ExgSocketType* socket);

extern UDAWriterType* UDAExchangeWriterNew(ExgSocketType* socket, UDABufferSize bufferSize);

/***********************************************************************
 * Memory reader
 ************************************************************************/

extern UDAReaderType* UDAMemoryReaderNew(const uint8_t* bufferP, UDABufferSize bufferSizeInBytes);

// *******************************************************************************
//	The new Reader and Writer working with Memory Handles
// *******************************************************************************
UDAReaderType* UDAMemHandleReaderNew(MemHandle inMemHandle);
UDAWriterType* UDAMemHandleWriterNew(MemHandle outMemHandle, uint32_t memHandleResizeInc);

// *******************************************************************************
//	The new Reader and Writer working with file streams
// *******************************************************************************
UDAReaderType* UDAFileStreamReaderNew(FileHand fileHandle);
UDAWriterType* UDAFileStreamWriterNew(FileHand fileHandle, UDABufferSize bufferSizeInBytes);

#ifdef __cplusplus
}
#endif

#endif  // __UDAMGR_H__
