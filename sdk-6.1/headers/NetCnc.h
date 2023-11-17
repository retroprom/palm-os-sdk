/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: NetCnc.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _NetCnc_h_
#define _NetCnc_h_

#include <CncMgr.h>
/*#include <CncServices.h>*/

#ifdef __cplusplus
extern "C" {
#endif

#define kCncParameter_LocalIP			'LoIP'		// local IP address
#define kCncParameter_RemoteIP			'ReIP'		// remote IP address for point-to-point links
#define kCncParameter_MTU				'_MTU'		// IF MTU
#define kCncParameter_Netmask			'NetM'		// IF (sub)Netmask
#define kCncParameter_Broadcast			'Brod'		// IF Broadcast address
#define kCncParameter_IFFlagsSet		'IFFS'	
#define kCncParameter_IFFlagsClear		'IFFC'
#define kCncParameter_IFId				'IFId'		// IF index
#define kCncParameter_Metric			'Mtic'		// IF metric
#define kCncParameter_DNServers			'DNSs'		// DNS servers
#define kCncParameter_Gateways			'GWys'		// Gateways
#define kCncParameter_Domains			'Doms'		// Bind search domain names
#define kCncParameter_DLPIDeviceName	'DDev'		// DLPI device name
#define kCncParameter_DLPIPPA			'DPPA'		// DLPI "physical point of attachment"
#define kCncParameter_ARP				'_ARP'		// If present, the link layer is ARPable (otherwise, it is automatically set)
#define kCncParameter_ILLName			'LLNa'		// Link layer name such as "eth0" or "ppp0". (Not interface name which are "eth0:0", "eth0:1", ...) 
#define kCncParameter_ILLAddress		'LLAd'		// Link Layer physical address
#define kCncParameter_IdleTimeout		'Idle'		
#define kCncParameter_IP6Type			'6Typ'		// IPv6 type
#define kCncParameter_LocalIP6			'LIP6'		// local IPv6 address
#define kCncParameter_PrefixLen6		'Pre6'		// prefix length 
#define kCncParameter_Gateway6			'GWy6'		// IPv6 gateway
#define kCncParameter_IFId6				'IFI6'		// IF index
#define kCncParameter_TunnelFarIP		'TFar'		// Tunnel far endpoint

// -----------------------------------------------------------------
// IPIF Cnc plugin: Manage network configuration (IP interfaces).

#define kCncIPIFPluginName				"IPIF"

// -----------------------------------------------------------------
// ILL Cnc Plugin: Manage Network Links (IP Link Layer)

#define kCncILLPluginName				"ILL"

// -----------------------------------------------------------------
// DLEther Cnc: Create DLPI virtual driver with DLEther streams

#define kCncDLEPluginName				"DLE"

// The various "anchoring points" in the networking Cnc graph
#define kCncNetOutgoingInterface		"NetOut"

// -----------------------------------------------------------------
// 

#define kCncPPPPluginName				"PPP"

// -----------------------------------------------------------------
#define kCncNetOutAnchorMacro			"{NetOut}"
#define kCncNetProtosMacro				"{Net}"
#define kCncEthernetDeviceMacro			"{ENet}"
#define kCncPPPMacro					"{PPP}"
#define kCncLANConnectionMacro			"{LANCnc}"
#define kCncPTPConnectionMacro			"{PTPCnc}"

#define kCncPTPProtoChooser				"{PTPChooser}"
#define kCncLANDeviceChooser			"{LANChooser}"

// -----------------------------------------------------------------
#define kCncNetOutgoingSearchString		kCncNetOutgoingInterface"/*"

#ifdef __cplusplus
}
#endif

#endif
