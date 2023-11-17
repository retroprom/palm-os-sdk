/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtEchoService.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              The bluetooth echo service.
 *
 *		This sample application implements a simple bluetooth persistent remote
 *		service. All data that the remote client sends to this service is simply
 *		sent back to the client.
 *
 *****************************************************************************/

#include <PalmTypes.h>

#include <CmnLaunchCodes.h>
#include <DataMgr.h>
#include <MemoryMgr.h>
#include <ErrorMgr.h>
#include <IOS.h>
#include <Loader.h>
#include <stdlib.h>
#include <string.h>
#include <SystemResources.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include <BtLib.h>

/*-----------+
 | Resources |
 +-----------*/

//	Resource: tSTR 20010
#define EchoServiceInfoNameString			20010

//	Resource: tSTR 20020
#define EchoServerServiceDescriptionString	20020

/*---------------------------------------+
 | Private global constants, types, data |
 +---------------------------------------*/

#define TRACE_MODULE	TraceDefine(blthErrorClass, 5)

// Buffer size for reading from and writing to the data stream.
//
#define BUFSIZE 1000

// A name for our service.
//
static const char* gServiceName = "PalmOS Echo Service";

// A service class UUID for our service.
// This is the same one that's used in BtLibTest.
//
static BtLibSdpUuidType gServiceClassUUID = {
	btLibUuidSize128,
	{0x7D,0x97,0xB3,0x61,0x62,0x21,0x44,0x59,0x92,0x41,0x22,0x75,0x76,0xCC,0x6A,0xB8}
};

static DmOpenRef 			gBtEchoDmOpenRef;

/*-------------------+
 | Private functions |
 +-------------------*/

//------------------------------------------------------------------------------
// Get the type and creator of this application's resource database.
//
static status_t GetMyTypeAndCreator( 
	uint32_t*			typeP,
	uint32_t*			creatorP
) {
	DatabaseID			dbID;
	DmDatabaseInfoType	dbInfo;
	status_t			err;

	MemSet( &dbInfo, sizeof(dbInfo), 0 );
	dbInfo.pType    = typeP;
	dbInfo.pCreator = creatorP;
	if ( (err = SysGetModuleDatabase( SysGetRefNum(), &dbID, &gBtEchoDmOpenRef )) == 0 ) {
		err = DmDatabaseInfo( dbID, &dbInfo );
	}
	return err;
}

/*---------------------------------+
 | Bluetooth service "entrypoints" |
 +---------------------------------*/

//------------------------------------------------------------------------------
// Register ourselves as a Bluetooth service.
// This need be done only once after the system boots; redundant registrations
// are ignored. After registering a service, the Bluetooth system must be
// (re)started in order for that service to actually start functioning.
//
// In this example, we use the RfComm protocol with the serial interface module
// pushed onto that, and we will execute the service in the System Process.
//
static void RegisterService( void )
{
	BtLibServiceRegistrationParamsType	params;
	status_t							err;

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: RegisterService: ENTER"));

	if ( (err = GetMyTypeAndCreator( &params.appType, &params.appCreator )) == 0 ) {
		params.appCodeRscId	    = sysResIDDefault;
		params.stackSize        = 2000;
		params.protocol         = btLibRfCommProtocol;
		params.pushSerialModule = true;
		params.execAsNormalApp  = false;
		err = BtLibRegisterService( &params );
	}

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: RegisterService: EXIT err=0x%lx", err));
}

//------------------------------------------------------------------------------
// Prepare our service.  Upon entry, we are given
//     params->serviceRecH ... handle on an empty local SDP service record
//     params->fdListener .... file descriptor to a bluetooth listener socket
// Upon exit, the service record must be set up to describe our service as being
// available through the listener socket. The caller of this function will then
// start advertising the service record.
//
static status_t PrepareService(
	BtLibServicePreparationParamsType*	params
) {
	status_t							err;

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: PrepareService: ENTER"));

	err = BtLibSdpServiceRecordSetAttributesForSocket(
		params->fdListener,		// listener socket
		&gServiceClassUUID,		// service class uuid list
		1,						// service class uuid list length
		gServiceName,			// service name
		strlen(gServiceName)+1,	// service name length including null
		params->serviceRecH		// the service record
	);

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: PrepareService: EXIT err=0x%lx", err));
	return err;
}

//------------------------------------------------------------------------------
// Execute our service. The file descriptor params->fdData refers to a connected
// L2CAP or RfComm instance, perhaps with a serial interface module pushed onto
// that. In the case of a raw RfComm instance, the remote transmitter has zero
// flow-control credits.
//
// This function must close params->fdData before exiting.
//
// In this example, we are using RfComm with the serial interface, and our
// service just echos everything it reads.
//
static status_t ExecuteService(
	BtLibServiceExecutionParamsType*	params
) {
	char*								buf;
	int32_t								n;
	status_t							err = 0;

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: ExecuteService: ENTER fd=%d", params->fdData));

	if ( (buf = MemPtrNew(BUFSIZE)) == NULL ) {
		err = sysErrNoFreeRAM;
		goto Exit;
	}

	while ( (n = IOSRead( params->fdData, buf, BUFSIZE, &err )) > 0 ) {
		IOSWrite( params->fdData, buf, n, &err );
	}

	MemPtrFree( buf );

Exit:
	IOSClose( params->fdData );
	TraceOutput(TL(TRACE_MODULE, "BtEchoService: ExecuteService: EXIT err=0x%lx", err));
	return 0;
}

//------------------------------------------------------------------------------
// Abort our service. We must deallocate any extra resources allocated in the
// PrepareService() entry point.
//
// In this example, there are no extra resources to deallocate.
//
static void AbortService( void )
{
	TraceOutput(TL(TRACE_MODULE, "BtEchoService: AbortService: ENTER"));
	TraceOutput(TL(TRACE_MODULE, "BtEchoService: AbortService: EXIT"));
}

//------------------------------------------------------------------------------
// Describe our service for the bluetooth panel services view.
//
static uint32_t DescribeService(
	BtLibServiceDescriptionType*	params
) {
	uint32_t	retVal = errNone;
	uint32_t	type;
	uint32_t	creator;

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: DescribeService: ENTER"));

	retVal = GetMyTypeAndCreator(&type, &creator);	// initialize gBtEchoDmOpenRef too
	if (retVal == errNone)
	{ 
		int 					size;
		MemHandle 	theResHdl;
		MemPtr			theResPtr;
		
		// Describe our service for the bluetooth panel services view.
		params->flags = 0;
//		params->nameP = "Echo";
//		params->descriptionP = "A sample service for testing that just echoes everything it reads";

		// Get the profile service name str
		theResHdl = DmGetResource(gBtEchoDmOpenRef, (DmResourceType) strRsc, EchoServiceInfoNameString);
		theResPtr = MemHandleLock(theResHdl);
		size = strlen( theResPtr ) + 1;
		if ( (params->nameP = MemPtrNew( size )) == NULL ) {
			return btLibErrOutOfMemory;
		}
		MemMove( params->nameP, theResPtr, size );
		(void) MemHandleUnlock(theResHdl);
		(void) DmReleaseResource(theResHdl);
		
		// Get the profile service description str
		theResHdl = DmGetResource(gBtEchoDmOpenRef, (DmResourceType) strRsc, EchoServerServiceDescriptionString);
		theResPtr = MemHandleLock(theResHdl);
		size = strlen( theResPtr ) + 1;
		if ( (params->descriptionP = MemPtrNew( size )) == NULL ) {
			return btLibErrOutOfMemory;
		}
		MemMove( params->descriptionP, theResPtr, size );
		(void) MemHandleUnlock(theResHdl);
		(void) DmReleaseResource(theResHdl);
	}

	TraceOutput(TL(TRACE_MODULE, "BtEchoService: DescribeService: EXIT"));
	return retVal;
}

//------------------------------------------------------------------------------
// Perform our service's custom UI for the bluetooth panel service view.
//
// In this example, we have no custom UI to perform.
//
static void DoServiceUI( void )
{
}

/*------------------+
 | Main Entrypoint  |
 +------------------*/

uint32_t PilotMain(
	uint16_t	cmd,
	MemPtr		cmdPBP,
	uint16_t	launchFlags
) {
	uint32_t	retVal = 0;

	switch ( cmd )
	{
	case sysLaunchCmdBoot:
	case sysAppLaunchCmdSystemReset:
		// Register ourselves as a persistent Bluetooth service.
		RegisterService();
		break;

	case sysBtLaunchCmdPrepareService:
		// Create service record(s) describing our service to remote devices.
		retVal = (uint32_t) PrepareService( (BtLibServicePreparationParamsType*) cmdPBP );
		break;

	case sysBtLaunchCmdExecuteService:
		// An inbound connection to our service has arrived.
		ExecuteService( (BtLibServiceExecutionParamsType*)cmdPBP );
		break;

	case sysBtLaunchCmdAbortService:
		// The service is being aborted. Clean up any extra resources allocated in PrepareService().
		AbortService();
		break;

	case sysBtLaunchCmdDescribeService:
		// Describe our service for the bluetooth panel services view.
		DescribeService( (BtLibServiceDescriptionType*) cmdPBP );
		break;

	case sysBtLaunchCmdDoServiceUI:
		// Perform custom UI for the bluetooth panel services view.
		DoServiceUI();
		break;

	case sysAppLaunchCmdNormalLaunch:
		break;
	}

	return retVal;
}
