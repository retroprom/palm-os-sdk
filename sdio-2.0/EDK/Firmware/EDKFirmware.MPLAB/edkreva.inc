;///////////////////////////////////////////////////////////////////////////
;
; SDIO PIC Microcontroller edkreva.inc Include File
;
; Copyright (c) 2001 Palm Inc. or its subsidiaries.
; All rights reserved.  This software may be copied and used solely for 
; developing products for the communicating with the Palm Inc. platform
; and for archival and backup purposes.  Except for the foregoing, no part 
; of this software may be reproduced or transmitted in any form or by any 
; means or used to make any derivative work (such as translation, 
; transformation or adaptation) without express written consent from Palm Inc.
;
; Palm Inc. reserves the right to revise this software and to make changes in 
; content from time to time without obligation on the part of Palm Inc. to 
; provide notification of such revision or changes.  PALM INC. MAKES NO 
; REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF ERRORS OR 
; THAT THE SOFTWARE IS SUITABLE FOR YOUR USE.  THE SOFTWARE IS PROVIDED ON
; AN "AS IS" BASIS.  PALM INC. MAKES NO WARRANTIES, TERMS OR CONDITIONS, EXPRESS
; OR IMPLIED, EITHER IN FACT OR BY OPERATION OF LAW, STATUTORY OR 
; OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
;
; TO THE FULL EXTENT ALLOWED BY LAW, PALM INC. ALSO EXCLUDES FOR ITSELF AND ITS
; SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING 
; NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
; PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
; BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING 
; OUT OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF PALM INC. HAS BEEN ADVISED
; OF THE POSSIBILITY OF SUCH DAMAGES.
;
; HotSync, Palm Inc., and Graffiti are registered trademarks, 
; and Palm III and Palm OS are trademarks of Palm Inc. or its 
; subsidiaries.
;
; IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK, THE OTHER SOFTWARE AND 
; DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT 
; ACCOMPANYING THE COMPACT DISK.
; 
;///////////////////////////////////////////////////////////////////////////
;
; Revision History:
;
; Date         Name        Description
; ----------------------------------------------------------
; 12/20/01		GAR         initial release
;
;----------------------------------------------------------------------
;
;    Author:        Geoff Richmond
;    Company:       Palm, Inc.
;
;	Description:	Include file for SDIO EDK Rev A board.
;
;----------------------------------------------------------------------
;
;    Notes:
;		1. This file is best viewed with Tab Size set to 4.
;			Go to "Options-->Current Editor Modes" and set tab size
;
;----------------------------------------------------------------------

; Firmware version 1.0
; high-order 4 bits are major version number
; lo-order 4 bits are minor version number
#define		FwVersion		b'00010000'

;--------------------------------------------------------------------------
; SDIO constants
;--------------------------------------------------------------------------

; SDIO Commands
SDIOCmd0					EQU		0x00		; SDIO CMD0
SDIOCmd5					EQU		0x05		; SDIO CMD5
SDIOCmd52					EQU		0x34		; SDIO CMD52
SDIOCmd53					EQU		0x35		; SDIO CMD53

; Useful constants for parsing SD commands
CommandMask					EQU		0x3F		; Mask 6 bits of command
StartBit					EQU		7			; Bit 7 is start bit
BytesPerSDCmd				EQU		6			; All SDIO commands (host to card) are 6 bytes
ParameterError				EQU		6			; CommandStatus: parameter error
FnNumberError				EQU		4			; CommandStatus: function number error
ComCRCError					EQU		3			; CommandStatus: CRC error
IllegalCommand				EQU		2			; CommandStatus: illegal command
IdleState					EQU		0			; CommandStatus: in idle state
StuffByte					EQU		0x00		; Stuff byte of all 0's
BlockMode					EQU		3			; Bit 3 of CMD53 2nd byte
RWFlag						EQU		7			; Bit 7 of CMD53 2nd byte
RAWFlag						EQU		3			; Bit 3 of CMD52 2nd byte
FnNumberMask				EQU		0x70		; Bits 6,5,4 of CMD53 2nd byte
OpCode						EQU		2			; Bit 2 of CMD53 2nd byte
MsbByteCountMask			EQU		b'00000001'	; Bit 0 of CMD53 4th byte
Cmd53StartOfBlock			EQU		0xFE		; CMD53 start of block byte
Cmd53CRCByte				EQU		0x00		; CMD53 dummy CRC
Cmd53DataAccepted			EQU		b'00000101'	; Data Response Token: Data Accepted
Cmd53DataCRCError			EQU		b'00001011'	; Data Response Token: CRC Error
Cmd53DataWriteError			EQU		b'00001101'	; Data Response Token: Write Error
Cmd53CardBusy				EQU		b'00000000'	; Data Response Token: Card Busy

; Used for setting function registers in CCCR
IOFunction1					EQU		0x02		; IO Function 1
IOFunction2					EQU		0x04		; IO Function 2
IOFunction3					EQU		0x08		; IO Function 3
IOFunction4					EQU		0x10		; IO Function 4
IOFunction5					EQU		0x20		; IO Function 5
IOFunction6					EQU		0x40		; IO Function 6
IOFunction7					EQU		0x80		; IO Function 7
IOFunctionAll				EQU		(IOFunction1 | IOFunction2)
IOFunctionNone				EQU		0x00

; Used for parsing function numbers in commands
SDIOFn0						EQU		0x00		; Function 0
SDIOFn1						EQU		0x10		; Function 1
SDIOFn2						EQU		0x20		; Function 2
SDIOFn3						EQU		0x30		; Function 3
SDIOFn4						EQU		0x40		; Function 4
SDIOFn5						EQU		0x50		; Function 5
SDIOFn6						EQU		0x60		; Function 6
SDIOFn7						EQU		0x70		; Function 7

; Card Capability Register Bits
SDC							EQU		0			; SDC is bit 0
SMB							EQU		1			; SMB is bit 1
SRW							EQU		2			; SRW is bit 2
SBS							EQU		3			; SBS is bit 3
S4MI						EQU		4			; S4MI is bit 4
E4MI						EQU		5			; E4MI is bit 5

; Function Information Register Bits
CSA_Enable					EQU		7			; CSA Enable is bit 7
CSA_Supported				EQU		6			; CSA Supported is bit 6
CsaWindowOffset				EQU     0x0F		; CSA Data Access Window

; Function Standard I/O Interfaces
SDIO_NonStandard			EQU		0x00		; Does not implement a standard i/o interface
SDIO_UART					EQU		0x01		; SDIO UART standard interface
SDIO_ThinBluetooth			EQU		0x02		; SDIO thin Bluetooth standard
SDIO_FatBluetooth			EQU		0x03		; SDIO fat Bluetooth standard


;--------------------------------------------------------------------------
; This card's CCCR Constants
;--------------------------------------------------------------------------
SDIORevision				EQU		0x00		; SDIO Specification 1.0
CCCRRevision				EQU		0x00		; CCCR Version 1.0
SDRevision					EQU		0x00		; SD Physical Specification 1.0

MyOcrRegisterByte1			EQU		0x90		; 2.7-3.6V, 1 Fcn, No memory, Card Ready after init.
MyOcrRegisterByte2			EQU		0xFF		; 
MyOcrRegisterByte3			EQU		0x80		; 
MyOcrRegisterByte4			EQU		0x00		; 

; Define Common CIS location
COMMON_CIS_START_HI_BYTE	EQU		0x00		; Common CIS starts at 0x001000
COMMON_CIS_START_MID_BYTE	EQU		0x10		; Common CIS starts at 0x001000
COMMON_CIS_START_LO_BYTE	EQU		0x00		; Common CIS starts at 0x001000

; Define Function 1 CIS location
FN1_CIS_START_HI_BYTE		EQU		0x00		; Function 1 CIS starts at 0x002000
FN1_CIS_START_MID_BYTE		EQU		0x20		; Function 1 CIS starts at 0x002000
FN1_CIS_START_LO_BYTE		EQU		0x00		; Function 1 CIS starts at 0x002000

; Define Function 1 CSA location
FN1_CSA_START_HI_BYTE		EQU		0x00		; Function 1 CSA starts at 0x000000
FN1_CSA_START_MID_BYTE		EQU		0x00		; Function 1 CSA starts at 0x000000
FN1_CSA_START_LO_BYTE		EQU		0x00		; Function 1 CSA starts at 0x000000

; Define Function 1 I/O Block Size
FN1_IO_BLK_SIZE_HI_BYTE		EQU		0x00		; Function 1 I/O Block Size 0 (0 indicates block-mode not-supported)
FN1_IO_BLK_SIZE_LO_BYTE		EQU		0x00		; Function 1 I/O Block Size 0 (0 indicates block-mode not-supported)

; Useful constants for fields in variables
ioMode			EQU		0			; Bit 0 of cardStatus is i/o mode (SPI=0, SD 1-bit=1)

; CIS defines for parameters in the tuples
#define			TPLFE_FN0_BLK_SIZE_ZERO_LO		0x00
#define			TPLFE_FN0_BLK_SIZE_ZERO_HI		0x00
#define			TPLFE_FN0_BLK_SIZE_128K_LO		0x80
#define			TPLFE_FN0_BLK_SIZE_128K_HI		0x00

#define			TPLFE_MAX_TRAN_SPEED_1200K		b'00010001'				; TPLFE_MAX_TRAN_SPEED (1.2 Mbit/s)
#define			TPLFE_MAX_TRAN_SPEED_800K		b'01111000'				; TPLFE_MAX_TRAN_SPEED (800 kbit/s)
#define			TPLFE_MAX_TRAN_SPEED_400K		b'01001000'				; TPLFE_MAX_TRAN_SPEED (400 kbit/s)
#define			TPLFE_MAX_TRAN_SPEED_200K		b'00101000'				; TPLFE_MAX_TRAN_SPEED (200 kbit/s)

#define			TPLFE_FN0_BLK_SIZE_LO	TPLFE_FN0_BLK_SIZE_128K_LO
#define			TPLFE_FN0_BLK_SIZE_HI	TPLFE_FN0_BLK_SIZE_128K_HI

#define			TPLFE_MAX_BLK_SIZE_FN1_LO		TPLFE_FN0_BLK_SIZE_LO
#define			TPLFE_MAX_BLK_SIZE_FN1_HI		TPLFE_FN0_BLK_SIZE_HI

#define			TPLFE_MAX_TRAN_SPEED	TPLFE_MAX_TRAN_SPEED_400K

#define			TPLFE_CSA_PROPERTY_WP_NOFORMAT	0x03					; TPLFE_CSA_PROPERTY (Write-Protected and No-Format)
#define			TPLFE_CSA_PROPERTY_RW_FORMAT	0x00					; TPLFE_CSA_PROPERTY (Read/Write and Format Permitted)

#define			TPLFE_CSA_SIZE_ZERO_LO			0x00
#define			TPLFE_CSA_SIZE_ZERO_MID			0x00
#define			TPLFE_CSA_SIZE_ZERO_HI			0x00

#define			TPLFE_CSA_SIZE_128K_LO			0x00
#define			TPLFE_CSA_SIZE_128K_MID1		0x00
#define			TPLFE_CSA_SIZE_128K_MID2		0x02
#define			TPLFE_CSA_SIZE_128K_HI			0x00

#define			TPLFE_CSA_SIZE_LO				TPLFE_CSA_SIZE_128K_LO
#define			TPLFE_CSA_SIZE_MID1				TPLFE_CSA_SIZE_128K_MID1
#define			TPLFE_CSA_SIZE_MID2				TPLFE_CSA_SIZE_128K_MID2
#define			TPLFE_CSA_SIZE_HI				TPLFE_CSA_SIZE_128K_HI

#define			TPLFE_CSA_PROPERTY				TPLFE_CSA_PROPERTY_WP_NOFORMAT

;--------------------------------------------------------------------------
; Set the locations of the various SDIO Register sets.
;--------------------------------------------------------------------------

CCCRStart		EQU     0x00A0			; Start of the CCCR Registers in RAM - Bank 1 (32 bytes)
CCCRStartLo8	EQU     0xA0			; Start of the CCCR Registers in RAM
CCCRLength		EQU     0x1F			; Length of the CCCR block in RAM
FBRStart		EQU		0x00C0			; Start of Function Basic Registers - Bank 1
FBRStartLo8		EQU		0xC0			; Start of Function Basic Registers
FBR1Start		EQU		FBRStartLo8+0x00	; Start of Function 1 Basic Registers - Bank 1 (32 bytes)
Fn1RegStart		EQU		0x0020			; Start of Function 1 register space - Bank 0 (64 bytes)
Fn1RegStartLo8	EQU		0x20			; Start of Function 1 register space

Fn1Led			EQU		(Fn1RegStart+0)	; Led on/off control
Fn1TempLo		EQU		(Fn1RegStart+1)	; lo-order temperature byte
Fn1TempHi		EQU		(Fn1RegStart+2)	; hi-order temperature byte
Fn1Command		EQU		(Fn1RegStart+3)	; Function 1 command register
Fn1PicVersion	EQU		(Fn1RegStart+4)	; PIC FW version.  high-order 4 bits are major version number
										; lo-order 4 bits are minor version number
Fn1CpldVersion	EQU		(Fn1RegStart+5)	; CPLD FW version.  high-order 4 bits are major version number
										; lo-order 4 bits are minor version number

;--------------------------------------------------------------------------
; SDIO CCCR Registers
;--------------------------------------------------------------------------

CccrRevision1	EQU     CCCRStart+0			; CCCR/SDIO Revision
CccrRevision2	EQU     CCCRStart+0x01		; SD Specification Revision
CccrIOEnable	EQU     CCCRStart+0x02		; I/O Enable
CccrIOReady		EQU     CCCRStart+0x03		; I/O Ready
CccrIntEnable	EQU     CCCRStart+0x04		; Interrupt Enable
CccrIntPending	EQU     CCCRStart+0x05		; Interrupt Pending
CccrIOAbort		EQU     CCCRStart+0x06		; I/O Abort
CccrBusIFCtl	EQU     CCCRStart+0x07		; Bus Interface Control
CccrCardCapbty	EQU     CCCRStart+0x08		; Card Capability
CccrCisPtr		EQU     CCCRStart+0x09		; Common CIS Pointer
CccrBusSuspend	EQU     CCCRStart+0x0C		; Bus Suspend
CccrFnSelect	EQU     CCCRStart+0x0D		; Function Select
CccrExecFlags	EQU     CCCRStart+0x0E		; Exec Flags
CccrReadyFlags	EQU     CCCRStart+0x0F		; Ready Flags
CccrFn0BlockSz	EQU     CCCRStart+0x10		; FN0 Block Size


;--------------------------------------------------------------------------
; SDIO FBR Registers
;--------------------------------------------------------------------------

Fbr1Info		EQU     FBR1Start+0			; FBR 1 Info
Fbr1CisPtr		EQU     FBR1Start+0x09		; Pointer to Fn 1 CIS
Fbr1CsaPtr		EQU     FBR1Start+0x0C		; Pointer to Fn 1 CSA
Fbr1CsaWindow	EQU     FBR1Start+CsaWindowOffset	; Fn 1 CSA Data Access Window
Fbr1IoBlockSz	EQU     FBR1Start+0x10		; FBR 1 I/O Block Size


;--------------------------------------------------------------------------
; Control Pin Definitions
;--------------------------------------------------------------------------

#ifdef	EDKPreRelease
;	These values apply to pre-release hardware
#define			ResetN		PORTE,1				; #RESET is PORTE<1>
#define			#OE			PORTB,1				; #OE is PORTB<1>
#define			#WE			PORTB,0				; #WE is PORTB<0>
#define			CmdN		PORTB,5				; CmdN is PORTB<5>
#define			PCLK		PORTB,4				; PCLK is PORTB<4>
#define			WriteN		PORTE,2				; WriteN is PORTE<2>
#define			RdyN		PORTB,2				; RdyN is PORTB<2>
#define			PortEn		PORTA,3				; Port_En is PORTA<3>
#define			Led			PORTA,1				; LED is PORTA<1>
#define			SwitchSW1	PORTA,2				; SW1 is PORTA<2>
#define			RS232Enable	PORTC,0				; RS232 Transceiver Enable PORTC<0>

#define			TRISA_Value	b'11111101'			; A1 is output (LED)
#define			TRISB_Value	b'11101100'			; B0 is #WE, B1 is #OE, B4 is PCLK
#define			TRISC_Value	b'11111110'			; C0 RS232 Transceiver Enable
#define			TRISD_Value	b'11111111'			; All inputs
#define			TRISE_Value	b'00000001'			; E1 is #RESET, E2 is WriteN

#define			PORTA_INIT_DATA	B'00000010'		; PORTA initial data value
#define			PORTB_INIT_DATA	B'00010011'		; PORTB initial data value
#define			PORTC_INIT_DATA	B'00000000'		; PORTC initial data value
#define			PORTD_INIT_DATA	B'00000000'		; PORTD initial data value
#define			PORTE_INIT_DATA	B'00000110'		; PORTE initial data value

#else
; These values apply to release hardware
#define			ResetN		PORTE,1				; #RESET is PORTE<1>
#define			#OE			PORTB,1				; #OE is PORTB<1>
#define			#WE			PORTA,2				; #WE is PORTA<2>
#define			CmdN		PORTB,5				; CmdN is PORTB<5>
#define			PCLK		PORTB,4				; PCLK is PORTB<4>
#define			WriteN		PORTE,2				; WriteN is PORTE<2>
#define			RdyN		PORTB,2				; RdyN is PORTB<2>
#define			PortEn		PORTA,3				; Port_En is PORTA<3>
#define			Led			PORTA,1				; LED is PORTA<1>
#define			SwitchSW1	PORTB,0				; SW1 is PORTB<0>
#define			RS232Enable	PORTC,0				; RS232 Transceiver Enable PORTC<0>

#define			TRISA_Value	b'11110001'			; A1 is output (LED), A2 is #WE
#define			TRISB_Value	b'11101101'			; B1 is #OE, B4 is PCLK
#define			TRISC_Value	b'11111110'			; C0 RS232 Transceiver Enable
#define			TRISD_Value	b'11111111'			; All inputs
#define			TRISE_Value	b'00000001'			; E1 is #RESET, E2 is WriteN

#define			PORTA_INIT_DATA	B'00001110'		; PORTA initial data value
#define			PORTB_INIT_DATA	B'00010010'		; PORTB initial data value
#define			PORTC_INIT_DATA	B'00000000'		; PORTC initial data value
#define			PORTD_INIT_DATA	B'00000000'		; PORTD initial data value
#define			PORTE_INIT_DATA	B'00000110'		; PORTE initial data value

#endif			; #ifdef EDKPreRelease

ADDR_INC		EQU			b'01000000'			; Address controller auto-increment
ABANK_0			EQU			b'00000000'			; Address Bank 0 select (a7-a0)
ABANK_1			EQU			b'00000001'			; Address Bank 0 select (a15-a8)
ABANK_2			EQU			b'00000010'			; Address Bank 0 select (a16)
ABANK_NONE		EQU			b'00000111'			; Address Bank select none
CmdEraseFlash	EQU			b'10100101'			; Erase Flash command (0xA5)


