/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtAudioGateway.c
 *
 * Release: 
 *
 * Description:
 *	Bluetooth Audio gateway Profiles (HSP HFP) - Audio Gateway role sample
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <string.h>
#include <TraceMgr.h>
#include <BtLib.h>

#include "BtAudioGateway.h"
#include "BtAudioGatewayPrv.h"
#include "BtAudioGatewayRsc.h"

/***********************************************************************
 *
 *	Entry Points
 *
 ***********************************************************************/

/***********************************************************************
 *
 *	Defines
 *
 ***********************************************************************/

#define TRACE_MODULE TraceDefine(appErrorClass, 2)

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

/***********************************************************************
 *
 *  Global variables
 *
 ***********************************************************************/

/***********************************************************************
 *
 *  Internal Constants
 *
 ***********************************************************************/

/***********************************************************************
 *
 *  Internal Functions
 *
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:   	PrvAGInitSDPRecord
 *
 * DESCRIPTION:	Prepare an SDP server record
 *
 * PARAMETERS:	fd - file descriptor on listener socket
 *				sdpHandle - SDP record handle
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGInitSDPRecord(int32_t fd, BtLibSdpRecordHandle sdpHandle)
{
	int32_t tmpFdME = -1;
	BtLibSdpAttributeDataType *sdpDescriptorP;
	status_t error = btLibErrNoError;

	TraceOutput(TL(TRACE_MODULE, "HS service : PrvAGAdvertiseSDP: Init Headset Profile SDP record"));

	// We need an ME device instance just to create and advertise the service record.
	// It must not be marked as "killable" so as not to cause havoc when it's closed.
	tmpFdME = IOSOpen( btDevMeName, O_RDWR, &error);

	if (HS_SERVER(sdpDescriptorHandle))
		MemHandleUnlock(HS_SERVER(sdpDescriptorHandle));
	else
	{
		HS_SERVER(sdpDescriptorHandle) = MemHandleNew(sizeof(BtLibSdpAttributeDataType));
		if (!HS_SERVER(sdpDescriptorHandle))
		{
			TraceOutput(TL(TRACE_MODULE, "HS service : PrvAGAdvertiseSDP: ERROR Allocate for service description failed"));
			return btLibErrOutOfMemory;
		}
	}
	sdpDescriptorP = MemHandleLock(HS_SERVER(sdpDescriptorHandle));
	BtLibSdpUuidInitialize(sdpDescriptorP->profileDescriptorListEntry.profUUID, btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY, btLibUuidSize16);
	sdpDescriptorP->profileDescriptorListEntry.version = HEADSET_GW_VERSION;

	error = BtLibSdpServiceRecordSetAttributesForSocket(fd, HS_SERVER(sdpUUIDList), 2, HEADSET_GW_SERVICE_NAME, StrLen(HEADSET_GW_SERVICE_NAME), sdpHandle);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "HS service : PrvAGAdvertiseSDP: ERROR Unable to set attributes (1) (0x%04hX)", error));
		goto ServerInit_end;
	}

	// Add profile descriptor attribute
	error = BtLibSdpServiceRecordSetAttribute(tmpFdME, sdpHandle, btLibProfileDescriptorList, sdpDescriptorP, 0, 0);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "HS service : PrvAGAdvertiseSDP: ERROR Unable to set attributes (2) (0x%04hX)", error));
		goto ServerInit_end;
	}

	IOSClose(tmpFdME);

	return error;

ServerInit_end:
	MemHandleUnlock(HS_SERVER(sdpDescriptorHandle));
	MemHandleFree(HS_SERVER(sdpDescriptorHandle));
	HS_SERVER(sdpDescriptorHandle) = NULL;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGDeinitSDPRecord
 *
 * DESCRIPTION:	Clean up an SDP server record
 *
 * PARAMETERS:	
 *				Nothing
 *
 * RETURNED:   	Nothing
 *
 ***********************************************************************/
static void PrvAGDeinitSDPRecord(void)
{
	MemHandleUnlock(HS_SERVER(sdpDescriptorHandle));
	MemHandleFree(HS_SERVER(sdpDescriptorHandle));
	HS_SERVER(sdpDescriptorHandle) = NULL;
}

/***********************************************************************
 *
 *  API
 *
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:    AGInit
 *
 * DESCRIPTION:	Initialize Audio Gateway.
 *
 * PARAMETERS:	
 *				callback - Audio gateway callback.
 *				fd - management entity file descriptor.
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	DmOpenRef dmOpenRef;
	int32_t size;
	MemHandle resH;
	MemPtr resP;
	int32_t fd;
	struct pollfd fdSet[1];
	int32_t oFdCount;
	BtLibServicePreparationParamsType *paramsPrepP;
	BtLibServiceDescriptionType *paramsDescP;
	BtLibServiceExecutionParamsType *paramsExecP;
	status_t error;

	TraceOutput(TL(TRACE_MODULE,"HS service : PilotMain: ENTER cmd=%hu", cmd));

	switch ( cmd )
	{
		case sysBtLaunchCmdPrepareService:
			paramsPrepP = (BtLibServicePreparationParamsType*) cmdPBP;

			BtLibSdpUuidInitialize(HS_SERVER(sdpUUIDList[0]), btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY, btLibUuidSize16);
			BtLibSdpUuidInitialize(HS_SERVER(sdpUUIDList[1]), btLibSdpUUID_SC_GENERIC_AUDIO, btLibUuidSize16);
			
			PrvAGInitSDPRecord(paramsPrepP->fdListener, paramsPrepP->serviceRecH);
			break;

		case sysBtLaunchCmdAbortService:
			PrvAGDeinitSDPRecord();
			break;

		case sysBtLaunchCmdExecuteService:
			paramsExecP = (BtLibServiceExecutionParamsType*) cmdPBP;
			fd = IOSOpen(kHFPipeName, O_RDWR, &error);
			IOSIoctl(fd, I_SENDFD, paramsExecP->fdData, &error);
			fdSet[0].fd = fd;
			fdSet[0].events = (short) (POLLHUP);
			error = IOSPoll(fdSet, 1, -1, &oFdCount);
			break;

		case sysBtLaunchCmdDescribeService:
			paramsDescP = (BtLibServiceDescriptionType*) cmdPBP;

			SysGetModuleDatabase(SysGetRefNum(), NULL, &dmOpenRef);
			// Describe our service for the bluetooth panel services view.
			paramsDescP->flags = 0;
			
			// Get the profile service name str
			resH = DmGetResource(dmOpenRef, (DmResourceType) strRsc, HeadsetServiceInfoNameString);
			resP = MemHandleLock(resH);
			size = strlen(resP) + 1;
			if ((paramsDescP->nameP = MemPtrNew(size)) == NULL)
				return btLibErrOutOfMemory;

			MemMove(paramsDescP->nameP, resP, size);
			(void) MemHandleUnlock(resH);
			(void) DmReleaseResource(resH);
			
			// Get the profile service description str
			resH = DmGetResource(dmOpenRef, (DmResourceType) strRsc, HeadsetServerServiceDescriptionString);
			resP = MemHandleLock(resH);
			size = strlen(resP) + 1;
			if ((paramsDescP->descriptionP = MemPtrNew(size)) == NULL)
				return btLibErrOutOfMemory;

			MemMove(paramsDescP->descriptionP, resP, size);
			(void) MemHandleUnlock(resH);
			(void) DmReleaseResource(resH);
			break;
	}

	return errNone;
}

