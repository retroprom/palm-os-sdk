/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: wifi.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef wifi_H
#define wifi_H

#include <PalmTypes.h>
#include <sys/ioccom.h>

//
// Constants
//

#define WifiSSIDMaxLen				32

// Security Capabilities
#define WifiSecOpen					(1UL << 0)
#define WifiSecWEP					(1UL << 1)

// Power Modes
#define WifiPowerOff				1UL
#define WifiPowerOffHardSwitch		2UL
#define WifiPowerOnNormal			3UL
#define WifiPowerOnPowerSave		4UL

// Scan Capabilities, used in WifiScanResultsType
#define WifiCapAccessPoint			(1UL << 0)
#define WifiCapAdHocNetwork			(1UL << 1)
#define WifiCapPrivacy				(1UL << 4)

// RSSI Update Modes, used in WifiRSSIUpdateType
#define	WifiRSSIUpdateNever			1UL
#define	WifiRSSIUpdateOnDelta		2UL
#define	WifiRSSIUpdatePeriodic		3UL
#define	WifiRSSIUpdateAlways		4UL

// WEP Flags, used with WIOCGET/SETWEPFLAGS
#define WifiWEPExcludeUnencrypted	(1UL << 0)
#define WifiWEPIVReuseEvery			(1UL << 1)
#define WifiWEPIVReuse10			(1UL << 2)
#define WifiWEPIVReuse50			(1UL << 3)
#define WifiWEPIVReuse100			(1UL << 4)
#define WifiWEPIVReuseMask			0x0000001E

// Transmission Rates
#define	WifiRate_0Mbit		0x00000000
#define	WifiRate_1Mbit  	(1UL << 0)
#define	WifiRate_2Mbit  	(1UL << 1)
#define	WifiRate_5_5Mbit 	(1UL << 2)
#define	WifiRate_6Mbit  	(1UL << 3)
#define	WifiRate_9Mbit  	(1UL << 4)
#define	WifiRate_11Mbit 	(1UL << 5)
#define	WifiRate_12Mbit 	(1UL << 6)
#define	WifiRate_18Mbit 	(1UL << 7)
#define	WifiRate_22Mbit 	(1UL << 8)
#define	WifiRate_24Mbit 	(1UL << 9)
#define	WifiRate_33Mbit 	(1UL << 10)
#define	WifiRate_36Mbit 	(1UL << 11)
#define	WifiRate_48Mbit 	(1UL << 12)
#define	WifiRate_54Mbit 	(1UL << 13)
#define WifiRate_All		0xffffffff

// Channels
#define WifiChannel_1		(1UL << 0)
#define WifiChannel_2		(1UL << 1)
#define WifiChannel_3		(1UL << 2)
#define WifiChannel_4		(1UL << 3)
#define WifiChannel_5		(1UL << 4)
#define WifiChannel_6		(1UL << 5)
#define WifiChannel_7		(1UL << 6)
#define WifiChannel_8		(1UL << 7)
#define WifiChannel_9		(1UL << 8)
#define WifiChannel_10		(1UL << 9)
#define WifiChannel_11		(1UL << 10)
#define WifiChannel_12		(1UL << 11)
#define WifiChannel_13		(1UL << 12)
#define WifiChannel_14		(1UL << 13)
#define WifiChannel_All		0xffffffff

// Status 
#define WifiStatusUndefined 			0UL
#define	WifiStatusDisconnected			1UL
#define	WifiStatusConnecting			2UL
#define	WifiStatusConnectedAccessPoint 	3UL
#define	WifiStatusConnectedAdHoc		4UL
#define	WifiStatusOutOfRange			5UL
#define	WifiStatusConnectionFailed		6UL

// Wifi Async Event Types
#define wifiUndefinedEvent		0UL
#define wifiConnecting			1UL
#define wifiConnectAccessPoint	2UL
#define wifiConnectAdhoc		3UL
#define	wifiConnectFailed		4UL
#define wifiOutOfRange			5UL 
#define	wifiDisconnect			6UL
#define	wifiScanResults			7UL
#define	wifiSignalStrength		8UL
#define	wifiMediaUnavailable	9UL
#define wifiScanFailed			10UL

//
// IOCTL structures
//

typedef struct {
	uint32_t current;
	uint32_t capabilities;
} WifiGetSecCapType;

typedef struct {
	uint32_t supported_rates;
	uint32_t preferred_rates;
	uint32_t current_rate;	
} WifiGetRatesType;

typedef struct {
	uint16_t key;
	uint16_t data_len;
	uint8_t data[16];
} WifiSetWEPKeyType;

typedef struct {
	uint32_t updateMode;
	uint32_t updateValue;
} WifiRSSIUpdateType;

typedef struct WifiGetRSSITag {
	uint8_t signal;
	uint8_t padding[3];
} WifiGetRSSIType;

typedef struct {
	uint32_t timeout;
	char ssid[33];
	uint8_t blockTillCompletion;
	uint8_t pad[2];
} WifiConnectType;

typedef struct {
	char ssid[33];
	uint8_t padding[3];
} WifiSSIDType;

typedef struct {
	uint8_t bssid[6];
} WifiBSSIDType;

typedef struct {
	uint32_t channels;
	uint32_t rates;
	uint32_t timeout;
	char	 ssid[33];
	uint8_t	 blockTillCompletion;
	uint8_t	 padding[2];
} WifiScanRequestType;

typedef struct {
	uint32_t current;
	uint32_t supportedMask;
} WifiChannelType;

typedef struct {
	char		ssid[33];
	int8_t		signal;
	int8_t		noise;
	uint8_t		channel;
	uint8_t		bssid[6];
	uint16_t	ATIMInterval;
	uint32_t	supportedRates;
	uint32_t	responseRate;
	uint32_t	capabilities;
	uint16_t    beaconInterval;	
	uint8_t		padding[2];
} WifiScanResultsType;

typedef struct {
	uint16_t	index;
	uint16_t	last;
	WifiScanResultsType results;
} WifiGetScanResultsType;

typedef struct {
	char ssid[33];    
	uint8_t channel;
	uint8_t padding[2];
} WifiCreateIBSSType;

typedef struct {
	uint8_t bssid[6];
	uint16_t channel;
} WifiJoinType;

typedef struct {
	uint32_t interval;
	uint32_t channelMask;
	uint32_t rateMask;
	char 	 ssid[33];
	uint8_t  enableScanning;
	uint8_t  padding[2];
} WifiPassiveScanType;

typedef struct WifiAttributesTag {
	uint32_t maxRate;
	uint32_t securityFlags;
} WifiAttributesType;

//
// IOCTLs
//

// Power Mode
#define WIOCSETPOWERMODE		_IOWR('w', 1, uint32_t) 
#define WIOCGETPOWERMODE 		_IOR('w', 2, uint32_t)

// Transmission Rates
#define WIOCSETRATES			_IOW('w', 3, uint32_t) 
#define WIOCGETRATES			_IOR('w', 4, WifiGetRatesType)

// Security
#define WIOCSETSECMODE			_IOW('w', 5, uint32_t)
#define WIOCGETSECCAPS			_IOR('w', 6, WifiGetSecCapType)

// WEP Security
#define WIOCSETDEFAULTKEY		_IOW('w', 7, uint32_t)
#define WIOCSETKEY				_IOW('w', 8, WifiSetWEPKeyType)
#define	WIOCSETWEPFLAGS			_IOW('w', 9, uint32_t)
#define WIOCGETWEPFLAGS			_IOR('w', 10, uint32_t)

// Signal Strength
#define WIOCSETRSSIUPDATE		_IOW('w', 11, WifiRSSIUpdateType)
#define WIOCGETRSSIUPDATE		_IOR('w', 12, WifiRSSIUpdateType)
#define WIOCGETCURRENTRSSI		_IOR('w', 13, WifiGetRSSIType)

// Connection
#define WIOCCONNECT				_IOW('w', 14, WifiConnectType)
#define WIOCDISCONNECT			_IO('w', 15)
#define WIOCGETSSID				_IOR('w', 16, WifiSSIDType)
#define WIOCGETBSSID			_IOR('w', 17, WifiBSSIDType)

// Scanning
#define WIOCSCAN				_IOW('w', 18, WifiScanRequestType)
#define WIOCGETSCANRESULTS		_IOWR('w', 19, WifiScanResultsType)
#define WIOCPASSIVESCAN			_IOW('w', 20, WifiPassiveScanType)

#define WIOCGETSTATUS			_IOR('w', 21, uint32_t)
#define WIOCGETCHANNEL			_IOR('w', 22, WifiChannelType)
#define WIOCCREATEIBSS			_IOW('w', 23, WifiCreateIBSSType)
#define WIOCJOIN				_IOW('w', 24, WifiJoinType)

#define WIOCGETMACADDR			_IOR('w', 25, WifiBSSIDType)

//
// Wifi Asynchronous Events
//
// These events are retrieved by polling the management descriptor for the
// wireless interface.
//

typedef struct {
	uint32_t event;
	
	union {
		struct scan{
			uint16_t index;
			uint16_t last;
			WifiScanResultsType results;		
		} scan;
	
		struct connectAccessPoint {
			char	ssid[33];
			uint8_t padding[3];
			uint8_t bssid[6];		
		} connectAccessPoint;
	
		struct connectAdhoc {
			char	ssid[33];
			uint8_t padding[3];
			uint8_t bssid[6];
		} connectAdhoc;
		
		struct connectFailed {
			status_t reasonCode;		
		} connectFailed;
		
		struct scanFailed {
			status_t reasonCode;
		} scanFailed;
		
		struct signalStrength {
			uint8_t signal;
		} signalStrength;
	} data;
} WifiEventType;

#endif	// wifi_H