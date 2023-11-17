/******************************************************************************
 *
 * Copyright (c) 1994-2002 Palmsource, Inc. All rights reserved.
 *
 * File: BtErr.c
 *
 * Release: 
 *
 * Description:
 * 	Converts Bluetooth errno to a string
 *
 *****************************************************************************/

#include <PalmTypes.h>
#include "BtLib.h"
#include "BtLibTypes.h"


char *ConvertBtErrToString(status_t error);


char *ConvertBtErrToString(status_t error)
{
	char *msg;
	
	switch (error)
	{
		case btLibErrNoError:
			msg = (char *) "btLibErrNoError";
			break;
			
		case btLibErrError:
			msg = (char *) "ErrError";
			break;
			
		case btLibErrNotOpen:
			msg = (char *) "ErrNotOpen";
			break;
			
		case btLibErrBluetoothOff:
			msg = (char *) "ErrBluetoothOff";
			break;
			
		case btLibErrNoPrefs:
			msg = (char *) "ErrNoPrefs";
			break;
			
		case btLibErrAlreadyOpen:
			msg = (char *) "ErrAlreadyOpen";
			break;
			
		case btLibErrOutOfMemory:
			msg = (char *) "ErrOutOfMemory";
			break;
			
		case btLibErrFailed:
			msg = (char *) "ErrFailed";
			break;
			
		case btLibErrInProgress:
			msg = (char *) "ErrInProgress";
			break;
			
		case btLibErrParamError:
			msg = (char *) "ErrParamError";
			break;
			
		case btLibErrTooMany:
			msg = (char *) "ErrTooMany";
			break;
			
		case btLibErrPending:
			msg = (char *) "ErrPending";
			break;
			
		case btLibErrNotInProgress:
			msg = (char *) "ErrNotInProgress";
			break;
			
		case btLibErrRadioInitFailed:
			msg = (char *) "ErrRadioInitFailed";
			break;
			
		case btLibErrRadioFatal:
			msg = (char *) "ErrRadioFatal";
			break;
			
		case btLibErrRadioInitialized:
			msg = (char *) "ErrRadioInitialized";
			break;
			
		case btLibErrRadioSleepWake:
			msg = (char *) "ErrRadioSleepWake";
			break;
			
		case btLibErrNoConnection:
			msg = (char *) "ErrNoConnection";
			break;
			
		case btLibErrAlreadyRegistered:
			msg = (char *) "ErrAlreadyRegistered";
			break;
			
		case btLibErrNoAclLink:
			msg = (char *) "ErrNoAclLink";
			break;
			
		case btLibErrSdpRemoteRecord:
			msg = (char *) "ErrSdpRemoteRecord";
			break;
			
		case btLibErrSdpAdvertised:
			msg = (char *) "ErrSdpAdvertised";
			break;
			
		case btLibErrSdpFormat:
			msg = (char *) "ErrSdpFormat";
			break;
			
		case btLibErrSdpNotAdvertised:
			msg = (char *) "ErrSdpNotAdvertised";
			break;
			
		case btLibErrSdpQueryVersion:
			msg = (char *) "ErrSdpQueryVersion";
			break;
			
		case btLibErrSdpQueryHandle:
			msg = (char *) "ErrSdpQueryHandle";
			break;
			
		case btLibErrSdpQuerySyntax:
			msg = (char *) "ErrSdpQuerySyntax";
			break;
			
		case btLibErrSdpQueryPduSize:
			msg = (char *) "ErrSdpQueryPduSize";
			break;
			
		case btLibErrSdpQueryContinuation:
			msg = (char *) "ErrSdpQueryContinuation";
			break;
			
		case btLibErrSdpQueryResources:
			msg = (char *) "ErrSdpQueryResources";
			break;
			
		case btLibErrSdpQueryDisconnect:
			msg = (char *) "ErrSdpQueryDisconnect";
			break;
			
		case btLibErrSdpInvalidResponse:
			msg = (char *) "ErrSdpInvalidResponse";
			break;
			
		case btLibErrSdpAttributeNotSet:
			msg = (char *) "ErrSdpAttributeNotSet";
			break;
			
		case btLibErrSdpMapped:
			msg = (char *) "ErrSdpMapped";
			break;
			
		case btLibErrSocket:
			msg = (char *) "ErrSocket";
			break;
			
		case btLibErrSocketProtocol:
			msg = (char *) "ErrSocketProtocol";
			break;
			
		case btLibErrSocketRole:
			msg = (char *) "ErrSocketRole";
			break;
			
		case btLibErrSocketPsmUnavailable:
			msg = (char *) "ErrSocketPsmUnavailable";
			break;
			
		case btLibErrSocketChannelUnavailable:
			msg = (char *) "ErrSocketChannelUnavailable";
			break;
			
		case btLibErrSocketUserDisconnect:
			msg = (char *) "ErrSocketUserDisconnect";
			break;
			
		case btLibErrCanceled:
			msg = (char *) "ErrCanceled";
			break;
			
		case btLibErrBusy:
			msg = (char *) "ErrBusy";
			break;
			
		case btLibMeStatusUnknownHciCommand:
			msg = (char *) "MeStatusUnknownHciCommand";
			break;
			
		case btLibMeStatusNoConnection:
			msg = (char *) "MeStatusNoConnection";
			break;
			
		case btLibMeStatusHardwareFailure:		
			msg = (char *) "MeStatusHardwareFailure";
			break;
			
		case btLibMeStatusPageTimeout:
			msg = (char *) "MeStatusPageTimeout";
			break;
			
		case btLibMeStatusAuthenticateFailure:
			msg = (char *) "MeStatusAuthenticateFailure";
			break;
			
		case btLibMeStatusMissingKey:
			msg = (char *) "MeStatusMissingKey";
			break;
			
		case btLibMeStatusMemoryFull:
			msg = (char *) "MeStatusMemoryFull";
			break;
			
		case btLibMeStatusConnnectionTimeout:
			msg = (char *) "MeStatusConnnectionTimeout";
			break;
			
		case btLibMeStatusMaxConnections:
			msg = (char *) "MeStatusMaxConnections";
			break;
			
		case btLibMeStatusMaxScoConnections:
			msg = (char *) "MeStatusMaxScoConnections";
			break;
			
		case btLibMeStatusMaxAclConnections:
			msg = (char *) "MeStatusMaxAclConnections";
			break;
			
		case btLibMeStatusCommandDisallowed:
			msg = (char *) "MeStatusCommandDisallowed";
			break;
			
		case btLibMeStatusLimitedResources:
			msg = (char *) "MeStatusLimitedResources";
			break;
			
		case btLibMeStatusSecurityError:
			msg = (char *) "MeStatusSecurityError";
			break;
			
		case btLibMeStatusPersonalDevice:
			msg = (char *) "MeStatusPersonalDevice";
			break;
			
		case btLibMeStatusHostTimeout:
			msg = (char *) "MeStatusHostTimeout";
			break;
			
		case btLibMeStatusUnsupportedFeature:
			msg = (char *) "MeStatusUnsupportedFeature";
			break;
			
		case btLibMeStatusInvalidHciParam:
			msg = (char *) "MeStatusInvalidHciParam";
			break;
			
		case btLibMeStatusUserTerminated:
			msg = (char *) "MeStatusUserTerminated";
			break;
			
		case btLibMeStatusLowResources:
			msg = (char *) "MeStatusLowResources";
			break;
			
		case btLibMeStatusPowerOff:
			msg = (char *) "MeStatusPowerOff";
			break;
			
		case btLibMeStatusLocalTerminated:
			msg = (char *) "MeStatusLocalTerminated";
			break;
			
		case btLibMeStatusRepeatedAttempts:
			msg = (char *) "MeStatusRepeatedAttempts";
			break;
			
		case btLibMeStatusPairingNotAllowed:
			msg = (char *) "MeStatusPairingNotAllowed";
			break;
			
		case btLibMeStatusUnknownLmpPDU:
			msg = (char *) "MeStatusUnknownLmpPDU";
			break;
			
		case btLibMeStatusUnsupportedRemote:
			msg = (char *) "MeStatusUnsupportedRemote";
			break;
			
		case btLibMeStatusScoOffsetRejected:
			msg = (char *) "MeStatusScoOffsetRejected";
			break;
			
		case btLibMeStatusScoIntervalRejected:
			msg = (char *) "MeStatusScoIntervalRejected";
			break;
			
		case btLibMeStatusScoAirModeRejected:
			msg = (char *) "MeStatusScoAirModeRejected";
			break;
			
		case btLibMeStatusInvalidLmpParam:
			msg = (char *) "MeStatusInvalidLmpParam";
			break;
			
		case btLibMeStatusUnspecifiedError:
			msg = (char *) "MeStatusUnspecifiedError";
			break;
			
		case btLibMeStatusUnsupportedLmpParam:
			msg = (char *) "MeStatusUnsupportedLmpParam";
			break;
			
		case btLibMeStatusRoleChangeNotAllowed:
			msg = (char *) "MeStatusRoleChangeNotAllowed";
			break;
			
		case btLibMeStatusLmpResponseTimeout:
			msg = (char *) "MeStatusLmpResponseTimeout";
			break;
			
		case btLibMeStatusLmpTransdCollision:
			msg = (char *) "MeStatusLmpTransdCollision";
			break;
			
		case btLibMeStatusLmpPduNotAllowed:
			msg = (char *) "MeStatusLmpPduNotAllowed";
			break;
			
		case btLibL2DiscReasonUnknown:
			msg = (char *) "L2DiscReasonUnknown";
			break;
			
		case btLibL2DiscUserRequest:
			msg = (char *) "L2DiscUserRequest";
			break;
			
		case btLibL2DiscRequestTimeout:
			msg = (char *) "L2DiscRequestTimeout";
			break;
			
		case btLibL2DiscLinkDisc:
			msg = (char *) "L2DiscLinkDisc";
			break;
			
		case btLibL2DiscQosViolation:
			msg = (char *) "L2DiscQosViolation";
			break;
			
		case btLibL2DiscSecurityBlock:
			msg = (char *) "L2DiscSecurityBlock";
			break;
			
		case btLibL2DiscConnPsmUnsupported:
			msg = (char *) "L2DiscConnPsmUnsupported";
			break;
			
		case btLibL2DiscConnSecurityBlock:
			msg = (char *) "L2DiscConnSecurityBlock";
			break;
			
		case btLibL2DiscConnNoResources:
			msg = (char *) "L2DiscConnNoResources";
			break;
			
		case btLibL2DiscConfigUnacceptable:
			msg = (char *) "L2DiscConfigUnacceptable";
			break;
			
		case btLibL2DiscConfigReject:
			msg = (char *) "L2DiscConfigReject";
			break;
			
		case btLibL2DiscConfigOptions:
			msg = (char *) "L2DiscConfigOptions";
			break;
			
		case btLibServiceShutdownAppUse:
			msg = (char *) "ServiceShutdownAppUse";
			break;
			
		case btLibServiceShutdownPowerCycled:
			msg = (char *) "ServiceShutdownPowerCycled";
			break;
			
		case btLibServiceShutdownAclDrop:
			msg = (char *) "ServiceShutdownAclDrop";
			break;
			
		case btLibServiceShutdownTimeout:
			msg = (char *) "ServiceShutdownTimeout";
			break;
			
		case btLibServiceShutdownDetached:
			msg = (char *) "ServiceShutdownDetached";
			break;
			
		case btLibErrInUseByService:
			msg = (char *) "ErrInUseByService";
			break;

		case btLibErrNoPiconet:
			msg = (char *) "ErrNoPiconet";
			break;
			
		case btLibErrRoleChange:
			msg = (char *) "ErrRoleChange";
			break;
						
		case btLibNotYetSupported:
			msg = (char *) "NotYetSupported";
			break;
			
		case btLibErrSdpNotMapped:
			msg = (char *) "ErrSdpNotMapped";
			break;
			
		case btLibErrAlreadyConnected:
			msg = (char *) "ErrAlreadyConnected";
			break;
			
		default:
			msg = (char *) "Unknown BT error";
			break;
	}
	
	return (msg);
}
