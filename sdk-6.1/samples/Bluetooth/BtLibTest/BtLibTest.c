/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtLibTest.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              Bluetooth library test tool
 *
 *****************************************************************************/

#include <PalmTypes.h>
#include <AboutBox.h>
#include <Chars.h>
#include <Font.h>
#include <CmnLaunchCodes.h>
#include <Control.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include <Event.h>
#include <FeatureMgr.h>
#include <Form.h>
#include <FormLayout.h>
#include <IOS.h>
#include <Loader.h>
#include <MemoryMgr.h>
#include <Menu.h>
#include <PollBox.h>
#include <ScrollBar.h>
#include <StdIO.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <Table.h>
#include <TraceMgr.h>
#include <string.h>
#include <BtLib.h>

#include "BtLibTestRsc.h"

#define UI_EVENTS_ON_FD	1 // can we poll a file descriptor for UI event?


/***********************************************************************
 *   Constants
 ***********************************************************************/

// {99ABEE90-122D-4242-9E8E-F47F7C5365E3}
static BtLibSdpUuidType kL2CapUUID = {
	btLibUuidSize128,
	{0x99,0xAB,0xEE,0x90,0x12,0x2D,0x42,0x42,0x9E,0x8E,0xF4,0x7F,0x7c,0x53,0x65,0xE3}
};

// {7D97B361-6221-4459-9241-227576CC6AB8}
static BtLibSdpUuidType kRfCommUUID = {
	btLibUuidSize128,
	{0x7D,0x97,0xB3,0x61,0x62,0x21,0x44,0x59,0x92,0x41,0x22,0x75,0x76,0xCC,0x6A,0xB8}
};


#define kAppFileCreator		'btlt'
#define kAppBtName			"BtLibTest"

#define kMaxSizeLog			4096
#define kBdAddrStringSize	18
#define kMyMtu				100 // Must be less then 672

#define kInvalidSdpRecordH	0

#define UNUSED_ARG(x) (x)=(x) // To avoid compiler warning about unused arguments


/***********************************************************************
 *   Global variables
 ***********************************************************************/

// File descriptors.
//
static int32_t	gFdUI			= -1;	// fd for UI events
static int32_t	gFdME			= -1;	// fd to Management Entity device
static int32_t	gFdSdp			= -1;	// fd to SDP device
static int32_t	gFdL2cListener	= -1;   // fd to L2CAP device, for listening
static int32_t	gFdL2cData		= -1;	// fd to L2CAP device, for data
static int32_t	gFdRfcListener	= -1;   // fd to RfComm device, for listening
static int32_t	gFdRfcData		= -1;	// fd to RfComm device, for data
static int32_t	gFdSCOListener	= -1;	// fd to SCO link, for listening
static int32_t	gFdSCOData		= -1;	// fd to SCO link, for data
static int32_t	gFdBNEPListener	= -1;	// fd to BNEP device, for listening
static int32_t	gFdBNEPData		= -1;	// fd to BNEP device, for data

// A PollBox is a collection of file descriptors, for polling.
static PollBox*					gPollBox;

// The local ID of this app's resource data base.
static DmOpenRef				gResourceDbRef;

// Remote bluetooth device's address.
static BtLibDeviceAddressType	gBdAddr;
static char						gBdAddrStr[kBdAddrStringSize];

// Buffer for L2CAP or RfComm data. 
static char 					gDataStr[kMyMtu];

// SDP service record handles.
static BtLibSdpRecordHandle		gL2cSdpRecordH = kInvalidSdpRecordH;
static BtLibSdpRecordHandle		gRfcSdpRecordH = kInvalidSdpRecordH;

static Boolean					gCanDraw = true;
static Boolean					gStopFlag = false;

// UI elements
static RectangleType			gCurrentWinBounds;	// current window bounds.
//static int16_t				sFieldTapY = -1;

// For saving and restoring the radio accessibility state.
static Boolean					gHaveOriginalAccessibility = false;
static BtLibAccessibleModeEnum	gOriginalAccessibility;

/***********************************************************************
 *   Internal Functions
 ***********************************************************************/

static void 	HandlePbxUIEvent(PollBox* pbx, struct pollfd* pollFd, void* context);
static void 	HandlePbxMEEvent(PollBox* pbx, struct pollfd* pollFd, void* context);
static void 	HandlePbxSocketEvent(PollBox* pbx, struct pollfd* pollFd, void* context);
static void 	ChangeLabel(uint16_t labelID, const char *newLabel);
static void 	UpdateScrollbar(void);
static void 	ScrollLines(uint16_t numLinesToScroll, WinDirectionType direction, Boolean update);
static void 	LogPrintF(const char *format, ...);


//-----------------------------------------
// Return a printable name for the given protocol.
//
static const char* ProtocolName( BtLibProtocolEnum protocol )
{
	switch ( protocol )
	{
	case btLibL2CapProtocol:	return "L2CAP";
	case btLibRfCommProtocol:	return "RfComm";
	case btLibSdpProtocol:		return "SDP";
	case btLibSCOProtocol:		return "SCO";
	case btLibBNEPProtocol:		return "BNEP";
	default:
		ErrFatalError( "unknown protocol" );
		return "Unknown";
	}
}

//-----------------------------------------
// Create a socket of the given protocol, and put it into the pollbox.
// Returns true iff socket already exists, or successfully created.
//
static Boolean CreateSocket(
	int32_t*			fdPtr,		// pointer to global fd variable
	BtLibProtocolEnum	protocol	// which protocol
) {
	status_t			error;

	if ( *fdPtr >= 0 ) {
		LogPrintF( "%s socket already open", ProtocolName(protocol) );
		return true;
	}

	error = BtLibSocketCreate( fdPtr, protocol );
	if ( error ) {
		LogPrintF( "%s socket creation error: 0x%lx", ProtocolName(protocol), error );
		ErrFatalErrorIf( *fdPtr != -1, "fd should be -1 on error" );
		return false;
	}

	PbxAddFd( gPollBox, *fdPtr, POLLIN, HandlePbxSocketEvent, fdPtr );
	return true;
}

//-----------------------------------------
// Close the given socket, and remove it from the pollbox.
//
static void CloseSocket(
	int32_t*			fdPtr	// pointer to global fd variable
) {
	status_t			error;

	if ( *fdPtr >= 0 ) {
		LogPrintF( "Closing socket %d", *fdPtr );
		PbxRemoveFd( gPollBox, *fdPtr );
		if ( (error = BtLibSocketClose( *fdPtr )) != 0 ) {
			LogPrintF( "Socket close ERROR 0x%lx", error );
		}
		*fdPtr = -1;
	}
}

//-----------------------------------------
// Create a socket of the given protocol, and put it into the pollbox.
// Then make it into a listener socket, create a service record, and 
// advertise that service record.
//
static void CreateListenerSocketAndAdvertiseService(
	BtLibProtocolEnum			protocol			// which protocol
) {
	int32_t*					fdPtr;
	BtLibSdpRecordHandle*		sdpRecordHandlePtr;
	BtLibSdpUuidType*			uuidPtr;
	BtLibSocketListenInfoType	listenInfo;
	BtLibSocketListenInfoType*	listenInfoPtr = &listenInfo;
	const char*					protocolName = ProtocolName( protocol );
	status_t					error;

	// Set up parameters according to the protocol.
	switch ( protocol )
	{
	case btLibL2CapProtocol:
		fdPtr = &gFdL2cListener;
		sdpRecordHandlePtr = &gL2cSdpRecordH;
		uuidPtr = &kL2CapUUID;
		listenInfo.data.L2Cap.localPsm     = BT_L2CAP_RANDOM_PSM;
		listenInfo.data.L2Cap.localMtu     = kMyMtu;
		listenInfo.data.L2Cap.minRemoteMtu = kMyMtu;
		break;

	case btLibRfCommProtocol:
		fdPtr = &gFdRfcListener;
		sdpRecordHandlePtr = &gRfcSdpRecordH;
		uuidPtr = &kRfCommUUID;
		listenInfo.data.RfComm.maxFrameSize   = kMyMtu;
		listenInfo.data.RfComm.advancedCredit = 0;
		break;

	case btLibSCOProtocol:
		fdPtr = &gFdSCOListener;
		sdpRecordHandlePtr = NULL;
		uuidPtr = NULL;
		listenInfoPtr = NULL;
		break;

	case btLibBNEPProtocol:
		fdPtr = &gFdBNEPListener;
		sdpRecordHandlePtr = NULL;
		uuidPtr = NULL;
		listenInfo.data.BNEP.listenGN = true;
		listenInfo.data.BNEP.listenPANU = true;
		listenInfo.data.BNEP.listenNAP = false;
		break;

	default:
		ErrFatalError( "bad protocol" );
		return;
	}

	// Create the socket.
	if ( ! CreateSocket( fdPtr, protocol ) ) {
		return;
	}

	// Make the socket a listener.
	error = BtLibSocketListen( *fdPtr, listenInfoPtr );
	if ( error ) {
		LogPrintF( "%s Listen Error: 0x%lx", protocolName, error );
		CloseSocket( fdPtr );
		return;
	}

	// Display listening info.
	switch ( protocol )
	{
	case btLibL2CapProtocol:
		{
			BtLibL2CapPsmType psm;

			BtLibSocketGetInfo( *fdPtr, btLibSocketInfo_L2CapPsm, &psm, sizeof(BtLibL2CapPsmType) );
			LogPrintF( "L2CAP Socket %d: listening on psm 0x%hx...", *fdPtr, psm );
		}
		break;

	case btLibRfCommProtocol:
		{
			BtLibRfCommServerIdType serviceID;

			BtLibSocketGetInfo( *fdPtr, btLibSocketInfo_RfCommServerId, &serviceID, sizeof(serviceID) );
			LogPrintF( "RfComm socket %d: listening on channel %d...", *fdPtr, serviceID );
		}
		break;

	case btLibSCOProtocol:
		LogPrintF( "SCO socket %d: listening...", *fdPtr );
		break;

	case btLibBNEPProtocol:
		LogPrintF( "BNEP socket %d: listening...", *fdPtr );
		break;
	}

	// Set up the service record if the protocol requires it.
	switch ( protocol )
	{
	case btLibL2CapProtocol:
	case btLibRfCommProtocol:

		// Create an empty local service record.
		ErrFatalErrorIf( *sdpRecordHandlePtr != kInvalidSdpRecordH, "service record already exists" );
		error = BtLibSdpServiceRecordCreate( gFdME, sdpRecordHandlePtr );
		if ( error ) {
			LogPrintF( "%s ServiceRecordCreate Error: 0x%lx", protocolName, error );
			return;
		}

		// Fill in the service record.
		error = BtLibSdpServiceRecordSetAttributesForSocket( *fdPtr, uuidPtr, 1, kAppBtName, (uint16_t)strlen(kAppBtName), *sdpRecordHandlePtr );
		if ( error ) {
			LogPrintF( "%s SetAttributes Error: 0x%lx", protocolName, error );
			return;
		}

		// Advertise it.
		error = BtLibSdpServiceRecordStartAdvertising( gFdME, *sdpRecordHandlePtr );
		if ( error ) {
			LogPrintF( "%s Advertise Error: 0x%lx", protocolName, error );
			return;
		}

		LogPrintF( "%s service advertised via SDP", protocolName );
	}
}

//-----------------------------------------
// Stop advertising and destroy the given local service record,
// and close the associated listener socket.
//
static void StopAdvertisingAndCloseListenerSocket(
	int32_t*				fdPtr,		// pointer to global listener fd variable
	BtLibSdpRecordHandle*	sdpRecHP	// pointer to service record handle
) {
	status_t				error;

	ErrFatalErrorIf(
		!(
			( fdPtr == &gFdL2cListener  && sdpRecHP == &gL2cSdpRecordH ) ||
			( fdPtr == &gFdRfcListener  && sdpRecHP == &gRfcSdpRecordH ) ||
			( fdPtr == &gFdSCOListener  && sdpRecHP == NULL ) ||
			( fdPtr == &gFdBNEPListener && sdpRecHP == NULL )
		),
		"bad args"
	);
		
	if ( sdpRecHP != NULL )
	{
		// Stop advertising the service.
		if ( (error = BtLibSdpServiceRecordStopAdvertising( gFdME, *sdpRecHP )) != 0 ) {
			LogPrintF( "ServiceRecordStopAdvertising ERROR 0x%lx", error );
		}

		// Destroy the local service record.
		if ( (error = BtLibSdpServiceRecordDestroy( gFdME, *sdpRecHP )) != 0 ) {
			LogPrintF( "SeviceRecordDestroy  ERROR 0x%lx", error );
		}
		*sdpRecHP = kInvalidSdpRecordH;	
	}

	// Close the listener socket and remove it from the pollbox.
	CloseSocket( fdPtr );
}

//-----------------------------------------
// Restore the original radio accessibility state.
//
static void RestoreOriginalAccessibility( void )
{
	status_t	error;

	if ( gFdME >= 0 && gHaveOriginalAccessibility )
	{
		error = BtLibSetGeneralPreference(gFdME, btLibPref_UnconnectedAccessible, &gOriginalAccessibility, sizeof(gOriginalAccessibility));
		if ( error )
		{
			LogPrintF("Restoring accessibliity failed: 0x%lx", error);
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the main form.
 *
 * PARAMETERS:  formptr of active window
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void MainFormInit(FormPtr frmP)
{
	// Draw the form
//	FrmDrawForm( frmP );
	ChangeLabel( MainAddrLabel, "No Connection" );
	MemSet( &gBdAddr, sizeof(gBdAddr), 0 );
	// keep the window bounding rect.
	FrmGetFormInitialBounds(frmP, &gCurrentWinBounds);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static Boolean MainFormDoCommand(
	uint16_t	command
) {
	Boolean		handled = false;
	status_t	error   = 0;

	switch ( command )
	{
		/******************/
		/* Main Menu      */
		/******************/
		case MainLibOpen:
			if ( gFdME >= 0 )
			{
				LogPrintF("BtLib already opened");
			}
			else
			{
				error = BtLibOpen( &gFdME );
				if ( error )
				{
					LogPrintF("BtLib Opened Failed: 0x%lx", error);
					ErrFatalErrorIf( gFdME >= 0, "nonnegative fd on error?" );
				}
				else
				{
					PbxAddFd( gPollBox, gFdME, POLLIN, HandlePbxMEEvent, 0 );
					LogPrintF("BtLib Opened");
				}
			}
			break;

		case MainLibClose:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				// First, restore the original radio accessibility state.
				RestoreOriginalAccessibility();

				// Now remove the ME fd from the pollbox, and close it.
				PbxRemoveFd( gPollBox, gFdME );
				error = BtLibClose( gFdME );
				if ( error )
				{
					LogPrintF("BtLib Close Failed: 0x%lx", error);
				}
				else
				{
					LogPrintF("BtLib Closed");
				}
				gFdME = -1;
			}
			break;

		case MainLibAbout:
			MenuEraseStatus(0);
			AbtShowAbout(kAppFileCreator);
			handled = true;
			break;
			
		/******************/
		/* ME Menu        */
		/******************/

		case MainMEInquiry:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				MenuEraseStatus(0);	// Clear the command bar if any	
				error = BtLibStartInquiry( gFdME, 10, 20 );
				if ( error != btLibErrPending )
				{
					LogPrintF("Start Inq Error: 0x%lx", error);
				}
				else
				{
					LogPrintF("Started Inquiry");
				}
			}
			break;

		case MainMECancelInquiry:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				error = BtLibCancelInquiry( gFdME );
				if ( error != btLibErrPending )
				{
					LogPrintF("Cancel Inq Error: 0x%lx", error);
				}
				else
				{
					LogPrintF("Canceling Inquiry...");
				}
			}
			break;

		case MainMEGetRole:
			if( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				BtLibConnectionRoleEnum role;

				error = BtLibLinkGetState( gFdME, &gBdAddr, btLibLinkPref_LinkRole, &role, sizeof(role) );
				if( error )
				{
					LogPrintF("Get Role Error: 0x%lx", error);
				}
				else
				{
					LogPrintF(role == btLibMasterRole ?  "Master" : "Slave");
				}
			}
			break;

		case MainMEConnectACL:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				uint8_t numDevicesSelected;
				error = BtLibDiscoverDevices(gFdME, NULL, NULL, false, NULL, 0, false, &gBdAddr, 1, &numDevicesSelected);
				if ( error == btLibErrCanceled )
				{
					LogPrintF("Discovery Canceled");
					break;
				}
				BtLibAddrBtdToA(&gBdAddr, gBdAddrStr, kBdAddrStringSize);
				LogPrintF("Discovery Selected %s", gBdAddrStr);
				error = BtLibLinkConnect( gFdME, &gBdAddr );
				if ( error != btLibErrPending )
				{
					LogPrintF("Link Create Error: 0x%lx", error);
				}
				else
				{
					BtLibAddrBtdToA(&gBdAddr, gBdAddrStr, kBdAddrStringSize);
					LogPrintF("Creating Link to %s...", gBdAddrStr);
				}
			}
			break;

		case MainMEGetDeviceName:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				uint8_t numDevicesSelected;
				error = BtLibDiscoverDevices(gFdME, NULL, NULL, true, NULL, 0, false, &gBdAddr, 1, &numDevicesSelected);
				if ( error == btLibErrCanceled )
				{
					LogPrintF( "Discovery Canceled" );
				}
				else
				{
					BtLibAddrBtdToA( &gBdAddr, gBdAddrStr, kBdAddrStringSize );
					LogPrintF( "Discovery Selected %s", gBdAddrStr );
					error = BtLibGetRemoteDeviceName( gFdME, &gBdAddr, btLibCachedThenRemote );
					if ( error == btLibErrPending )
					{
						BtLibAddrBtdToA( &gBdAddr, gBdAddrStr, kBdAddrStringSize );
						LogPrintF( "Getting name of %s...", gBdAddrStr );
					}
					else
					{
						LogPrintF( "Get Name Error: 0x%lx", error );
					}
				}
			}
			break;

		case MainMEDisconnectACL:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				error = BtLibLinkDisconnect( gFdME, &gBdAddr );
				if ( error != btLibErrPending && error != btLibErrNoError )
				{
					LogPrintF( "Link Disc Error: 0x%lx", error );
				}
				else
				{
					BtLibAddrBtdToA( &gBdAddr, gBdAddrStr, kBdAddrStringSize );
					LogPrintF( "Killing Link to %s...", gBdAddrStr );
				}
			}
			break;

		case MainMEAuthenticateLink:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				error = BtLibLinkSetState( gFdME, &gBdAddr, btLibLinkPref_Authenticated, NULL, 0 );
				if ( error != btLibErrPending && error != btLibErrNoError )
				{
					LogPrintF( "Auth. Error: 0x%lx", error );
				}
				else
				{
					BtLibAddrBtdToA( &gBdAddr, gBdAddrStr, kBdAddrStringSize );
					LogPrintF( "Auth. Link to %s...", gBdAddrStr );
				}
			}
			break;

		case MainMEToggleEncryption:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				Boolean encryption;

				error = BtLibLinkGetState( gFdME, &gBdAddr, btLibLinkPref_Encrypted, &encryption, sizeof(Boolean) );
				if ( error != btLibErrNoError )
				{
					LogPrintF("Auth. Error: 0x%lx", error);
				}
				else
				{
					encryption = !encryption;
					error = BtLibLinkSetState( gFdME, &gBdAddr, btLibLinkPref_Encrypted, &encryption, sizeof(Boolean) );
					if ( error != btLibErrPending && error != btLibErrNoError )
					{
						LogPrintF("Encryption Error: 0x%lx", error);
					}
					else
					{
						BtLibAddrBtdToA(&gBdAddr, gBdAddrStr, kBdAddrStringSize);
						LogPrintF( "%s link to %s...", encryption ? "Encrypt" : "Unencrypt", gBdAddrStr );
					}
				}
			}
			break;

		case MainMEDiscoverMulti:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				#define howManyDevice 2
				uint8_t numDevicesSelected = 0;
				BtLibDeviceAddressType selectedDeviceArray[howManyDevice];

				error = BtLibDiscoverDevices(gFdME, NULL, NULL, false, NULL, 0, false, selectedDeviceArray, howManyDevice, &numDevicesSelected);
				if ( error == btLibErrCanceled )
				{
					LogPrintF( "Discovery Canceled" );
				}
				else if ( numDevicesSelected == 0 )
				{
					LogPrintF( "Discover Multi: None Selected" );
				}
				else
				{
					uint8_t					i;

					LogPrintF( "Discover Multi: %hu selected", numDevicesSelected );
					for( i = 0; i < numDevicesSelected; i++ )
					{
						BtLibAddrBtdToA( &selectedDeviceArray[i], gBdAddrStr, kBdAddrStringSize );
						LogPrintF( gBdAddrStr );
					}
				}
			}
			break;

		case MainMEAccessibility:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				FormType*				frmP;
				BtLibAccessibleModeEnum	access;
				uint16_t				objIndex = 0;

				// Get the current accessibility state
				error = BtLibGetGeneralPreference( gFdME, btLibPref_UnconnectedAccessible, &access, sizeof(access) );
				if ( error != btLibErrNoError )
				{
					LogPrintF( "Get Accessibility ERROR 0x%lx", error );
				}
				else
				{
					// Save the accessibility state so we can restore it later.
					if ( !gHaveOriginalAccessibility )
					{
						gHaveOriginalAccessibility = true;
						gOriginalAccessibility = access;
					}
					
					// Get the input string
					MenuEraseStatus( 0 ); // Clear the command bar if any
					frmP = FrmInitForm( gResourceDbRef, AccessForm );

					// Make sure the Lock/Unlock button matches our current accessible state
					switch ( access )
					{
					case btLibNotAccessible:				objIndex = AccessNotConnectableRadio;			break;
					case btLibConnectableOnly:				objIndex = AccessConnectableRadio;				break;
					case btLibDiscoverableAndConnectable:	objIndex = AccessDiscoverableConnectableRadio;	break;
					default:								ErrFatalError( "out of accessibility switch" );	break;
					}
					CtlSetValue( FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, objIndex ) ), true );

					// Display the input dialog
					if ( FrmDoDialog( frmP ) == AccessCancelButton )
					{
						LogPrintF( "Set Accessibility CANCELED" );
					}
					else
					{
						if ( CtlGetValue( FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, AccessNotConnectableRadio ) ) ) )
						{
							access = btLibNotAccessible;
						}
						else if ( CtlGetValue( FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, AccessConnectableRadio ) ) ) )
						{
							access = btLibConnectableOnly;
						}
						else if ( CtlGetValue( FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, AccessDiscoverableConnectableRadio) ) ) )
						{
							access = btLibDiscoverableAndConnectable;
						}
						else
						{
							ErrFatalError( "impossible accessibility choice" );
						}
						// Set the accessibility to the user's choice.
						error = BtLibSetGeneralPreference( gFdME, btLibPref_UnconnectedAccessible, &access, sizeof(access) );
						if ( error == btLibErrNoError )
						{
							LogPrintF( "Set Accessibility OK" );
						}
						else if ( error == btLibErrNoError || error == btLibErrPending )
						{
							LogPrintF( "Set Accessibility PENDING" );
						}
						else
						{
							LogPrintF( "Set Accessibility ERROR 0x%lx", error );
						}
					}

					// Free the form resources
					FrmDeleteForm( frmP );
				}
			}
			break;
		/******************/
		/* L2CAP Menu     */
		/******************/
		case L2CAPListen:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				CreateListenerSocketAndAdvertiseService( btLibL2CapProtocol );
			}
			break;

		case L2CAPStopListening:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdL2cListener < 0 ) 
			{
				LogPrintF( "No L2CAP listener socket" );
			}
			else
			{
				LogPrintF( "Closing L2CAP listener socket" );
				StopAdvertisingAndCloseListenerSocket( &gFdL2cListener, &gL2cSdpRecordH  );
			}
			break;

		case L2CAPConnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdL2cData >= 0 )
			{
				LogPrintF( "L2Cap already connected" );
			}
			else if ( CreateSocket( &gFdSdp, btLibSdpProtocol ) )
			{
				// Use SDP to get the PSM, the connection is made in the callback
				error = BtLibSdpGetPsmByUuid( gFdSdp, &gBdAddr, &kL2CapUUID, 1 );
				if ( error != btLibErrPending )
				{
					LogPrintF( "SDP Query Error: 0x%lx", error );
				}
				else
				{
					LogPrintF( "SDP Socket %ld: Getting PSM...", gFdSdp );
				}
			}
			break;

		case L2CAPDisconnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdL2cData < 0 ) 
			{
				LogPrintF( "No L2CAP data socket" );
			}
			else
			{
				CloseSocket( &gFdL2cData );
			}
			break;

		case L2CAPSend:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdL2cData < 0 ) 
			{
				LogPrintF( "No L2CAP data socket" );
			}
			else
			{
				FormType*	frmP;
				char*		dialogResult;

				// Get the input string
				MenuEraseStatus(0);	// Clear the command bar if any
				frmP = FrmInitForm( gResourceDbRef, InputForm );
				FrmSetFocus( frmP, FrmGetObjectIndex(frmP, InputDataField) );

				// Display the input dialog
				if ( FrmDoDialog(frmP) == InputCancelButton )
				{
					FrmDeleteForm(frmP);
					break;
				}
				dialogResult = FldGetTextPtr( FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, InputDataField)) );

				// If the user input data, send it. Otherwise send the default string.
				StrNCopy( gDataStr, dialogResult ? dialogResult : "Testing 1 2 3", kMyMtu );
				FrmDeleteForm(frmP); // Free the form resources
				error = BtLibSocketSend( gFdL2cData, (uint8_t*)gDataStr, (uint32_t)(strlen(gDataStr) + 1)/*include NULL */);
				if ( !error )
				{
					LogPrintF( "Socket %ld: Sending...", gFdL2cData );
				}
				else
				{
					LogPrintF( "Socket %ld: Send Error: 0x%lx", gFdL2cData, error );
				}

			}
			break;

		/******************/
		/* RfComm Menu    */
		/******************/
		case RFCOMMListen:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				CreateListenerSocketAndAdvertiseService( btLibRfCommProtocol );
			}
			break;

		case RFCOMMStopListening:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdRfcListener < 0 ) 
			{
				LogPrintF( "No RfComm listener socket" );
			}
			else
			{
				LogPrintF( "Closing RfComm listener socket" );
				StopAdvertisingAndCloseListenerSocket( &gFdRfcListener, &gRfcSdpRecordH  );
			}
			break;

		case RFCOMMConnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdRfcData >= 0 )
			{
				LogPrintF( "RfComm already connected" );
			}
			else
			{
				error = BtLibSocketCreate( &gFdSdp, btLibSdpProtocol );
				if ( error )
				{
					LogPrintF( "Create SDP socket error: 0x%lx", error );
				}
				else
				{
					PbxAddFd( gPollBox, gFdSdp, POLLIN, HandlePbxSocketEvent, &gFdSdp );
					// Use SDP to get the channel ID, the connection is made in the callback
					error = BtLibSdpGetServerChannelByUuid( gFdSdp, &gBdAddr, &kRfCommUUID, 1 );
					if ( error != btLibErrPending )
					{
						LogPrintF( "SDP Query Error: 0x%lx", error );
					}
					else
					{
						LogPrintF( "SDP Socket %ld: Getting Channel ID ...", gFdSdp );
					}
				}
			}
			break;

		case RFCOMMDisconnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdRfcData < 0 )
			{
				LogPrintF( "RfComm not connected" );
			}
			else
			{
				CloseSocket( &gFdRfcData );
			}
			break;

		case RFCOMMSend:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdRfcData < 0 ) 
			{
				LogPrintF( "No RfComm data socket" );
			}
			else
			{
				FormType*	frmP;
				char*		dialogResult;

				// Get the input string
				MenuEraseStatus(0);	// Clear the command bar if any
				frmP = FrmInitForm( gResourceDbRef, InputForm );
				FrmSetFocus( frmP, FrmGetObjectIndex(frmP, InputDataField) );

				// Display the input dialog
				if( FrmDoDialog(frmP) == InputCancelButton )
				{
					FrmDeleteForm(frmP);
				}
				else
				{
					dialogResult = FldGetTextPtr( FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, InputDataField ) ) );

					// If the user input data, send it. Otherwise send the default string.
					StrNCopy( gDataStr, dialogResult ? dialogResult : "Testing 1 2 3", kMyMtu );
					FrmDeleteForm(frmP); // Free the form resources
					error = BtLibSocketSend( gFdRfcData, (uint8_t*)gDataStr, (uint32_t)(strlen(gDataStr) + 1)/*include NULL */);
					if ( !error )
					{
						LogPrintF( "Socket %ld: Sending...", gFdRfcData );
					}
					else
					{
						LogPrintF( "Socket %ld: Send Error: 0x%lx", gFdRfcData, error );
					}
				}
			}
			break;

		case RFCOMMAdvanceCredit:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdRfcData < 0 ) 
			{
				LogPrintF( "No RfComm data socket" );
			}
			else
			{
				// Credit advance is cumulative, so advance one credit at a time
				error = BtLibSocketAdvanceCredit( gFdRfcData, 1 );
				if ( !error )
				{
					LogPrintF( "Socket %ld: Advanced 1 credit", gFdRfcData );
				}
				else
				{
					LogPrintF("Socket %ld: Adv Credit Error: 0x%lx", gFdRfcData, error);
				}
			}
			break;
		/****************/
		/* SCO Menu     */
		/****************/
		case SCOListen:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				CreateListenerSocketAndAdvertiseService( btLibSCOProtocol );
			}
			break;

		case SCOStopListening:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdSCOListener < 0 ) 
			{
				LogPrintF( "No SCO listener socket" );
			}
			else
			{
				LogPrintF( "Closing SCO listener socket" );
				StopAdvertisingAndCloseListenerSocket( &gFdSCOListener, NULL  );
			}
			break;

		case SCOConnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdSCOData >= 0 )
			{
				LogPrintF( "SCO already connected" );
			}
			// Create an SCO socket
			else if ( CreateSocket( &gFdSCOData, btLibSCOProtocol ) )
			{
				// Connect the SCO socket.
				BtLibSocketConnectInfoType connectInfo;
				connectInfo.remoteDeviceP = &gBdAddr;
				error = BtLibSocketConnect( gFdSCOData, &connectInfo );
				if ( error != btLibErrPending )
				{
					LogPrintF( "SCO Connect Error: 0x%lx", error );
				}
				else
				{
					LogPrintF( "SCO socket %ld: connecting...", gFdSCOData );
				}
			}
			break;

		case SCODisconnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdSCOData < 0 ) 
			{
				LogPrintF( "No SCO data socket" );
			}
			else
			{
				CloseSocket( &gFdSCOData );
			}
			break;

		case SCOSend:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdSCOData < 0 ) 
			{
				LogPrintF( "No SCO data socket" );
			}
			else
			{
				FormType*	frmP;
				char*		dialogResult;

				// Get the input string
				MenuEraseStatus(0);	// Clear the command bar if any
				frmP = FrmInitForm( gResourceDbRef, InputForm );
				FrmSetFocus( frmP, FrmGetObjectIndex(frmP, InputDataField) );

				// Display the input dialog
				if ( FrmDoDialog(frmP) == InputCancelButton )
				{
					FrmDeleteForm(frmP);
					break;
				}
				dialogResult = FldGetTextPtr( FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, InputDataField)) );

				// If the user input data, send it. Otherwise send the default string.
				StrNCopy( gDataStr, dialogResult ? dialogResult : "Testing 1 2 3", kMyMtu );
				FrmDeleteForm(frmP); // Free the form resources
				error = BtLibSocketSend( gFdSCOData, (uint8_t*)gDataStr, (uint32_t)(strlen(gDataStr) + 1)/*include NULL */);
				if ( !error )
				{
					LogPrintF( "Socket %ld: Sending...", gFdSCOData );
				}
				else
				{
					LogPrintF( "Socket %ld: Send Error: 0x%lx", gFdSCOData, error );
				}

			}
			break;
		/****************/
		/* BNEP Menu     */
		/****************/
		case BNEPListen:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else
			{
				CreateListenerSocketAndAdvertiseService( btLibBNEPProtocol );
			}
			break;

		case BNEPStopListening:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdBNEPListener < 0 ) 
			{
				LogPrintF( "No BNEP listener socket" );
			}
			else
			{
				LogPrintF( "Closing BNEP listener socket" );
				StopAdvertisingAndCloseListenerSocket( &gFdBNEPListener, NULL  );
			}
			break;

		case BNEPConnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdBNEPData >= 0 )
			{
				LogPrintF( "BNEP already connected" );
			}
			// Create an BNEP socket
			else if ( CreateSocket( &gFdBNEPData, btLibBNEPProtocol ) )
			{
				static const char* uuidPANU = btLibSdpUUID_SC_PANU;
				static const char* uuidGN   = btLibSdpUUID_SC_GN;
				static const char* uuidNAP  = btLibSdpUUID_SC_NAP;

				// Connect the BNEP socket.
				BtLibSocketConnectInfoType connectInfo;
				connectInfo.remoteDeviceP = &gBdAddr;
				connectInfo.data.bnep.localService  = ((uint8_t)uuidPANU[0] << 8) + (uint8_t)uuidPANU[1]; // local as PANU
				connectInfo.data.bnep.remoteService = ((uint8_t)uuidGN[0]   << 8) + (uint8_t)uuidGN[1];   // remote as GN
				error = BtLibSocketConnect( gFdBNEPData, &connectInfo );
				if ( error != btLibErrPending )
				{
					LogPrintF( "BNEP Connect Error: 0x%lx", error );
				}
				else
				{
					LogPrintF( "BNEP socket %ld: connecting...", gFdBNEPData );
				}
			}
			break;

		case BNEPDisconnect:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdBNEPData < 0 ) 
			{
				LogPrintF( "No BNEP data socket" );
			}
			else
			{
				CloseSocket( &gFdBNEPData );
			}
			break;

		case BNEPSend:
			if ( gFdME < 0 )
			{
				LogPrintF( "BtLib not open" );
			}
			else if ( gFdBNEPData < 0 ) 
			{
				LogPrintF( "No BNEP data socket" );
			}
			else
			{
				FormType*	frmP;
				char*		dialogResult;

				// Get the input string
				MenuEraseStatus(0);	// Clear the command bar if any
				frmP = FrmInitForm( gResourceDbRef, InputForm );
				FrmSetFocus( frmP, FrmGetObjectIndex(frmP, InputDataField) );

				// Display the input dialog
				if ( FrmDoDialog(frmP) == InputCancelButton )
				{
					FrmDeleteForm(frmP);
					break;
				}
				dialogResult = FldGetTextPtr( FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, InputDataField)) );

				// If the user input data, send it. Otherwise send the default string.
				StrNCopy( gDataStr, dialogResult ? dialogResult : "Testing 1 2 3", kMyMtu );
				FrmDeleteForm(frmP); // Free the form resources
				error = BtLibSocketSend( gFdBNEPData, (uint8_t*)gDataStr, (uint32_t)(strlen(gDataStr) + 1)/*include NULL */);
				if ( !error )
				{
					LogPrintF( "Socket %ld: Sending...", gFdBNEPData );
				}
				else
				{
					LogPrintF( "Socket %ld: Send Error: 0x%lx", gFdBNEPData, error );
				}

			}
			break;
	}
	return handled;
}

/***********************************************************************
 *
 * FUNCTION:		mainViewResize
 *
 * DESCRIPTION: This routine resize the list view fomr and its objects
 *				when the input area appears or disappears
 *
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs 	1/18/02 	Initial Revision
 *
 ************************************************************************/
static void mainViewResize(EventType *eventP, Boolean scrollToInsPt)
{
	uint16_t		objIndex;
	uint16_t		fieldIndex;
	Coord 			offset;
	RectangleType 	rect;
	FormType *		frmP;
	FontID			currFont;
	Coord 			width, height;
	uint16_t		insPtPos;
	FieldType * 	fieldP;
	int16_t 		lineHeight;

	TraceOutput(TL(blthErrorClass, "BtLibTest: mainViewResize"));
	frmP = FrmGetFormPtr(MainForm);

	// get the new window extent
	width = eventP->data.winResized.newBounds.extent.x;
	height = eventP->data.winResized.newBounds.extent.y;

	// get the delta between the old window bounding rect and the new display extent.
	offset = height - gCurrentWinBounds.extent.y;
	
	if(offset == 0)
		// if the window has the height of the screen
		// then the windows is already at the right size
		return;

	// The field
	fieldIndex = objIndex = FrmGetObjectIndex(frmP, MainLogField);
	fieldP = FrmGetObjectPtr(frmP, objIndex);
	insPtPos = (uint16_t)FldGetInsPtPosition(fieldP);
	
	// If hiding the IA without giving it the focus, set the insertion
	// point to 0 to avoid misplaced insertion point error.
	if (!scrollToInsPt)
		FldSetInsertionPoint(fieldP, 0);

	FrmGetObjectBounds(frmP, objIndex, &rect);
	// Stick the table bottom to the 'New' button.
	rect.extent.y += offset;

	// Get font height
	currFont = FntSetFont (stdFont);
	lineHeight = FntLineHeight();
	FntSetFont (currFont);

	// Remove the last incomplete line
	rect.extent.y -= (rect.extent.y % lineHeight);
	FrmSetObjectBounds(frmP, objIndex, &rect);

	// Reset insertion point point position
	if (scrollToInsPt)
		FldSetInsPtPosition(fieldP, insPtPos);
	else
		FldSetInsertionPoint(fieldP, insPtPos);

	FldRecalculateField (fieldP, true);

	// The field will be scrolled one more line to not have the
	// insertion point stuck at the field bottom
/*	if (sFieldTapY >= 0)	// User taps within the field bounds
	{
		if (rect.topLeft.y + rect.extent.y - lineHeight < sFieldTapY	&&	// Tap where the input area now is
			gEditViewIAState == pinInputAreaShow				&&	// Show the input area
			FldScrollable(fieldP, winDown)) 				// Field is scrollable
		{
			EditViewScroll(1, false);
		}
		sFieldTapY = -1;
	}
*/
	// The scrollbar
	objIndex = FrmGetObjectIndex(frmP, MainLogScrollBar);
	FrmGetObjectBounds(frmP, objIndex, &rect);
	rect.extent.y += offset;
	FrmSetObjectBounds(frmP, objIndex, &rect);
//	EditViewUpdateScrollBar();
	
	// The Graffiti Shift Indicator
	objIndex = FrmGetNumberOfObjects(frmP);
	while (objIndex--)
	{
		if (FrmGetObjectType(frmP, objIndex) == frmGraffitiStateObj)
		{
			FrmGetObjectBounds(frmP, objIndex, &rect);
			rect.topLeft.y += offset;
			FrmSetObjectBounds(frmP, objIndex, &rect);
			break;
		}
	}

	FrmSetFocus(frmP, fieldIndex);
	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;
} 


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent( EventPtr eventP )
{
	Boolean handled = false;

	switch ( eventP->eType )
	{
	case menuEvent:
		{
//			RectangleType 	r;	
			handled = 	MainFormDoCommand( eventP->data.menu.itemID );
/*			
			FormPtr	frmP = FrmGetFormPtr(MainForm );

			if (frmP)
			{
				FieldPtr	fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainLogField));
				FldGetBounds (fldP, &r);	
				r.extent.y = r.topLeft.y;
				r.topLeft.x = 0;
				r.topLeft.y = 0;
				WinInvalidateRect(FrmGetWindowHandle(frmP), &r);
			}
			EventType tempoEvent;

			TraceOutput(TL(appErrorClass, "BtLibTest: MainFormHandleEvent: menuEvent"));			

			tempoEvent.eType = frmUpdateEvent;
			tempoEvent.data.frmUpdate.updateCode = frmRedrawUpdateCode;
			tempoEvent.data.frmUpdate.formID = MainForm;
			EvtAddEventToQueue( &tempoEvent);
*/
			break;
		}

	case menuCmdBarOpenEvent:
			TraceOutput(TL(appErrorClass, "BtLibTest: MainFormHandleEvent: menuCmdBarOpenEvent"));
			break;

	case menuOpenEvent:
			TraceOutput(TL(appErrorClass, "BtLibTest: MainFormHandleEvent: menuOpenEvent"));
			break;

	case menuCloseEvent:
			TraceOutput(TL(appErrorClass, "BtLibTest: MainFormHandleEvent: menuCloseEvent"));
			break;
	
	case frmOpenEvent:
		{
			FormPtr		frmP = FrmGetFormPtr(eventP->data.frmOpen.formID );
			MainFormInit(frmP);
			FrmUpdateForm(eventP->data.frmOpen.formID, frmRedrawUpdateCode);
//			WinInvalidateWindow(FrmGetWindowHandle(frmP));
			handled = true;
			break;
		}

	case frmCloseEvent:
		{
			FormPtr	frmP = FrmGetFormPtr( eventP->data.frmOpen.formID );
			MemHandle fieldH = FldGetTextHandle(FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, MainLogField ) ));
			FldSetTextHandle (FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, MainLogField )), NULL);
			if ( fieldH )
				MemHandleFree (fieldH);
			FrmEraseForm(frmP);
			FrmDeleteForm(frmP);
			handled = true;
			break;
		}

	case frmUpdateEvent:
		{		
			FormPtr		frmP = FrmGetFormPtr(eventP->data.frmUpdate.formID );
//			FieldPtr		fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainLogField));

			TraceOutput(TL(appErrorClass, "BtLibTest: MainFormHandleEvent: updateCode (%hX)", eventP->data.frmUpdate.updateCode));

			switch ( eventP->data.frmUpdate.updateCode )
			{
			case frmRedrawUpdateCode:
				FrmDrawForm( frmP );
//				FldDrawField(fldP);
				break;
			default:
				break;
			}
			handled = true;
			break;
		}

	case appStopEvent:
		gStopFlag = true;
		break;

	case winResizedEvent:
		TraceOutput(TL(blthErrorClass, "BtLibTest: MainFormHandleEvent: winResizedEvent"));
		if (FrmGetWindowHandle(FrmGetFormPtr(MainForm)) != eventP->data.winResized.window)
			break;
		mainViewResize(eventP, false);
//		FrmUpdateForm(BtDiscoveryMultiForm, frmRedrawUpdateCode);
		handled = true;
		break;

	case sclRepeatEvent:
		{
			int32_t	val = eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value;

			if (val > 0)
				ScrollLines((uint16_t)val, winDown, false);
			else
				ScrollLines((uint16_t)-val, winUp, false);
			handled = true;
			break;
		}

	case keyDownEvent:
		switch ( eventP->data.keyDown.chr )
		{
		case pageUpChr:
			ScrollLines( 0, winUp, true );
			handled = true;
			break;
		case pageDownChr:
			ScrollLines( 0, winDown, true );
			handled = true;
			break;
		default:
			break;
		}
		break;

	default:
		TraceOutput(TL(blthErrorClass, "BtLibTest: MainFormHandleEvent: eType= %ld", eventP->eType));
		break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean AppHandleEvent( EventPtr eventP )
{
	if ( eventP->eType == frmLoadEvent )
	{
		DmOpenRef	formDb = eventP->data.frmLoad.formDatabase;
		uint16_t		formID = eventP->data.frmLoad.formID;
		FormType*	formP  = FrmInitForm( formDb, formID );
		WinHandle	winH   = FrmGetWindowHandle( formP );

		WinSetDrawWindow( winH );
		FrmSetActiveForm( formP );
		if ( formID == MainForm ) {
			FrmSetEventHandler( formP, MainFormHandleEvent );
		}
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    HandlePbxSocketEvent
 *
 * DESCRIPTION: Handle events from "socket" devices (L2CAP, RfComm, SDP).
 *
 ***********************************************************************/
static void HandlePbxSocketEvent(
	struct PollBox*				pbx,		// the PollBox
	struct pollfd*				pollFd,		// fd, event mask, and received events
	void*						context		// in this app, ptr to global fd variable
) {
	int32_t*					fdPtr = (int32_t*)context;
	status_t					error;
	int32_t						flags;

	static BtLibSocketEventType	sEvent;
	static char					sData[kMyMtu];
	static struct strbuf		ctlBuf = { sizeof(sEvent), 0, (char*)&sEvent   };
	static struct strbuf		datBuf = { sizeof(sData),  0, (char*)&sData[0] };

	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxSocketEvent: fd=%d, *context=%d", pollFd->fd, *fdPtr));

	// We must be here for a reason ...
	ErrFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// Our context variable points to our global variable containing the file descriptor.
	ErrFatalErrorIf(
		fdPtr != &gFdSdp &&
		fdPtr != &gFdL2cData  && fdPtr != &gFdL2cListener &&
		fdPtr != &gFdRfcData  && fdPtr != &gFdRfcListener &&
		fdPtr != &gFdSCOData  && fdPtr != &gFdSCOListener &&
		fdPtr != &gFdBNEPData && fdPtr != &gFdBNEPListener,
		"bad context"
	);
	ErrFatalErrorIf( pollFd->fd < 0, "bad fd value" );
	ErrFatalErrorIf( pollFd->fd != *fdPtr, "context not fd" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR|POLLHUP|POLLNVAL)) ||				// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		LogPrintF("HandlePbxSocketEvent: closing fd %ld", pollFd->fd);
		CloseSocket( fdPtr );
		return;
	}

	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxSocketEvent: %ld bytes ctl, %ld bytes data", ctlBuf.len, datBuf.len));

	// Check for pure data.
	if ( ctlBuf.len == -1 )
	{
		LogPrintF( "Socket %ld: Data: %s", pollFd->fd, datBuf.buf );
		return;
	}

	// Since it's not pure data, we must have an event struct in the control part.
	ErrFatalErrorIf( ctlBuf.len != sizeof(BtLibSocketEventType), "no event struct" );
	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxSocketEvent: event %s, status 0x%lx", BtLibSocketEventName(sEvent.event), sEvent.status));

	// Decode the event.
	switch ( sEvent.event )
	{
	case btLibSocketEventSdpGetServerChannelByUuid:

		// Close the SDP socket that was used to make the query.
		ErrFatalErrorIf( pollFd->fd != gFdSdp, "not sdp socket" );
		CloseSocket( &gFdSdp );

		if ( sEvent.status != btLibErrNoError )
		{
			LogPrintF( "SDP Get Channel error: 0x%lx", sEvent.status );
		}
		else
		{
			LogPrintF("Retrieved Channel ID: %ld", (int)sEvent.eventData.sdpByUuid.param.channel);
			if ( gFdRfcData >= 0 )
			{
				LogPrintF( "RfComm data socket %ld already connected!", gFdRfcData );
			}
			else
			{
				// Create an RfComm data socket.
				if ( CreateSocket( &gFdRfcData, btLibRfCommProtocol ) )
				{
					// Connect the RfComm data socket.
					BtLibSocketConnectInfoType connectInfo;
					connectInfo.remoteDeviceP = &gBdAddr;
					connectInfo.data.RfComm.remoteService  = sEvent.eventData.sdpByUuid.param.channel;
					connectInfo.data.RfComm.maxFrameSize   = kMyMtu;
					connectInfo.data.RfComm.advancedCredit = 0;
					error = BtLibSocketConnect( gFdRfcData, &connectInfo );
					if ( error != btLibErrPending )
					{
						LogPrintF( "RfComm Connect Error: 0x%lx", error );
					}
					else
					{
						LogPrintF( "RfComm socket %ld: connecting...", gFdRfcData );
					}
				}
			}
		}
		break;

	case btLibSocketEventSdpGetPsmByUuid:

		// Close the SDP socket that was used to make the query.
		ErrFatalErrorIf( pollFd->fd != gFdSdp, "not sdp socket" );
		CloseSocket( &gFdSdp );

		if ( sEvent.status != btLibErrNoError )
		{
			LogPrintF("SDP Get Psm error: 0x%lx", sEvent.status);
		}
		else
		{
			LogPrintF( "Retrieved PSM: %ld", sEvent.eventData.sdpByUuid.param.psm );
			if ( gFdL2cData >= 0 )
			{
				LogPrintF( "Data socket %ld already connected!", gFdL2cData );
			}
			else
			{
				// Create an L2Cap data socket.
				if ( CreateSocket( &gFdL2cData, btLibL2CapProtocol ) )
				{
					// Connect the L2Cap data socket.
					BtLibSocketConnectInfoType connectInfo;
					connectInfo.remoteDeviceP = &gBdAddr;
					connectInfo.data.L2Cap.remotePsm    = sEvent.eventData.sdpByUuid.param.psm;
					connectInfo.data.L2Cap.localMtu     = kMyMtu;
					connectInfo.data.L2Cap.minRemoteMtu = kMyMtu;
					error = BtLibSocketConnect( gFdL2cData, &connectInfo );
					if ( error != btLibErrPending )
					{
						LogPrintF( "L2CAP Connect Error: 0x%lx", error );
					}
					else
					{
						LogPrintF( "L2CAP socket %ld: connecting...", gFdL2cData );
					}
				}
			}
		}
		break;

	case btLibSocketEventConnectRequest:

		// Inbound connection request.
		{
			int32_t* fdDataPtr = NULL;

			if      ( pollFd->fd == gFdL2cListener )  { fdDataPtr = &gFdL2cData;  }
			else if ( pollFd->fd == gFdRfcListener )  { fdDataPtr = &gFdRfcData;  }
			else if ( pollFd->fd == gFdSCOListener )  { fdDataPtr = &gFdSCOData;  }
			else if ( pollFd->fd == gFdBNEPListener ) { fdDataPtr = &gFdBNEPData; }
			else                                      { ErrFatalError( "not a listener socket" ); }

			if ( *fdDataPtr < 0 )
			{
				// No existing data connection for this protocol, so accept it.
				BtLibSocketRespondToConnection( pollFd->fd, true );
				LogPrintF( "Socket %ld: Inbound Req Accepted", pollFd->fd );
			}
			else
			{
				// Data connection exists for this protocol, so refuse it.
				BtLibSocketRespondToConnection( pollFd->fd, false );
				LogPrintF( "Socket %ld: Inbound Req Denied", pollFd->fd );
			}
		}
		break;

	case btLibSocketEventConnectedOutbound:

		ErrFatalErrorIf(
			pollFd->fd != gFdL2cData &&
			pollFd->fd != gFdRfcData &&
			pollFd->fd != gFdSCOData &&
			pollFd->fd != gFdBNEPData,
			"not a data socket"
		);
		if ( sEvent.status == btLibErrNoError )
		{
			// Outbound connection succeeded.
			LogPrintF( "Socket %ld: Outbound Conn OK", pollFd->fd );
		}
		else
		{
			// Outbound connection failed.
			LogPrintF( "Socket %ld: Outbound Conn ERROR 0x%lx", pollFd->fd, sEvent.status );
			CloseSocket( fdPtr );
		}
		break;

	case btLibSocketEventConnectedInbound:

		// Inbound connection succeeded.
		{
			int32_t* fdDataPtr = NULL;

			if      ( pollFd->fd == gFdL2cListener )  { fdDataPtr = &gFdL2cData;  }
			else if ( pollFd->fd == gFdRfcListener )  { fdDataPtr = &gFdRfcData;  }
			else if ( pollFd->fd == gFdSCOListener )  { fdDataPtr = &gFdSCOData;  }
			else if ( pollFd->fd == gFdBNEPListener ) { fdDataPtr = &gFdBNEPData; }
			else                                      { ErrFatalError( "not a listener socket" ); }
			
			// Put the newly created data socket into the pollbox.
			ErrFatalErrorIf( *fdDataPtr >= 0, "already connected" );
			*fdDataPtr = sEvent.eventData.newSocket;
			PbxAddFd( gPollBox, *fdDataPtr, POLLIN, HandlePbxSocketEvent, fdDataPtr );
			LogPrintF( "Socket %ld: New Inbound Conn on socket %ld", pollFd->fd, *fdDataPtr );
		}
		break;

	case btLibSocketEventDisconnected:

		LogPrintF( "Socket %ld: Disconnect", pollFd->fd );
		if (
			pollFd->fd == gFdL2cData ||
			pollFd->fd == gFdRfcData ||
			pollFd->fd == gFdSCOData ||
			pollFd->fd == gFdBNEPData
		) {
			// Disconnect event on a data socket: an existing connection was broken.
			// Close the data socket.
			CloseSocket( fdPtr );
		}
		else if (
			pollFd->fd == gFdL2cListener ||
			pollFd->fd == gFdRfcListener ||
			pollFd->fd == gFdSCOListener ||
			pollFd->fd == gFdBNEPListener
		) {
			// Disconnect event on a listener socket: accepting an inbound connection failed.
			// Must close both the listener socket and the new socket returned in the event.
			CloseSocket( fdPtr );
			BtLibSocketClose( sEvent.eventData.newSocket );
		}
		break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    HandlePbxMEEvent
 *
 * DESCRIPTION: Handle events from the Management Entity device.
 *
 ***********************************************************************/
static void HandlePbxMEEvent(
	struct PollBox*					pbx,		// the PollBox
	struct pollfd*					pollFd,		// fd, event mask, and received events
	void*							context		// not used here
) {
	status_t						error;
	int32_t							flags;

	static BtLibManagementEventType	mEvent;
	static char						mData[sizeof(BtLibFriendlyNameType)];
	static struct strbuf			ctlBuf = { sizeof(mEvent), 0, (char*)&mEvent   };
	static struct strbuf			datBuf = { sizeof(mData),  0, (char*)&mData[0] };

	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxMEEvent: fd=%d", pollFd->fd));

	// We must be here for a reason ...
	ErrFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// We must have the Management Entity file descriptor.
	ErrFatalErrorIf( pollFd->fd != gFdME, "not the ME fd" );
	ErrFatalErrorIf( pollFd->fd < 0, "ME fd closed" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR|POLLHUP|POLLNVAL)) ||				// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		LogPrintF( "HandlePbxMEEvent: closing fd %d", pollFd->fd );
		PbxRemoveFd( pbx, pollFd->fd );
		BtLibClose( pollFd->fd );
		gFdME = -1;
		return;
	}

	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxMEEvent: %d bytes ctl, %d bytes data", ctlBuf.len, datBuf.len));

	// We must have an event struct in the control part.
	ErrFatalErrorIf( ctlBuf.len != sizeof(BtLibManagementEventType), "no event struct" );

	TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxMEEvent: event %s, status 0x%lx", BtLibMEEventName(mEvent.event), mEvent.status));

	// Decode the event.
	switch ( mEvent.event )
	{
	case btLibManagementEventRadioState:
		{
			const char* s;

			switch ( mEvent.status )
			{
			case btLibErrRadioInitialized: s = "Radio Initialized"; break;
			case btLibErrRadioInitFailed:  s = "Radio Init Failed"; break;
			case btLibErrRadioFatal:       s = "Radio Fatal Error"; break;
			case btLibErrRadioSleepWake:   s = "Radio Wakeup";      break;
			default:                       s = "Radio Event 0x%lx"; break;
			}
			LogPrintF( s, mEvent.status );
		}
		break;

	case btLibManagementEventInquiryResult:
		BtLibAddrBtdToA( &mEvent.eventData.inquiryResult.bdAddr, gBdAddrStr, kBdAddrStringSize );
		LogPrintF( "%s", gBdAddrStr );
		break;

	case btLibManagementEventInquiryComplete:
		LogPrintF( "Inquiry complete" );
		break;

	case btLibManagementEventInquiryCanceled:
		LogPrintF( "Inquiry canceled" );
		break;

	case btLibManagementEventACLConnectOutbound: // fall through
	case btLibManagementEventACLConnectInbound:
		memmove( &gBdAddr, &mEvent.eventData.bdAddr, sizeof(BtLibDeviceAddressType) );
		BtLibAddrBtdToA(&mEvent.eventData.bdAddr, gBdAddrStr, kBdAddrStringSize );
		if ( mEvent.status == btLibErrNoError )
		{
			LogPrintF( "Link to %s Established", gBdAddrStr );
			ChangeLabel( MainAddrLabel, gBdAddrStr );
		}
		else
		{
			LogPrintF( "Link to %s Failed", gBdAddrStr );
		}
		break;

	case btLibManagementEventAuthenticationComplete:
		if ( mEvent.status == btLibErrNoError )
			LogPrintF( "Authenticate complete" );
		else
			LogPrintF( "Authenticate Failed: 0x%lx", mEvent.status );
		break;

	case btLibManagementEventEncryptionChange:
		BtLibAddrBtdToA( &mEvent.eventData.encryptionChange.bdAddr, gBdAddrStr, kBdAddrStringSize );
		if ( mEvent.status == btLibErrNoError )
		{
			if ( mEvent.eventData.encryptionChange.enabled )
				LogPrintF("%s Encrypted", gBdAddrStr);
			else
				LogPrintF("%s Not Encrypted", gBdAddrStr);
		}
		else
		{
			LogPrintF("Encryption Failed: 0x%lx", mEvent.status);
		}
		break;

	case btLibManagementEventACLDisconnect:
		BtLibAddrBtdToA( &mEvent.eventData.bdAddr, gBdAddrStr, kBdAddrStringSize );
		LogPrintF( "Disconnected from %s, reason: 0x%lx", gBdAddrStr, mEvent.status );
		ChangeLabel( MainAddrLabel, "No Connection" );
		MemSet( &gBdAddr, sizeof(gBdAddr), 0 );
		break;

	case btLibManagementEventNameResult:
		BtLibAddrBtdToA( &mEvent.eventData.nameResult.bdAddr, gBdAddrStr, kBdAddrStringSize );
		if ( mEvent.status == btLibErrNoError )
			LogPrintF( "'%s' is name of %s", ((BtLibFriendlyNameType*)mData)->name, gBdAddrStr );
		else
			LogPrintF( "Get Remote Name from %s failed: 0x%lx", gBdAddrStr, mEvent.status );
		break;

	case btLibManagementEventRoleChange:
		BtLibAddrBtdToA( &mEvent.eventData.roleChange.bdAddr, gBdAddrStr, kBdAddrStringSize );
		if ( mEvent.eventData.roleChange.newRole == btLibMasterRole )
			LogPrintF( "Became Master of %s", gBdAddrStr );
		else
			LogPrintF( "Became Slave of %s", gBdAddrStr );
		break;

	case btLibManagementEventAccessibilityChange:
		{
			const char* s = NULL;

			switch ( mEvent.eventData.accessible )
			{
			case btLibNotAccessible:				s = "Accessibility: Not Accessible";				break;
			case btLibConnectableOnly:				s = "Accessibility: Connectable Only";				break;
			case btLibDiscoverableAndConnectable:	s = "Accessibility: Discoverable and Connectable";	break;
			default:								ErrFatalError( "impossible accessibility" );		break;
			}
			LogPrintF( s, mEvent.eventData.accessible );
		}
		break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    HandlePbxUIEvent
 *
 * DESCRIPTION: Handle UI events.
 *
 ***********************************************************************/
static void HandlePbxUIEvent(
	PollBox*		pbx,		// the PollBox
	struct pollfd*	pollFd,		// fd, event mask, and received events
	void*			context		// user context
) {
	EventType		event;
	status_t		error;

	UNUSED_ARG( pbx );
	UNUSED_ARG( pollFd );
	UNUSED_ARG( context );

	// TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxUIEvent: ENTER fd=%d", pollFd->fd));

	do
	{
		EvtGetEvent( &event, evtNoWait );
		// TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxUIEvent: event.eType=%d", event.eType));

		// Enable drawing if the main form is active.
		// Disable if another form or menu is opened on top.
		if (
			event.eType == winExitEvent &&
			event.data.winExit.exitWindow == (WinHandle)FrmGetFormPtr(MainForm)
		) {
			gCanDraw = false;
		}
		else if (
			event.eType == winEnterEvent &&
			event.data.winEnter.enterWindow == (WinHandle)FrmGetFormPtr(MainForm)
		) {
			gCanDraw = true;
			LogPrintF(NULL);
		}

		// Try the usual series of UI event handlers.
		if (
			!SysHandleEvent( &event ) &&
			!MenuHandleEvent( 0, &event, &error ) &&
			!AppHandleEvent( &event )
		) {
			FrmDispatchEvent( &event );
		}
	} while ( EvtEventAvail() );

	// TraceOutput(TL(appErrorClass, "BtLibTest: HandlePbxUIEvent: EXIT"));
}


//-----------------------------------------
// until polling fd 0 works correctly.
//
static status_t MyPbxPoll(
	PollBox*	pbx,
	int32_t*	nReadyP
) {
#if UI_EVENTS_ON_FD
    return PbxPoll( pbx, -1, nReadyP );
#else
	HandlePbxUIEvent( 0, 0, 0 );
    return PbxPoll( pbx, 1, nReadyP );
#endif
}

/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void AppEventLoop( void )
{
	int32_t		nReady;
	status_t	err;

	while ( !gStopFlag )
	{
		err = MyPbxPoll( gPollBox, &nReady );
		// TraceOutput(TL(appErrorClass, "BtLibTest: AppEventLoop: MyPbxPoll returns nReady=%d, err=0x%lx", nReady, err));
		ErrFatalErrorIf( err, "poll error" );
		ErrFatalErrorIf( gPollBox->count == 0, "empty pollbox" );
	}
}

/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Initialize the application.
 *
 * PARAMETERS:   Nothing
 *
 * RETURNED:     Nothing
 *
 ***********************************************************************/
static void AppStart(void)
{
	status_t	err;

	TraceOutput(TL(appErrorClass, "BtLibTest: AppStart"));

	// Open our resource database.
	err = SysGetModuleDatabase( SysGetRefNum(), NULL, &gResourceDbRef );
	ErrFatalErrorIf( err, "can't get resource db" );

	// Create our pollbox, and put the UI event file descriptor into it.
	gPollBox = PbxCreate();
#if UI_EVENTS_ON_FD
	gFdUI = EvtGetEventDescriptor();
	ErrFatalErrorIf( gFdUI < 0, "can't get fd for ui events" );
	PbxAddFd( gPollBox, gFdUI, POLLIN, HandlePbxUIEvent, NULL );
#else
	gFdUI = IOSOpen( "NULL", O_RDWR, &err );
	ErrFatalErrorIf( gFdUI < 0, "can't open NULL dev" );
	PbxAddFd( gPollBox, gFdUI, POLLIN, NULL, NULL );
#endif
	TraceOutput(TL(appErrorClass, "BtLibTest: AppStart: using fd %d", gFdUI));

	// Go to the main form.
	FrmGotoForm( gResourceDbRef, MainForm );
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Terminate the application.
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void AppStop(void)
{
	TraceOutput(TL(appErrorClass, "BtLibTest: AppStop"));

	RestoreOriginalAccessibility();
	FrmCloseAllForms();
#if UI_EVENTS_ON_FD
	PbxRemoveFd( gPollBox, gFdUI );	// must not close the UI fd
#endif
	PbxDestroy( gPollBox ); // note this closes any fds still in the box
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 ***********************************************************************/
uint32_t PilotMain(
	uint16_t	cmd,
	MemPtr	cmdPBP,
	uint16_t	launchFlags
) {
	UNUSED_ARG( cmdPBP );
	UNUSED_ARG( launchFlags );
#if 0
	uint32_t	btVersion;

	// Check that Bluetooth components are intalled, which imply Palm OS 4.0 or upper
	if ( FtrGet( btLibFeatureCreator, btLibFeatureVersion, &btVersion ) != errNone )
	{
		// Alert the user if it's the active application
		if ( (launchFlags & sysAppLaunchFlagNewGlobals) && (launchFlags & sysAppLaunchFlagUIApp) )
		{
			FrmAlert( gResourceDbRef, MissingBtComponentsAlert );
		}
		return sysErrRomIncompatible;
	}
#endif

	// Handle the launch code.
	if ( cmd == sysAppLaunchCmdNormalLaunch )
	{
		AppStart();
		AppEventLoop();
		AppStop();
	}
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    ChangeLabel
 *
 * DESCRIPTION: Utility function to change a label
 *				Note: The initial label (done in Constructor) MUST be
 *                    longer than any label you copy over it!!
 *
 * PARAMETERS:  label Id and new text
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void ChangeLabel(uint16_t labelID, const char *newLabel)
{
	FormPtr frm = FrmGetActiveForm();
	uint16_t	labelIndex = FrmGetObjectIndex(frm, labelID);

	//Hide the label first to erase old text
	FrmHideObject(frm, labelIndex);
	//Copy in the new string
	FrmCopyLabel(frm, labelID, newLabel);
	//Now force the redraw
	FrmShowObject(frm, labelIndex);
}


/***********************************************************************
 *
 * FUNCTION:    UpdateScrollbar
 *
 * DESCRIPTION: Update the scroll bar related to the log field.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void UpdateScrollbar(void)
{
	uint32_t 		currentPosition;
	uint32_t 		textHeigth;
	uint32_t 		fieldHeigth;
	uint32_t		maxValue;
	FormPtr			frmP = FrmGetFormPtr(MainForm);
	ScrollBarPtr	scrollP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainLogScrollBar));
	FieldPtr		fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainLogField));

	FldGetScrollValues(fldP, &currentPosition, &textHeigth, &fieldHeigth);
	if (textHeigth > fieldHeigth)
		maxValue = textHeigth - fieldHeigth;
	else if (currentPosition)
		maxValue = currentPosition;
	else
		maxValue = 0;

	SclSetScrollBar(scrollP, (int16_t)currentPosition, 0, (int16_t)maxValue, (int16_t)(fieldHeigth - 1));
}


/***********************************************************************
 *
 * FUNCTION:    ScrollLines
 *
 * DESCRIPTION: Scroll log field and update its scroll bar.
 *
 * PARAMETERS:  number of lines and direction
 *              0 lines means a page scroll
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void ScrollLines(
	uint16_t			numLinesToScroll,
	WinDirectionType	direction,
	Boolean				update
) {
	FormPtr				frmP = FrmGetFormPtr( MainForm );
	FieldPtr			fldP = FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, MainLogField ) );

	if ( numLinesToScroll == 0 )
		numLinesToScroll = (uint16_t)(FldGetVisibleLines(fldP) - 1);

	FldScrollField(fldP, numLinesToScroll, direction);

	if ( update || (FldGetNumberOfBlankLines(fldP) && direction == winUp) )
		UpdateScrollbar();
}


/***********************************************************************
 *
 * FUNCTION:    LogPrintF
 *
 * DESCRIPTION: Add text to the log field, max 128 chars
 *
 * PARAMETERS:  same as sprintf
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void LogPrintF(const char *format, ...)
{
	static uint32_t	gLenConv = 0;
	uint16_t		lenMess;
	va_list			args;
	char			mess[128];
	FormPtr			frmP = FrmGetFormPtr(MainForm);
	FieldPtr		fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainLogField));
	MemHandle 	fieldH = FldGetTextHandle(fldP);
	char *			txtP;
	
	// The first time, allocate the field with the maximum supported field size
	if (!fieldH)
	{
		fieldH = MemHandleNew (kMaxSizeLog);
		if (fieldH)
		{
			txtP = (char *)MemHandleLock (fieldH);
			gLenConv = FldGetVisibleLines(fldP);
			MemSet(txtP, gLenConv, '\n');
			txtP[gLenConv] = '\0';
		}
		else
			return;
	}
	else
	{
		txtP = (char *)MemHandleLock (fieldH);
	}

	// Format the message, if any, and append it to the field
	if (format)
	{
		// Format the message
		va_start(args, format);
		StrVPrintF(mess, format, args);
		va_end(args);

		// When field does not have enough room, keep only last half of the conversation
		// Move the last half in replacement of first half
		lenMess = (uint16_t) strlen(mess) + 1;
		if (gLenConv + lenMess >= kMaxSizeLog)
		{
			gLenConv >>= 1;
			memmove(txtP, txtP + gLenConv + (gLenConv & 1), gLenConv );
		}

		// Append a carriage return and update length
		sprintf(txtP + gLenConv, "\n%s", mess);

#ifdef _DEBUG
		// Dump all the log string to trace
		TraceOutput(TL(appErrorClass, "BtLibTest: LOG: '%s'", mess));
#endif

		gLenConv += lenMess;
		MemHandleUnlock( fieldH);
		FldSetTextHandle (fldP, fieldH);
	}

	// If drawing is enabled (no menu or form is opened on top of the field)
	if (gCanDraw)
	{
		RectangleType 	r;	

		// Redraw field and scroll down to the bottom		
		FldRecalculateField(fldP, false);
		FldScrollField(fldP, (uint16_t) kMaxSizeLog, winDown);
		UpdateScrollbar();		
		FldGetBounds (fldP, &r);
		FrmUpdateForm( FrmGetFormId(frmP), frmRedrawUpdateCode);
//		WinInvalidateRect(FrmGetWindowHandle(frmP), &r);
	}
}
