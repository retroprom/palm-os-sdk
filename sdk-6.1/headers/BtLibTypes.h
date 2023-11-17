/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtLibTypes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      Public types, constants, macros for users of the Bluetooth Library.
 *      You should really just include <BtLib.h>, which includes this one.
 *
 *****************************************************************************/

#ifndef _BTLIBTYPES_H
#define _BTLIBTYPES_H

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <sys/socket.h>

// Features
//
#define btLibFeatureCreator      sysFileCBtLib

// The "version" feature, and all known versions to this one.
#define btLibFeatureVersion      0
#define btLibVersion_6_0         2  // OS release 6.0
#define btLibVersion_6_1         3  // OS release 6.1
#define btLibVersion_Current     btLibVersion_6_1

/*-----------------------+
 | Bluetooth Error Codes |
 +-----------------------*/

// btLibErrNoError .................... operation completed succesfully
// btLibErrNotOpen .................... the library is not open
// btLibErrBluetoothOff ............... the user has turned bluetooth off
// btLibErrNoPrefs .................... the preferences are missing
// btLibErrAlreadyOpen ................ library alreasy open (not an error)
// btLibErrOutOfMemory ................ memory allocation failed
// btLibErrFailed ..................... remote operation completed but failed
// btLibErrInProgress ................. an operation is already in progress
// btLibErrParamError ................. invalid parameter to function
// btLibErrTooMany .................... capacity reached for some collection
// btLibErrPending .................... operation will complete later; status
//                                      and results will arrive in an event
// btLibErrNotInProgress .............. operation not in progress
// btLibErrRadioInitFailed ............ initialization of the bluetooth hardware
//                                      unit has failed
// btLibErrRadioFatal ................. the bluetooth hardware unit has failed
//                                      while in use
// btLibErrRadioInitialized ........... successful initialization of the
//                                      bluetooth hardware unit (not an error)
// btLibErrRadioSleepWake ............. the bluetooth hardware unit failed
//                                      because the device went to sleep
// btLibErrNoConnection ............... no connection on socket
// btLibErrAlreadyRegistered .......... callback already registered
// btLibErrNoAclLink .................. no ACL link to remote device
// btLibErrSdpRemoteRecord ............ invalid operation on remote record
// btLibErrSdpAdvertised .............. invalid operation on advertised record
// btLibErrSdpFormat .................. service record improperly formatted
// btLibErrSdpNotAdvertised ........... invalid operation on unadvertised record
// btLibErrSdpQueryVersion ............ invalid/unsupported SDP version
// btLibErrSdpQueryHandle ............. invalid service record handle
// btLibErrSdpQuerySyntax ............. invalid request syntax
// btLibErrSdpQueryPduSize ............ invalid PDU size
// btLibErrSdpQueryContinuation ....... invalid continuation data
// btLibErrSdpQueryResources .......... insufficient resources for request
// btLibErrSdpQueryDisconnect ......... SDP disconnected
// btLibErrSdpInvalidResponse ......... invalid data in SDP response
// btLibErrSdpAttributeNotSet ......... attribute is not set for record
// btLibErrSdpMapped .................. invalid operation on mapped record
// btLibErrSocket ..................... invalid socket reference
// btLibErrSocketProtocol ............. invalid protocol for operation
// btLibErrSocketRole ................. invalid role (listener/connector)
// btLibErrSocketPsmUnavailable ....... PSM already in use
// btLibErrSocketChannelUnavailable ... channel unavailable on remtoe device
// btLibErrSocketUserDisconnect ....... ?
// btLibErrCanceled ................... the operation was canceled
// btLibErrBusy ....................... the resource in question is busy
//
// The following errors (along with btLibErrNoError) are returned in the status
// block of the BtLibManagementEventType. They are normally used to give the
// reason for the event
//
// btLibMeStatusUnknownHciCommand ..... unknown HCI Command
// btLibMeStatusNoConnection .......... no connection
// btLibMeStatusHardwareFailure ....... hardware failure
// btLibMeStatusPageTimeout ........... page timeout
// btLibMeStatusAuthenticateFailure ... authentication failure
// btLibMeStatusMissingKey ............ missing key
// btLibMeStatusMemoryFull ............ not enough memory
// btLibMeStatusConnnectionTimeout .... connection timeout
// btLibMeStatusMaxConnections ........ max number of connections
// btLibMeStatusMaxScoConnections ..... max number of SCO connections
// btLibMeStatusMaxAclConnections ..... max number of ACL connections
// btLibMeStatusCommandDisallowed ..... command disallowed
// btLibMeStatusLimitedResources ...... host rejected due to limited resources
// btLibMeStatusSecurityError ......... host rejected for security reasons
// btLibMeStatusPersonalDevice ........ host rejected, remote is personal device
// btLibMeStatusHostTimeout ........... host timeout
// btLibMeStatusUnsupportedFeature .... unsupported feature or parameter value
// btLibMeStatusInvalidHciParam ....... invalid HCI command parameters
// btLibMeStatusUserTerminated ........ other end terminated (user)
// btLibMeStatusLowResources .......... other end terminated (low resources)
// btLibMeStatusPowerOff .............. other end terminated (powering off)
// btLibMeStatusLocalTerminated ....... terminated by local host
// btLibMeStatusRepeatedAttempts ...... repeated attempts
// btLibMeStatusPairingNotAllowed ..... pairing not allowed
// btLibMeStatusUnknownLmpPDU ......... unknown LMP PDU
// btLibMeStatusUnsupportedRemote ..... unsupported Remote Feature
// btLibMeStatusScoOffsetRejected ..... SCO Offset Rejected
// btLibMeStatusScoIntervalRejected ... SCO Interval Rejected
// btLibMeStatusScoAirModeRejected .... SCO Air Mode Rejected
// btLibMeStatusInvalidLmpParam ....... invalid LMP Parameters
// btLibMeStatusUnspecifiedError ...... inspecified Error
// btLibMeStatusUnsupportedLmpParam ... unsupported LMP Parameter Value
// btLibMeStatusRoleChangeNotAllowed .. can't perform master/slave role switch
// btLibMeStatusLmpResponseTimeout .... timeout waiting for LMP response
// btLibMeStatusLmpTransdCollision ....
// btLibMeStatusLmpPduNotAllowed ......
//
// The following come back in the status field of btLibSocketEventDisconnected.
// They give the reasone for the disconnection.
//
// btLibL2DiscReasonUnknown ........... unknown reason
// btLibL2DiscUserRequest ............. requested by local or remote device
// btLibL2DiscRequestTimeout .......... an L2CAP request timed out
// btLibL2DiscLinkDisc ................ underlying ACL Link disconnected
// btLibL2DiscQosViolation ............ Quality Of Service violation
// btLibL2DiscSecurityBlock ........... local security mgr refused connection
// btLibL2DiscConnPsmUnsupported ...... remote device does not support the
//                                      requested protocol service (PSM)
// btLibL2DiscConnSecurityBlock ....... remote security mgr refused connection
// btLibL2DiscConnNoResources ......... remote device is out of resources
// btLibL2DiscConfigUnacceptable ...... configuration failed (bad parameters)
// btLibL2DiscConfigReject ............ configuration rejected (unknown reason)
// btLibL2DiscConfigOptions ........... configuration failed (unrecognized
//                                      configuration option)
// Still more error codes
//
// btLibErrNoPiconet .................. piconet required for this operation
// btLibErrRoleChange ................. could not perform M/S switch
// btLibErrSdpNotMapped ............... invalid operation on unmapped record
// btLibErrAlreadyConnected ........... connection already in place
// btLibErrStackNotOpen ............... couldn't open the stack
// btLibErrBatteryTooLow .............. battery too low to perform the operation
// btLibErrNotFound ................... requested value was not found
// btLibNoAdminDaemon ................. daemon has not opened the admin device
// btLibErrGoepPacketTooSmall ......... packet is too small
// btLibErrGoepInvalidObjectHandle .... invalid object store handle
// btLibErrGoepTpDisconnect ........... transport is disconnected
// btLibErrGoepNotSupported ........... feature not supported
// btLibErrGoepNoSession .............. no active reliable session
// btLibErrBipInUse ................... operation failed - already in use
// btLibErrNotYetSupported ............ unsupported feature
// btLibErrError ...................... generic, non-specific error

#define btLibErrNoError                   0
#define btLibErrNotOpen                   (blthErrorClass|0x01)
#define btLibErrBluetoothOff              (blthErrorClass|0x02)
#define btLibErrNoPrefs                   (blthErrorClass|0x03)
#define btLibErrAlreadyOpen               (blthErrorClass|0x04)
#define btLibErrOutOfMemory               (blthErrorClass|0x05)
#define btLibErrFailed                    (blthErrorClass|0x06)
#define btLibErrInProgress                (blthErrorClass|0x07)
#define btLibErrParamError                (blthErrorClass|0x08)
#define btLibErrTooMany                   (blthErrorClass|0x09)
#define btLibErrPending                   (blthErrorClass|0x0A)
#define btLibErrNotInProgress             (blthErrorClass|0x0B)
#define btLibErrRadioInitFailed           (blthErrorClass|0x0C)
#define btLibErrRadioFatal                (blthErrorClass|0x0D)
#define btLibErrRadioInitialized          (blthErrorClass|0x0E)
#define btLibErrRadioSleepWake            (blthErrorClass|0x0F)
#define btLibErrNoConnection              (blthErrorClass|0x10)
#define btLibErrAlreadyRegistered         (blthErrorClass|0x11)
#define btLibErrNoAclLink                 (blthErrorClass|0x12)
#define btLibErrSdpRemoteRecord           (blthErrorClass|0x13)
#define btLibErrSdpAdvertised             (blthErrorClass|0x14)
#define btLibErrSdpFormat                 (blthErrorClass|0x15)
#define btLibErrSdpNotAdvertised          (blthErrorClass|0x16)
#define btLibErrSdpQueryVersion           (blthErrorClass|0x17)
#define btLibErrSdpQueryHandle            (blthErrorClass|0x18)
#define btLibErrSdpQuerySyntax            (blthErrorClass|0x19)
#define btLibErrSdpQueryPduSize           (blthErrorClass|0x1A)
#define btLibErrSdpQueryContinuation      (blthErrorClass|0x1B)
#define btLibErrSdpQueryResources         (blthErrorClass|0x1C)
#define btLibErrSdpQueryDisconnect        (blthErrorClass|0x1D)
#define btLibErrSdpInvalidResponse        (blthErrorClass|0x1E)
#define btLibErrSdpAttributeNotSet        (blthErrorClass|0x1F)
#define btLibErrSdpMapped                 (blthErrorClass|0x20)
#define btLibErrSocket                    (blthErrorClass|0x21)
#define btLibErrSocketProtocol            (blthErrorClass|0x22)
#define btLibErrSocketRole                (blthErrorClass|0x23)
#define btLibErrSocketPsmUnavailable      (blthErrorClass|0x24)
#define btLibErrSocketChannelUnavailable  (blthErrorClass|0x25)
#define btLibErrSocketUserDisconnect      (blthErrorClass|0x26)
#define btLibErrCanceled                  (blthErrorClass|0x27)
#define btLibErrBusy                      (blthErrorClass|0x28)
#define btLibMeStatusUnknownHciCommand    (blthErrorClass|0x29)
#define btLibMeStatusNoConnection         (blthErrorClass|0x2A)
#define btLibMeStatusHardwareFailure      (blthErrorClass|0x2B)
#define btLibMeStatusPageTimeout          (blthErrorClass|0x2C)
#define btLibMeStatusAuthenticateFailure  (blthErrorClass|0x2D)
#define btLibMeStatusMissingKey           (blthErrorClass|0x2E)
#define btLibMeStatusMemoryFull           (blthErrorClass|0x2F)
#define btLibMeStatusConnnectionTimeout   (blthErrorClass|0x30)
#define btLibMeStatusMaxConnections       (blthErrorClass|0x31)
#define btLibMeStatusMaxScoConnections    (blthErrorClass|0x32)
#define btLibMeStatusMaxAclConnections    (blthErrorClass|0x33)
#define btLibMeStatusCommandDisallowed    (blthErrorClass|0x34)
#define btLibMeStatusLimitedResources     (blthErrorClass|0x35)
#define btLibMeStatusSecurityError        (blthErrorClass|0x36)
#define btLibMeStatusPersonalDevice       (blthErrorClass|0x37)
#define btLibMeStatusHostTimeout          (blthErrorClass|0x38)
#define btLibMeStatusUnsupportedFeature   (blthErrorClass|0x39)
#define btLibMeStatusInvalidHciParam      (blthErrorClass|0x3A)
#define btLibMeStatusUserTerminated       (blthErrorClass|0x3B)
#define btLibMeStatusLowResources         (blthErrorClass|0x3C)
#define btLibMeStatusPowerOff             (blthErrorClass|0x3D)
#define btLibMeStatusLocalTerminated      (blthErrorClass|0x3E)
#define btLibMeStatusRepeatedAttempts     (blthErrorClass|0x3F)
#define btLibMeStatusPairingNotAllowed    (blthErrorClass|0x40)
#define btLibMeStatusUnknownLmpPDU        (blthErrorClass|0x41)
#define btLibMeStatusUnsupportedRemote    (blthErrorClass|0x42)
#define btLibMeStatusScoOffsetRejected    (blthErrorClass|0x43)
#define btLibMeStatusScoIntervalRejected  (blthErrorClass|0x44)
#define btLibMeStatusScoAirModeRejected   (blthErrorClass|0x45)
#define btLibMeStatusInvalidLmpParam      (blthErrorClass|0x46)
#define btLibMeStatusUnspecifiedError     (blthErrorClass|0x47)
#define btLibMeStatusUnsupportedLmpParam  (blthErrorClass|0x48)
#define btLibMeStatusRoleChangeNotAllowed (blthErrorClass|0x49)
#define btLibMeStatusLmpResponseTimeout   (blthErrorClass|0x4A)
#define btLibMeStatusLmpTransdCollision   (blthErrorClass|0x4B)
#define btLibMeStatusLmpPduNotAllowed     (blthErrorClass|0x4C)
#define btLibL2DiscReasonUnknown          (blthErrorClass|0x4D)
#define btLibL2DiscUserRequest            (blthErrorClass|0x4E)
#define btLibL2DiscRequestTimeout         (blthErrorClass|0x4F)
#define btLibL2DiscLinkDisc               (blthErrorClass|0x50)
#define btLibL2DiscQosViolation           (blthErrorClass|0x51)
#define btLibL2DiscSecurityBlock          (blthErrorClass|0x52)
#define btLibL2DiscConnPsmUnsupported     (blthErrorClass|0x53)
#define btLibL2DiscConnSecurityBlock      (blthErrorClass|0x54)
#define btLibL2DiscConnNoResources        (blthErrorClass|0x55)
#define btLibL2DiscConfigUnacceptable     (blthErrorClass|0x56)
#define btLibL2DiscConfigReject           (blthErrorClass|0x57)
#define btLibL2DiscConfigOptions          (blthErrorClass|0x58)
#define btLibServiceShutdownAppUse        (blthErrorClass|0x59)
#define btLibServiceShutdownPowerCycled   (blthErrorClass|0x5A)
#define btLibServiceShutdownAclDrop       (blthErrorClass|0x5B)
#define btLibServiceShutdownTimeout       (blthErrorClass|0x5C)
#define btLibServiceShutdownDetached      (blthErrorClass|0x5D)
#define btLibErrInUseByService            (blthErrorClass|0x5E)
#define btLibErrNoPiconet                 (blthErrorClass|0x5F)
#define btLibErrRoleChange                (blthErrorClass|0x60)
#define btLibErrSdpNotMapped              (blthErrorClass|0x61)
#define btLibErrAlreadyConnected          (blthErrorClass|0x62)
#define btLibErrStackNotOpen              (blthErrorClass|0x63)
#define btLibErrBatteryTooLow             (blthErrorClass|0x64)
#define btLibErrNotFound                  (blthErrorClass|0x65)
#define btLibErrNoAdminDaemon             (blthErrorClass|0x66)
#define btLibErrGoepPacketTooSmall        (blthErrorClass|0x66)
#define btLibErrGoepInvalidObjectHandle   (blthErrorClass|0x67)
#define btLibErrGoepTpDisconnect          (blthErrorClass|0x68)
#define btLibErrGoepNotSupported          (blthErrorClass|0x69)
#define btLibErrGoepNoSession             (blthErrorClass|0x70)
#define btLibErrBipInUse                  (blthErrorClass|0x71)
#define btLibNotYetSupported              (blthErrorClass|0xEF)
#define btLibErrError                     (blthErrorClass|0xFF)


/********************************************************************
 * Names of the Bluetooth STREAMS devices and modules
 ********************************************************************/

#define btDevMeName        "BtME"         // Management Entity device
#define btDevL2cName       "BtL2C"        // L2CAP device
#define btDevRfcName       "BtRFC"        // RFCOMM device
#define btDevSdpName       "BtSDP"        // SDP device
#define btDevSCOName       "BtSCO"        // SCO device
#define btDevBNEPName      "BtBNEP"       // BNEP device
#define btModSerL2cName    "BtSerL2C"     // Serial-on-L2CAP module
#define btModSerRfcName    "BtSerRFC"     // Serial-on-RFCOMM module
#define btModTPISerRfcName "BtTpiSerRFC"  // TPI-on-Serial-on-RFCOMM module


/********************************************************************
 * Management Entity
 ********************************************************************/

//--------------------------------------
// Bluetooth Device Address

#define btLibDeviceAddressSize        6  // size of a device address in binary form
#define btLibDeviceAddressStringSize 18  // size of a device address in string form
                                         // (counting the terminal null char)

typedef struct BtLibDeviceAddressType
{
    uint8_t    address[btLibDeviceAddressSize];
}
BtLibDeviceAddressType;

typedef BtLibDeviceAddressType* BtLibDeviceAddressTypePtr;

//--------------------------------------
// Link preferences, role.
//
typedef enum
{
    btLibLinkPref_Authenticated,
    btLibLinkPref_Encrypted,
    btLibLinkPref_LinkRole
}
BtLibLinkPrefsEnumTag;

typedef Enum8 BtLibLinkPrefsEnum;

typedef enum
{
    btLibMasterRole,
    btLibSlaveRole
}
BtLibConnectionRoleEnumTag;

typedef Enum8 BtLibConnectionRoleEnum;

//--------------------------------------
// How to use the cache when calling BtLibGetRemoteDeviceName()

typedef enum
{
    btLibCachedThenRemote, // look in the cache; if that fails query the remote
    btLibCachedOnly,       // only look in the cache
    btLibRemoteOnly        // only query the remote
}
BtLibGetNameEnumTag;

typedef Enum8 BtLibGetNameEnum;

// The BtLibGeneralPrefEnum is used to get and set general bluetooth preferences
//
typedef enum
{
    btLibPref_Name,
    btLibPref_UnconnectedAccessible,
    btLibPref_CurrentAccessible,     // BtLibGetGeneralPreference only
    btLibPref_LocalClassOfDevice,
    btLibPref_LocalDeviceAddress,    // BtLibGetGeneralPreference only
    btLibPref_ChannelClassification
}
BtLibGeneralPrefEnumTag;

typedef Enum8 BtLibGeneralPrefEnum;

// The BtLibPinType is used to get/set the device PIN value
//
typedef struct BtLibPinType
{
   uint8_t* pinP;       // ptr to an array containg the pin value
   uint8_t  pinLength;  // number of bytes in the pin
   uint8_t  padding1;
   uint16_t padding2;
}
BtLibPinType;

// The BtLibFriendlyNameType is used to get/set a device's friendly name
// Note that the nameLength counts the terminal null character.

#define btLibMaxDeviceNameLength 249 // max device name length, including null

typedef struct BtLibFriendlyNameType
{
   uint8_t  nameLength;                     // length of the name, including null
   uint8_t  name[btLibMaxDeviceNameLength]; // buffer for null-terminated name
}
BtLibFriendlyNameType;

typedef BtLibFriendlyNameType* BtLibFriendlyNameTypePtr;

// The BtLibAccessibleModeEnum is used to get/set accessibility information
//
typedef enum
{
   btLibNotAccessible = 0x00,
   btLibConnectableOnly = 0x02,
   btLibDiscoverableAndConnectable = 0x03
}
BtLibAccessibleModeEnumTag;

typedef Enum8 BtLibAccessibleModeEnum;


// Mode of the acl link with a remote device.
// ONLY HOLD AND ACTIVE MODE CURRENTLY SUPPORTED
//
typedef enum
{
   btLibSniffMode,
   btLibHoldMode,
   btLibParkMode,
   btLibActiveMode
}
BtLibLinkModeEnumTag;

typedef Enum8 BtLibLinkModeEnum;

//------------------------------------------------------------------------------
// BtLibClassOfDeviceType
//
// Bit pattern representing the class of device along with the supported
// services. There can be more than one supported service. Service classes can
// be ORed together. The Device Class is composed of a major device class plus
// a minor device class. The class of device value is created by ORing together
// the service classes plus one major device class plus on minor device class.
// The minor device class is interpreted in the context of the major device
// class.
//
typedef uint32_t BtLibClassOfDeviceType;

// Group: Major Service Classes.(Can be ORed together)
#define btLibCOD_LimitedDiscoverableMode   0x00002000
#define btLibCOD_Reserved1                 0x00004000
#define btLibCOD_Reserved2                 0x00008000
#define btLibCOD_Positioning               0x00010000
#define btLibCOD_Networking                0x00020000
#define btLibCOD_Rendering                 0x00040000
#define btLibCOD_Capturing                 0x00080000
#define btLibCOD_ObjectTransfer            0x00100000
#define btLibCOD_Audio                     0x00200000
#define btLibCOD_Telephony                 0x00400000
#define btLibCOD_Information               0x00800000
#define btLibCOD_ServiceAny                0x00ffE000

// Group: Major Device Classes (Select one)
#define btLibCOD_Major_Misc                0x00000000
#define btLibCOD_Major_Computer            0x00000100
#define btLibCOD_Major_Phone               0x00000200
#define btLibCOD_Major_Lan_Access_Point    0x00000300
#define btLibCOD_Major_Audio               0x00000400
#define btLibCOD_Major_Peripheral          0x00000500
#define btLibCOD_Major_Imaging             0x00000600
#define btLibCOD_Major_Unclassified        0x00001F00
#define btLibCOD_Major_Any                 0x00001F00

#define btLibCOD_Minor_Any                 0x000000FC

// Group: Minor Device Class - Computer Major class (Select one)
#define btLibCOD_Minor_Comp_Unclassified   0x00000000
#define btLibCOD_Minor_Comp_Desktop        0x00000004
#define btLibCOD_Minor_Comp_Server         0x00000008
#define btLibCOD_Minor_Comp_Laptop         0x0000000C
#define btLibCOD_Minor_Comp_Handheld       0x00000010
#define btLibCOD_Minor_Comp_Palm           0x00000014
#define btLibCOD_Minor_Comp_Any            btLibCOD_Minor_Any

// Group: Minor Device Class - Phone Major class (Select one)
#define btLibCOD_Minor_Phone_Unclassified  0x00000000
#define btLibCOD_Minor_Phone_Cellular      0x00000004
#define btLibCOD_Minor_Phone_Cordless      0x00000008
#define btLibCOD_Minor_Phone_Smart         0x0000000C
#define btLibCOD_Minor_Phone_Modem         0x00000010
#define btLibCOD_Minor_Phone_ISDN          0x00000014
#define btLibCOD_Minor_Phone_Any           btLibCOD_Minor_Any

// Group: Minor Device Class - LAN Access Point Major class (Select one)
#define btLibCOD_Minor_Lan_0               0x00000000     // fully available
#define btLibCOD_Minor_Lan_17              0x00000020     // 1-17% utilized
#define btLibCOD_Minor_Lan_33              0x00000040     // 17-33% utilized
#define btLibCOD_Minor_Lan_50              0x00000060     // 33-50% utilized
#define btLibCOD_Minor_Lan_67              0x00000080     // 50-67% utilized
#define btLibCOD_Minor_Lan_83              0x000000A0     // 67-83% utilized
#define btLibCOD_Minor_Lan_99              0x000000C0     // 83-99% utilized
#define btLibCOD_Minor_Lan_NoService       0x000000E0     // 100% utilized
#define btLibCOD_Minor_Lan_Any             btLibCOD_Minor_Any

// Group: Minor Device Class - Audio Major class (Select one)
#define btLibCOD_Minor_Audio_Unclassified               0x00000000
#define btLibCOD_Minor_Audio_Headset                    0x00000004
#define btLibCOD_Minor_Audio_HandFree                   0x00000008
#define btLibCOD_Minor_Audio_MicroPhone                 0x00000010
#define btLibCOD_Minor_Audio_LoudSpeaker                0x00000014
#define btLibCOD_Minor_Audio_HeadPhone                  0x00000018
#define btLibCOD_Minor_Audio_PortableAudio              0x0000001C
#define btLibCOD_Minor_Audio_CarAudio                   0x00000020
#define btLibCOD_Minor_Audio_SetTopBox                  0x00000024
#define btLibCOD_Minor_Audio_HifiAudio                  0x00000028
#define btLibCOD_Minor_Audio_VCR                        0x0000002C
#define btLibCOD_Minor_Audio_VideoCamera                0x00000030
#define btLibCOD_Minor_Audio_CamCorder                  0x00000034
#define btLibCOD_Minor_Audio_VideoMonitor               0x00000038
#define btLibCOD_Minor_Audio_VideoDisplayAndLoudSpeaker 0x0000003C
#define btLibCOD_Minor_Audio_VideoConf                  0x00000040
#define btLibCOD_Minor_Audio_GameToy                    0x00000048
#define btLibCOD_Minor_Audio_Any                        btLibCOD_Minor_Any

// Group: Minor Device Class - Peripheral Major class (Select one)
#define btLibCOD_Minor_Peripheral_Unclassified  0x00000000
#define btLibCOD_Minor_Peripheral_Keyboard      0x00000040
#define btLibCOD_Minor_Peripheral_Pointing      0x00000080
#define btLibCOD_Minor_Peripheral_Combo         0x000000B0

// Group: Minor sub Device Class - Peripheral Major class (Select one)
#define btLibCOD_Minor_Peripheral_Joystick         0x00000004
#define btLibCOD_Minor_Peripheral_GamePad          0x00000008
#define btLibCOD_Minor_Peripheral_RemoteControl    0x0000000C
#define btLibCOD_Minor_Peripheral_Sensing          0x00000010
#define btLibCOD_Minor_Peripheral_DigitizerTablet  0x00000014
#define btLibCOD_Minor_Peripheral_CardReader       0x00000018

// Group: Minor Device Class - Imaging Major class (Select multiple is possible)
#define btLibCOD_Minor_Imaging_Unclassified  0x00000000
#define btLibCOD_Minor_Imaging_Display       0x00000010
#define btLibCOD_Minor_Imaging_Camera        0x00000020
#define btLibCOD_Minor_Imaging_Scanner       0x00000040
#define btLibCOD_Minor_Imaging_Printer       0x00000080

// Group: Masks used to isolate the class of device components
#define btLibCOD_Service_Mask              0x00ffE000
#define btLibCOD_Major_Mask                0x00001F00
#define btLibCOD_Minor_Mask                0x000000FC

//------------------------------------------------------------------------------
// Management Entity event codes.
//
typedef enum
{
    // The radio's state has changed.
    btLibManagementEventRadioState = 1,

    // A Remote Device has responded to an Inquiry.
    btLibManagementEventInquiryResult,

    // The inquiry process has completed.
    btLibManagementEventInquiryComplete,

    // The inquiry process has been cancelled.
    btLibManagementEventInquiryCanceled,

    // An ACL link has disconnected.
    btLibManagementEventACLDisconnect,

    // A remote device has established an ACL link to us.
    btLibManagementEventACLConnectInbound,

    // We have established an ACL link to a remote device.
    btLibManagementEventACLConnectOutbound,

    // The piconet has been set up.
    btLibManagementEventPiconetCreated,

    // The piconet has been destroyed.
    btLibManagementEventPiconetDestroyed,

    // Mode change (Hold, Park, Sniff, or Active)
    btLibManagementEventModeChange,

    // Accessibility change (do we accept connections, respond to Inquiries?).
    btLibManagementEventAccessibilityChange,

    // A link has enabled or disabled encryption
    btLibManagementEventEncryptionChange,

    // A link has switched its master/slave role
    btLibManagementEventRoleChange,

    // A remote name request has completed.
    btLibManagementEventNameResult,

    // A local name change has occurred
    btLibManagementEventLocalNameChange,

    // A remote device authentication is complete.
    btLibManagementEventAuthenticationComplete,

    // A passkey request is received. The event is for informational purposes
    // only; the OS will handle the request. Because this can happen during or
    // after link establishment, you may want to consider disabling any failure
    // timers while the passkey dialog is up. The requestComplete event signals
    // the completion of the passkey entry.
    btLibManagementEventPasskeyRequest,

    // A passkey request was processed by the OS. The status code for this event
    // will tell you whether the passkey was entered or or cancelled. This event
    // does NOT tell you that the authentication completed.
    btLibManagementEventPasskeyRequestComplete,

    // Pairing process is complete and the link is authenticated.
    btLibManagementEventPairingComplete,

    // Receiver Signal Strength Indicator (not used yet)
    btLibManagementEventRSSI,

    // End of temporary discoverability
    btLibManagementEventTempDiscovEnd,
}
BtLibManagementEventEnumTag;

typedef Enum8 BtLibManagementEventEnum;

//------------------------------------------------------------------------------
// BtLibManagementEventType -- Management Entity event object.
//
// Applications obtain ME events by calling IOSGetmsg() on a file descriptor
// opened to an ME device. The "control part" of the message thus obtained
// contains a BtLibManagementEventType object. For certain events, there is
// also a "data part" containing additional information.
//
typedef struct _BtLibManagementEventType
{
    //--------------------------------------------------------------------------
    // event -- the basic op code for the event.
    //
    // Events: All
    //
    BtLibManagementEventEnum  event;
    uint8_t                   padding1;
    uint16_t                  padding2;

    //--------------------------------------------------------------------------
    // status -- error code for the event
    //
    // Events: All
    //
    status_t  status;

    union
    {
        //----------------------------------------------------------------------
        // bdAddr -- Bluetooth device address
        //
        // Events: btLibManagementEventACLConnectInbound
        //         btLibManagementEventACLConnectOutbound
        //         btLibManagementEventACLDisconnect
        //         btLibManagementEventAuthenticationComplete
        //
        BtLibDeviceAddressType  bdAddr;

        //----------------------------------------------------------------------
        // accessible -- radio's accessibility state
        //
        // Events: btLibManagementEventAccessibilityChange
        //
        BtLibAccessibleModeEnum  accessible;

        //----------------------------------------------------------------------
        // nameResult -- Bluetooth device's address and friendly name
        //
        // Events: btLibManagementEventNameResult
        //         btLibManagementEventLocalNameChange
        //
        // NOTE: the "data part" of the message contains a structure of type
        // BtLibFriendlyNameType.
        //
        struct
        {
            BtLibDeviceAddressType  bdAddr;
        }
        nameResult;

        //----------------------------------------------------------------------
        // inquiryResult -- information about a single device found during an
        // Inquiry procedure.
        //
        // Events: btLibManagementEventInquiryResult
        //
        // NOTE: the "data part" of the message contains a structure of type
        // BtLibFriendlyNameType whose contents are the remote device's name
        // according to the local device name cache; if the name is not in the
        // cache, the returned name is a null (zero-length) string.
        //
        struct
        {
            BtLibDeviceAddressType  bdAddr;
            uint16_t                padding;
            BtLibClassOfDeviceType  classOfDevice;
        }
        inquiryResult;

        //----------------------------------------------------------------------
        // modeChange -- Bluetooth device address identifying an ACL link whose
        // mode has changed, the new current mode, and the length of time to be
        // in that mode if applicable.
        //
        // Events: btLibManagementEventModeChange
        //
        struct
        {
            BtLibDeviceAddressType  bdAddr;
            BtLibLinkModeEnum       curMode;
            uint8_t                 padding;
            uint16_t                interval;
        }
        modeChange;

        //----------------------------------------------------------------------
        // encryptionChange -- Bluetooth device address identifying an ACL link
        // whose encryptedness has changed, and the new encryptedness state.
        //
        // Events: btLibManagementEventEncryptionChange
        //
        struct
        {
            BtLibDeviceAddressType  bdAddr;
            Boolean                 enabled;
        }
        encryptionChange;

        //----------------------------------------------------------------------
        // roleChange -- Bluetooth device address identifying an ACL link, and
        // the new role of the local device on that link.
        //
        // Events: btLibManagementEventRoleChange
        //
        struct
        {
            BtLibDeviceAddressType   bdAddr;
            BtLibConnectionRoleEnum  newRole;
        }
        roleChange;

        //----------------------------------------------------------------------
        // rssi (receiver signal strength indicator) -- indicates whether the
        // receiver signal strength is below (negative), within (zero), or above
        // (positive) the "Golden Receive Power Range", in units of one decibel.
        // Not used yet.
        //
        // Events: btLibManagementEventRSSI
        //
        struct
        {
            BtLibDeviceAddressType  bdAddr;
            int8_t                  rssi;

        }
        rssi;
    }
    eventData;
}
BtLibManagementEventType;

/********************************************************************
 * L2CAP
 ********************************************************************/

typedef uint16_t BtLibL2CapPsmType;
typedef uint16_t BtLibL2CapChannelIdType;

#define BT_L2CAP_RANDOM_PSM 0xFFFF
#define BT_L2CAP_MTU 672


/********************************************************************
 * RFCOMM
 ********************************************************************/

typedef uint8_t BtLibRfCommServerIdType;
#define BT_RF_MAX_FRAMESIZE (BT_L2CAP_MTU - 5)
#define BT_RF_MIN_FRAMESIZE 23
#define BT_RF_DEFAULT_FRAMESIZE 127

/********************************************************************
 * SDP
 ********************************************************************/

typedef MemHandle BtLibSdpRecordHandle;
typedef uint32_t    BtLibSdpRemoteServiceRecordHandle;

//------------------------------------------------------------------------------
// BtLibSdpUuidType -- universally unique identifier.
//
// In reality, all UUIDs are 128 bits. As an optimization, the Bluetooth special
// interest group reserved a range of UUIDs, and defined and encoding of those
// reserved UUIDs in 16 and 32 bit values.

typedef enum
{
    btLibUuidSize16  = 2,  // 16 bits is 2 bytes ...
    btLibUuidSize32  = 4,  // 32 bits is 4 bytes ...
    btLibUuidSize128 = 16  // 128 bits is 16 bytes ...
}
BtLibSdpUuidSizeEnumTag;

typedef Enum8 BtLibSdpUuidSizeEnum;

typedef struct BtLibSdpUuidType
{
    BtLibSdpUuidSizeEnum  size;
    uint8_t                 UUID[16];
}
BtLibSdpUuidType;

//------------------------------------------------------------------------------
// Macro for initializing a BtLibSdpUuidType variable, given a raw UUID value.
// Example usage:
//
// BtLibSdpUuidType myUuid;
// BtLibSdpUuidInitialize(myUuid, btLibSdpUUID_SC_SERIAL_PORT, btLibUuidSize16);
//
#define BtLibSdpUuidInitialize( uuidVar, rawValue, uuidSize ) \
    do { \
        (uuidVar).size = (uuidSize); \
        memmove( (uuidVar).UUID, (rawValue), (uuidVar).size ); \
    } while ( 0 )

//------------------------------------------------------------------------------
// Raw values for various UUIDs pre-defined by the Bluetooth spec.

// Service Class UUIDs
//
#define btLibSdpUUID_SC_SERVICE_DISCOVERY_SERVER    "\x10\x00"
#define btLibSdpUUID_SC_BROWSE_GROUP_DESC           "\x10\x01"
#define btLibSdpUUID_SC_PUBLIC_BROWSE_GROUP         "\x10\x02"
#define btLibSdpUUID_SC_SERIAL_PORT                 "\x11\x01"
#define btLibSdpUUID_SC_LAN_ACCESS_PPP              "\x11\x02"
#define btLibSdpUUID_SC_DIALUP_NETWORKING           "\x11\x03"
#define btLibSdpUUID_SC_IRMC_SYNC                   "\x11\x04"
#define btLibSdpUUID_SC_OBEX_OBJECT_PUSH            "\x11\x05"
#define btLibSdpUUID_SC_OBEX_FILE_TRANSFER          "\x11\x06"
#define btLibSdpUUID_SC_IRMC_SYNC_COMMAND           "\x11\x07"
#define btLibSdpUUID_SC_HEADSET                     "\x11\x08"
#define btLibSdpUUID_SC_CORDLESS_TELEPHONY          "\x11\x09"
#define btLibSdpUUID_SC_AUDIO_SOURCE                "\x11\x0A"
#define btLibSdpUUID_SC_AUDIO_SINK                  "\x11\x0B"
#define btLibSdpUUID_SC_AV_REMOTE_CONTROL_TARGET    "\x11\x0C"
#define btLibSdpUUID_SC_ADVANCED_AUDIO_DISTRIBUTION "\x11\x0D"
#define btLibSdpUUID_SC_AV_REMOTE_CONTROL           "\x11\x0E"
#define btLibSdpUUID_SC_VIDEO_CONFERENCING          "\x11\x0F"
#define btLibSdpUUID_SC_INTERCOM                    "\x11\x10"
#define btLibSdpUUID_SC_FAX                         "\x11\x11"
#define btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY       "\x11\x12"
#define btLibSdpUUID_SC_WAP                         "\x11\x13"
#define btLibSdpUUID_SC_WAP_CLIENT                  "\x11\x14"
#define btLibSdpUUID_SC_PANU                        "\x11\x15"
#define btLibSdpUUID_SC_NAP                         "\x11\x16"
#define btLibSdpUUID_SC_GN                          "\x11\x17"
#define btLibSdpUUID_SC_DIRECT_PRINTING             "\x11\x18"
#define btLibSdpUUID_SC_REFERENCE_PRINTING          "\x11\x19"
#define btLibSdpUUID_SC_IMAGING                     "\x11\x1A"
#define btLibSdpUUID_SC_IMAGING_RESPONDER           "\x11\x1B"
#define btLibSdpUUID_SC_IMAGING_AUTOMATIC_ARCHIVE   "\x11\x1C"
#define btLibSdpUUID_SC_IMAGING_REFERENCED_OBJECTS  "\x11\x1D"
#define btLibSdpUUID_SC_HANDSFREE                   "\x11\x1E"
#define btLibSdpUUID_SC_HANDSFREE_AUDIO_GATEWAY     "\x11\x1F"
#define btLibSdpUUID_SC_DIRECT_PRINTING_REF_OBJ     "\x11\x20"
#define btLibSdpUUID_SC_REFLECTED_UI                "\x11\x21"
#define btLibSdpUUID_SC_BASIC_PRINTING              "\x11\x22"
#define btLibSdpUUID_SC_PRINTING_STATUS             "\x11\x23"
#define btLibSdpUUID_SC_HUMAN_INTERFACE_DEVICE      "\x11\x24"
#define btLibSdpUUID_SC_HARDCOPY_CABLE_REPLACEMENT  "\x11\x25"
#define btLibSdpUUID_SC_HCR_PRINT                   "\x11\x26"
#define btLibSdpUUID_SC_HCR_SCAN                    "\x11\x27"
#define btLibSdpUUID_SC_COMMON_ISDN_ACCESS          "\x11\x28"
#define btLibSdpUUID_SC_VIDEO_CONFERENCING_GW       "\x11\x29"
#define btLibSdpUUID_SC_UDI_MT                      "\x11\x2A"
#define btLibSdpUUID_SC_UDI_TA                      "\x11\x2B"
#define btLibSdpUUID_SC_AUDIO_VIDEO                 "\x11\x2C"
#define btLibSdpUUID_SC_SIM_ACCESS                  "\x11\x2D"
#define btLibSdpUUID_SC_PNP_INFORMATION             "\x12\x00"
#define btLibSdpUUID_SC_GENERIC_NETWORKING          "\x12\x01"
#define btLibSdpUUID_SC_GENERIC_FILE_TRANSFER       "\x12\x02"
#define btLibSdpUUID_SC_GENERIC_AUDIO               "\x12\x03"
#define btLibSdpUUID_SC_GENERIC_TELEPHONY           "\x12\x04"
#define btLibSdpUUID_SC_UPNP_SERVICE                "\x12\x05"
#define btLibSdpUUID_SC_UPNP_IP_SERVICE             "\x12\x06"
#define btLibSdpUUID_SC_ESDP_UPNP_IP_PAN            "\x13\x00"
#define btLibSdpUUID_SC_ESDP_UPNP_IP_LAP            "\x13\x01"
#define btLibSdpUUID_SC_ESDP_UPNP_L2CAP             "\x13\x02"

// Protocol UUIDs
//
#define btLibSdpUUID_PROT_SDP                       "\x00\x01"
#define btLibSdpUUID_PROT_UDP                       "\x00\x02"
#define btLibSdpUUID_PROT_RFCOMM                    "\x00\x03"
#define btLibSdpUUID_PROT_TCP                       "\x00\x04"
#define btLibSdpUUID_PROT_TCS_BIN                   "\x00\x05"
#define btLibSdpUUID_PROT_TCS_AT                    "\x00\x06"
#define btLibSdpUUID_PROT_OBEX                      "\x00\x08"
#define btLibSdpUUID_PROT_IP                        "\x00\x09"
#define btLibSdpUUID_PROT_FTP                       "\x00\x0A"
#define btLibSdpUUID_PROT_HTTP                      "\x00\x0C"
#define btLibSdpUUID_PROT_WSP                       "\x00\x0E"
#define btLibSdpUUID_PROT_BNEP                      "\x00\x0F"
#define btLibSdpUUID_PROT_UPNP                      "\x00\x10"
#define btLibSdpUUID_PROT_HIDP                      "\x00\x11"
#define btLibSdpUUID_PROT_HARDCOPY_CONTROL_CHANNEL  "\x00\x12"
#define btLibSdpUUID_PROT_HARDCOPY_DATA_CHANNEL     "\x00\x14"
#define btLibSdpUUID_PROT_HARDCOPY_NOTIFICATION     "\x00\x16"
#define btLibSdpUUID_PROT_AVCTP                     "\x00\x17"
#define btLibSdpUUID_PROT_AVDTP                     "\x00\x19"
#define btLibSdpUUID_PROT_CMTP                      "\x00\x1B"
#define btLibSdpUUID_PROT_UDI_C_PLANE               "\x00\x1D"
#define btLibSdpUUID_PROT_L2CAP                     "\x01\x00"

//------------------------------------------------------------------------------
// Language Base Triplet -- specifies what natural language, and what character
// encoding for that natural language, are used for certain attributes.
//
typedef struct BtLibLanguageBaseTripletType
{
   uint16_t  naturalLanguageIdentifier; // see btLibLangXxx defines below
   uint16_t  characterEncoding;         // see btLibCharSet_Xxx defines below
   uint16_t  baseAttributeID;
}
BtLibLanguageBaseTripletType;

//------------------------------------------------------------------------------
// Natural Language Indentifiers from the ISO 639:1988 specification.
//
#define btLibLangAfar            'aa'
#define btLibLangAbkihazian      'ab'
#define btLibLangAfrikaans       'af'
#define btLibLangAmharic         'am'
#define btLibLangArabic          'ar'
#define btLibLangAssamese        'as'
#define btLibLangAymara          'ay'
#define btLibLangAzerbaijani     'az'
#define btLibLangBashkir         'ba'
#define btLibLangByelorussian    'be'
#define btLibLangBulgarian       'bg'
#define btLibLangBihari          'bh'
#define btLibLangBislama         'bi'
#define btLibLangBengali         'bn'
#define btLibLangTibetan         'bo'
#define btLibLangBreton          'br'
#define btLibLangCatalan         'ca'
#define btLibLangCorsican        'co'
#define btLibLangCzech           'cs'
#define btLibLangWelsh           'cy'
#define btLibLangDanish          'da'
#define btLibLangGerman          'de'
#define btLibLangBhutani         'dz'
#define btLibLangGreek           'el'
#define btLibLangEnglish         'en'
#define btLibLangEsperanto       'eo'
#define btLibLangSpanish         'es'
#define btLibLangEstonian        'et'
#define btLibLangBasque          'eu'
#define btLibLangPersian         'fa'
#define btLibLangFinnish         'fi'
#define btLibLangFiji            'fj'
#define btLibLangFaroese         'fo'
#define btLibLangFrench          'fr'
#define btLibLangFrisian         'fy'
#define btLibLangIrish           'ga'
#define btLibLangScotsGaelic     'gd'
#define btLibLangGalician        'gl'
#define btLibLangGuarani         'gn'
#define btLibLangGujarati        'gu'
#define btLibLangHausa           'ha'
#define btLibLangHindi           'hi'
#define btLibLangCroation        'hr'
#define btLibLangHungarian       'hu'
#define btLibLangArmenian        'hy'
#define btLibLangInterlingua     'ia'
#define btLibLangInterlingue     'ie'
#define btLibLangInupiak         'ik'
#define btLibLangIndonesian      'in'
#define btLibLangIcelandic       'is'
#define btLibLangItalian         'it'
#define btLibLangHebrew          'iw'
#define btLibLangJapanese        'ja'
#define btLibLangYiddish         'ji'
#define btLibLangJavanese        'jw'
#define btLibLangGeorgian        'ka'
#define btLibLangKazakh          'kk'
#define btLibLangGreenlandic     'kl'
#define btLibLangCambodian       'km'
#define btLibLangKannada         'kn'
#define btLibLangKorean          'ko'
#define btLibLangKashmiri        'ks'
#define btLibLangKurdish         'ku'
#define btLibLangKirghiz         'ky'
#define btLibLangLatin           'la'
#define btLibLangLingala         'ln'
#define btLibLangLaothian        'lo'
#define btLibLangLithuanian      'lt'
#define btLibLangLatvian         'lv'
#define btLibLangMalagasy        'mg'
#define btLibLangMaori           'mi'
#define btLibLangMacedonian      'mk'
#define btLibLangMalayalam       'ml'
#define btLibLangMongolian       'mn'
#define btLibLangMoldavian       'mo'
#define btLibLangMarathi         'mr'
#define btLibLangMalay           'ms'
#define btLibLangMaltese         'mt'
#define btLibLangBurmese         'my'
#define btLibLangNaura           'na'
#define btLibLangNepali          'ne'
#define btLibLangDutch           'nl'
#define btLibLangNorwegian       'no'
#define btLibLangOccitan         'oc'
#define btLibLangOromo           'om'
#define btLibLangOriya           'or'
#define btLibLangPunjabi         'pa'
#define btLibLangPolish          'pl'
#define btLibLangPashto          'ps'
#define btLibLangPortuguese      'pt'
#define btLibLangQuechua         'qu'
#define btLibLangRhaeto_Romance  'rm'
#define btLibLangKirundi         'rn'
#define btLibLangRomanian        'ro'
#define btLibLangRussian         'ru'
#define btLibLangKinyarwanda     'rw'
#define btLibLangSanskrit        'sa'
#define btLibLangSindhi          'sd'
#define btLibLangSangho          'sg'
#define btLibLangSerbo_Croation  'sh'
#define btLibLangSinghalese      'si'
#define btLibLangSlovak          'sk'
#define btLibLangSlovenian       'sl'
#define btLibLangSamoan          'sm'
#define btLibLangShona           'sn'
#define btLibLangSomali          'so'
#define btLibLangAlbanian        'sq'
#define btLibLangSerbian         'sr'
#define btLibLangSiswati         'ss'
#define btLibLangSesotho         'st'
#define btLibLangSundanese       'su'
#define btLibLangSwedish         'sv'
#define btLibLangSwahili         'sw'
#define btLibLangTamil           'ta'
#define btLibLangTelugu          'te'
#define btLibLangTajik           'tg'
#define btLibLangThai            'th'
#define btLibLangTigrinya        'ti'
#define btLibLangTurkmen         'tk'
#define btLibLangTagalog         'tl'
#define btLibLangSetswanna       'tn'
#define btLibLangTonga           'to'
#define btLibLangTurkish         'tr'
#define btLibLangTsonga          'ts'
#define btLibLangTatar           'tt'
#define btLibLangTwi             'tw'
#define btLibLangUkranian        'uk'
#define btLibLangUrdu            'ur'
#define btLibLangUzbek           'uz'
#define btLibLangVietnamese      'vi'
#define btLibLangVolapuk         'vo'
#define btLibLangWolof           'wo'
#define btLibLangXhosa           'xh'
#define btLibLangYoruba          'yo'
#define btLibLangChinese         'zh'
#define btLibLangZulu            'zu'

//------------------------------------------------------------------------------
// Character encodings.
// http://www.isi.edu/in-notes/iana/assignments/character-sets
//
#define btLibCharSet_US_ASCII 3
#define btLibCharSet_ISO_10646_UTF_1 27
#define btLibCharSet_ISO_646_basic_1983 28
#define btLibCharSet_INVARIANT 29
#define btLibCharSet_ISO_646_irv_1983 30
#define btLibCharSet_BS_4730 20
#define btLibCharSet_NATS_SEFI 31
#define btLibCharSet_NATS_SEFI_ADD 32
#define btLibCharSet_NATS_DANO 33
#define btLibCharSet_NATS_DANO_ADD 34
#define btLibCharSet_SEN_850200_B 35
#define btLibCharSet_SEN_850200_C 21
#define btLibCharSet_KS_C_5601_1987 36
#define btLibCharSet_ISO_2022_KR 37
#define btLibCharSet_EUC_KR 38
#define btLibCharSet_ISO_2022_JP 39
#define btLibCharSet_ISO_2022_JP_2 40
#define btLibCharSet_ISO_2022_CN 104
#define btLibCharSet_ISO_2022_CN_EXT 105
#define btLibCharSet_JIS_C6220_1969_jp 41
#define btLibCharSet_JIS_C6220_1969_ro 42
#define btLibCharSet_IT 22
#define btLibCharSet_PT 43
#define btLibCharSet_ES 23
#define btLibCharSet_greek7_old 44
#define btLibCharSet_latin_greek 45
#define btLibCharSet_DIN_66003 24
#define btLibCharSet_NF_Z_62_010__1973_ 46
#define btLibCharSet_Latin_greek_1 47
#define btLibCharSet_ISO_5427 48
#define btLibCharSet_JIS_C6226_1978 49
#define btLibCharSet_BS_viewdata 50
#define btLibCharSet_INIS 51
#define btLibCharSet_INIS_8 52
#define btLibCharSet_INIS_cyrillic 53
#define btLibCharSet_ISO_5427_1981 54
#define btLibCharSet_ISO_5428_1980 55
#define btLibCharSet_GB_1988_80 56
#define btLibCharSet_GB_2312_80 57
#define btLibCharSet_NS_4551_1 25
#define btLibCharSet_NS_4551_2 58
#define btLibCharSet_NF_Z_62_010 26
#define btLibCharSet_videotex_suppl 59
#define btLibCharSet_PT2 60
#define btLibCharSet_ES2 61
#define btLibCharSet_MSZ_7795_3 62
#define btLibCharSet_JIS_C6226_1983 63
#define btLibCharSet_greek7 64
#define btLibCharSet_ASMO_449 65
#define btLibCharSet_iso_ir_90 66
#define btLibCharSet_JIS_C6229_1984_a 67
#define btLibCharSet_JIS_C6229_1984_b 68
#define btLibCharSet_JIS_C6229_1984_b_add 69
#define btLibCharSet_JIS_C6229_1984_hand 70
#define btLibCharSet_JIS_C6229_1984_hand_add 71
#define btLibCharSet_JIS_C6229_1984_kana 72
#define btLibCharSet_ISO_2033_1983 73
#define btLibCharSet_ANSI_X3_110_1983 74
#define btLibCharSet_ISO_8859_1 4
#define btLibCharSet_ISO_8859_2 5
#define btLibCharSet_T_61_7bit 75
#define btLibCharSet_T_61_8bit 76
#define btLibCharSet_ISO_8859_3 6
#define btLibCharSet_ISO_8859_4 7
#define btLibCharSet_ECMA_cyrillic 77
#define btLibCharSet_CSA_Z243_4_1985_1 78
#define btLibCharSet_CSA_Z243_4_1985_2 79
#define btLibCharSet_CSA_Z243_4_1985_gr 80
#define btLibCharSet_ISO_8859_6 9
#define btLibCharSet_ISO_8859_6_E 81
#define btLibCharSet_ISO_8859_6_I 82
#define btLibCharSet_ISO_8859_7 10
#define btLibCharSet_T_101_G2 83
#define btLibCharSet_ISO_8859_8 11
#define btLibCharSet_ISO_8859_8_E 84
#define btLibCharSet_ISO_8859_8_I 85
#define btLibCharSet_CSN_369103 86
#define btLibCharSet_JUS_I_B1_002 87
#define btLibCharSet_ISO_6937_2_add 14
#define btLibCharSet_IEC_P27_1 88
#define btLibCharSet_ISO_8859_5 8
#define btLibCharSet_JUS_I_B1_003_serb 89
#define btLibCharSet_JUS_I_B1_003_mac 90
#define btLibCharSet_ISO_8859_9 12
#define btLibCharSet_greek_ccitt 91
#define btLibCharSet_NC_NC00_10_81 92
#define btLibCharSet_ISO_6937_2_25 93
#define btLibCharSet_GOST_19768_74 94
#define btLibCharSet_ISO_8859_supp 95
#define btLibCharSet_ISO_10367_box 96
#define btLibCharSet_ISO_8859_10 13
#define btLibCharSet_latin_lap 97
#define btLibCharSet_JIS_X0212_1990 98
#define btLibCharSet_DS_2089 99
#define btLibCharSet_us_dk 100
#define btLibCharSet_dk_us 101
#define btLibCharSet_JIS_X0201 15
#define btLibCharSet_KSC5636 102
#define btLibCharSet_ISO_10646_UCS_2 1000
#define btLibCharSet_ISO_10646_UCS_4 1001
#define btLibCharSet_DEC_MCS 2008
#define btLibCharSet_hp_roman8 2004
#define btLibCharSet_macintosh 2027
#define btLibCharSet_IBM037 2028
#define btLibCharSet_IBM038 2029
#define btLibCharSet_IBM273 2030
#define btLibCharSet_IBM274 2031
#define btLibCharSet_IBM275 2032
#define btLibCharSet_IBM277 2033
#define btLibCharSet_IBM278 2034
#define btLibCharSet_IBM280 2035
#define btLibCharSet_IBM281 2036
#define btLibCharSet_IBM284 2037
#define btLibCharSet_IBM285 2038
#define btLibCharSet_IBM290 2039
#define btLibCharSet_IBM297 2040
#define btLibCharSet_IBM420 2041
#define btLibCharSet_IBM423 2042
#define btLibCharSet_IBM424 2043
#define btLibCharSet_IBM437 2011
#define btLibCharSet_IBM500 2044
#define btLibCharSet_IBM775 2087
#define btLibCharSet_IBM850 2009
#define btLibCharSet_IBM851 2045
#define btLibCharSet_IBM852 2010
#define btLibCharSet_IBM855 2046
#define btLibCharSet_IBM857 2047
#define btLibCharSet_IBM860 2048
#define btLibCharSet_IBM861 2049
#define btLibCharSet_IBM862 2013
#define btLibCharSet_IBM863 2050
#define btLibCharSet_IBM864 2051
#define btLibCharSet_IBM865 2052
#define btLibCharSet_IBM866 2086
#define btLibCharSet_IBM868 2053
#define btLibCharSet_IBM869 2054
#define btLibCharSet_IBM870 2055
#define btLibCharSet_IBM871 2056
#define btLibCharSet_IBM880 2057
#define btLibCharSet_IBM891 2058
#define btLibCharSet_IBM903 2059
#define btLibCharSet_IBM904 2060
#define btLibCharSet_IBM905 2061
#define btLibCharSet_IBM918 2062
#define btLibCharSet_IBM1026 2063
#define btLibCharSet_EBCDIC_AT_DE 2064
#define btLibCharSet_EBCDIC_AT_DE_A 2065
#define btLibCharSet_EBCDIC_CA_FR 2066
#define btLibCharSet_EBCDIC_DK_NO 2067
#define btLibCharSet_EBCDIC_DK_NO_A 2068
#define btLibCharSet_EBCDIC_FI_SE 2069
#define btLibCharSet_EBCDIC_FI_SE_A 2070
#define btLibCharSet_EBCDIC_FR 2071
#define btLibCharSet_EBCDIC_IT 2072
#define btLibCharSet_EBCDIC_PT 2073
#define btLibCharSet_EBCDIC_ES 2074
#define btLibCharSet_EBCDIC_ES_A 2075
#define btLibCharSet_EBCDIC_ES_S 2076
#define btLibCharSet_EBCDIC_UK 2077
#define btLibCharSet_EBCDIC_US 2078
#define btLibCharSet_UNKNOWN_8BIT 2079
#define btLibCharSet_MNEMONIC 2080
#define btLibCharSet_MNEM 2081
#define btLibCharSet_VISCII 2082
#define btLibCharSet_VIQR 2083
#define btLibCharSet_KOI8_R 2084
#define btLibCharSet_KOI8_U 2088
#define btLibCharSet_IBM00858 2089
#define btLibCharSet_IBM00924 2090
#define btLibCharSet_IBM01140 2091
#define btLibCharSet_IBM01141 2092
#define btLibCharSet_IBM01142 2093
#define btLibCharSet_IBM01143 2094
#define btLibCharSet_IBM01144 2095
#define btLibCharSet_IBM01145 2096
#define btLibCharSet_IBM01146 2097
#define btLibCharSet_IBM01147 2098
#define btLibCharSet_IBM01148 2099
#define btLibCharSet_IBM01149 2100
#define btLibCharSet_Big5_HKSCS 2101
#define btLibCharSet_UNICODE_1_1 1010
#define btLibCharSet_SCSU 1011
#define btLibCharSet_UTF_7 1012
#define btLibCharSet_UTF_16BE 1013
#define btLibCharSet_UTF_16LE 1014
#define btLibCharSet_UTF_16 1015
#define btLibCharSet_UNICODE_1_1_UTF_7 103
#define btLibCharSet_UTF_8 106
#define btLibCharSet_iso_8859_13 109
#define btLibCharSet_iso_8859_14 110
#define btLibCharSet_ISO_8859_15 111
#define btLibCharSet_JIS_Encoding 16
#define btLibCharSet_Shift_JIS 17
#define btLibCharSet_EUC_JP 18
#define btLibCharSet_Extended_UNIX_Code_Fixed_Width_for_Japanese 19
#define btLibCharSet_ISO_10646_UCS_Basic 1002
#define btLibCharSet_ISO_10646_Unicode_Latin1 1003
#define btLibCharSet_ISO_Unicode_IBM_1261 1005
#define btLibCharSet_ISO_Unicode_IBM_1268 1006
#define btLibCharSet_ISO_Unicode_IBM_1276 1007
#define btLibCharSet_ISO_Unicode_IBM_1264 1008
#define btLibCharSet_ISO_Unicode_IBM_1265 1009
#define btLibCharSet_ISO_8859_1_Windows_3_0_Latin_1 2000
#define btLibCharSet_ISO_8859_1_Windows_3_1_Latin_1 2001
#define btLibCharSet_ISO_8859_2_Windows_Latin_2 2002
#define btLibCharSet_ISO_8859_9_Windows_Latin_5 2003
#define btLibCharSet_Adobe_Standard_Encoding 2005
#define btLibCharSet_Ventura_US 2006
#define btLibCharSet_Ventura_International 2007
#define btLibCharSet_PC8_Danish_Norwegian 2012
#define btLibCharSet_PC8_Turkish 2014
#define btLibCharSet_IBM_Symbols 2015
#define btLibCharSet_IBM_Thai 2016
#define btLibCharSet_HP_Legal 2017
#define btLibCharSet_HP_Pi_font 2018
#define btLibCharSet_HP_Math8 2019
#define btLibCharSet_Adobe_Symbol_Encoding 2020
#define btLibCharSet_HP_DeskTop 2021
#define btLibCharSet_Ventura_Math 2022
#define btLibCharSet_Microsoft_Publishing 2023
#define btLibCharSet_Windows_31J 2024
#define btLibCharSet_GB2312 2025
#define btLibCharSet_Big5 2026
#define btLibCharSet_windows_1250 2250
#define btLibCharSet_windows_1251 2251
#define btLibCharSet_windows_1252 2252
#define btLibCharSet_windows_1253 2253
#define btLibCharSet_windows_1254 2254
#define btLibCharSet_windows_1255 2255
#define btLibCharSet_windows_1256 2256
#define btLibCharSet_windows_1257 2257
#define btLibCharSet_windows_1258 2258
#define btLibCharSet_TIS_620 2259
#define btLibCharSet_HZ_GB_2312 2085

typedef struct BtLibProtocolDescriptorListEntryType
{
    BtLibSdpUuidType protoUUID;
    uint8_t          padding1;
    uint16_t         padding2;
    union
    {
        BtLibL2CapPsmType        psm;     // valid only if protoUUID is L2Cap
        BtLibRfCommServerIdType  channel; // valid only if protoUUID is RFCOMM
    }
    param;
}
BtLibProtocolDescriptorListEntryType;

typedef struct BtLibProfileDescriptorListEntryType
{
    BtLibSdpUuidType  profUUID;
    uint8_t           padding1;
    uint16_t          version;
    uint16_t          padding2;
}
BtLibProfileDescriptorListEntryType;

typedef struct BtLibStringType
{
    char*   str;
    uint16_t  strLen;
    uint16_t  padding;
}
BtLibStringType;

typedef struct BtLibUrlType
{
    char*   url;
    uint16_t  urlLen;
    uint16_t  padding;
}
BtLibUrlType;

//------------------------------------------------------------------------------
// BtLibSdpAttributeIdType -- Universal Service Attribute IDs.
//
// Universal attributes are those service attributes whose definitions are
// common to all service records. Note that this does not mean that every
// service record must contain values for all of these service attributes.
// However, if a service record has a service attribute with an attribute ID
// allocated to a universal attribute, the attribute value must conform to the
// universal attribute’s definition.
//
// Only two attributes are required to exist in every service record instance.
// They are the ServiceRecordHandle and the ServiceClassIDList All other service
// attributes are optional within a service record.

typedef uint16_t BtLibSdpAttributeIdType;

#define btLibServiceRecordHandle          0x0000
#define btLibServiceClassIdList           0x0001
#define btLibServiceRecordState           0x0002
#define btLibServiceId                    0x0003
#define btLibProtocolDescriptorList       0x0004
#define btLibBrowseGroupList              0x0005
#define btLibLanguageBaseAttributeIdList  0x0006
#define btLibTimeToLive                   0x0007
#define btLibAvailability                 0x0008
#define btLibProfileDescriptorList        0x0009
#define btLibDocumentationUrl             0x000A
#define btLibClientExecutableUrl          0x000B
#define btLibIconUrl                      0x000C

// These offsets must be added to the baseAttributeID field of the
// BtLibLanguageBaseTripletType attribute in order to get the attribute ID
// of the service name, service description, or provider name if they are set
// in a service record.

#define btLibServiceNameOffset            0x0000
#define btLibServiceDescriptionOffset     0x0001
#define btLibProviderNameOffset           0x0002

//------------------------------------------------------------------------------
// BtLibSdpAtributeDataType -- structure used to get and set the values of
// Universal Service Attributes.
//
// When getting a string, call BtLibSdpServiceRecordGetStringOrUrlLength()
// first to get the string size. Then set the pointer value of the
// BtLibStringType to point to a memory block large enough to get the string
// with BtLibSdpServiceRecordGetAttribute().

typedef union BtLibSdpAttributeDataType
{
    BtLibSdpUuidType                      serviceClassUuid;
    uint32_t                                serviceRecordState;
    BtLibSdpUuidType                      serviceIdUuid;
    BtLibProtocolDescriptorListEntryType  protocolDescriptorListEntry;
    BtLibSdpUuidType                      browseGroupUuid;
    BtLibLanguageBaseTripletType          languageBaseTripletListEntry;
    uint32_t                                timeToLive;
    uint8_t                                 availability;
    BtLibProfileDescriptorListEntryType   profileDescriptorListEntry;
    BtLibUrlType                          documentationUrl;
    BtLibUrlType                          clientExecutableUrl;
    BtLibUrlType                          iconUrl;
    BtLibStringType                       serviceName;
    BtLibStringType                       serviceDescription;
    BtLibStringType                       providerName;
}
BtLibSdpAttributeDataType;

/********************************************************************
 * SDP raw data parsing types and defines
 ********************************************************************/

//---------------------------------------------------------------------------
// Data Element header.
//
// Data Elements begin with a single byte that contains type and size info.
// To get the type from this byte, use BtLibSdpGetRawElementType().
// To get the size from this byte, use BtLibSdpGetRawDataElementSize().
//
// To create the first byte of a Data Element, bitwise OR the type and size
// values into a single byte. Examples:
//
// A 16-bit unsigned integer with a value of 0x0057:
//
//     uint8_t val[3] = { btLibDETD_UINT | btLibDESD_2BYTES, 0x00, 0x57 };
//
// The text string "foo":
//
//     uint8_t str[5] = { btLibDETD_TEXT | btLibDESD_ADD_8BITS, 3, 'f','o','o' };

//--------------------
// Data Element types.
//
#define btLibDETD_NIL  0x00 /* Specifies nil, the null type.
                             * Requires a size of btLibDESD_1BYTE, which for this
                             * type means an actual size of 0.
                             */
#define btLibDETD_UINT 0x08 /* Specifies an unsigned integer. Must use size
                             * btLibDESD_1BYTE, btLibDESD_2BYTES,
                             * btLibDESD_4BYTES, btLibDESD_8BYTES, or
                             * btLibDESD_16BYTES.
                             */
#define btLibDETD_SINT 0x10 /* Specifies a signed integer. May use size
                             * btLibDESD_1BYTE, btLibDESD_2BYTES,
                             * btLibDESD_4BYTES, btLibDESD_8BYTES, or
                             * btLibDESD_16BYTES.
                             */
#define btLibDETD_UUID 0x18 /* Specifies a Universally Unique Identifier (UUID).
                             * Must use size btLibDESD_2BYTES, btLibDESD_4BYTES,
                             * or btLibDESD_16BYTES.
                             */
#define btLibDETD_TEXT 0x20 /* Specifies a text string. Must use sizes
                             * btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS, or
                             * btLibDESD_ADD32_BITS.
                             */
#define btLibDETD_BOOL 0x28 /* Specifies a boolean value. Must use size
                             * btLibDESD_1BYTE.
                             */
#define btLibDETD_SEQ  0x30 /* Specifies a data element sequence. The data
                             * contains a sequence of Data Elements. Must use
                             * btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS,
                             * or btLibDESD_ADD_32BITS.
                             */
#define btLibDETD_ALT  0x38 /* Specifies a data element alternative. The data
                             * contains a sequence of Data Elements. This type is
                             * sometimes used to distinguish between two possible
                             * sequences. Must use size btLibDESD_ADD_8BITS,
                             * btLibDESD_ADD_16BITS, or btLibDESD_ADD_32BITS.
                             */
#define btLibDETD_URL  0x40 /* Specifies a Uniform Resource Locator (URL).
                             * Must use size btLibDESD_ADD_8BITS,
                             * btLibDESD_ADD_16BITS, or btLibDESD_ADD_32BITS.
                             */

#define btLibDETD_MASK 0xf8 /* AND this value with the first byte of a Data
                             * Element to return the element's type.
                             */

//--------------------
// Data Element sizes.
//
#define btLibDESD_1BYTE      0x00 /* Specifies a 1-byte element. However, if
                                   * type is btLibDETD_NIL then the size is 0.
                                   */
#define btLibDESD_2BYTES     0x01 /* Specifies a 2-byte element. */
#define btLibDESD_4BYTES     0x02 /* Specifies a 4-byte element. */
#define btLibDESD_8BYTES     0x03 /* Specifies an 8-byte element. */
#define btLibDESD_16BYTES    0x04 /* Specifies a 16-byte element. */
#define btLibDESD_ADD_8BITS  0x05 /* The element's actual data size, in bytes,
                                   * is contained in the next 8 bits.
                                   */
#define btLibDESD_ADD_16BITS 0x06 /* The element's actual data size, in bytes,
                                   * is contained in the next 16 bits.
                                   */
#define btLibDESD_ADD_32BITS 0x07 /* The element's actual data size, in bytes,
                                   * is contained in the next 32 bits.
                                   */

#define btLibDESD_MASK       0x07 /* AND this value with the first byte of a Data
                                   * Element to return the element's size.
                                   */

/********************************************************************
 * BtLib Sockets
 ********************************************************************/

//
// A socket reference is in fact an IOS file descriptor.
//
typedef int32_t BtLibSocketRef;

// Max number of service record handles returned in "data part" of the message
// bearing a btLibSocketEventSdpServiceRecordHandle event.
#define btLibMaxSrvRecListLen 20

//
// Event codes for Socket events
//
typedef enum
{
    // A remote device has requested a connection.
    // Respond by calling BtLibSocketRespondToConnection().
    btLibSocketEventConnectRequest = 1,

    // An outbound connection request has completed. Check status for success.
    btLibSocketEventConnectedOutbound,

    // An inbound connection request has been accepted and established.
    btLibSocketEventConnectedInbound,

    // Either (1) this is a data socket, and it has just been disconnected,
    // or (2) this is a listener socket that just tried to accept an inbound
    // connection request, but the connection could not be established.
    // In the first case you must close this socket. In the second case you
    // must close the file descriptor returned in eventData.newSocket.
    btLibSocketEventDisconnected,

    // OBSOLETE. When inbound data arrives, IOSGetMsg() returns a message with
    // no "control part" and a "data part" containing the inbound data.
    btLibSocketEventData_obsolete,

    // A data packet that was sent down via BtLibSocketSend() has reached
    // the radio. NOTE: in earlier versions of BtLib, applications had to
    // wait for this event before reusing the data buffer passed to
    // BtLibSocketSend(), but this is no longer the case!
    btLibSocketEventSendComplete,

    // The get service record handle by Service Class call has completed.
    btLibSocketEventSdpServiceRecordHandle,

    // An attribute request has completed.
    btLibSocketEventSdpGetAttribute,

    // A string or URL length request has completed.
    btLibSocketEventSdpGetStringLen,

    // A number of list entries request has completed.
    btLibSocketEventSdpGetNumListEntries,

    // A number of lists request has completed
    btLibSocketEventSdpGetNumLists,

    // A get raw attribute request has completed
    btLibSocketEventSdpGetRawAttribute,

    // A get raw attribute size request has completed
    btLibSocketEventSdpGetRawAttributeSize,

    // A get server channel request has completed
    btLibSocketEventSdpGetServerChannelByUuid,

    // A get psm request has completed
    btLibSocketEventSdpGetPsmByUuid
}
BtLibSocketEventEnumTag;

typedef Enum8 BtLibSocketEventEnum;

//------------------------------------------------------------------------------
// BtLibSocketEventType -- socket event object.
//
// Applications obtain socket events by calling IOSGetmsg() on a file descriptor
// opened to an RfComm, L2CAP, or SDP device.
//
// If the message thus obtained has a "control part", then that control part
// contains a BtLibSocketEventType object. Depending upon the event code, there
// may also be additional information in the "data part" of the message.
//
// If the message has no "control part", then the "data part" of the message
// contains inbound application-level data.
//
typedef struct _BtLibSocketEventType
{
    //--------------------------------------------------------------------------
    // event -- the basic op code for the event.
    //
    // Events: All
    //
    BtLibSocketEventEnum event;
    uint8_t              padding1;
    uint16_t             padding2;

    //--------------------------------------------------------------------------
    // status -- error code for the event.
    //
    // Events: All
    //
    status_t status;

    union
    {
        //----------------------------------------------------------------------
        // newSocket -- new socket from accepting an inbound connection.
        //
        // Events: btLibSocketEventConnectedInbound - the accept was successful,
        //             and newSocket is now connected.
        //         btLibSocketEventDisconnected - the accept failed, but you
        //             still must close newSocket.
        //
        BtLibSocketRef newSocket;

        //----------------------------------------------------------------------
        // requestingDevice -- address of the remote connecting device
        //
        // Events: btLibSocketEventConnectRequest
        //
        BtLibDeviceAddressType requestingDevice;

        //----------------------------------------------------------------------
        // sdpByUuid -- an L2cap psm or RfComm channel from an SDP query
        //
        // Events: btLibSocketEventSdpGetServerChannelByUuid
        //         btLibSocketEventSdpGetPsmByUuid
        //
        struct
        {
            BtLibSdpRemoteServiceRecordHandle remoteHandle; // remote SDP record
            union
            {
                BtLibL2CapPsmType        psm;
                BtLibRfCommServerIdType  channel;
            }
            param;
            uint16_t padding;
        }
        sdpByUuid;

        //----------------------------------------------------------------------
        // serviceRecordHandles -- list of remote service record handles.
        //
        // Events: btLibSocketEventSdpServiceRecordHandle
        //
        // NOTE: the "data part" of the message contains the actual service
        // record handles: a table of numSrvRec (max btLibMaxSrvRecListLen)
        // entries of type BtLibSdpRemoteServiceRecordHandle.
        //
        struct
        {
            uint16_t      numSrvRec;
            // BtLibSdpRemoteServiceRecordHandle*  serviceRecordList;
        }
        sdpServiceRecordHandles;

        struct
        {
            // Valid for all SDP events except for
            // btLibSocketEventSdpServiceRecordHandle,
            // btLibSocketEventSdpGetServerChannelByUuid, and
            // btLibSocketEventSdpGetPsmByUuid

            BtLibSdpAttributeIdType  attributeID; // attribute of query
            uint16_t                 padding;
            BtLibSdpRecordHandle     recordH;     // local record handle

            union
            {
                //--------------------------------------------------------------
                // data -- attribute data requested from remote device
                //
                // Events: btLibSocketEventSdpGetAttribute
                //
                // NOTE: in the case of attributes of type string or url, the
                // "data part" of the message contains the string or url.
                //
                struct
                {
                   BtLibSdpAttributeDataType  attributeValues;
                   uint16_t                     listNumber;
                   uint16_t                     listEntry;
                }
                data;

                //--------------------------------------------------------------
                // rawData -- raw attribute data requested from remote device
                //
                // Events: btLibSocketEventSdpGetRawAttribute
                //
                // NOTE: the "data part" of the message contains the attribute
                // value.
                //
                struct
                {
                    uint16_t  valSize;
                    // uint8_t*  value;
                }
                rawData;

                //--------------------------------------------------------------
                // valSize -- size of buffer needed to get a raw attirbute.
                //
                // Events: btLibSocketEventSdpGetRawAttributeSize
                //
                uint16_t valSize;


                //--------------------------------------------------------------
                // strLength -- buffer size for getting string or URL attribute.
                //
                // Events: btLibSocketEventSdpGetStringLen
                //
                uint16_t strLength;


                //--------------------------------------------------------------
                // numItems -- count of lists or list entries for an attribute.
                //
                // Events: btLibSocketEventSdpGetNumListEntries
                //         btLibSocketEventSdpGetNumLists
                uint16_t numItems;
            }
            info;
        }
        sdpAttribute;
    }
    eventData;
}
BtLibSocketEventType;

typedef BtLibSocketEventType* BtLibSocketEventTypePtr;

typedef enum
{
    btLibL2CapProtocol,
    btLibRfCommProtocol,
    btLibSdpProtocol,
    btLibSCOProtocol,
    btLibBNEPProtocol
}
BtLibProtocolEnumTag;

typedef Enum8 BtLibProtocolEnum;

//------------------------------------------------------------------------------
// BtLibSocketInfoEnum -- used in calls to BtLibSocketGetInfo() to specify which
// information to retrieve.
//
typedef enum
{
    // General Info
    btLibSocketInfo_Protocol = 0,
    btLibSocketInfo_RemoteDeviceAddress,

    // L2Cap and RfComm Shared
    btLibSocketInfo_SendPending = 100,
    btLibSocketInfo_MaxTxSize,
    btLibSocketInfo_MaxRxSize,

    // L2Cap Info
    btLibSocketInfo_L2CapPsm = 200,
    btLibSocketInfo_L2CapChannel,

    // RFComm specific
    btLibSocketInfo_RfCommServerId = 300,
    btLibSocketInfo_RfCommOutstandingCredits,

    // SDP specific
    btLibSocketInfo_SdpServiceRecordHandle = 400,

    // For internal use only
    btLibSocketInfo_DeviceNum = 1000
}
BtLibSocketInfoEnumTag;

typedef Enum16 BtLibSocketInfoEnum;

//------------------------------------------------------------------------------
// BtLibSocketListenInfoType -- used in calls to BtLibSocketListen() to specify
// protocol-specific parameters.
//
typedef struct BtLibSocketListenInfoType
{
    union
    {
        struct
        {
            // Pre-assigned assigned PSM values are permitted; however, they
            // must be odd, within the range of 0x1001 to 0xFFFF, and have
            // the 9th bit (0x0100) set to zero. Passing in BT_L2CAP_RANDOM_PSM
            // will automatically create a usable PSM for the channel. In this
            // case the actual PSM value will be filled in by the call.
            BtLibL2CapPsmType  localPsm;
            uint16_t           localMtu;
            uint16_t           minRemoteMtu;
        }
        L2Cap;

        struct
        {
            // The Service Id is assigned by RFCOMM and returned in the
            // serviceID field
            //
            // BT_RF_MIN_FRAMESIZE <= maxFrameSize <= BT_RF_MAX_FRAMESIZE
            // Use BT_RF_DEFAULT_FRAMESIZE if you don't care.
            //
            // Setting advancedCredit to a value other then 0 causes that
            // amount of credit to be advanced to the remote device upon
            // successful connection. Additional credit can be
            // advanced after connection establishment by calling
            // BtLibSocketAdvanceCredit().
            BtLibRfCommServerIdType  serviceID;
            uint8_t                  advancedCredit;
            uint16_t                 maxFrameSize;
        }
        RfComm;

        struct
        {
            // A BNEP listener specifies which subset of the three PAN profile
            // services (NAP, GN, PANU) that it is willing to support.
            Boolean  listenNAP;   // true <=> the NAP service is supported
            Boolean  listenGN;    // true <=> the GN service is supported
            Boolean  listenPANU;  // true <=> the PANU service is supported.
        }
        BNEP;
    }
    data;
    uint16_t padding;
}
BtLibSocketListenInfoType;

//------------------------------------------------------------------------------
// BtLibSocketConnectInfoType -- used in calls to BtLibSocketConnect() to
// specify the remote device and protocol-specific parameters.
//
typedef struct BtLibSocketConnectInfoType
{
    BtLibDeviceAddressTypePtr       remoteDeviceP;  // address of remote device

    union
    {
        struct
        {
            BtLibL2CapPsmType       remotePsm;
            uint16_t                minRemoteMtu;
            uint16_t                localMtu;
        }
        L2Cap;

        struct
        {
            BtLibRfCommServerIdType remoteService;  // remote service channel id
            uint8_t                 advancedCredit; // initial flow credit
            uint16_t                maxFrameSize;   // max Tx and Rx frame size
        }
        RfComm;

        struct
        {
            // When connecting a BNEP socket, you must specify the role you want
            // the local device to to play, and the role you want the remote
            // device to play. A role is specified as a 16-bit UUID in host
            // byte-order; possible values are:
            //   0x1115 - uuid for PANU role
            //   0x1116 - uuid for NAP role
            //   0x1117 - uuid for GN role
            uint16_t                localService;   // uuid of local role
            uint16_t                remoteService;  // uuid of remote role
        }
        bnep;
    }
    data;

    uint16_t padding;
}
BtLibSocketConnectInfoType;


/********************************************************************
 * BSD Sockets Interface
 ********************************************************************/

// Protocol to use when creating an RFComm socket
//
#define BTHPROTO_RFCOMM 1

// The bluetooth sockaddr structure
//
typedef struct sockaddr_bth
{
    sa_family_t             sa_family;
    BtLibDeviceAddressType  btAddr;
    BtLibSdpUuidType        serviceClassId;
    uint8_t                 padding1;
    uint16_t                padding2;
}
sockaddr_bth;

// Special "anybody" device address
//
#define BTADDR_ANY {0, 0, 0, 0, 0, 0}


/********************************************************************
 * Services
 ********************************************************************/

//------------------------------------------------------------------------------
// Service registration parameters passed to BtLibRegisterService().
//
// The (appType, appCreator, appCodeResourceId) triple identifies the service
// app's code resource.
//
// The protocol parameter defines whether L2CAP or RfComm is to be used. The
// pushSerialModule flag specifies whether a serial interface module is to be
// pushed onto the protocol device instance.
//
// The thread(s) that execute the service's entrypoints will be created with
// a stack of at least stackSize bytes.
//
// The service's "preparation" entrypoint is always invoked in the context of
// the System Process.
//
// The service's "execution" entrypoint will be invoked either in the System
// Process or in the Application Process (on the main UI thread), according to
// the execAsNormalApp flag. NOTE: for PalmOS 6.0, only execution in the System
// Process (execAsNormaApp == 0) is supported.
//
typedef struct
{
    uint32_t           stackSize;          // service thread stack size (bytes)
    uint32_t           appType;            // app's resource database's type
    uint32_t           appCreator;         // app's resource database's creator
    uint16_t           appCodeRscId;       // app's code resource id
    BtLibProtocolEnum  protocol;           // which protocol (L2CAP or RfComm)
    uint8_t            execAsNormalApp:1,  // execute in normal UI context?
                       pushSerialModule:1; // push serial interface module?
}
BtLibServiceRegistrationParamsType;

//------------------------------------------------------------------------------
// Parameters passed to a service app's "preparation" entrypoint via the
// sysBtLaunchCmdPrepareService launch code. The service app receives this
// launch code once after it registers itself, and then after each service
// execution session, in the context of the System Process.
//
// The fdListener parameter is a file descriptor opened to an L2CAP or RfComm
// device instance. Upon entry it has already been marked as a listener. Upon
// return it must be left unchanged; the Bluetooth system will take care of
// calling BtLibSdpServiceRecordStartAdvertising() to advertise the service,
// and BtLibSocketClose() after an inbound connection has been made.
//
// The serviceRec parameter is a handle on a local SDP service record. Upon
// entry, it is empty. Upon exit, it must be set up to describe the service
// that the app has to offer.
//
// In most cases, the app can respond to this launch code by simply calling
// BtLibSdpServiceRecordSetAttributesForSocket(), passing the fdListener and
// serviceRec parameters, along with a servce class UUID and a service name.
//
// In more complicated cases, the app will need to use other BtLibSdpXxx()
// functions to massage the service record appropriately. In such cases,
// it is the app's responsibility to open a Management Entity device instance
// to pass to those functions, and to close it before returning.
//
// If the app allocates any additional resources in its "preparation" entry
// point, then it must also respond to the sysBtLaunchCmdAbortService launch
// code and deallocate those resources.
//
typedef struct
{
    int32_t               fdListener;   // L2CAP or RfComm listener socket
    BtLibSdpRecordHandle  serviceRecH;  // empty service record to be filled in
}
BtLibServicePreparationParamsType;

//------------------------------------------------------------------------------
// Parameters passed to a service app's "execution" entrypoint via the
// sysBtLaunchCmdExecuteService launch code. The service app receives this
// launch code each time a remote client connects. It receives it in the context
// of the System Process or the Application Process according to the
// execAsNormalApp registration flag.
//
// The fdData parameter is a file descriptor opened to a connected L2CAP or
// RfComm device instance, with a serial interface module optionally pushed
// onto that, dending upon the pushSerialModule registration flag.
// Upon entry, it is connected to its remote peer and ready for data transfer.
// Upon exit, it must be closed.
//
typedef struct
{
    int32_t  fdData;  // the connected L2CAP or RfComm socket
}
BtLibServiceExecutionParamsType;

//------------------------------------------------------------------------------
// Parameters returned from a service app's "description" entrypoint via the
// sysBtLaunchCmdDescribeService launch code. The bluetooth panel sends this
// launch code in order to obtain the info it needs to display in its services
// view.
//
// Upon return, the 'flags' parameter must be set to zero, or to the bitwise OR
// of one or more of the btLibServDescFlag_Xxx constants below.
//
// Upon return, the nameP and descriptionP parameters must point to (localized)
// strings in dynamically allocated memory (i.e. allocated via MemPtrNew() or
// malloc()). The string pointed to by nameP identifies the service in the
// services view of the panel. It should be relatively short, like "Personal
// Area Networking".  The string pointed to by descriptionP is displayed when
// the given service is selected in the services view. It can be longer like
// "Allow other devices to connect and form an ad-hoc local network."
//
typedef struct
{
    uint32_t  flags;         // out: bitwise OR of btLibServDescFlag_Xxx flags
    char*     nameP;         // out: brief name to display in bt panel
    char*     descriptionP;  // out: verbose description to display in bt panel
}
BtLibServiceDescriptionType;

//------------------------------------------------------------------------------
// Service description flags -- bits of BtLibServiceDescriptionType::flags
//
// CAN_DO_UI: this bit indicates if the service app is capable of responding
// to the sysBtLaunchCmdDoServiceUI launch code. If this bit is set, then
// when the given service is selected in the services view of the panel, an
// "advanced" button will appear, and tapping that button will cause the launch
// code to be sent to the service. The service at that point should do some
// sort of UI specific to that service.
//
#define btLibServDescFlag_CAN_DO_UI  0x00000001  // can do custom UI in panel


#endif   // _BTLIBTYPES_H
