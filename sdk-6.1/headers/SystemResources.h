/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SystemResources.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Include file for both PalmRez and the C Compiler. This file contains
 *  equates used by both tools. When compiling using the C compiler
 *  the variable RESOURCE_COMPILER must be defined.
 *
 *****************************************************************************/

#ifndef _SYSTEMRESOURCES_H_
#define	_SYSTEMRESOURCES_H_

//-----------------------------------------------------------
// This section is common to both the C and Resource Compiler
//-----------------------------------------------------------

//................................................................
// File types and creators
//
//	Each database shall have a creator ID and a type.
//
//	The creator ID shall establish which application, patch, or extension
//	a particular database is associated with.  The creator ID should identify
//	the application/patch/extension, NOT who created it.
//
//	The type will determine which part of an application,
//	patch, or extension a particular database is.
//
//	There can be only one database per application, patch, or extension
//	which has type 'application', 'patch', or 'extension'.
//
//	Creators:
//
//	ROM-based applications created by Palm Computing have all-lower case
//	creator ID's.  Third-party applications have creator ID's which
//	are either all caps, or mixed case.  The same requirements go for
//	system patches and extensions.
//
//	All applications, system patches and extensions shall have unique creator
//	ID's.
//
//	Types:
//
//	'Application', 'Extension', and 'Patch' file/database types for ROM-based
//	apps shall be all-lower case (they are defined below).  Other
//	file/database types must be mixed-case,
//	or all caps.  These other types are internal to the applications, and
//	therefore the system is unconcerned with their exact values.
//................................................................

// --------------------------------------------------------------------------------------
// Servers
// --------------------------------------------------------------------------------------

#define	sysFileCDAL						'pdal'	// Creator type for DAL files
#define	sysFileCSystem					'psys'	// Creator type for System files
#define	sysFileCUI						'puil'	// Creator type for the UI shared lib
#define	sysFileCAppMgr					'papp'	// Creator type for AppMgr
#define	sysFileCDataMgr					'pdat'	// Creator type for AppMgr
#define	sysFileCOEMSystem				'poem'	// Creator type for OEM System files
#define	sysFileCPalmDevice				'pdvc'	// Creator type for Palm Devices, analogous to 'poem'
#define	sysFileCBackgroundProcess		'dump'	// Creator type for the main app of the background process
#define sysFileCFaerie					'faer'	// Creator type for the faeries
#define	sysFileCBackupProcess			'bkpr'	// Creator type for the faerie of the backup process
#define sysFileCSmooveD					'smoo'	// The root of all evil

// --------------------------------------------------------------------------------------
// Boot Modules
// --------------------------------------------------------------------------------------

// New creator types for 6.0 boot modules (which are of type 'boot')
#define sysFileCSecSvcs                 'scsv'  // Creator type for Security Services
#define sysFileCAccessControl			'acsv'	// Creator type for Access Control
#define	sysFileCSvcMgr					'svc_'	// Creator type for ServiceMgr
#define	sysFileCIOS						'ios_'	// Creator type for IOS
#define	sysFileCInstallSvr				'insr'	// Creator type for Install Server
#define	sysFileCAttnDriver				'datn'	// Creator type for boot attn driver
#define	sysFileCKeyboardDriver			'dkbd'	// Creator type for boot keyboard driver
#define	sysFileCLCDDriver				'dlcd'	// Creator type for boot LCD driver
#define	sysFileCGRDriver				'dgr_'	// Creator type for boot GR driver
#define	sysFileCAudioDriver				'daud'	// Creator type for boot audio driver
#define	sysFileCPulseDriver				'puls'	// Creator type for boot audio driver
#define sysFileCMMC2Driver				'mmc2'	// Creator type for boot MMC2 driver
#define	sysFileCPenDriver				'dpen'	// Creator type for boot pen driver
#define	sysFileCRtcDriver				'drtc'	// Creator type for boot rtc driver
#define sysFileCPerformanceMgr          'perf'  // Creator type for boot PerformanceMgr driver
#define sysFileCIrDADriver				'irdr'	// Creator type for boot IrDA driver
#define sysFileCDriverServices			'dsvc'  // Creator type for Drivers Services library
#define sysFileCStdinDriver				'stdi'  // Creator type for Standard In driver.
#define sysFileCTermModule				'term'  // Creator type for Terminal module.
#define sysFileCAdminDriver				'ssad'  // Creator type for Streams Admin driver.
#define sysFileCLogDriver				'slog'	// Creator type for Streams Log driver.
#define sysFileCNullDriver				'null'  // Creator type for NULL driver
#define sysFileCCncMgr					'cncm'	// Creator type for CncMgr
#define sysFileCCncSetup				'cncs'	// Creator for connection setup application
#define sysFileCExpansionVFSDriver		'exvd'  // Creator type for Expansion/VFS driver
#define sysFileCModuleMgr				'mmgr'  // Creator type for the Module Manager.
#define sysFileCTCPIP					'ip__'	// Creator type for TCP/IP
#define sysFileCTCPIP6					'ip6_'	// Creator type for TCP/IP (v6)
#define sysFileCTelephonyServer			'tlsv'	// Creator type for the Telephony Server


// Creator IDs for 6.0 device drivers (which are of type 'drvr')
#define sysFileCCDK238SystemDriver    'c238'  // Creator type for DK238 board system driver


// Creator IDs for 6.0 shared libraries (which are of type 'libr')
#define sysFileCSignVfy                 'sign'  // Creator type for Signiture Verification library


// Type and Creator IDs for 6.0 Bluetooth stuff
#define sysFileCBtAdminDaemon			'btad'	// Type ID for Bluetooth Admin Daemon
#define sysFileTBtTransLib				'bttx'	// Type ID for Bluetooth HCI Transport library
#define sysFileTBtPrefsDB				'btpf'	// type ID for Bluetooth Preferences database
#define sysFileTBtCacheDB				'btch'	// Type ID for Bluetooth device cache database
#define sysFileTBttrustedDB				'bttr'	// Type ID for Bluetooth trusted devices database

#define sysFileCBtTransUSB				'btus'	// Creator ID for Bluetooth HCI USB Transport library
#define sysFileCBtTransUART				'btsu'	// Creator ID for Bluetooth HCI UART Transport library
#define sysFileCBtTransRS232			'btsr'	// Creator ID for Bluetooth HCI RS232 Transport library
#define sysFileCBtTransSDIO				'btsd'	// Creator ID for Bluetooth HCI SDIO Transport library
#define sysFileCBtTransBCSP				'btbc'	// Creator ID for Bluetooth HCI BCSP Transport library
#define sysFileCBtStreams				'btst'	// Creator ID for Bluetooth Stack (BtStreams+BtLibLo+BtStack)
#define sysFileCBtLib					'blth'	// Creator ID for Bluetooth API Library
#define sysFileCBtObexServer			'btob'	// Creator ID for Bluetooth Obex Server
#define sysFileCBtEchoService			'btes'	// Creator ID for Bluetooth Echo Service
#define sysFileCBluetoothPanel			'bltp'	// Creator ID for Bluetooth Panel
#define sysFileCBtCncPlugin				'btcp'	// Creator ID for Bluetooth CncMgr Plugin
#define sysFileCBtLibTest				'btlt'	// Creator ID for Bluetooth API Library test app (BtLibTest)
#define sysFileCBtCommTest				'btct'	// Creator ID for Bluetooth Comm test app (BtCommTest)
#define sysFileCBtPrefsDB				sysFileCSystem	// Creator ID for Bluetooth Preferences database
#define sysFileCBtCacheDB				sysFileCSystem	// Creator ID for Bluetooth Device Cache database
#define sysFileCBtTrustedDB				sysFileCSystem	// Creator ID for Bluetooth Trusted Devices database

// --------------------------------------------------------------------------------------
// Libraries.
// --------------------------------------------------------------------------------------

#define	sysFileTLibrary					'libr'	// File type of Shared Libraries
#define	sysFileTLibraryExtension		'libx'	// File type of library extensions

#define	sysFileCNet						'netl'	// Creator type for pre-6.0 Net (TCP/IP) Library
#define	sysFileCDhcpLib					'dhcp'	// Creator type for Dhcp library.
#define	sysFileCRmpLib					'netp'	// Creator type for RMP Library (NetLib plug-in)
#define	sysFileCINetLib 				'inet'	// Creator type for INet Library
#define	sysFileCSecLib 					'secl'	// Creator type for Ir Library
#define	sysFileCWebLib 					'webl'	// Creator type for Web Library
#define	sysFileCIrLib 					'irda'	// Creator type for Ir Library
#define	sysFileCLocalLib				'locl'	// Creator type for Local exchange library
#define	sysFileCLz77Lib					'lz77'	// Creator type for LZ77 Library (Registered)
#define	sysFileCSmsLib					'smsl'	// Creator type for SMS Library
#define	sysFileCBtExgLib				'btex'	// Creator type for Bluetooth Exchange Library
#define	sysFileCPdiLib  				'pdil'	// Creator type for PDI Library
#define sysFileCGraphicsLib				'grli'	// Creator type for the Graphics library.  some day...
#define sysFileCMultimedia				'mmmm'	// Creator type for the C Multimedia (MM*) library
#define sysFileClibbinder				'lbbe'	// Creator for Binder library (featuring support, storage, service, xml, and app kits)
#define sysFileClibinterface			'liin'	// Creator for Interface library (featuring render and view kits)
#define sysFileClibsoap					'soap'	// Creator for SOAP library
#define sysFileClibpng					'lpng'	// Creator for PNG library.  because PNG is cool
#define sysFileClibtest					'tsfw'	// Creator for test framework library
#define sysFileClibstudio				'stud'	// Creator for Studio library (featuring element and widget kits)
#define sysFileCmgl						'lmgl'	// Creator type for mgl library (the last two letters are the key)
#define sysFileClibmedia				'lbme'	// Creator type for libmedia
#define sysFileClibmovieserver			'lbmv'	// Creator type for libmovieserver
#define sysFileClibcodec				'lbmc'	// Creator type for libcodec
#define	sysFileCTelMgrLib  				'tmgr'	// Creator type for Telephony Manager Library
#define	sysFileTPhoneDriver				'phdr'	// File type for Phone Drivers
#define	sysFileTPhoneBaseDriver			'phbd'	// File type for Phone Base Drivers
#define sysFileCGsmGprsBaseDriver		'ggbd'  // Creator type for the Gsm Gprs Base Driver
#define sysFileCGsmGprsStandardDriver	'ggsd'  // Creator type for the Gsm Gprs Standard Driver
#define sysFileCNoteViewLib				'nvwl'	// Note View shared lib for PIM apps.
#define sysFileXSyncLib					'xsyn'	// Direct sync to exchange lib for PIM apps.
#define sysFileCSearchLib				'srch'	// Creator type for the Search library

#define sysFileCIDLookUpLib				'idlk'	// Creator type for IDLookUp Library.

#define sysFileCPhoneBookIOLib			'pbio'	// Creator type for PhoneBookIO Library.

#define	sysFileCSerialMgr				'smgr'	// Creator for SerialMgrNew used for features.
#define	sysFileCSerialWrapper			'swrp'	// Creator type for Serial Wrapper Library.
#define	sysFileCIrSerialWrapper			'iwrp'	// Creator type for Ir Serial Wrapper Library.

#define	sysFileCExpansionMgr			'expn'	// Creator of Expansion Manager extension database
#define	sysFileCVFSMgr					'vfsm'	// Creator code for VFSMgr...
#define sysFileCExpansionVFS			'exvf'  // Creator type for combined Expansion/VFSMgr client library
#define	sysFileCFATFS					'fatf'	// Creator type for FAT filesystem library

#define	sysFileCSdSpiCard				'sdsd'	// Creator type for Slot Driver: SD bus, SPI mode, memory cards

#define sysFileCSlotDriverPnps			'pnps'	//Creator ID for Pnps Serial Peripheral Slot Driver 

#define sysFileCSoundMgr				'sndm'  // Creator type for Sound Manager features

#define sysFileC68KEmulator				'a68k'	//Creator ID for the 68K emulator (PACE) 

#define sysFileCRuntimeServices			'rtsv'	//Creator ID for Runtime Services 

#define sysFileCSocket					'sock'	// Creator for BSD sockets library
#define sysFileCBind					'bind'	// Creator for Bind name resolution library

#define sysFileCPPP						'ppp_'	// Creator for PPP helper library

// --------------------------------------------------------------------------------------
// Locale modules -- type 'locm'.
// --------------------------------------------------------------------------------------

#define	sysFileCLatinLocModule			'lati'	// Creator type for charEncodingPalmLatin locale module
#define	sysFileCShiftJISLocModule		'sjis'	// Creator type for charEncodingPalmSJIS locale module
#define	sysFileCGBLocModule				'gbkl'	// Creator type for charEncodingPalmGB locale module
#define	sysFileCBig5LocModule			'big5'	// Creator type for charEncodingPalmBig5 locale module
												//	Note: Although 'big5' isn't technically
												//	reserved (because of the "5"), it has been
												//	entered into the public registry.
#define	sysFileCKRLocModule				'euck'	// Creator type for charEncodingPalmKorean locale module

// --------------------------------------------------------------------------------------
// Binder components -- type 'libr'
// --------------------------------------------------------------------------------------

// Service components
#define sysFileCActiveInputController	'acic'	// Creator type for ActiveInputController component
#define sysFileCPenInputController      'pnic'  // Creator type for PenInputController component
#define sysFileCKbdInputController      'kbic'  // Creator type for KbdInputController component
#define sysFileCAlarm					'almn'	// Creator type for Alarm (manager) component
#define sysFileCAppMgrComponent			'apmg'	// Creator type for high-level App (manager) component
#define sysFileCAttention				'atts'	// Creator type for Attention (manager) component
#define sysFileCAutoMounter				'amtr'	// Creator type for Auto Mounter component
#define sysFileCBootView				'boov'	// Creator type for Boot View component
#define sysFileCCandyMan				'cand'	// Creator type for Candy Man component
#define sysFileCDefaultDirectories		'dfdr'	// Creator type for Default Directories component
#define sysFileCFeatures				'fetr'	// Creator type for Features (manager) component
#define sysFileCFontSuite				'fnts'	// Creator type for FontSuite component
#define sysFileCInformant				'infr'	// Creator type for Informant component
#define sysFileCInputServer				'inpt'	// Creator type for Input Server component
#define sysFileCMemoryDealer			'mdlr'	// Creator type for Memory Dealer component
#define sysFileCVirtualSilkscreen		'vssp'	// Creator type for VirtualSilkscreen component
#define sysFileCMovieServer				'movs'	// Creator type for MovieServer component
#define sysFileCNotification			'noti'	// Creator type for Notification (manager) component
#define sysFileCObexExchange			'oexg'	// Creator type for the Obex Exchange Component
#define sysFileCPenInputManager			'pinm'	// Creator type for Pen Input (manager) component
#define sysFileCPowerManagement			'powr'	// Creator type for Power Management component
#define sysFileCSettingsCatalog			'sett'	// Creator type for Settings Catalog component
#define sysFileCStatusBar				'stsb'	// Creator type for StatusBar component
#define sysFileCSyncService				'cnys'	// Creator type for Sync Service component
#define sysFileCSysInfo					'sins'	// Creator type for System Info component
#define sysFileCTaskOMatic				'tskm'	// Creator type for TaskOMatic component
#define sysFileCTelephonyService		'tels'	// Creator type for Telephony Service component
#define sysFileCWindowManager			'wdwm'	// Creator type for WindowManager component
#define sysFileCSurface					'pcso'	// Creator type for Surface component
#define sysFileCTokenSource				'toke'  // Creator type for the token source component
#define sysFileCTraces					'trce'	// Creator type for Traces component
#define sysFileCTSMService				'tsms'	// Creator type for Text Services Manager Service component
#define sysFileCExchangeService			'exgs'	// Creator type for the Exchange Service prc.

// Addon components
#define sysFileCUFST					'ufst'	// Creator type for Agfa UFST font engine component
#define sysFileCBitmapFont              'bmft'  // Creator type for BitmapFont font engine component
#define sysFileCFreeType				'frtp'	// Creator type for FreeType font engine component
#define sysFileCKeyboardPinlet			'kbdp'  // Creator type for KeyboardPinlet component
#define sysFileCSymbolPinlet			'sypn'  // Creator type for SymbolPinlet component
#define sysFileCTelephonySlip			'tlsl'  // Creator type for Telephony Slip component
#define sysFileCBtObexExchange			'bexg'	// Creator type for the Bluetooth Obex Exchange Component

// Test components
#define sysFileCCompatTest				'cpat'	// Creator type for CompatTest component
#define sysFileCSun						'pcsn'	// Creator type for Sun test component
#define sysFileCSurfaceTest				'pcts'	// Creator type for SurfaceTest component
#define sysFileCTestText				'pctt'	// Creator type for TestText component

#define sysFileCSineProducer            'sinp'  // Creator type for SineProducer component
#define sysFileCBeeper                  'beep'  // Creator type for Beeper component

#define sysFileCVideoOutput				'vido'	// Creator type for video-out media node
#define sysFileCDirectVideo             'dvid'  // Creator type for direct video media node
#define sysFileCMGLVideo                'mvid'  // Creator type for MGL video media node
#define sysFileCPartitioner             'part'  // Creator type for partitioner media node
#define sysFileCADC               		'sadc'  // Creator type for Palm OS 6 ADC media node
#define sysFileCDAC              		'sdac'  // Creator type for Palm OS 6 DAC media node
#define sysFileCTestVideoProducer       'tvpr'  // Creator type for test video producer media node

#define sysFileCAACDecoder              'aaca'  // Creator type for AAC decoder
#define sysFileCAVIComposer				'avic'  // Creator type for AVI composer
#define sysFileCAVIExtractor            'avix'  // Creator type for AVI extractor
#define sysFileCBMPComposer				'bmpc'  // Creator type for BMP composer
#define sysFileCDVIADPCMDecoder         'dvia'  // Creator type for DVI-ADPCM decoder
#define sysFileCMP12Decoder             'mp12'  // Creator type for MPEG-audio layer I/II decoder
#define sysFileCMP3Decoder              'mpad'  // Creator type for MPEG-audio layer III decoder
#define sysFileCMPEG1Extractor          'mp1x'  // Creator type for MPEG-1 extractor
#define sysFileCMPEG1VideoDecoder       'mp1v'  // Creator type for MPEG-1 video decoder
#define sysFileCMPEG4VideoDecoder       'mp4v'  // Creator type for MPEG-4 video decoder
#define sysFileCMPEGAudioExtractor      'mpax'  // Creator type for MPEG audio extractor
#define sysFileCMSADPCMDecoder          'msad'  // Creator type for MSADPCM decoder
#define sysFileCMSADPCMEncoder          'msae'  // Creator type for MSADPCM encoder
#define sysFileCRawDecoder              'rawd'  // Creator type for raw decoder
#define sysFileCRawEncoder              'rawe'  // Creator type for raw encoder
#define sysFileCRawMPEG4VideoExtractor  'rmp4'  // Creator type for raw MPEG-4 video extractor
#define sysFileCTestPatternDecoder      'tpde'  // Creator type for test pattern decoder
#define sysFileCTestPatternExtractor    'tpex'  // Creator type for test pattern extractor
#define sysFileCWAVComposer             'wavc'  // Creator type for WAV composer
#define sysFileCWAVExtractor            'wavx'  // Creator type for WAV extractor
#define sysFileCVideoXForm              'vxfr'  // Creator type for video transform node
#define sysFileCMXxMPEG4VideoDecoder    'm4md'  // Creator type for MXx MPEG-4 video decoder
#define sysFileCMXxMPEG4VideoEncoder    'm4me'  // Creator type for MXx MPEG-4 video decoder

#define sysFileCGoldbugTestADC          'tadc'  // Creator type for Goldbug ADC test component
#define sysFileCGoldbugTestDAC          'gbt0'  // Creator type for Goldbug DAC test component
#define sysFileCGoldbugTestSoundplayer  'gbt1'  // Creator type for Goldbug soundplayer test component
#define sysFileCVideoDisplayTest        'gbt2'  // Creator type for VideoDisplay test component
#define sysFileCGoldbugTestConstraint   'gbt3'  // Creator type for Goldbug constraint test component
#define sysFileCGoldbugTestResamplers   'gbt4'  // Creator type for Goldbug resampler test component
#define sysFileCMovieServerTest         'gbt5'  // Creator type for MovieServer test component
#define sysFileCMMDeviceTest            'mmdt'  // Creator type for MM device test component

// Command tool components
#define	sysFileCBinderShell					'_bsh'	// Creator type for the shell itself.
#define	sysFileCLanguagePicker				'lpkr'	// Creator type for language_picker tool.
#define	sysFileCAtomCommand					'atom'	// Creator ID for atom command tool.
#define	sysFileCBPerfCommand				'bprf'	// Creator ID for bperf command tool.
#define	sysFileCFortuneCommand				'fort'	// Creator ID for fortune command tool.
#define	sysFileCMediaPerfCommand			'mprf'	// Creator ID for mediaperf command tool.
#define	sysFileCNetDBConvertCommand			'netc'	// Creator ID for NetDBConvert command tool.
#define	sysFileCNetupCommand				'bsnu'	// Creator ID for netup command tool.
#define	sysFileCParrotControlCommand		'prct'	// Creator ID for parrotcontrol command tool.
#define	sysFileCPrefCommand					'prfc'	// Creator ID for pref command tool.
#define	sysFileCPrintfCommand				'prtf'	// Creator ID for printf command tool.
#define	sysFileCSysInfoCommand				'sysi'	// Creator ID for sysinfo command tool.
#define	sysFileCSysLocaleCommand			'sysl'	// Creator ID for syslocale command tool.
#define	sysFileCScreenCommand				'scrn'	// Creator ID for screen command tool.

// --------------------------------------------------------------------------------------
// Apps
// --------------------------------------------------------------------------------------

#define sysFileCGraffiti				'graf'	// Creator type for Graffiti databases
#define sysFileCGraffiti2				'grfr'	// Creator type for Graffiti 2 database
#define	sysFileCSystemPatch				'ptch'	// Creator for System resource file patches

#define	sysFileCCalculator				'cals'	// Creator type for Calculator App
#define	sysFileCSecurity				'secr'	// Creator type for Security App
#define	sysFileCPreferences				'pref'	// Creator type for Preferences App
#define	sysFileCAddress					'rama'	// Creator type for Address App
#define	sysFileCToDo					'ramt'	// Creator type for To Do App
#define	sysFileCDatebook				'ramd'	// Creator type for Datebook App
#define	sysFileCMemo					'ramm'	// Creator type for MemoPad App
#define	sysFileCSync					'sync'	// Creator type for HotSync App
#define	sysFileCBackupToCard			'bk2c'	// Creator type for BackupToCard App
#define	sysFileCMemory					'memr'	// Creator type for Memory App
#define	sysFileCMail					'mail'	// Creator type for Mail App
#define	sysFileCExpense					'exps'	// Creator type for Expense App
#define	sysFileCLauncher				'lnch'	// Creator type for Launcher App
#define	sysFileCTextLauncher			'txln'	// Creator type for text-based Launcher App
#define	sysFileCClipper					'clpr'	// Creator type for clipper app.
#define	sysFileCDial					'dial'	// Creator type for dial app.
#define	sysFileCSetup					'sets'	// Creator type for setup app.
#define sysFileCActivate				'actv'	// Creator type for activation app.
#define sysFileCGenericActivate			'gafd'	// New Generic Activation application working for all Palm models
#define sysFileCFlashInstaller			'fins'	// Creator type for FlashInstaller app.
#define	sysFileCRFDiag					'rfdg'	// Creator type for RF diagnostics app.
#define	sysFileCMessaging				'msgs'	// Creator type for Messaging App
#define	sysFileCModemFlashTool			'gsmf'	// Creator type for Palm V modem flash app.
#define sysFileCWordLookup				'word'	// Creator type for Word Lookup app.
#define	sysFileCJEDict					'dict'	// Creator type for JEDict app.
#define	sysFileHotSyncServer			'srvr'	// Creator type for HotSync(R) Server app.
#define	sysFileHotSyncServerUpdate		'hssu'	// Creator type for HotSync(R) Server update app.
#define	sysFileCCardInfo				'cins'	// Creator type for the Card info app.
#define	sysFileCPhone					'fone'	// Creator type for integrated phone components.
#define	sysFileCSmsMessenger			'smsa'	// Creator type for SMS messenger app.
#define	sysFileCTerminal				'term'	// Creator type for terminal application.
#define sysFileCPSIAppInstaller			'psix'	// Creator type for the PalmSource application installer

// The following apps are manufacturing, calibration and maintenance related
#define	sysFileCMfgExtension			'mfx1'	// Creator type for Manufacturing Extension.
#define	sysFileCMfgFunctional			'mfgf'	// Creator type for Manufacturing functional test autostart app.
#define sysFileCMfgCalibration			'mfgc'	// Creator type for Manufacturing radio calibration app.

// Demo Apps
#define	sysFileCGraffitiDemo			'gdes'	// Creator type for Graffiti Demo
#define	sysFileCMailDemo				'mdem'	// Creator type for Mail Demo
#define	sysFileCFontDemo				'ftdm'	// Show off scalable fonts
#define	sysFileCNotebook				'book'	// 
#define	sysFileCNotes					'note'	// 

// Test apps
#define sysFileCPerfTest				'prft'	// High-level performance tests
#define sysFileCLaunchPerfTest			'lpft'	// Launching performance tests

// PIM Apps - legacy 68K Creator IDs 
#define	sysFileCAddress68K				'addr'	// Creator type for Address App 68K
#define	sysFileCToDo68K					'todo'	// Creator type for To Do App 68K
#define	sysFileCDatebook68K				'date'	// Creator type for Datebook App 68K
#define	sysFileCMemo68K					'memo'	// Creator type for MemoPad App 68K

#define	sysFileCFirstApp				sysFileCLauncher	// Creator type for First App after reset
#define	sysFileCAltFirstApp				sysFileCSetup			// Creator type for First alternate App after reset (with hard key pressed)
#define	sysFileCDefaultApp				sysFileCLauncher	// Creator type for Default app
#define	sysFileCDefaultButton1App		sysFileCDatebook		// Creator type for dflt hard button 1 app
#define	sysFileCDefaultButton2App		sysFileCAddress		// Creator type for dflt hard button 2 app
#define	sysFileCDefaultButton3App		sysFileCToDo			// Creator type for dflt hard button 3 app
#define	sysFileCDefaultButton4App		sysFileCMemo			// Creator type for dflt hard button 4 app
#define	sysFileCDefaultCalcButtonApp	sysFileCCalculator	// Creator type for dflt calc button app
#define	sysFileCDefaultCradleApp		sysFileCSync			// Creator type for dflt hot sync button app
#define	sysFileCDefaultModemApp			sysFileCSync			// Creator type for dflt modem button app
#define	sysFileCDefaultAntennaButtonApp	sysFileCLauncher	// Creator type for dflt antenna up button app
#define	sysFileCNullApp					'0000'	// Creator type for non-existing app 
#define	sysFileCSimulator					'\?\?\?\?'	// Creator type for Simulator files (app.tres, sys.tres)
												//	'????' does not compile with VC++ (Elaine Server)

// --------------------------------------------------------------------------------------
// Plugins
// --------------------------------------------------------------------------------------

#define	sysFileCDigitizer					'digi'	// Creator type for Digitizer Panel
#define	sysFileCDateTime					'dttm'	// Creator type for Date & Time Panel
#define	sysFileCGeneral					'gnrl'	// Creator type for General Panel
#define	sysFileCFormats					'fmat'	// Creator type for Formats Panel
#define	sysFileCShortCuts					'shct'	// Creator type for ShortCuts Panel
#define	sysFileCButtons					'bttn'	// Creator type for Buttons Panel
#define	sysFileCOwner						'ownr'	// Creator type for Owner Panel
#define	sysFileCModemPanel				'modm'	// Creator type for Modem Panel
#define	sysFileCDialPanel					'dial'	// Creator type for Dial Panel
#define	sysFileCNetworkPanel				'netw'	// Creator type for Network Panel
#define  sysFileCWirelessPanel			'wclp'	// Creator type for the Web Clipping  Panel.
#define	sysFileCUserDict       			'udic'	// Creator type for the UserDict panel.
#define	sysFileCPADHtal					'hpad'	// Creator type for PAD HTAL library
#define	sysFileCTCPHtal					'htcp'	// Creator type for TCP HTAL library
#define	sysFileCRELHtal					'hrel'	// Creator type for REL HTAL library
#define	sysFileCMineHunt					'mine'	// Creator type for MineHunt App
#define	sysFileCPuzzle15					'puzl'	// Creator type for Puzzle "15" App
#define	sysFileCOpenLibInfo				'olbi'	// Creator type for Feature Manager features
																// used for saving open library info under PalmOS v1.x
#define	sysFileCHwrFlashMgr				'flsh'	// Creator type for HwrFlashMgr features

#define	sysFileCPhoneSetupApp			'pcnc'	// Creator type for Phone Setup application

#define	sysFileCTextServices			'tsml'	// Previously was the creator type for 68K Japanese FEP in 3.x and 4.x.
												//   Now its used as tsmFtrCreator in TextServicesMgr.h.
#define	sysFileCJapaneseFep				'jfep'	// Creator type for Japanese FEP.
#define sysFileCPinyinFep				'piny'	// Creator type for Simplified Chinese Pinyin FEP (Text Services Library) & panel.
#define	sysFileCShortcutFep				'shrt'	// Creator type for the Shortcut FEP

#define	sysFileTUartPlugIn				'sdrv'	// File type for SerialMgrNew physical port plug-in.
#define	sysFileTVirtPlugin				'vdrv'	// Flir type for SerialMgrNew virtual port plug-in.
#define	sysFileCUart328 					'u328'	// Creator type for '328 UART plug-in
#define	sysFileCUart328EZ					'u8EZ'	// Creator type for '328EZ UART plug-in
#define	sysFileCUart650 					'u650'	// Creator type for '650 UART plug-in
#define	sysFileCVirtIrComm				'ircm'	// Creator type for IrComm virtual port plug-in.
#define	sysFileCVirtRfComm				'rfcm'	// Creator type for RfComm (Bluetooth) virtual port plug-in.
#define sysFileCUARTDriver				'durt'	// Createor type for a UART driver

#define	sysFileCPDIUSBD12					'pusb'	// Creator type for USB database
#define	sysPortUSBDesktop					'usbd'	// Creator type for USB Desktop 
#define	sysPortUSBConsole					'usbc'	// Creator type for USB Console
#define	sysPortUSBPeripheral				'usbp'	// Creator type for USB Peripheral

#define sysPortUSBSerial				'usbs'	// Serial Mgr Driver Creator for USB Serial
#define sysPortUSBDebugger				'usds'  // Serial Mgr Driver Creator for USB Debugger

#define	sysFileCExternalConnector		'econ'	// Creator type for the external connector

#define sysFileCEthernetAdapter			'def_'	// Creator type for the Ethernet Adapter
#define sysFileCEthernetStreamsDriver	'ethm'	// Creator type for the Ethernet STREAMS Driver
#define sysFileCEthernetDeviceDriver	'ethr'	// Creator type for the Ethernet Device Driver
#define sysFileCSerialAdapter			'sera'	// Creator type for the Serial Adapter

#define sysFileCTun6Plugin				'tun6'	// Creator type for the manual tunnel configuration 
#define sysFileCWinsock					'wsoc'	// Creator for simulator socket lib redirection

#define sysFileCWifiAdapter				'wifi'	// Creator for 802.11 Platform Plugin and Connection Mgr Plugin
#define sysFileCPrismDriver				'prsm'  // Creator for Prism 802.11 hardware
#define sysFileCWifiCommand				'wcmd'	// Creator for the 802.11 Wifi Shell Command

#define sysFileCTestConsole				'test'  // The test console app / shell command

// --------------------------------------------------------------------------------------
// Sample Code
// --------------------------------------------------------------------------------------

#define sysFileCSampleView				'vwex'	// Basic view example
#define sysFileCTranslucentView			'trex'	// Translucent view example code
#define sysFileCHardInputView			'hiex'	// "Hard input area" view example code

// --------------------------------------------------------------------------------------
// Data
// --------------------------------------------------------------------------------------

// Font data (type 'ofnt')
#define	sysFileCAgfaFonts					'agfp'	// Creator ID for PRC containing plain Agfa fonts.
#define	sysFileCAgfaItalicFonts				'agfi'	// Creator ID for PRC containing italic Agfa fonts.
#define SysFileCPDFonts						'pdfp'	// Creator ID for PRC containing public domain fonts.
#define	sysFileCAgfaJapaneseFonts			'agjp'	// Creator ID for PRC containing Agfa Japanese fonts.
#define	sysFileCAgfaKoreanFonts				'agkr'	// Creator ID for PRC containing Agfa Korean fonts.
#define	sysFileCAgfaSimpChineseFonts		'agsc'	// Creator ID for PRC containing Agfa Simplified Chinese fonts.
#define	sysFileCAgfaTradChineseFonts		'agtc'	// Creator ID for PRC containing Agfa Traditional Chinese fonts.
#define	sysFileCStrokeJapaneseFonts			'asjp'	// Creator ID for PRC containing Agfa stroke Japanese fonts.
#define	sysFileCStrokeKoreanFonts			'askr'	// Creator ID for PRC containing Agfa stroke Korean fonts.
#define	sysFileCStrokeSimpChineseFonts		'assc'	// Creator ID for PRC containing Agfa stroke Simplified Chinese fonts.
#define	sysFileCStrokeTradChineseFonts		'astc'	// Creator ID for PRC containing Agfa stroke Traditional Chinese fonts.

#define	sysFileCTimeZones					'tztz'	// Creator ID for PRC containing TimeZone database (type is 'data').
#define sysFileCDefaultSkin					'skin'	// Where the default skin lives (type is 'skin').
#define sysFileCExtraKeymaps				'kmap'	// PRC containing additional keymaps (type is 'kmap').

// Types
#define	sysFileTSystem						'rsrc'	// File type for Main System File
#define	sysFileTSystemPatch				'ptch'	// File type for System resource file patches
#define	sysFileTProductUpdate			'pupd'	//	File type for Product Update patches
#define	sysFileTKernel						'krnl'	// File type for System Kernel (AMX)
#define	sysFileTBoot						'boot'	// File type for SmallROM System File
#define	sysFileTRuntimeConfig			'bmap'	//File type for Runtime Config database
#define	sysFileTPreDataMgr				'prdm'	//File type for pre-Dm modules that must be listed in RAL Boot List
#define	sysFileTSmallHal					'shal'	// File type for SmallROM HAL File
#define	sysFileTBigHal						'bhal'	// File type for Main ROM HAL File
#define	sysFileTSplash						'spls'	// File type for Main ROM Splash File

#define	sysFileTBootApp					'uish'	// File type for Boot Application (creator is 'psys')
#define sysFileTBackgroundTask				'back'	// File type for PRC to be automatically launched in the User Background Process
#define	sysFileTOverlay					'ovly'	// File type for UI overlay database
#define	sysFileTExtension					'extn'	// File type for System Extensions
#define	sysFileTApplication				'appl'	// File type for applications
#define	sysFileTPanel						'panl'	// File type for preference panels
#define	sysFileTSavedPreferences		'sprf'	// File type for saved preferences
#define	sysFileTPreferences				'pref'	// File type for preferences
#define	sysFileTMidi						'smfr'	// File type for Standard MIDI File record databases
#define	sysFileTpqa							'pqa '	// File type for the PQA files.
#define	sysFileTLocaleModule				'locm'	// File type for locale modules.
#define	sysFileTActivationPlugin		'actp'	// File type for activation plug-ins.
#define	sysFileTUserDictionary			'dict'	// File type for input method user dictionary.
#define	sysFileTLearningData				'lean'	// File type for input method learning data.

#define	sysFileTGraffitiMacros			'macr'	//  Graffiti Macros database

#define	sysFileTHtalLib					'htal'	//  HTAL library

#define  sysFileTExgLib						'exgl'	// Type of Exchange libraries

#define	sysFileTSlotDriver				'libs'	// File type for slot driver libraries
#define	sysFileTFileSystem				'libf'	// File type for file system libraries

#define sysFileTFep							'libt'	// File type for Text Services Manager Fep plugin libraries

#define	sysFileTFileStream				'strm'	//  Default File Stream database type

#define	sysFileTTemp						'temp'	//  Temporary database type; as of Palm OS 4.0, the
																//  system WILL automatically delete any db's of
																//  this type at reset time (however, apps are still
																//  responsible for deleting the ones they create
																//  before exiting to protect valuable storage space)
										

#define	sysFileTNetworkPanelPlugin		'nppi'	// File type for network preference panel plug-ins

#define	sysFileTScriptPlugin			'scpt'	// File type for plugin to the Network Panel to 
												//extend scripting capabilities.

#define	sysFileTStdIO					'sdio'	// File type for standard IO apps 
#define	sysFileTGenericData				'data'	// Generic data file type.

#define	sysFileTPatchConfig				'pchc'	// File type for Patch Configuration Database created by Runtime Services
#define	sysFileTPatch					'apch'	// File type for Patches

#define sysFileTStreamsModule			'smod'	// File type for Streams modules (or drivers)

#define sysFileTSkin					'skin'	// The default location for the ui skins.

//................................................................
// Testing Components
//................................................................
#define sysFileTBITS					'bits'	// File type for the BITS(Built In Testing Services) Application
#define sysFileTHowdy					'hwdy'	// File type for the Howdy component
#define sysFileTGremlins				'grml'	// File type for the Gremlins component
#define sysFileTBinderChannelService	'bcsv'	// File type for the BinderChannelService component

//................................................................
// Types introduced from 6.0
//................................................................
#define sysFileTDeviceDriver			'drvr'	// File type for device drivers
#define sysFileTSlipApp					'slip'	// applications that run within a slip
#define sysFileTPinletApp				'pnlt'	// applications that are used as a pinlet
#define sysFIleTOutlineFonts			'ofnt'	// PRC containing outline font data

#define sysFileTBackgroundProcess		'dump'	// Database type for the main app of the background process
#define sysFileTBackupProcess			'bkpr'	// type for the faerie backup process

//................................................................
// Resource types and IDs
//................................................................
#define sysResTBinderShellScript		'bshs'	// Resource containing a bsh script
#define sysResTSkin						'skin'	// Resource containing a ui skin
#define	sysResIDBootReset				10000		// Reset code 
#define	sysResIDBootInitCode			10001		// Init code 
#define	sysResIDBootSysCodeStart		10100		// System code resources start here
#define	sysResIDBootSysCodeMin			10102		// IDs 'Start' to this must exist!!
#define	sysResIDBootUICodeStart			10200		// UI code resources start here
#define	sysResIDBootUICodeMin			10203		// IDs 'Start' to this must exist!!
#define	sysResIDProdUpdCodeStart		10300		//	Product Update code resources start here

#define	sysResIDBootHAL					19000		// HAL initial code resource (from HAL.prc)
#define	sysResIDBootHALCodeStart		19100		// start of additional high-level HAL code resources 

#define	sysResIDBitmapSplash			19000		// ID of (boot) splash screen bitmap
#define	sysResIDBitmapConfirm			19001		// ID of hard reset confirmation bitmap

#define	sysResTAppPrefs68K				'pref'	// Resource type of App preferences resources (only used for 68K apps)
#define	sysResIDAppPrefs					0			// Application preference

#define	sysResTExtPrefs					'xprf'	// Resource type of extended preferences (only used for 68K apps)
#define	sysResIDExtPrefs					0			// Extended preferences

#define	sysResTAppLaunchPrefs			'alpf'	// Resource type of 6.0 application launch preferences

#define	sysResTAppCode68K				'code'	// Resource type of App code resources
#define	sysResTAppGData68K				'data'	// Resource type of App global data resources

#define	sysResTFeatures					'afea'	// Resource type of features table initialization data.

#define	sysResTLibrary68K				'libr'	// Resource type of 68K shared library code.

#define	sysResTModuleCode				'acod'	// Resource type code for any ARM module (was code1)
#define	sysResTModuleData				'adat'	// Resource type data for any ARM module (was data0)
#define	sysResTModulePatch				'amdp'	// Resource type patch for any ARM module

#define sysResTARMlet					'armc'	// recommended type of ARM code fragments for PceNativeCall

#define sysResTModuleIDLimits			'bmap'	// store the limits of module IDs allocated by ROM
#define sysResIDModuleIDLimits			(1001)	// store the limits of module IDs allocated by ROM

#define	sysResTDefaultDB68K				'dflt'	// Default database resource type for 68K apps.
#define	sysResTDefaultDB				'adft'	// Default database resource type
#define	sysResIDDefaultDB				1		// resource ID of sysResTDefaultDB in each app

#define sysResTDefaultSchemaDB			'scdb'	// Default schema database resource type

#define	sysResTCompressedDB				'cpdb'	// Compressed database resource type
#define	sysResIDCompressedDB			10000	// Resource ID of first sysResTCompressedDB

#define	sysResTErrStrings				'tSTL'	// list of strings

#define	sysResIDOEMDBVersion			20001	// resource ID of "tver" and "tint" versions in OEM stamped databases

#define	sysResTButtonDefaults			'abda'	// Hard/soft button default apps
#define	sysResIDButtonDefaults			10000	// resource ID of system button defaults resource

#define sysResTTrueTypeFont				'fttf'	// raw TrueType (or other outline) font data

// This resource (type dwordListRscType) is an array of apps/panels/libraries/etc that should not be
// run on the OS.  The dword entries in the list are an array of triples, with each triple consisting
// of a creator id, type and creation date.  The triples must be entered in sorted order (sorted by creator id).
#define sysResIDObsoleteExecutables		10000	// Obsolete apps/panels/libraries/etc

#define sysResTFep						'tint'
#define sysResIDFep						10005

// System Sounds
#define sysResTSound					'wave'	// Resource type for sound resources.
// Resource IDs for various system sounds.
// These resources are contained in the "systemsounds" database.
#define sysResIDSndInfo					10000
#define sysResIDSndWarning				10001
#define sysResIDSndError				10002
#define sysResIDSndStartUp				10003
#define sysResIDSndAlarm				10004
#define sysResIDSndConfirmation			10005
#define sysResIDSndClick				10006
#define sysResIDSndCardInsert			10010
#define sysResIDSndCardRemove			10011
#define sysResIDSndSyncStart			10020
#define sysResIDSndSyncStop				10021


// System Preferences

// preference resource is created at hard reset so it generally
// doesn't need to have a LE32 type, or does it?
#define	sysResTSysPref						sysFileCSystem
#define	sysResIDSysPrefMain				0			// Main preferences
#define	sysResIDSysPrefPassword			1			// Password
#define	sysResIDSysPrefFindStr			2			// Find string
#define	sysResIDSysPrefCalibration		3			// Digitizer calibration.
#define	sysResIDDlkUserInfo				4			// Desktop Link user information.
#define	sysResIDDlkLocalPC				5			// Desktop Link local PC host name
#define	sysResIDDlkCondFilterTab		6			// Desktop Link conduit filter table
#define	sysResIDModemMgrPref			7			// Modem Manager preferences
#define	sysResIDDlkLocalPCAddr			8			// Desktop Link local PC host address
#define	sysResIDDlkLocalPCMask			9			// Desktop Link local PC host subnet mask

// Preference to store names of unknown expansion slots (by slot media type)
// Used by desktop HotSync to name unknown slots
#define	sysResTSyncMgr					'sync'
#define	sysResIDSyncExpSlotInfo			11
#define	sysResVer1SyncExpSlotInfo		(0x1)

// These prefs store parameters to pass to an app when launched with a button
#define	sysResIDButton1Param			10			// Parameter for hard button 1 app
#define	sysResIDButton2Param			11			// Parameter for hard button 2 app
#define	sysResIDButton3Param			12			// Parameter for hard button 3 app
#define	sysResIDButton4Param			13			// Parameter for hard button 4 app
#define	sysResIDCalcButtonParam			14			// Parameter for calc button app
#define	sysResIDCradleParam				15			// Parameter for hot sync button app
#define	sysResIDModemParam				16			// Parameter for modem button app
#define	sysResIDAntennaButtonParam		17			// Parameter for antenna up button app

// New for Color, user's color preferences
#define	sysResIDPrefUIColorTableBase	17			// base + depth = ID of actual pref
#define	sysResIDPrefUIColorTable1		18			// User's UI colors for 1bpp displays
#define	sysResIDPrefUIColorTable2		19			// User's UI colors for 2bpp displays
#define	sysResIDPrefUIColorTable4		21			// User's UI colors for 4bpp displays
#define	sysResIDPrefUIColorTable8		25			// User's UI colors for 8bpp displays

#define	sysResIDSysPrefPasswordHint	26			// Password hint
#define sysResIDSysPrefPasswordHash 27			// Password hash (MD5)


// FlashMgr Resources - old
#define	sysResTFlashMgr					'flsh'
#define	sysResIDFlashMgrWorkspace		1			// RAM workspace during flash activity

// FlashMgr Resources - new
#define	sysResTHwrFlashIdent			'flid'	// Flash identification code resource
#define	sysResIDHwrFlashIdent			10000		// Flash identification code resource

#define	sysResTHwrFlashCode				'flcd'	// Flash programming code resource
																// (resource ID determined by device type)

// SystemInitializationList Resource
#define sysResTSysInitList                              'sysi'
																// (resource ID determined by device type)
// Default resource ID in 6.0 PRCs
#define sysResIDDefault						(0)

// OEM Feature type and id.
// DOLATER - this doesn't seem to be used/supported in 6.0
#define	sysFtrTOEMSys						sysFileCOEMSystem
#define	sysFtrIDOEMSysHideBatteryGauge	1

// Clipboard storage.  Feature number is the clipboard type.
#define sysFtrTClipboard				'clip'

// Activation status values.
#define	sysActivateStatusFeatureIndex	1
#define	sysActivateNeedGeorgeQuery		0
#define	sysActivateNeedMortyQuery		1
#define	sysActivateFullyActivated		2

#define sysMaxUserDomainNameLength		64

// Current clipper feature indices
#define sysClipperPQADbHIndex	1

//-----------------------------------------------------------
// This section is only valid when running the resource compiler
//
// Actually, this section is obsolete.  Instear, .r files should
// inlude SysResTypes.rh to get these definitions.
//

// DOLATER еее leaving this here for now just in case.

//-----------------------------------------------------------


#ifdef RESOURCE_COMPILER

#include <SysResTypes.rh>

#endif


#endif // _SYSTEMRESOURCES_H_
