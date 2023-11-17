/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtLib.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *     Public header file for users of the PalmOS Bluetooth API.
 *
 *****************************************************************************/

#ifndef _BTLIB_H
#define _BTLIB_H

#include <BtLibTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Management Entity
 *
 * The Management Entity API is used for device discovery, for managing ACL
 * links (including piconets), and for controlling global bluetooth settings.
 *******************************************************************************/

//------------------------------------------------------------------------------
// Open a file descriptor to the Management Entity device, and initialize the
// hardware and stack if necessary.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrRadioInitFailed - the Bluetooth hardware is not available
//      <other> - the ME device could not be opened.
//
// Events:
//      None
//
status_t BtLibOpen(
    int32_t*   fdME  // <- receives the Management Entity file descriptor
);

//------------------------------------------------------------------------------
// Close the given Management Entity file descriptor.
//
// If all of the following conditions hold after closing the ME file descriptor:
//      - there exist one or more ACL links
//      - there are no opened ME file descriptors
//      - there are no opened and connected L2CAP file descriptors
//      - there are no opened and connected RFCOMM file descriptors
// then this function will also destroy all existing ACL links.
//
// Returns:
//      btLibErrNoError - success
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
status_t BtLibClose(
    int32_t   fdME  // -> Mgmt Entity file descriptor
);

//------------------------------------------------------------------------------
// Start an inquiry. The addresses of discovered devices will be returned by
// asynchonous events.
//
// The inquiry will be halted after min(timeOut,60) seconds, or after
// maxResp responses have been received, whichever occurs first. The
// timeOut value will be rounded down to the nearest multiple of 1.28
// seconds. The responses are are not necessarily unique -- the same
// device may be reported multiple times.
//
// Returns:
//      btLibErrPending - results will be returned by events.
//      btLibErrInProgress - an inquiry is already in process.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventInquiryResult - occurs every time a device
//          is discovered.
//      btLibManagementEventInquiryComplete - occurs when the inquiry
//          is complete.
//
status_t BtLibStartInquiry(
    int32_t   fdME,     // -> Mgmt Entity file descriptor
    uint8_t   timeOut,  // -> max time (seconds) until inquiry terminated
    uint8_t   maxResp   // -> max number of (not necessarily unique) responses
);

//------------------------------------------------------------------------------
// Cancel an inquiry.
//
//  Returns:
//      btLibErrPending - the event btLibManagementEventInquiryCanceled
//          will indicate the completion of this operation.
//      btLibErrNotInProgress - no inquiry is in progress.
//      btLibErrInProgress - an inquiry is in already being cancled.
//      btLibErrNoError - the inquiry was canceled before it really got started.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventInquiryCanceled - confirms that the
//          inquiry has been canceled.
//
status_t BtLibCancelInquiry(
    int32_t   fdME      // -> Mgmt Entity file descriptor
);


//------------------------------------------------------------------------------
// Perform remote device discovery, presenting a UI that allows the user to
// select remote devices or cancel the operation.
//
// If the addressAsName argument is true, then the user will be presented with
// the discovered devices' bluetooth addresses. Otherwise an attempt will be
// be made to obtain each device's user-friendly name (from the cache if it
// is there, otherwise by connecting to the remote device), and if successful,
// the name will be presented.
//
// The filterTable argument can be used to restrict the devices that are
// presented to the user based upon class-of-device. If a discovered device's
// class-of-device is not among the class-of-devices in the filterTable, then
// it will not be presented to the user. If filterTable is null, then no
// filtering will be done.
//
// The hideFavorites argument can be used to suppress the presentation of
// devices that are in the favorite devices database.
//
// The selected devices are returned in the deviceTable argument. The
// deviceTableLen argument specifies the capacity of the deviceTable, and has
// an effect upon the UI -- the user will be prevented from selecting more than
// deviceTableLen devices.
//
// The number of devices selected by the user is returned in *numSelectedPtr.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrCanceled - user canceled
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibDiscoverDevices(
    int32_t                 fdME,            // -> Management Entity file
                                             //    descriptor
    char*                   instructionTxt,  // -> text at top of selection
                                             //    box (NULL: use default)
    char*                   buttonTxt,       // -> text for 'done' button
                                             //    (NULL: use default)
    Boolean                 addressAsName,   // -> show devices' addresses
                                             //    rather than their names
    BtLibClassOfDeviceType* filterTable,     // -> table of device classes for
                                             //    filtering (NULL: don't filter)
    uint8_t                 filterTableLen,  // -> length (number of entries) of
                                             //    the filter table
    Boolean                 hideFavorites,   // -> do not show devices that are
                                             //    in our "favorite devices" d.b.
    BtLibDeviceAddressType* deviceTable,     // <- table to receive selected
                                             //    devices' addresses
    uint8_t                 deviceTableLen,  // -> length (number of entries) of
                                             //    the selected devices table
    uint8_t*                numSelectedPtr   // <- receives the number of
                                             //    selected devices
);

//------------------------------------------------------------------------------
// Get the user-friendly name of the given remote device, asynchronously.
//
// The bluetooth library maintains a small cache of device (address, name)
// pairs. The value of retrievalMethod determines how the cache is used:
//
//   btLibCachedOnly ........ look only in the cache
//   btLibCachedThenRemote .. look in the cache; if not found, ask remote device
//   btLibRemoteOnly ........ ask the remote device without trying the cache
//
// In all cases, the result is returned in an event. If the name was not found,
// then the event will contain a null name.
//
// Returns:
//      btLibErrPending - the result will be returned in an event.
//      btLibErrBusy - a name request is already pending.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventNameResult - this event with a status of
//          btLibErrNoError contains the result of the query; if the name was
//          not found, the name returned in the event is null. This event
//          with any other status indicates some sort of failure.
//
status_t BtLibGetRemoteDeviceName(
    int32_t                  fdME,           // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP,  // -> address of the remote device
    BtLibGetNameEnum         retrievalMethod // -> how to use the cache
);

//------------------------------------------------------------------------------
// Get the user-friendly name of the given remote device, synchronously.
//
// The bluetooth library maintains a small cache of device (address, name)
// pairs. The value of retrievalMethod determines how the cache is used:
//
//   btLibCachedOnly ........ look only in the cache
//   btLibCachedThenRemote .. look in the cache; if not found, ask remote device
//   btLibRemoteOnly ........ ask the remote device without trying the cache
//
// This function blocks until the remote device name request completes, and
// returns the name, including the terminal null char, in the given buffer.
// If the name is too long for the buffer, it will be truncated, but still
// null-terminated.
//
// Note: the maximum length of a bluetooth device name, including the null
// terminator, is btLibMaxDeviceNameLength.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrBusy - a name request is already pending.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//      btLibErrParamError - address pointer is null, or buffer pointer is null,
//          or buffer length is zero.
//
// Events:
//      None
//
status_t BtLibGetRemoteDeviceNameSynchronous(
    int32_t                  fdME,            // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP,   // -> address of the remote device
    BtLibGetNameEnum         retrievalMethod, // -> how to use the cache
    char*                    buffer,          // <- buffer to receive the name
    size_t                   bufferLen        // -> size (bytes) of the buffer
);

//------------------------------------------------------------------------------
// Create an ACL link to the given remote device.
//
// To create links to more than one device, you must call BtLibPiconetCreate().
// Outbound connects allow the master-slave switch when you have not called
// BtLibPiconetCreate().
//
// Returns:
//      btLibErrPending - the result will be returned in an event.
//      btLibErrAlreadyConnected - the link is already established.
//      btLibErrTooMany - the maximum number of ACL links is already established.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventACLConnectOutbound - this event with a status of
//          btLibErrNoError indicates that the ACL link has been established.
//
status_t BtLibLinkConnect(
    int32_t                  fdME,         // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP // -> address of the remote device
);

//------------------------------------------------------------------------------
// Disconnect an ACL link.
//
// Returns:
//      btLibErrPending - the result will be returned in an event.
//      btLibErrNoError - link establishment had not yet completed (no event).
//      btLibErrNoAclLink - no link exists to disconnect (no event).
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventACLDisconnect - indicates that the link has been
//          disconnected. The status field of the event indicates why.
//
status_t BtLibLinkDisconnect(
    int32_t                  fdME,         // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP // -> address of the remote device
);

//------------------------------------------------------------------------------
// Set up to be the master of a piconet.
//
// The unlockInbound parameter determines whether the piconet will initially be
// unlocked (inbound connections possible). If unlockInbound is true, then the
// discoverable parameter determines whether the radio will be set to respond to
// inquiries.
//
// Returns:
//      btLibErrNoError - successfully configured to be master.
//      btLibErrPending - there is a pre-existing ACL link and the link policy
//             needs to be changed and/or a role switch may be required.
//      btLibErrFailed - a piconet already exists.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventPiconetCreated - if the status is zero, the piconet
//          was created, otherwise the status indicates the reason for failure.
//      btLibManagementEventAccessibilityChange - if the radio is discoverable
//          and accessable then this event will let you know that the radio is
//          no longer discoverable or connectable.
//
status_t BtLibPiconetCreate(
    int32_t  fdME,          // -> Mgmt Entity file descriptor
    Boolean  unlockInbound, // -> true <=> inbound connections will be possible
    Boolean  discoverable   // -> true <=> radio will respond to inquiries
                            //    (ignored if unlockInbound is false)
);

//------------------------------------------------------------------------------
// Disconnect links to all devices, and remove all retrictions on our
// master/slave role.
//
// Returns:
//      btLibErrPending - destroying the piconet.
//      btLibErrNoError - piconet destroyed.
//      btLibErrNoPiconet - piconet does not exist.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventACLDisconnect - indicates that a link has been
//          disconnected.
//      btLibManagementEventPiconetDestroyed - indicates that the piconet has
//          been completely destroyed.
//
status_t BtLibPiconetDestroy(
    int32_t   fdME  // -> Mgmt Entity file descriptor
);

//------------------------------------------------------------------------------
// Accept inbound ACL links into the piconet.
//
// Allowing inbound connections lowers the bandwidth availble for data
// transmission among the members of the piconet because the radio has to
// periodically scan for incoming links.
//
//
// Returns:
//      btLibErrNoError - Devices can be added to the piconet.
//      btLibErrNoPiconet - BtLibPicoCreate has not been succesfully called.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventAccessibilityChange - If allowInbound is true then
//          this event informs you that you are discoverable and connectable
//
status_t BtLibPiconetUnlockInbound(
    int32_t  fdME,         // -> Mgmt Entity file descriptor
    Boolean  discoverable  // determines whether the radio will be set to respond
                           // to inquiries
);

//------------------------------------------------------------------------------
// Lock this piconet, i.e. prevent remoted devices from connecting to it.
// Outbound connections are still possible.
//
// Locking a piconet maximizes the bandwith available for data transmission
// among the members of that piconet.
//
// Returns:
//      btLibErrNoError - piconet successfully locked.
//      btLibErrNoPiconet - BtLibPicoCreate() has not been succesfully called.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventAccessibilityChange - if this device was previously
//          discoverable and/or connectable, then this event will indicate that
//          it is no longer discoverable or connectable.
//
status_t BtLibPiconetLockInbound(
    int32_t   fdME  // -> Mgmt Entity file descriptor
);

//------------------------------------------------------------------------------
// Set the state of the given ACL link. The parameter pref determines which
// attribute of the state to set, and the linkStateP parameter must be of
// the appropriate type according to pref, as follows:
//
// pref                          type of linkStateP
// ----                          ------------------
// btLibLinkPref_Authenticated   ignored, size is also ignored
// btLibLinkPref_Encrypted       Boolean*
//
// Note that before setting the link to be encrypted, it must be authenticated.
//
// Returns:
//      btLibErrPending - the result will be returned in an event
//      btLibErrFailed  - the link has not been authenticated
//      btLibErrNoAclLink - no link exists to the given device
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventAuthenticationComplete - the link authentication
//          request has completed.
//      btLibManagementEventEncryptionChange - the encryptedness of the link
//          has changed.
//
status_t BtLibLinkSetState(
    int32_t                  fdME,          // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP, // -> remote device address
                                            //    (identifies the link)
    BtLibLinkPrefsEnum       pref,          // -> which attribute of the link
                                            //    state to set
    void*                    linkStateP,    // -> value for link state attrib
                                            //    (type depends upon pref)
    uint16_t                 linkStateSize  // -> size in bytes of *linkStateP
);

//------------------------------------------------------------------------------
// Get the state of the given ACL link. The parameter pref determines which
// attribute of the state to obtain, and the linkState parameter must be of
// the appropriate type according to pref, as follows:
//
// pref                          type of linkStateP
// ----                          ------------------
// btLibLinkPref_Authenticated   Boolean*
// btLibLinkPref_Encrypted       Boolean*
// btLibLinkPref_LinkRole        BtLibConnectionRoleEnum*
//
// Returns:
//      btLibErrNoError - the linkState variable has been filled in.
//      btLibErrNoAclLink - no link exists to the given device
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibLinkGetState(
    int32_t                  fdME,          // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  remoteDeviceP, // -> remote device address
                                            //    (identifies ACL link)
    BtLibLinkPrefsEnum       pref,          // -> which link state info to get
    void*                    linkStateP,    // <- memory to receive info
                                            //    (type depens upon pref)
    uint16_t                 linkStateSize  // <- size in bytes of *linkStateP
);

//------------------------------------------------------------------------------
// Set the general management preferences. This function should not be used by
// normal applications ...
//
// pref -> management preference to set.
// prefValue -> value corresponding to preference.
// prefValueSize -> size in bytes of prefValue
//
// pref                             type of prefValueP
// ----                             ------------------
// btLibPref_Name                   BtLibFriendlyNameType*
// btLibPref_UnconnectedAccessible  BtLibAccessiblityInfoType*
// btLibPref_LocalClassOfDevice     BtLibClassOfDeviceType*
//
// Returns:
//      btLibErrNoError - Preference set.
//      btLibErrPending - The results will be returned in an event.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibManagementEventAccessibilityChange - accessibility has been changed
//      btLibManagementEventLocalNameChange - name has been changed
//
status_t BtLibSetGeneralPreference(
    int32_t               fdME,         // -> Mgmt Entity file descriptor
    BtLibGeneralPrefEnum  pref,         // -> management preference to set
    void*                 prefValueP,   // -> value corresponding to preference
    uint16_t              prefValueSize // -> size in bytes of prefValue
);

//------------------------------------------------------------------------------
// Get the general management preferences
//
// pref                             type of prefValueP
// ----                             ------------------
// btLibPref_Name                   BtLibFriendlyNameType*
// btLibPref_UnconnectedAccessible  BtLibAccessibleModeEnum*
// btLibPref_CurrentAccessible      BtLibAccessibleModeEnum*
// btLibPref_LocalClassOfDevice     BtLibClassOfDeviceType*
// btLibPref_LocalDeviceAddress     BtLibDeviceAddressType*
//
// Returns:
//      btLibErrNoError - Preference stored in prefValue.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibGetGeneralPreference(
    int32_t               fdME,         // -> Mgmt Entity file descriptor
    BtLibGeneralPrefEnum  pref,         // -> management preference to get
    void*                 prefValueP,   // -> memory to store prefValue
    uint16_t              prefValueSize // -> size in bytes of prefValue
);

//------------------------------------------------------------------------------
// Return the name of the given ME event code.
//
const char* BtLibMEEventName( BtLibManagementEventEnum  event );

/*******************************************************************************
 * Sockets
 *******************************************************************************/

//------------------------------------------------------------------------------
// Create a socket. The returned socket reference is in fact an IOS file
// descriptor. Applications should detroy all sockets before terminating.
//
// Returns:
//      btLibErrNoError - Indicates that the socket was created.
//      btLibErrOutOfMemory - Not enough memory to create socket.
//      btLibErrTooMany - Failed because reached maximum number of sockets
//          allocated for system.
//
// Events:
//      None
//
status_t BtLibSocketCreate(
    BtLibSocketRef*    socketRefP,    // <- receives the socket reference
    BtLibProtocolEnum  socketProtocol // -> the underlying protocol
);

//------------------------------------------------------------------------------
// Close a socket: frees associated resources and kills connections.
//
// If all of the following conditions hold after this socket is closed:
//      - there exist one or more ACL links
//      - there are no opened ME file descriptors
//      - there are no opened and connected L2CAP file descriptors
//      - there are no opened and connected RFCOMM file descriptors
//      - there are no opened and connected SCO file descriptors
//      - there are no opened and connected BNEP file descriptors
// then this function will also destroy all existing ACL links.
//
// Returns:
//      btLibErrNoError - Socket closed successfully.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//       None
//
status_t BtLibSocketClose(
    BtLibSocketRef  socketRef // -> the socket (file descriptor) to close
);

//------------------------------------------------------------------------------
// Set up an L2Cap, RfComm, SCO, or BNEP socket as a listener.
//
// The listenInfoP argument points to a block of protocol-specific information
// used to set up the listener socket. In one case, the contents of that block
// may be modified upon return. See the comments about BtLibSocketListenInfoType
// for further details.
//
// Returns:
//      btLibErrNoError - Socket listening for incoming connections.
//      btLibErrSocketPsmUnavailable - the given PSM is in use (L2CAP only)
//      btLibErrTooMany - There are no resources to create a listener socket of
//          this type
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventConnectRequest - indicates that a remote device wants
//          to connect to a listener socket. You must respond to this event by
//          calling BtLibSocketRespondToConnection on that listener socket to
//          accept or reject the connection.
//
status_t BtLibSocketListen(
    BtLibSocketRef              socketRef,  // -> the socket to listen on
    BtLibSocketListenInfoType*  listenInfoP // <-> protocol-specific info
);

//------------------------------------------------------------------------------
// Create an outbound L2Cap, RfComm, SCO, or BNEP connection.
//
// Returns:
//      btLibErrPending - The results will be returned in an event.
//      btLibErrNoAclLink - ACL link for remote device does not exist
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventConnectedOutbound - This event with a status of zero
//          indicates successful connection. A non-zero status gives a reason
//          for failure.
//      btLibSocketEventDisconnected - If the connection fails or if connection
//          establishment is successful then this event can occur when the
//          channel disconnects.
//
status_t BtLibSocketConnect(
    BtLibSocketRef               socketRef,  // -> the socket to connect
    BtLibSocketConnectInfoType*  connectInfo // -> remote device addr and
                                             //    protocol-specific information
);

//------------------------------------------------------------------------------
// Accept or reject an in-bound connection on a given listener socket. Called in
// response to btLibSocketEventConnectRequest event devlivered to a listener
// socket. A New connection socket is returned in a
// btLibSocketEventConnectedInbound event to the listener socket after
// BtLibSocketRespondToConnection is called.
//
// Returns:
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSocketRole - the socket passed in is not Listening.
//      btLibErrFailed - Listener socket does not have a pending connection. If
//          this error occurs it means that the function has been called with
//          the wrong listener, or that this function was called even though no
//          btLibSocketEventConnectRequest event occured for the socket.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventConnectedInbound - the connection was made. Contains
//          the reference for the new connection socket.
//      btLibSocketEventDisconnected - the connection failed, or the rejection
//          is complete.
//
//
status_t BtLibSocketRespondToConnection(
    BtLibSocketRef  socketRef, // -> a listener socket
    Boolean         accept     // -> true to accept, false to reject
);

//------------------------------------------------------------------------------
// Send data over a connected L2Cap, RFCOMM, or BNEP socket. If the socket is
// not one of those types, or it is not connected, then the data will be
// silently discarded.
//
// NOTE: in versions of BtLib prior to OS 6, this function normally returned
// btLibErrPending, and the caller was required to wait for the
// btLibSocketEventSendComplete event before reusing the data buffer. This is
// no longer the case; the application can reuse the buffer immediately.
//
// Returns:
//      btLibErrNoError - success
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//      <other> - error code from IOSWrite
//
// Events:
//      btLibSocketSendComplete -- the data has reached the bluetooth radio
//
// Note that
//      err = BtLibSocketSend( socketRef, data, dataLen );
// is equivalent to
//      IOSWrite( socketRef, data, dataLen, &err );
//
status_t BtLibSocketSend(
    BtLibSocketRef  socketRef, // -> a connected L2Cap or RfComm socket
    uint8_t*        data,      // -> buffer containing the data to send
    uint32_t        dataLen    // -> size in bytes of the data to send
);

//------------------------------------------------------------------------------
// Advance flow-control credits on the given connected RFCOMM socket.
//
// RFCOMM uses a credit-based flow control mechanism. Each credit value
// represents one RFCOMM packet. Advancing N credits allows the remote device
// to send N packets. Once those packets have been sent, the remote device can
// no longer send (flow is off). Subsequent calls to this function will allow
// the remote device to send again (flow is on). Credits are additive, so
// calling this function once with 3 credits and then with 2 credits will grant
// a total of 5 credits to the remote device, allowing the remote device to send
// 5 packets.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrFailed - to many credits advanced
//      btLibErrSocketProtocol - socket is not an RFCOMM socket
//      btLibErrSocketRole - socket is not a connection socket
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSocketAdvanceCredit(
    BtLibSocketRef  socketRef, // a connected RFCOMM socket
    uint8_t         credit     // the number of credits to advance
);

//------------------------------------------------------------------------------
// Get socket information. The parameter infoType determines what kind of info
// to get, and type type of the object pointed to by valueP.
//
// infoType (note)                           type of valueP
// ---------------                           --------------
// btLibSocketInfo_Protocol                  BtLibProtocolEnum*
// btLibSocketInfo_RemoteDeviceAddress       BtLibDeviceAddressType*
// btLibSocketInfo_SendPending               Boolean*
// btLibSocketInfo_MaxTxSize (1)             uint32_t*
// btLibSocketInfo_MaxRxSize (1)             uint32_t*
// btLibSocketInfo_L2CapPsm (2)              BtLibL2CapPsmType*
// btLibSocketInfo_L2CapChannel              BtLibL2CapChannelIdType*
// btLibSocketInfo_RfCommOutstandingCredits  uint16_t*
// btLibSocketInfo_RfCommServerId (3)        BtLibRfCommServerIdType*
// btLibSocketInfo_SdpServiceRecordHandle    BtLibSdpRemoteServiceRecordHandle*
//
// (1) The MaxTxSize and MaxRxSize will be the same except in the case where
// RFCOMM is using credit-based flow control. In that case MaxRxSize will be
// equal to the negotiated frame size and MaxTxSize will equal MaxRxSize - 1.
// We transmit packets with credit information which takes one byte away from
// the maximum frame size available for transmit. However, the spec does not
// require that the credit information is always included so it is possible that
// a non-palm implementation of RFCOMM could send a packet where the byte we
// we reserve for credit information is used for additional data.
//
// (2) For an outbound L2Cap socket, the L2CapPsm returned is the psm used to
// connect to the remote device. For an inbound L2Cap socket the L2CapPsm
// returned is the psm the remote devices would use to connect to us.
//
// (3) The RfCommServerId is only valid for RFCOMM listener sockets.
//
// Returns:
//      btLibErrNoError - success, results are placed in *valueP.
//      btLibErrSocketRole - the socket has the wrong role for the request
//      btLibErrSocketProtocol - wrong protocol for the given request
//      btLibErrParamError - an invalid param was passed in.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSocketGetInfo(
    BtLibSocketRef       socketRef, // -> the socket
    BtLibSocketInfoEnum  infoType,  // -> type of info to get
    void*                valueP,    // <- memory to receive the info (type
                                    //    depends upon infoType)
    uint32_t             valueSize  // -> size in bytes of the receiving memory
);

//------------------------------------------------------------------------------
// Return the name of the given Socket event code.
//
const char* BtLibSocketEventName( BtLibSocketEventEnum event );


/*******************************************************************************
 * SDP
 *******************************************************************************/

// The SDP API is used create and advertise service records to remote devices,
// and to discover services available on remote devices. Only one outstanding
// query at a time is allowed per socket.
//
// To fully understand SDP service records, how they are encoded, interpreted,
// etc., see the Service Discovery Protocol section in Volume 1 of the the
// Bluetooth Specification, version 1.1. What follows are extracts from that
// document summarizing the main ideas.
//
// A service record is a sequence of service attributes.
//
// A service attribute consists of two components: an attribute ID and an
// attribute value.
//
// Universal attributes are those service attributes whose definitions are
// common to all service records. Among them is the ServiceClassIDList -- a
// list of 'service class' identifiers. Every service record must have a
// ServiceClassIDList.
//
// An attribute ID is a 16-bit unsigned integer that distinguishes each
// service attribute from other service attributes within a service record.
// The attribute ID also identifies the semantics of the associated attribute
// value.
//
// An attribute value is a data element whose meaning is determined by the
// attribute ID associated with it and by the service class of the service
// record in which the attribute is contained.
//
// A data element is a typed data representation. It consists of two fields: a
// header field and a data field. The header field, in turn, is composed of two
// parts: a type descriptor and a size descriptor. The data is a sequence of
// bytes whose length is specified in the size descriptor and whose meaning is
// (partially) specified by the type descriptor.
//

//------------------------------------------------------------------------------
// Query the given remote device for any SDP service record that contains all of
// the given service class UUIDs, and that offers the service via RFCOMM.
// Note that if there are multiple service records satisfying the query, only
// one of them will be found.
//
// Returns:
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSocketRole - Socket has incorrect role.
//      btLibErrSocketProtocol - Must pass in an SDP socket.
//      btLibErrOutOfMemory - Could not get memory to perform the query.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetServerChannelByUuid - This event with a status of
//          btLibErrNoError indicates that the query was successful; in that
//          case the the following fields of the event are filled in:
//              sdpByUuid.remoteHandle -> handle on the remote service record
//              sdpByUuid.param.channel -> RFCOMM channel for the service
//          This event with any other status indicates that the query failed.
//
status_t BtLibSdpGetServerChannelByUuid(
    BtLibSocketRef           socketRef,       // -> an SDP socket
    BtLibDeviceAddressType*  rDev,            // -> remote device to query
    BtLibSdpUuidType*        serviceUUIDList, // -> list of service class UUIDs
    uint8_t                  uuidListLen      // -> count of service class UUIDs
                                              //    (max 12)
);

//------------------------------------------------------------------------------
// Query the given remote device for any SDP service record that contains all of
// the given service class UUIDs, and that offers the service via L2CAP.
// Note that if there are multiple service records satisfying the query, only
// one of them will be found.
//
// Returns:
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSocketRole - Socket has incorrect role.
//      btLibErrSocketProtocol - Must pass in an SDP socket.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetPsmByUuid - This event with a status of
//          btLibErrNoError indicates that the query was successful; in that
//          case the the following fields of the event are filled in:
//              sdpByUuid.remoteHandle -> handle on the remote service record
//              sdpByUuid.param.psm -> L2CAP psm for the service
//          This event with any other status indicates that the query failed.
//
status_t BtLibSdpGetPsmByUuid(
    BtLibSocketRef           socketRef,       // -> an SDP socket
    BtLibDeviceAddressType*  rDev,            // -> remote device to query
    BtLibSdpUuidType*        serviceUUIDList, // -> list of service class UUIDs
    uint8_t                  uuidListLen      // -> count of service class UUIDs
                                              //   (max 12)
);

//------------------------------------------------------------------------------
// Get handles to all service records that are advertised on the given remote
// device and that contain all of the given service class UUIDs.
//
// Returns:
//      btLibErrPending - The results will be returned in an event.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrSocketProtocol - Must pass in an SDP socket.
//      btLibErrInProgress - Query already pending on this socket.
//      btLibErrNoAclLink - No ACL link in place to remote device.
//      btLibErrOutOfMemory - Could not get memory to do the query.
//      btLibErrBusy - Failed because connection is parked.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpServiceRecordHandle - This event with a status of
//          btLibErrNoError contains the handles to the remote service records
//          that satisfy the query. This event with any other status means
//          that the SDP operation did not complete successfully.
//
status_t BtLibSdpServiceRecordsGetByServiceClass(
    BtLibSocketRef           socketRef,   // -> an SDP socket
    BtLibDeviceAddressType*  rDev,        // -> remote device to query
    BtLibSdpUuidType*        uuidList,    // -> list of service class UUIDs
    uint16_t                 uuidListLen  // -> number of UUIDs (max 12)
);

//------------------------------------------------------------------------------
// Create an SDP service record.
//
// Returns:
//      btLibErrNoError - Indicates that the SDP Record was created.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to create the record.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordCreate(
    int32_t                fdME,   // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle*  recordH // <- receives the handle to the newly
                                   //    created service record
);

//------------------------------------------------------------------------------
// Destroy an SDP service record (frees memory).
//
// Returns:
//      btLibErrNoError - Indicates that the SDP Record was destroyed.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordDestroy(
    int32_t               fdME,   // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle  recordH // -> local service record to destroy
);

//------------------------------------------------------------------------------
// Advertise a local service record so that remote devices can query it.
//
// Returns:
//      btLibErrNoError - The record was previously not advertised, but now is.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrSdpFormat - Sdp record is improperly formatted.
//      btLibErrSdpRemoteRecord - Operation not valid on remote record.
//      btLibErrSdpAdvertised - The record was already being advertised.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordStartAdvertising(
    int32_t               fdME,   // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle  recordH // -> local service record to advertise
);

//------------------------------------------------------------------------------
// Stop advertising a local service record.
//
// recordH -> SDP record that is currently advertised.
//
// Returns:
//      btLibErrNoError - Indicates that the SDP Record is no longer advertised.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrSdpNotAdvertised - Record was not advertised.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordStopAdvertising(
    int32_t              fdME,    // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle recordH  // -> the local service record
);

//------------------------------------------------------------------------------
// Set up a basic local service record for L2Cap and RFCOMM listener sockets.
//
// Returns:
//      btLibErrNoError - Indicates that the SDP Record was set up.
//      btLibErrSdpRemoteRecord - Operation not valid on remote record.
//      btLibErrSdpAdvertised - Operation not valid on advertised record.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordSetAttributesForSocket(
    BtLibSocketRef        socketRef,       // -> an RFCOMM or L2CAP socket in
                                           //    listening mode
    BtLibSdpUuidType*     serviceUuidList, // -> list of UUIDs for service rec
    uint8_t               uuidListLen,     // -> length of list (max 12)
    const char*           serviceName,     // -> name of service (ascii only)
    uint16_t              serviceNameLen,  // -> length of serviceName
    BtLibSdpRecordHandle  recordH          // -> local service record to set up
);

//------------------------------------------------------------------------------
// Map an empty, local service record to the given remote service record. After
// the mapping has been established, the BtLibSdpServiceRecordGetXxx functions
// can be applied to the local service record to obtain info from the remote
// service record.
//
// Returns:
//      btLibErrNoError - Indicates that the mapping was successful.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrSocketProtocol - Must pass in an SDP socket.
//      btLibErrSdpMapped - socket or SDP record is already mapped.
//      btLibErrOutOfMemory - Could not get memory to do the mapping.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordMapRemote(
    BtLibSocketRef                     socketRef,    // -> an SDP socket
    BtLibDeviceAddressType*            rDev,         // -> the remote device
    BtLibSdpRemoteServiceRecordHandle  remoteHandle, // -> remote service record
    BtLibSdpRecordHandle               recordH       // -> empty local serv rec
);

//------------------------------------------------------------------------------
// Set the value of (an element of) the given Universal Service Attribute of the
// given service record. This function only applies to service records that are
// local and not currently advertised.
//
// For attributes whose value is a list of non-list elements, listNumber must
// be zero, and listEntry defines which element of the list to modify. For
// attributes whose value is a list of lists (e.g. the ProtocolDescriptorList
// attribue), listNumber defines which inner list to modify, and listEntry
// defines which element of that inner list to modify.
//
// For both listNumber and listEntry, a value strictly less than the number of
// existing lists or elements implies the modification of an existing list or
// element, while a value equal to the number of lists or elements implies the
// creation of a new list or element.
//
// Returns:
//      btLibErrNoError - the attribute value was set successfully
//      btLibErrSdpRemoteRecord - Operation not valid on remote record.
//      btLibErrSdpAdvertised - Operation not valid on advertised record.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      None
//
status_t BtLibSdpServiceRecordSetAttribute(
    int32_t                     fdME,            // -> Mgmt Entity file desc
    BtLibSdpRecordHandle        recordH,         // -> local service record
    BtLibSdpAttributeIdType     attributeID,     // -> id of attribute to be set
    BtLibSdpAttributeDataType*  attributeValue,  // -> value to assign to attr
    uint16_t                    listNumber,      // -> which list to use
                                                 //    (0 for first list)
    uint16_t                    listEntry        // -> which element in list
                                                 //    (0 for first element)
);

//------------------------------------------------------------------------------
// Get the value of (an element of) the given Universal Service Attribute in the
// given service record.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// For attributes whose value is a list of non-list elements, listNumber must
// be zero, and listEntry defines which element of the list to obtain. For
// attributes whose value is a list of lists (e.g. the ProtocolDescriptorList
// attribue), listNumber defines which inner list to modify, and listEntry
// defines which element of that inner list to obtain.
//
// Returns:
//      btLibErrNoError - Successful retrieval of the attribute value.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only).
//      btLibErrBusy - Failed because connection is parked (remote only).
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetAttribute - This event with a status of
//          btLibErrNoError returns the value of the desired attribute.
//          Any other status means that the SDP operation did not complete
//          successfully.
//
status_t BtLibSdpServiceRecordGetAttribute(
    int32_t                    fdME,           // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle       recordH,        // -> local service record
                                               //    (perhaps mapped to remote)
    BtLibSdpAttributeIdType    attributeID,    // -> id of the attribute
    BtLibSdpAttributeDataType* attributeValue, // <- receives the value
    uint16_t                   listNumber,     // -> which list to use (0==1st)
    uint16_t                   listEntry       // -> which entry in list (0==1st)
);

//------------------------------------------------------------------------------
// Get the size of the value of the given attribute in the given service record.
// The attribute's value must be of type string or URL.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// Returns:
//      btLibErrNoError - Sucessful retrieval of the the attribute value's size.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only).
//      btLibErrBusy - Failed because connection is parked (remote only).
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetStringLen - This event with a status of
//          btLibErrNoError indicates that size variable has been filled in with
//          the SDP response results. If the status is btLibErrNoError then the
//          associated event data is valid, otherwise the eventData is not valid
//          because the SDP operation did not complete successfully.
//
status_t BtLibSdpServiceRecordGetStringOrUrlLength(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> local service record
                                          //    (perhaps mapped to remote)
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribute of interest
    uint16_t*                size         // <- receives the size of attribute
);

//------------------------------------------------------------------------------
// Get the number of entries in the given list in the value of the given
// attribute in the given service record. The attribute's value must be of type
// list.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// Returns:
//      btLibErrNoError - Successful retrieval of the number of entries.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only)
//      btLibErrBusy - Failed because connection is parked (remote only)
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetNumListEntries - This event with a status of
//          btLibErrNoError indicates that *numEntries has been filled in with
//          the SDP response results. If the status is btLibErrNoError then the
//          associated event data is valid, otherwise the eventData is not valid
//          because the SDP operation did not complete successfully.
//
status_t BtLibSdpServiceRecordGetNumListEntries(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> local service record
                                          //    (perhaps mapped to remote)
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribute
    uint16_t                 listNumber,  // -> which list to use (usually 0)
    uint16_t*                numEntries   // <- receives the number of entries
                                          //    in the list
);

//------------------------------------------------------------------------------
// Get the number of lists in the value of the given attribute in the given
// service record. The attribute's value must be of type list. If it is a
// list of non-list elements, the returned count is zero. If it is a list of
// lists, the returned count is the number of inner lists.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// recordH -> SDP record.
// attribute -> type of attribute to get number of lists for.
// numLists <-  number of lists.
//
// Returns:
//      btLibErrNoError - Successful retrieval of the number of lists.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only)
//      btLibErrBusy - Failed because connection is parked (remote only)
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetNumLists - This event with a status of
//          btLibErrNoError indicates that numLists variable has been filled in
//          with the SDP response results. If the status is btLibErrNoError then
//          the associated event data is valid, otherwise the eventData is not
//          valid because the SDP operation did not complete successfully.
//
status_t BtLibSdpServiceRecordGetNumLists(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> local service record
                                          //    (perhaps mapped to remote)
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribute
    uint16_t*                numLists     // <- receives the the number of lists
);

//------------------------------------------------------------------------------
// Compare two UUIDs
//
// Returns:
//      btLibErrNoError - UUIDs match
//      btLibErrError - UUIDs do not match
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
//    Events:
//          None
//
status_t BtLibSdpCompareUuids(
    int32_t            fdME,  // -> Mgmt Entity file descriptor
    BtLibSdpUuidType*  uuid1, // -> a UUID
    BtLibSdpUuidType*  uuid2  // -> another UUID
);

/*******************************************************************************
 * Raw SDP
 *******************************************************************************/

// Raw SDP APIs - the data set/retrieved by these functions are in the 'wire'
// (ok, 'air') format as described by the the the Bluetooth 1.1 specification.

//------------------------------------------------------------------------------
// Set the value of the given attribute of the given service record. This
// function only applies to service records that are local and not currently
// advertised.
//
// Returns:
//      btLibErrNoError - The attribute value was set successfully.
//      btLibErrSdpRemoteRecord - Operation not valid on remote record.
//      btLibErrSdpAdvertised - Operation not valid on advertised record.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
//    Events:
//          None
//
status_t BtLibSdpServiceRecordSetRawAttribute(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> local service record
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribute
    const uint8_t*           value,       // -> raw value of the attribute
    uint16_t                 valSize      // -> size in bytes of the value
);

//------------------------------------------------------------------------------
// Get the value of the given attribute of the given service record.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// Returns:
//      btLibErrNoError - Successful retrival of the attribute's value.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only).
//      btLibErrBusy - Failed because connection is parked (remote only).
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetRawAttribute - This event with a status of
//          btLibErrNoError indicates that value and valSize variables have
//          been filled in with the response results. If the status is
//          btLibErrNoError then the associated event data is valid,
//          otherwise the eventData is not valid because the SDP operation
//          did not complete successfully.
//
status_t BtLibSdpServiceRecordGetRawAttribute(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> local service record
                                          //   (perhaps mapped to remote)
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribue
    uint8_t*                 value,       // <- buffer to receive raw value
    uint16_t*                valSize      // <-> in:  size (bytes) of buffer
                                          //     out: size of received value
);

//------------------------------------------------------------------------------
// Get the size of value of the given attribute in the given service record.
//
// The paramter recordH identifies a local service record, but if that record
// has been mapped to a remote service record by BtLibSdpServiceRecordMapRemote,
// then the value returned is from the remote record. If it is necessary to
// access the remote record to satisfy the query, then this function will return
// btLibErrPending, and the result will be returned in an event on the SDP
// socket that was used to define the mapping.
//
// Returns:
//      btLibErrNoError - Successful retrieval of the attribute value's size.
//      btLibErrPending - The results will be returned in an event.
//      btLibErrSdpAttributeNotSet - Attribute is not in record.
//      btLibErrNoAclLink - ACL link for remote device does not exist.
//      btLibErrParamError - Invalid parameter passed to function.
//      btLibErrOutOfMemory - Could not get memory to perform the operation.
//      btLibErrInProgress - Query already pending on this socket (remote only)
//      btLibErrBusy - Failed because connection is parked (remote only)
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      btLibSocketEventSdpGetRawAttributeSize - This event with a status
//          of btLibErrNoError indicates that the size variable has been
//          filled in with the SDP response results. If the status is
//          btLibErrNoError, then the associated event data is valid,
//          otherwise the eventData is not valid because the SDP operation
//          did not complete successfully.
//
status_t BtLibSdpServiceRecordGetSizeOfRawAttribute(
    int32_t                  fdME,        // -> Mgmt Entity file descriptor
    BtLibSdpRecordHandle     recordH,     // -> the service record
                                          //    (perhaps mapped to remote)
    BtLibSdpAttributeIdType  attributeID, // -> id of the attribute
    uint16_t*                size         // <- receives size of value
);

//------------------------------------------------------------------------------
// A raw data element consists of a header portion followed by a data portion.
// This function determines the offset to, and the size of, the data portion of
// the given data element.
//
// Returns:
//      btLibErrNoError - successfully parsed the attribute.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      none
//
status_t BtLibSdpParseRawDataElement(
    int32_t         fdME,   // -> Mgmt Entity file descriptor
    const uint8_t*  value,  // -> the raw data element
    uint16_t*       offset, // <- receives the offset to the data portion
    uint32_t*       length  // <- receives the size of the data portion
);

//------------------------------------------------------------------------------
// Verify that the given raw data element is properly formed, and occupies no
// more than valSize bytes.
//
// In the case of data element Sequences or Alternates, the verification is
// applied recursively to the elements within the Sequences or Alternates.
// The parameter maxLevel is used to stop the recursion in the case of bad data.
// maxLevel must be large enough to handle the whole data element. For example,
// a simple data element such as btLibDETD_UINT would need a maxLevel of 1, but
// a Data Element Sequence of UUIDs would need a maxLevel of 2.
//
// Returns:
//      btLibErrNoError - the Data Element is properly formatted, and occupies
//          no more than valSize bytes.
//      btLibErrError - the Data Element is not properly formatted, or occupies
//          more than valSize bytes.
//      btLibErrParamError - Invalid parameter passed to function.
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
// Events:
//      none
//
status_t BtLibSdpVerifyRawDataElement(
    int32_t         fdME,    // -> Mgmt Entity file descriptor
    const uint8_t*  value,   // -> the raw Data Element
    uint16_t        valSize, // -> size of a buffer to contain the Data Element
    uint8_t         maxLevel // -> maximum number of recursions (at least 1)
);

//------------------------------------------------------------------------------
// Get a raw data element's type descriptor.
//
// header -> the first byte of a data element
//
// Returns:
//      The type of the Data Element.
//
#define  BtLibSdpGetRawElementType(header) ((header) & btLibDETD_MASK)

//------------------------------------------------------------------------------
// Get a raw data element's size descriptor.
//
// header -> the first byte of a data element
//
// Returns:
//      The size of the Data Element.
//
#define BtLibSdpGetRawDataElementSize(header) ((header) & btLibDESD_MASK)

/*******************************************************************************
 * Security
 *******************************************************************************/

//------------------------------------------------------------------------------
// Search the trusted device database by Bluetooth device address.
//
// The name of this database should probably be the "paired devices database".
// Every device in this db has been paired with the local device (i.e. shares
// a secret link key with the local device), but some devices are marked as
// being 'trusted' (i.e. 'bonded'), and others not. This function returns the
// index to the record for the device with the given address whether the
// 'trusted' bit is set or not.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrNotFound - the given device could not be found in the database
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
status_t BtLibSecurityFindTrustedDeviceRecord(
    int32_t                  fdME,  // -> Mgmt Entity file descriptor
    BtLibDeviceAddressType*  addrP, // -> Bluetooth device address to look for
    uint16_t*                indexP // <- receives index to found record
);

//------------------------------------------------------------------------------
// Remove a record from the trusted device database.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrNotFound - invalid index
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
status_t BtLibSecurityRemoveTrustedDeviceRecord(
    int32_t   fdME,  // -> Mgmt Entity file descriptor
    uint16_t  index  // -> index to the record to remove
);

//------------------------------------------------------------------------------
// Get information from a given record in the trusted device database:
// - the Bluetooth device address
// - the user-friendly name of the device
// - the class-of-device
// - the time of last connection (number of seconds since midnight Jan 1, 1904)
// - the 'trusted' flag (all devices in the db are paired, only those marked
//   with the 'trusted' flag are persistent)
//
// If any of the above pieces of info is not desired, the corresponding
// parameter that points to the buffer to receive that piece of info may be set
// to NULL.
//
// If the buffer to receive the user-friendly name is too small, the name will
// be truncated if necessary (a name may be as long as btLibMaxDeviceNameLength
// bytes, including the null terminator).
//
// Returns:
//      btLibErrNoError - success
//      btLibErrNotFound - if a record with the given index could not be found
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
status_t BtLibSecurityGetTrustedDeviceRecordInfo(
    int32_t                  fdME,           // -> Mgmt Entity file descriptor
    uint16_t                 index,          // -> index to a record
    BtLibDeviceAddressType*  addrP,          // <- receives the device address
    char*                    nameBuffer,     // <- receives the device name
    uint8_t                  nameBufferSize, // -> size of name buffer (bytes)
    BtLibClassOfDeviceType*  codP,           // <- receives the class-of-device
    uint32_t*                lastConnectedP, // <- receives the time of last
                                             //    connnect (secs from 1/1/1904)
    Boolean*                 trustedP        // <- receives the 'trusted' flag
);

//------------------------------------------------------------------------------
// Return in *numP either (1) the total number of devices, or (2) the number of
// devices with the 'trusted' bit, in the trusted device database.
//
// Returns:
//      btLibErrNoError - success
//      iosErrBadFD - invalid file descriptor
//      iosErrNotOpened - file descriptor does not refer to an opened file
//
status_t BtLibSecurityNumTrustedDeviceRecords(
    int32_t    fdME,        // -> Mgmt Entity file descriptor
    Boolean    trustedOnly, // -> true <=> only trusted devices are to be counted
    uint16_t*  numP         // <- receives the number of (trusted) devices
);

/*******************************************************************************
 * Services
 *******************************************************************************/

//------------------------------------------------------------------------------
// Register a persistent Bluetooth service application.
//
// It suffices to register a service app once after system boot. Subsequent
// registrations will be ignored.  For a complete discussion of the registration
// parameters, see the commentary about BtLibServiceRegistrationParamsType in
// <BtLibTypes.h>.
//
// Service apps must respond to the launch codes sysBtLaunchCmdPrepareService
// and sysBtLaunchCmdExecuteService. See the commentary in <BtLibTypes.h> about
// BtLibServicePreparationParamsType and BtLibServiceExecutionParamsType for
// more info.
//
// Returns:
//      btLibErrNoError - success
//      btLibErrTooMany - the maximum number of services is already registered
//
status_t BtLibRegisterService(
    BtLibServiceRegistrationParamsType*  params
);

/*******************************************************************************
 * Bluetooth byte ordering routines
 *******************************************************************************/
#define _BtLibNetSwap16(x) \
   ((((x) >> 8) & 0xFF) | \
    (((x) & 0xFF) << 8))

#define _BtLibNetSwap32(x) \
   ((((x) >> 24) & 0x00FF) | \
    (((x) >>  8) & 0xFF00) | \
    (((x) & 0xFF00) <<  8) | \
    (((x) & 0x00FF) << 24))

#if  (CPU_ENDIAN == CPU_ENDIAN_BIG)
// convert SDP byte orders (SDP is Big Endian)
// convert network long to host long
#define     BtLibSdpNToHL(x)  (x)

// convert network int16_t to host int16_t
#define     BtLibSdpNToHS(x)  (x)

// convert host long to network long
#define     BtLibSdpHToNL(x)  (x)

// convert host int16_t to network int16_t
#define     BtLibSdpHToNS(x)  (x)

// convert RFCOMM byte orders (RFCOMM is Big Endian)
#define     BtLibRfCommNToHL(x)  (x)
#define     BtLibRfCommNToHS(x)  (x)
#define     BtLibRfCommHToNL(x)  (x)
#define     BtLibRfCommHToNS(x)  (x)

// convert L2CAP byte orders (L2CAP is Little Endian)
#define     BtLibL2CapNToHL(x) _BtLibNetSwap32(x)
#define     BtLibL2CapNToHS(x) _BtLibNetSwap16(x)
#define     BtLibL2CapHToNL(x) _BtLibNetSwap32(x)
#define     BtLibL2CapHToNS(x) _BtLibNetSwap16(x)

#elif  (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
// convert SDP byte orders (SDP is Big Endian)
// convert network long to host long
#define     BtLibSdpNToHL(x)  _BtLibNetSwap32(x)

// convert network int16_t to host int16_t
#define     BtLibSdpNToHS(x)  _BtLibNetSwap16(x)

// convert host long to network long
#define     BtLibSdpHToNL(x)  _BtLibNetSwap32(x)

// convert host int16_t to network int16_t
#define     BtLibSdpHToNS(x)  _BtLibNetSwap16(x)

// convert RFCOMM byte orders (RFCOMM is Big Endian)
#define     BtLibRfCommNToHL(x)  _BtLibNetSwap32(x)
#define     BtLibRfCommNToHS(x)  _BtLibNetSwap16(x)
#define     BtLibRfCommHToNL(x)  _BtLibNetSwap32(x)
#define     BtLibRfCommHToNS(x)  _BtLibNetSwap16(x)

// convert L2CAP byte orders (L2CAP is Little Endian)
#define     BtLibL2CapNToHL(x) (x)
#define     BtLibL2CapNToHS(x) (x)
#define     BtLibL2CapHToNL(x) (x)
#define     BtLibL2CapHToNS(x) (x)
#else
#error "No CPU_ENDIAN defined"
#endif

/*******************************************************************************
 * Bluetooth device address translation and conversion routines
 *******************************************************************************/

//------------------------------------------------------------------------------
// Convert a Bluetooth device address from binary form to ascii string form.
//
// The binary form occupies six octets. The output string form encodes the
// value of each octet with two hex chars, and the octets are separated by
// colons, e.g. "01:23:45:67:89:AB". The first octet in the binary form is the
// last octet in the string form, and vice-versa.
//
// Returns:
//      btLibErrNoError - Indicates successful conversion
//      btLibErrParamError - devAddrP is null, or strBuf is null, or
//          strBufSize is too small
//
// Events:
//      None
//
status_t BtLibAddrBtdToA(
    BtLibDeviceAddressType*  devAddrP,   // -> the binary form
    char*                    strBuf,     // <- buffer to receive the string form
    uint16_t                 strBufSize  // -> size of the string buffer
);

//------------------------------------------------------------------------------
// Convert a Bluetooth device address from ascii string form to binary form.
//
// The input string must encode the value of each octet using one or more
// hex characters, and the octets must separated by colons, but there can be
// no leading nor trailing colons. Here's an example of a valid input string:
// "0:1:23:0045:ab:CD". The output binary form occupies six octets. The first
// octet in the string form is the last octet in the binary form, and
// vice-versa.
//
// Returns:
//      btLibErrNoError - Indicates successful conversion
//      btLibErrParamError - strBuf is null, or devAddrP is null, or the string
//          is badly formatted
//
// Events:
//      None
//
status_t BtLibAddrAToBtd(
    const char*              strBuf,  // -> the string form
    BtLibDeviceAddressType*  devAddrP // <- receives the binary form
);


#ifdef __cplusplus
}
#endif

#endif /* _BTLIB_H */
