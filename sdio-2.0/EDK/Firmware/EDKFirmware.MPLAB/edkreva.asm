;///////////////////////////////////////////////////////////////////////////
;
; SDIO PIC Microcontroller edkreva.asm Src File
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
;///////////////////////////////////////////////////////////////////////////
;
;    Author:        Geoff Richmond
;    Company:       Palm, Inc.
;
;
;----------------------------------------------------------------------
;
;    Notes:
;		1. This file is best viewed with Tab Size set to 4.
;			Go to "Options-->Current Editor Modes" and set tab size
;
;		2. All multi-byte data formats are stored little-endian
;			(least-significant byte first)
;
;----------------------------------------------------------------------


	list      p=16f877            ; list directive to define processor
	#include <p16f877.inc>        ; processor specific variable definitions
	#include "edkreva.inc"
	
; #define	READ_STIMULUS_FILE

;--------------------------------------------------------------------------
; Configuration Word
;--------------------------------------------------------------------------
; '__CONFIG' directive is used to embed configuration data within .asm file.
; The labels following the directive are located in the respective .inc file.
; See respective data sheet for additional information on configuration word.
;
;	_CP_OFF			Code Protect Off		(DO NOT TURN THIS ON!!! If you 
;											turn this on, you will be unable to
;											re-program PIC controller!)
;	_WDT_OFF		Watchdog timer off
;	_BODEN_OFF		Brown-out detect enable off
;	_PWRTE_OFF		Power-up timer enable off
;	_RC_OSC			External RC oscillator
; 	_WRT_ENABLE_ON	Flash program write enable on
;	_LVP_ON			Low-voltage program enable on
;	_DEBUG_OFF		Background debug enable off
;	_CPD_OFF		Data EE protect off
;
	__CONFIG _CP_OFF & _WDT_OFF & _BODEN_OFF & _PWRTE_OFF & _RC_OSC & _WRT_ENABLE_ON & _LVP_ON & _DEBUG_OFF & _CPD_OFF 



;--------------------------------------------------------------------------
; Shared Variables - mapped in all banks 0-3
;--------------------------------------------------------------------------
w_temp			EQU     0x70        ; (UInt8) variable used for context saving 
status_temp		EQU     0x71        ; (UInt8) variable used for context saving
w1				EQU		0x73		; (UInt8) temporary work variable
cardStatus		EQU		0x74		; (UInt8) card status register
ByteCount		EQU		0x75		; (UInt16) CMD53 byte count
SpiRcvBuffer	EQU		0x7A		; (UChar[6]) SD command buffer, 6 bytes


;--------------------------------------------------------------------------
; Variables
;--------------------------------------------------------------------------

; Bank 0 variables
FunctionNumber	EQU		0x60			; (UInt8) Function Number	
DataByte		EQU		0x61			; (UInt8) Response R5 DataByte		
Address			EQU		0x62			; (UInt24) CMD52/CMD53 Address (3 bytes)		
StartAddress	EQU		0x65			; (UInt16) CMD53 Start Address		
EndAddress		EQU		0x67			; (UInt16) CMD53 End Address
SpiTxData		EQU		0x69			; (UInt8) SPI transmit data

; Bank 1 variables
SpiRcvCount		EQU		0xE9			; (Uint8) empty byte count for SpiRcvBuffer



;--------------------------------------------------------------------------
; Program Code
;--------------------------------------------------------------------------

; This is the start of program code

;--------------------------------------------------------------------------
; Reset Vector
;--------------------------------------------------------------------------

                ORG     0x00					; RESET vector
				CLRF    PCLATH					; ensure page bits are cleared
                GOTO    Start					; go to beginning of program
                
;--------------------------------------------------------------------------
; Interrupt Vector
;--------------------------------------------------------------------------

				ORG     0x04					; INTERUPT vector location
				MOVWF   w_temp					; save current W register contents
				MOVF	STATUS,w				; move status register into W register
				MOVWF	status_temp				; save contents of STATUS register


; If you add code here, make sure the address of "Start" is adjusted, if necessary
; to make room for the added code.


EndInt
				MOVF    status_temp,w			; retrieve copy of STATUS register
				MOVWF	STATUS					; restore pre-isr STATUS register contents
				SWAPF   w_temp,f
				SWAPF   w_temp,w				; restore pre-isr W register contents
				RETFIE							; return from interrupt

;--------------------------------------------------------------------------
; Page 0 Functions
;--------------------------------------------------------------------------

                ORG     0x20					; Page 0 of Program Memory (256 byte pages)

;----------------------------------------------------------------------
;
;	Function:	    SendSpiCISBytes
;
;	Description:	This procedure outputs a block of CIS (RAM) data to the
;					SPI port. On entry, StartAddress, EndAddress, and
;					ByteCount should be set. 
;					If (EndAddress-StartAddress) < ByteCount, then
;					the remaining bytes are sent as 0's (Stuff byte).
;					The OpCode bit in the SpiRcvBuffer is checked to
;					determine if this is a multi-byte read to fixed
;					address or multi-byte read to incrementing
;					address.
;					Note: ByteCount is UInt8 (max 0xFF). All our
;					current tuples are < 255 bytes.
;
;----------------------------------------------------------------------
SendSpiCISBytes
				CLRF	STATUS					; Bank 0
				MOVLW	Cmd53StartOfBlock		; Send a CMD53 "Start of block" byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVF	StartAddress,W			; Get start address
				MOVWF	FSR						; Use indirect addressing
				MOVLW	b'00000111'				; Ensure FunctionNumber is in range 0-7.
				ANDWF	FunctionNumber,F
				RLF		FunctionNumber,F		; Compute jump table offset (8*FunctionNumber)
				RLF		FunctionNumber,F		; Compute jump table offset
				RLF		FunctionNumber,W		; Compute jump table offset
				ANDLW	b'11111000'				; Mask 3 right-most bits rotated in as 1's
				MOVWF	w1						; Store in a temporary location
SSCB_Loop1
				MOVF	FSR,W					; Get current address
				SUBWF	EndAddress,W			; 
				BTFSC	STATUS,C				; IF (FSR<=endAddress)
				GOTO	SSCB_Label1				; THEN continue sending bytes
				GOTO	SSCB_Label2				; ELSE send "Stuff Bytes"
SSCB_Label1
				MOVF	w1,W
												; Make sure you adjust PCLATH if this
												; function moves in memory !!!!!!!!!!!!
				ADDWF	PCL,F					; Jump Table based on FN number
				
				MOVF	FSR,W					; Function 0 CIS
				BSF		PCLATH,4				; We must set PCLATH before jumping
				BSF		PCLATH,3
				CALL	CIS_TABLE				; Byte at offset is returned in W
				CLRF	PCLATH					; Clear PCLATH. It gets modified in CIS_TABLE.
				GOTO	SSCB_Label3	
				NOP
				NOP
				
				MOVF	FSR,W					; Function 1 CIS
				BSF		PCLATH,4				; We must set PCLATH before jumping
				BSF		PCLATH,3
				CALL	FN1_CIS_TABLE			; Byte at offset is returned in W
				CLRF	PCLATH					; Clear PCLATH. It gets modified in CIS_TABLE.
				GOTO	SSCB_Label3	
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 2 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 3 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 4 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 5 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 6 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
				MOVLW	StuffByte				; Function 7 CIS
				GOTO	SSCB_Label3				; Just return stuff byte
				NOP	
				NOP		
				NOP		
				NOP		
				NOP
				NOP
				
SSCB_Label2
				MOVLW	StuffByte				; Just return stuff byte
SSCB_Label3
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				BTFSC	(SpiRcvBuffer+1),OpCode	; Is this a multi-byte read
				INCF	FSR,F					; to incrementing address?
				DECFSZ	ByteCount,F				; Are we done?
				GOTO	SSCB_Loop1				; Not done
								
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				
				RETURN							; Return from subroutine


;----------------------------------------------------------------------
;
;	Function:	    DoR5Response
;
;	Description:	This procedure sends an R5 response. The R5 response
;					is a status byte, followed by a data byte.
;					DataByte must be set before calling this routine.
;					We wait until DataByte is actually sent over
;					the bus before returning.
;
;----------------------------------------------------------------------
DoR5Response
				CLRF	STATUS					; Bank 0
				MOVF	cardStatus,W			; Put cardStatus in W
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVF	DataByte,W				; Append data byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host

				CALL	WaitCsabusReady			; Wait for DataByte to be transmitted before we proceed.
												; This ensures the last byte is sent before allowing
												; other routines to access csabus.
				RETURN
				

;----------------------------------------------------------------------
;
;	Function:	    WaitCsabusReady
;
;	Description:	This procedure waits until the CSA Bus is not busy
;					and is ready for new data.
;
;----------------------------------------------------------------------
WaitCsabusReady
				CLRF	STATUS					; Bank 0
WCR_Loop1				
#ifndef	READ_STIMULUS_FILE
				BTFSC	RdyN					; Wait for last byte to be transmitted before we proceed.
												; This ensures the last byte is sent before allowing
												; other routines to access csabus.
				GOTO	WCR_Loop1
#endif
				RETURN
				

;----------------------------------------------------------------------
;
;	Function:	    WaitDataAck
;
;	Description:	This procedure waits until the DataAck signal
;					goes active indicating that the output buffer
;					is ready to latch.
;
;----------------------------------------------------------------------
WaitDataAck
				CLRF	STATUS					; Bank 0
WDA_Loop1
#ifndef	READ_STIMULUS_FILE
				BTFSC	CmdN					; Wait for Data Ack signal
				GOTO	WDA_Loop1
#endif
				RETURN
				
				
;----------------------------------------------------------------------
;
;	Function:	    FlashErase
;
;	Description:	This procedure performs a chip erase on flash.
;					Refer to Am29LV001 data sheets for complete
;					description of Flash command set.
;
;----------------------------------------------------------------------
FlashErase
				CLRF	STATUS					; Bank 0
				
				CALL	WaitCsabusReady			; Wait for CSA Bus ready before we
												; put data on it.

				BCF		Led						; Turn off LED
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					; PORTD<7:0> = outputs
				BCF		TRISE,4					; Ensure PSP Mode reset
				BCF		STATUS,RP0				; Bank 0
				CLRF	(Address+2)				; Clear msb of Address
				
; Send first instruction to Flash  (555 AA)
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0xAA
				MOVWF	(Address+1)
				MOVLW	0xAA
				MOVWF	DataByte
				CALL	SendFlashCommand
				
; Send second instruction to Flash (2AA 55)
				MOVLW	0xAA					; Send instruction to Flash
				MOVWF	Address					; Address = 0x02AA
				MOVLW	0x02					; Data = 0x55
				MOVWF	(Address+1)
				MOVLW	0x55
				MOVWF	DataByte
				CALL	SendFlashCommand
				
; Send third instruction to Flash (555 80)
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0x80
				MOVWF	(Address+1)
				MOVLW	0x80
				MOVWF	DataByte
				CALL	SendFlashCommand

; Send fourth instruction to Flash  (555 AA)
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0xAA
				MOVWF	(Address+1)
				MOVLW	0xAA
				MOVWF	DataByte
				CALL	SendFlashCommand

; Send fifth instruction to Flash (2AA 55)
				MOVLW	0xAA					; Send instruction to Flash
				MOVWF	Address					; Address = 0x02AA
				MOVLW	0x02					; Data = 0x55
				MOVWF	(Address+1)
				MOVLW	0x55
				MOVWF	DataByte
				CALL	SendFlashCommand

; Send sixth instruction to Flash (555 10)
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0x10
				MOVWF	(Address+1)
				MOVLW	0x10
				MOVWF	DataByte
				CALL	SendFlashCommand

; Wait for data to be 0xFF (erase complete)
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0

				BCF		#OE						; Set #OE low
CheckStatusErase
				MOVLW	0xFF					;
				XORWF	PORTD,W					; Verify that it is 0xFF
				BTFSS	STATUS,Z				; IF not match
				GOTO	CheckStatusErase		; THEN try again

;End Chip Erase Sequence				
				BSF		#OE						; Set #OE high
				CLRF	STATUS
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					; PORTD<7:0> = outputs
				BCF		STATUS,RP0				; Bank 0
				CLRF	(Fn1RegStartLo8+3)		; Register 0x03 is the command register
				BSF		Led						; Turn on LED
				
				RETURN



;----------------------------------------------------------------------
;
;	Function:	    ReadTemperature
;
;	Description:	This procedure performs an A/D conversion on the 
;					temperature input (AN0 pin).
;
;----------------------------------------------------------------------
ReadTemperature
				CLRF	STATUS					; Bank 0
												;
												; Ensure that the required sampling time for the selected input
												; channel has elapsed. Then the conversion may be started.
												;
;RTLoop1
;				BTFSC	ADCON0,GO				; The GO/DONE bit is cleared upon completion of the
;												; A/D Conversion. We ignore this and assume the 
;				GOTO	RTLoop1					; conversion is complete. (commented out this code)

				MOVF	ADRESH,W				; Get high-order byte of A/D result
				MOVWF	(Fn1RegStartLo8+2)		; Put in Function 1 registers
				BSF		STATUS,RP0				; Bank 1
				MOVF	ADRESL,W				; Get low-order byte of A/D result
				BCF		STATUS,RP0				; Bank 0
				MOVWF	(Fn1RegStartLo8+1)		; Put in Function 1 registers
				BSF		ADCON0,GO 				; Start A/D Conversion for the next measurement

				RETURN






;--------------------------------------------------------------------------
; Main Program
;--------------------------------------------------------------------------

                ORG     0x100					; Page 1 of Program Memory (256 byte pages)

Start
				
; Intialize Port A
        		CLRF	STATUS					; Bank 0
				MOVLW	PORTA_INIT_DATA			; Initialize PORTA data
				MOVWF	PORTA					; 
				BSF		STATUS,RP0				; Bank 1
				MOVLW	0x8E					; Configure AN0 as analog pin
				MOVWF	ADCON1					; as digital pins
				MOVLW	TRISA_Value				; Value used to initialize data direction
				MOVWF	TRISA
				BCF 	STATUS, RP0 			; Bank0
				MOVLW 	0xC1 					; RC Clock, A/D is on, Channel 0 is selected
				MOVWF 	ADCON0 					;

; Intialize Port B
        		CLRF	STATUS					; Bank 0 
				MOVLW	PORTB_INIT_DATA			; Initialize PORTB data
				MOVWF	PORTB					; 
				BSF		STATUS,RP0				; Bank 1
				MOVLW	TRISB_Value				; Value used to initialize data direction
				MOVWF	TRISB					; 

; Intialize Port C
        		CLRF	STATUS					; Bank 0 
				MOVLW	PORTC_INIT_DATA			; 
				MOVWF	PORTC					; Initialize PORTC outputs
				BSF		STATUS,RP0				; Bank 1
				MOVLW	TRISC_Value				; Value used to initialize data direction
				MOVWF	TRISC					; 

; Intialize Port D (csabus)
        		CLRF	STATUS					; Bank 0 
				MOVLW	PORTD_INIT_DATA			; 
				MOVWF	PORTD					; Initialize PORTD outputs
				BSF		STATUS,RP0				; Bank 1
				MOVLW	TRISD_Value				; Value used to initialize data direction
				MOVWF	TRISD					; PORTD<7:0> = inputs

; Intialize Port E
	       		CLRF	STATUS					; Bank 0 
				MOVLW	PORTE_INIT_DATA			; Set #RESET, WriteN high
				MOVWF	PORTE					; Initialize PORTE outputs
				BSF		STATUS,RP0				; Bank 1
				MOVLW	TRISE_Value				; Value used to initialize data direction
				MOVWF	TRISE					; PORTE<2:1> = outputs, others = inputs

; Initialize the serial port
	       		CLRF	STATUS					; Bank 0 
				BSF		STATUS,RP0				; Bank1
;				MOVLW	19h						; 9600 baud
				MOVLW	0Dh						; 57600 baud
				MOVWF	SPBRG
				MOVLW	b'10100100'				; Async, High baud rate
				MOVWF	TXSTA
				BCF		STATUS,RP0				; Bank 0
				BSF		RS232Enable				; Enable RS232 Transceiver
	
				MOVLW	b'10010000'				; Enable continous reception
				MOVWF	RCSTA

; Initialize all RAM (General Purpose Registers) by clearing to zero
				CLRF	STATUS					; Clear STATUS register (Bank0)
				MOVLW	0x20					; 1st address (in bank) of GPR area
				MOVWF	FSR						; Move it to Indirect address register
Bank0_LP
				CLRF	INDF					; Clear GPR at address pointed to by FSR
				INCF	FSR,F					; Next GPR (RAM) address
				BTFSS	FSR,7					; End of current bank ? (FSR = 80h, Z = 0)
				GOTO	Bank0_LP				; NO, clear next location
				;
				; Next Bank (Bank1)
				;
				MOVLW	0xA0					; 1st address (in bank) of GPR area
				MOVWF	FSR						; Move it to Indirect address register
Bank1_LP
				CLRF	INDF					; Clear GPR at address pointed to by FSR
				INCF	FSR,F					; Next GPR (RAM) address
				BTFSS	STATUS,Z				; End of current bank? (FSR = 00h, Z = 1)
				GOTO	Bank1_LP				; NO, clear next location
				;	
				; Next Bank (Bank2)
				;
				BSF		STATUS,IRP				; Select Bank2 and Bank3
												; for Indirect addressing
				MOVLW	0x10					; 1st address (in bank) of GPR area
				MOVWF	FSR						; Move it to Indirect address register
Bank2_LP
				CLRF	INDF					; Clear GPR at address pointed to by FSR
				INCF	FSR,F					; Next GPR (RAM) address
				BTFSS	FSR,7					; End of current bank? (FSR = 80h, Z = 0)
				GOTO	Bank2_LP				; NO, clear next location
				;
				; Next Bank (Bank3)
				;
				MOVLW	0x90					; 1st address (in bank) of GPR area
				MOVWF	FSR						; Move it to Indirect address register
Bank3_LP
				CLRF	INDF					; Clear GPR at address pointed to by FSR
				INCF	FSR,F					; Next GPR (RAM) address
				BTFSS	STATUS,Z				; End of current bank? (FSR = 00h, Z = 1)
				GOTO	Bank3_LP				; NO, clear next location
				CLRF	STATUS					; Clear STATUS register

; Reset the CPLD and the flash RAM
        		CLRF	STATUS					; Bank 0 
				BCF		ResetN					; Set #RESET low
				NOP								; Leave #RESET low for 4 instruction clocks
				MOVF	PORTD,W					; Read the CPLD version number
				MOVWF	Fn1CpldVersion			; Save the version number in Function 1 register space
				NOP
				BSF		ResetN					; Set #RESET high

; Initialize the Firmware version number
				CLRF	STATUS					; Bank 0
				MOVLW	FwVersion				; Put the PIC FW version number in W
				MOVWF	Fn1PicVersion			; Save the version number in Function 1 register space

; Initialize the CCCR registers to initial values
Init_CCCR
				CLRF	STATUS					; Bank 0
				BSF		STATUS,RP0				; Bank 1
				MOVLW	SDIORevision			; Initialize SDIO Revision
				MOVWF	CccrRevision1			;
				SWAPF	CccrRevision1,F			;
				MOVLW	CCCRRevision			; Initialize CCCR Revision
				IORWF	CccrRevision1,F			;
				MOVLW	SDRevision				; Initialize SD Revision
				ANDLW	0x0F					; 
				IORWF	CccrRevision2,F			;
				BCF		CccrCardCapbty,SMB		; We do NOT support CMD53 in block mode
				MOVLW	COMMON_CIS_START_LO_BYTE	; Set the Card Common CIS Pointer (LSB first)
				MOVWF	CccrCisPtr				;
				MOVLW	COMMON_CIS_START_MID_BYTE	; Set the Card Common CIS Pointer (LSB first)
				MOVWF	CccrCisPtr+1			;
				MOVLW	COMMON_CIS_START_HI_BYTE	; Set the Card Common CIS Pointer (LSB first)
				MOVWF	CccrCisPtr+2			;

; Initialize the Function 1 FBR registers to initial values
InitFn1_FBR
				CLRF	STATUS					; Bank 0
				BSF		STATUS,RP0				; Bank 1
				BCF		Fbr1Info,CSA_Enable		; CSA IS NOT enabled
				BSF		Fbr1Info,CSA_Supported	; We DO support CSA
				MOVLW	SDIO_NonStandard		; We do not implement a standard
				ANDLW	0x0F					; I/O device like a UART, etc.
				IORWF	Fbr1Info,F				;
				MOVLW	FN1_CIS_START_LO_BYTE	; Set the Fn 1 CIS Pointer (LSB first)
				MOVWF	Fbr1CisPtr				;
				MOVLW	FN1_CIS_START_MID_BYTE	; Set the Fn 1 CIS Pointer (LSB first)
				MOVWF	Fbr1CisPtr+1			;
				MOVLW	FN1_CIS_START_HI_BYTE	; Set the Fn 1 CIS Pointer (LSB first)
				MOVWF	Fbr1CisPtr+2			;
				MOVLW	FN1_IO_BLK_SIZE_LO_BYTE	; Set the Fn 1 I/O Block Size (LSB first)
				MOVWF	Fbr1IoBlockSz			;
				MOVLW	FN1_IO_BLK_SIZE_HI_BYTE	; Set the Fn 1 I/O Block Size (LSB first)
				MOVWF	Fbr1IoBlockSz+1			;
				
#ifdef	EDKPreRelease
; Check if Port_En is set for programming CPLD
; If it is, we just keep looping here while the CPLD
; is programmed. (NOTE: Does not apply to release hardware)
				CLRF	STATUS					; Bank 0 
				BTFSS	PortEn					; IF Port_En is low
				GOTO	InitSPI					; THEN continue
				GOTO	ProgramCPLDLoop			; ELSE just wait to program the CPLD
#endif

				
; Initialize the SPI Port
InitSPI
				CLRF	STATUS					; Bank 0
        		CLRF	SSPCON					; Clear SSP Control
				BSF		SSPCON,SSPM2			; SPI Slave Mode, CS enabled
				BSF		SSPCON,CKP				; SPI CLK idle high
				BSF		SSPCON,SSPEN			; Enable SPI Port
; Send Config byte to CPLD to enable SPI output
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					; PORTD<7:0> = outputs
				BCF		TRISE,4					; Disable PSP Mode (Parallel Port)
				

; Poll for SD command on the SPI port. On entry, STATUS must be cleared to ensure
; addressing is correct. (Bank 1).
PollSPI
				CLRF	STATUS					; Bank 0
				BSF		STATUS,RP0				; Bank 1
				MOVLW	SpiRcvBuffer			; Address of command buffer
				MOVWF	FSR						; Move it to Indirect address register
				MOVLW	BytesPerSDCmd			; number of bytes per SD command (6)
				MOVWF	SpiRcvCount				; Load the empty byte count
SPI_LP
#ifdef	READ_STIMULUS_FILE
				GOTO	InputBYTE
#endif
        		BTFSC   SSPSTAT,BF				; Is Data Received?
        		GOTO    InputBYTE				; Yes, then process the byte
EndSPI_LP
        		GOTO    SPI_LP					; Else check again

; Process one byte of input on the SPI port.
; At this point, time is critical. We can assume that RP1 is cleared prior to calling this. 
InputBYTE
				BCF		STATUS,RP0				; Bank 0
				MOVF	SSPBUF,W				; Get received data
				BSF		STATUS,RP0				; Bank 1
				MOVWF   INDF					; Store byte in command buffer (indirect addressing)
				BTFSC	SpiRcvBuffer,StartBit	; Has a start bit been found?
				GOTO	EndInputBYTE			; Ignore this byte if start bit not set
				INCF	FSR,F					; Increment indirect address register
				DECF    SpiRcvCount,F			; Decrement empty byte count
				BTFSC	STATUS,Z				; Got a full SD command? (Z = 1)
				GOTO	ParseCommand
        		
EndInputBYTE
				GOTO    SPI_LP					; Wait for next byte
				
; We have a full 6-byte SD command. Process it
ParseCommand
				CLRF	STATUS					; Bank 0
				MOVF	SpiRcvBuffer,W			; Get command[0]
				ANDLW	CommandMask				; Mask the start bit and direction bit
				MOVWF	w1						; Save a temporary copy of command index
				XORLW	SDIOCmd0				; Is this a CMD0?
				BTFSC	STATUS,Z				;
				GOTO	DoCMD0					; Process CMD0
				MOVF	w1,W					;
				XORLW	SDIOCmd5				; Is this a CMD5?
				BTFSC	STATUS,Z				;
				GOTO	DoCMD5					; Process CMD5
				MOVF	w1,W					;
				XORLW	SDIOCmd52				; Is this a CMD52?
				BTFSC	STATUS,Z				;
				GOTO	DoCMD52					; Process CMD52
				MOVF	w1,W					;
				XORLW	SDIOCmd53				; Is this a CMD53?
				BTFSC	STATUS,Z				;
				GOTO	DoCMD53					; Process CMD53
				GOTO	DoR5IllegalCommand		; We received an unsupported command
				


;----------------------------------------------------------------------
;
;	Function:	    DoCMD0
;
;	Description:	This procedure processes a CMD0 instruction.
;					(RESET)
;
;----------------------------------------------------------------------
DoCMD0
				CLRF	STATUS					; Bank 0
				CLRF	cardStatus				; No errors
DoR1Response
				MOVF	cardStatus,W			; Put cardStatus in W
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host

				CALL	WaitCsabusReady			; Wait for data to be sent before we proceed.
												; This ensures the last byte is sent before
												; allowing other routines to access csabus.

				GOTO	PollSPI					; Return to command loop

;----------------------------------------------------------------------
;
;	Function:	    DoCMD5
;
;	Description:	This procedure processes a CMD5 instruction.
;					(IO_SEND_OP_COND)
;
;----------------------------------------------------------------------
DoCMD5
				CLRF	STATUS					; Bank 0
				CLRF	cardStatus				; No errors
DoR4Response
				MOVF	cardStatus,W			; Put cardStatus in W
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	MyOcrRegisterByte1
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	MyOcrRegisterByte2
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	MyOcrRegisterByte3
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	MyOcrRegisterByte4
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host

				CALL	WaitCsabusReady			; Wait for data to be sent before we proceed.
												; This ensures the last byte is sent before
												; allowing other routines to access csabus.

				GOTO	PollSPI					; Return to command loop
				
;----------------------------------------------------------------------
;
;	Function:	    DoCMD52
;
;	Description:	This procedure processes a CMD52 instruction.
;					(IO_RW_DIRECT)
;
;----------------------------------------------------------------------
DoCMD52
				CLRF	STATUS					; Bank 0
				CLRF	cardStatus				; No errors
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing the function number
				ANDLW	FnNumberMask			; Mask off unwanted bits
				MOVWF	w1						; Save a temporary copy of function number
				XORLW	SDIOFn0					; Is this Function 0 (CIA)?
				BTFSC	STATUS,Z				;
				GOTO	DoFn0Cmd52				; Process Fn 0 CMD52
				MOVF	w1,W					;
				XORLW	SDIOFn1					; Is this Function 1?
				BTFSC	STATUS,Z				;
				GOTO	DoFn1Cmd52				; Process Fn 1 CMD52
				GOTO	DoR5FnNumberError		; Else, function not present

DoFn0Cmd52				
				; Is the address > b'00111111111111111'
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing bits [16:15] of Address
				ANDLW	0x03					; Test if bits 16 or 15 are set
				BTFSS	STATUS,Z
				GOTO	Fn0Cmd52AddrOutOfRange	; Address is out of range
				
				; Get the lower 15 bits of address in Address register
				MOVF	(SpiRcvBuffer+2),W		; Get the byte containing bits [14:7] of Address
				MOVWF	(Address+1)				; Move the high order address byte
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing bits [6:0] of Address
				ANDLW	b'11111110'				; Mask unwanted bit
				MOVWF	Address					; Move the low order address byte
				CLRF	STATUS
				RRF		(Address+1),F			; Shift right to byte align it
				BCF		(Address+1),7
				RRF		Address,F
				
				; Is this a CCCR address? (Address <= 0xFF)
				MOVF	(Address+1),W			; Test the high order byte
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd52CCCRByte		; This is a CCCR read/write
				
				; Is this an FBR1 address? (0x100 <= Address <= 0x1FF)
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x01
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd52FBR1Byte		; This is an FBR1 read/write
				
				; Is this a CIS address? (0x1000 <= Address <= 0x10FF)
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x10
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd52CISByte		; This is a CIS read/write
				
				; Is this a FN1 CIS address? (0x2000 <= Address <= 0x20FF)
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x20
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd52FN1CISByte	; This is a FN1 CIS read/write
				
				; Else this is an unsupported address
				GOTO	Fn0Cmd52AddrOutOfRange				

DoFn0Cmd52CCCRByte
				MOVF	Address,W				; Get low-order byte of address
				ANDLW	b'11100000'
				BTFSS	STATUS,Z				; IF (Address > 0x1F)
				GOTO	Fn0Cmd52AddrOutOfRange	; THEN send 0's if address out of range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVLW	CCCRStartLo8
				ADDWF	StartAddress,W			; Offset address to start of CCCR
				MOVWF	FSR						; Use indirect addressing
				BCF		STATUS,IRP				; CCCR RAM accesses are Bank 0,1
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a write operation?
				GOTO	Fn0Cmd52ReadCCCR		; No, it's a read operation
Fn0Cmd52WriteCCCR
				MOVF	(SpiRcvBuffer+4),W		; Get the byte to be written
				MOVWF	INDF					; Write byte at address FSR
				BTFSS	(SpiRcvBuffer+1),RAWFlag	; Is this a read after write operation?
Fn0Cmd52ReadCCCR
				MOVF	INDF,W					; Yes, read the data just written.
				MOVWF	DataByte
				CALL	DoR5Response			; Send the R5 Response
				GOTO	EndCMD52				; All done

DoFn0Cmd52FBR1Byte
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11100000'
				BTFSS	STATUS,Z				; IF (Address > 0x1F)
				GOTO	Fn0Cmd52AddrOutOfRange	; THEN send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				XORLW	CsaWindowOffset			; Is this a CSA operation?
				BTFSC	STATUS,Z
				GOTO	DoCmd52CSA1Byte			; Yes, this is a CSA operation
				MOVLW	FBR1Start
				ADDWF	StartAddress,W			; Offset address to start of FBR1
				MOVWF	FSR						; Use indirect addressing
				BCF		STATUS,IRP				; FBR1 RAM accesses are Bank 0,1
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	Fn0Cmd52ReadFBR1		; No, it's a write operation
Fn0Cmd52WriteFBR1
				MOVF	(SpiRcvBuffer+4),W		; Get the byte to be written
				MOVWF	INDF					; Write byte at address FSR
				BTFSC	(SpiRcvBuffer+1),RAWFlag	; Is this a read after write operation?
Fn0Cmd52ReadFBR1
				MOVF	INDF,W					; Yes, read the data just written.
				MOVWF	DataByte
				CALL	DoR5Response			; Send the R5 Response
				GOTO	EndCMD52				; All done


DoFn0Cmd52CISByte
				BTFSC	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoR5IllegalCommand		; No, send error. CIS is read-only.
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11000000'
				BTFSS	STATUS,Z				; IF (Address > 0x3F)
				GOTO	Fn0Cmd52AddrOutOfRange	; THEN send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVF	StartAddress,W
				BSF		PCLATH,4				; We must set PCLATH before jumping
				BSF		PCLATH,3
				CALL	CIS_TABLE
				CLRF	PCLATH
				NOP
				NOP
				NOP
				NOP
				MOVWF	DataByte
				CALL	DoR5Response			; Send the R5 Response
				GOTO	EndCMD52				; All done

DoFn0Cmd52FN1CISByte
				BTFSC	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoR5IllegalCommand		; No, send error. CIS is read-only.
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11000000'
				BTFSS	STATUS,Z				; IF (Address > 0x3F)
				GOTO	Fn0Cmd52AddrOutOfRange	; THEN Send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVF	StartAddress,W
				BSF		PCLATH,4				; We must set PCLATH before jumping
				BSF		PCLATH,3
				CALL	FN1_CIS_TABLE
				CLRF	PCLATH
				NOP
				NOP
				NOP
				NOP
				MOVWF	DataByte
				CALL	DoR5Response			; Send the R5 Response
				GOTO	EndCMD52				; All done

DoCmd52CSA1Byte
				BSF		STATUS,RP0				; Bank 1
				BCF		STATUS,RP1				;
#ifndef	READ_STIMULUS_FILE
				BTFSS	FBR1Start,CSA_Enable	; Is the CSA enabled?
				GOTO	DoR5IllegalCommand		; No, send illegal command error code
#endif
												; Yes, then send Config byte to CPLD
				CLRF	STATUS					; Bank 0
				MOVLW	ABANK_2					; CPLD Address Bank #2
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr+2,W			; Get bits [23:16] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[16]
				BSF		PCLK					; Set PCLK high
				MOVLW	ABANK_1					; CPLD Address Bank #1
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr+1,W			; Get bits [15:8] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[15:8]
				BSF		PCLK					; Set PCLK high
				MOVLW	ABANK_0					; CPLD Address Bank #0
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr,W			; Get bits [7:0] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[7:0]
				BSF		PCLK					; Set PCLK high
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	Fn0Cmd52ReadCSA1		; No, it's a write operation
Fn0Cmd52WriteCSA1
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				BCF		#OE						; Set #OE low
				MOVF	PORTD,W					; Read the byte
				MOVWF	w1						; Store the byte temporarily
				BSF		#OE						; Set #OE high
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
#ifndef	READ_STIMULUS_FILE
				MOVF	w1,W					; Get the byte we read
				XORLW	0xFF					; Ensure this byte has been erased
				BTFSS	STATUS,Z
				GOTO	DoR5IllegalCommand		; Send illegal command if trying to write an un-erased byte
#endif
				MOVF	(SpiRcvBuffer+4),W		; Get the byte to be written
				BSF		STATUS,RP0				; Bank 1
				MOVWF	Fbr1CsaWindow
				CLRF	STATUS					; Bank 0
				CALL	WriteFlashByte			; Write the data to Flash
				CLRF	STATUS					; Bank 0
				BTFSS	(SpiRcvBuffer+1),RAWFlag	; Is this a read after write operation?
				GOTO	DC52RCSA1BLabel1
Fn0Cmd52ReadCSA1
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				BCF		#OE						; Set #OE low
				MOVF	PORTD,W					; Read the byte
				BSF		#OE						; Set #OE high
				BSF		STATUS,RP0				; Bank 1
				MOVWF	Fbr1CsaWindow
				MOVLW	B'00000000'				; Enable csabus outputs
				MOVWF	TRISD					;
DC52RCSA1BLabel1				
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaWindow,W			; Get the byte that was written
				CLRF	STATUS					; Bank 0
				MOVWF	DataByte
				CALL	DoR5Response
				BSF		STATUS,RP0				; Bank 1
				INCF	Fbr1CsaPtr,F			; Increment the CSA Ptr after each operation
				BTFSS	STATUS,Z
				GOTO	DC52RCSA1BLabel2
				INCF	Fbr1CsaPtr+1,F
				BTFSS	STATUS,Z
				GOTO	DC52RCSA1BLabel2
				INCF	Fbr1CsaPtr+2,F
DC52RCSA1BLabel2
				CLRF	STATUS					; Bank 0
				GOTO	EndCMD52				; All done
				

Fn0Cmd52AddrOutOfRange
				CLRF	DataByte
				CALL	DoR5Response
				GOTO	EndCMD52				; All done


DoFn1Cmd52
				; Is the address out of range? (Address >= 0x0003F)
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing bits [16:15] of Address
				ANDLW	0x03					; Test if bits 16 or 15 are set
				BTFSS	STATUS,Z
				GOTO	DoR5ParameterError		; Function 1 supports 64 bytes of register space
				
				; Is the address out of range? (Address >= 0x0003F)
				MOVF	(SpiRcvBuffer+2),W		; Get the byte containing bits [14:7] of Address
				BTFSS	STATUS,Z
				GOTO	DoR5ParameterError		; Function 1 supports 64 bytes of register space
				
				; Get the lower 7 bits of address in Address register
				CLRF	(Address+1)				; Clear high-order address byte
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing bits [6:0] of Address
				ANDLW	b'11111110'				; Mask unwanted bit
				MOVWF	Address					; Move the low order address byte
				CLRF	STATUS
				RRF		Address,F
				BCF		Address,7
				
				; Is the address out of range? (Address >= 0x003F)
				BTFSC	Address,6
				GOTO	DoR5ParameterError		; Function 1 supports 64 bytes of register space
				
				; This is a valid Function 1 address? (0x0000 <= Address <= 0x003F)
				GOTO	DoFn1Cmd52Byte			; This is Function 1 read/write
				
DoFn1Cmd52Byte
				MOVF	Address,W			; Get low-order byte
				MOVWF	StartAddress
				NOP
				MOVLW	0x01					; Register 0x01 is the A/D (Thermometer)
				XORWF	StartAddress,W
				BTFSC	STATUS,Z				; IF (A/D operation)
				CALL	ReadTemperature			; THEN Read the temperature
				NOP
				MOVLW	Fn1RegStartLo8
				ADDWF	StartAddress,W			; Offset address to start of Function 1 register space
				MOVWF	FSR						; Use indirect addressing
				BCF		STATUS,IRP				; Function 1 RAM accesses are Bank 0
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	Fn1Cmd52ReadByte		; No, it's a write operation
Fn1Cmd52WriteByte
				MOVF	(SpiRcvBuffer+4),W		; Get the byte to be written
				MOVWF	INDF					; Write byte at address FSR
				BTFSS	(SpiRcvBuffer+1),RAWFlag	; Is this a read after write operation?
Fn1Cmd52ReadByte
				MOVF	INDF,W					; Yes, read the data just written.
				MOVWF	DataByte
				CALL	DoR5Response			; Send the R5 Response
				
				CLRF	STATUS					; Bank 0
				MOVF	Fn1RegStartLo8,W		; Bit 0 of register 0x00 controls LED
				MOVWF	w1
				CLRF	STATUS					; Bank 0
				BCF		Led
				BTFSC	w1,0					; Bit 0 of register 0x00 controls LED
				BSF		Led

				CLRF	STATUS					; Bank 0
				MOVLW	CmdEraseFlash			; Register 0x03 is the command register
				XORWF	(Fn1RegStartLo8+3),W	; 0xA5 will erase the flash
				BTFSC	STATUS,Z				; IF (FormatCard)
				CALL	FlashErase				; THEN erase the Flash Memory
				CLRF	(Fn1RegStartLo8+3)
DF1C52BLabel1
				CLRF	STATUS					; Bank 0
				GOTO	EndCMD52				; All done


EndCMD52
				GOTO	PollSPI					; Wait for next command

EndCMD53
				CALL	WaitCsabusReady			; Wait for data to finish transmitting
												; before we proceed. This ensures the
												; last byte is sent before allowing
												; other routines to access csabus.

				GOTO	PollSPI					; Wait for next command

DoR5FnNumberError
				CLRF	STATUS					; Bank 0
				BSF		cardStatus,FnNumberError	; Set the Fn Number error bit
				CLRF	DataByte
				CALL	DoR5Response			; Send an R5 response
				GOTO	EndCMD52				; All done

DoR5IllegalCommand
				CLRF	STATUS					; Bank 0
				BSF		cardStatus,IllegalCommand	; Set the illegal command bit
				CLRF	DataByte
				CALL	DoR5Response			; Send an R5 response
				GOTO	EndCMD52				; All done

DoR5ParameterError
				CLRF	STATUS					; Bank 0
				BSF		cardStatus,ParameterError	; Set the parameter error bit
				CLRF	DataByte
				CALL	DoR5Response			; Send an R5 response
				GOTO	EndCMD52				; All done

DoR1CmdError
				CLRF	STATUS					; Bank 0
				BSF		cardStatus,IllegalCommand	; Set the illegal command bit in status
				GOTO	DoR1Response			; Send the R1 Response
				

				

;----------------------------------------------------------------------
;
;	Function:	    DoCMD53
;
;	Description:	This procedure processes a CMD53 instruction.
;					(IO_RW_EXTENDED)
;
;----------------------------------------------------------------------
DoCMD53
				CLRF	cardStatus				; No errors
				CLRF	STATUS
;				BSF		Led
				BTFSC	(SpiRcvBuffer+1),BlockMode	; Is this a block operation?
				GOTO	DoR5IllegalCommand		; Yes, then send illegal command response
				
Cmd53Read
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing the function number
				ANDLW	FnNumberMask			; Mask off unwanted bits
				MOVWF	w1						; Save a temporary copy of function number
				XORLW	SDIOFn0					; Is this Function 0 (CIA)?
				BTFSC	STATUS,Z				;
				GOTO	DoFn0Cmd53				; Process Fn 0 CMD53
				MOVF	w1,W					;
				XORLW	SDIOFn1					; Is this Function 1?
				BTFSC	STATUS,Z				;
				GOTO	DoFn1Cmd53				; Process Fn 1 CMD53
				GOTO	DoR5FnNumberError		; Else, function not present

DoFn0Cmd53
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing the msb of Byte count
				ANDLW	MsbByteCountMask		; Mask all but bit 0.
				MOVWF	(ByteCount+1)			; Byte count is 9 bits
				MOVF	(SpiRcvBuffer+4),W		; Get the byte containing the LSB of ByteCount
				MOVWF	ByteCount				; Set ByteCount
				BTFSS	STATUS,Z				; IF (ByteCount != 0)
				GOTO	DF0C53RLabel1			; THEN continue
				MOVF	(ByteCount+1),W			; ELSE IF (ByteCount(msb) != 0)
				BTFSS	STATUS,Z
				GOTO	DF0C53RLabel1			; THEN continue
				BTFSC	(SpiRcvBuffer+1),BlockMode		; ELSE IF (BlockMode)
				GOTO	DoR5ParameterError		; THEN BlockMode Infinite Read not supported
				MOVLW	0x02					; ELSE ByteCount = 512
				MOVWF	(ByteCount+1)
DF0C53RLabel1				
				CLRF	DataByte
				CALL	DoR5Response			; Send the R5 Response

				; Is the address > b'00111111111111111'
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing bits [16:15] of Address
				ANDLW	0x03					; Test if bits 16 or 15 are set
				BTFSC	STATUS,Z
				GOTO	DF0C53Label2
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoCmd53ReadStuffBytes	; Send 0's if out of address range
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.
				
DF0C53Label2
				; Get the lower 15 bits of address in Address register
				MOVF	(SpiRcvBuffer+2),W		; Get the byte containing bits [14:7] of Address
				MOVWF	(Address+1)				; Move the high order address byte
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing bits [6:0] of Address
				ANDLW	b'11111110'				; Mask unwanted bit
				MOVWF	Address					; Move the low order address byte
				CLRF	STATUS
				RRF		(Address+1),F			; Shift right to byte align it
				BCF		(Address+1),7
				RRF		Address,F
				
				; Is this a CCCR address? (Address <= 0xFF)
				MOVF	(Address+1),W			; Test the high order byte
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd53CCCRBytes		; This is a CCCR operation
				
				; Is this an FBR1 address? (0x100 <= Address <= 0x1FF)
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x01
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd53FBR1Bytes		; This is an FBR1 operation
				
				; Is this a CIS address? (0x1000 <= Address <= 0x10FF)
				CLRF	FunctionNumber
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x10
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd53CISBytes		; This is a CIS operation
				
				; Is this a FN1 CIS address? (0x2000 <= Address <= 0x20FF)
				INCF	FunctionNumber,F
				MOVF	(Address+1),W			; Test the high order byte
				XORLW	0x20
				BTFSC	STATUS,Z
				GOTO	DoFn0Cmd53CISBytes		; This is a FN1 CIS operation
				
				; Else this is an unsupported address
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoCmd53ReadStuffBytes	; Send 0's if out of address range
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoFn0Cmd53CCCRBytes
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoFn0Cmd53ReadCCCRBytes	; It's a read operation.
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoFn0Cmd53ReadCCCRBytes
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11100000'
				BTFSS	STATUS,Z				; IF (Address > 0x1F)
				GOTO	DoCmd53ReadStuffBytes	; THEN Send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVWF	EndAddress
				MOVF	ByteCount,W				; EndAddress = StartAddress + ByteCount
				ADDWF	EndAddress,F
				DECF	EndAddress,F
				MOVF	EndAddress,W
				ANDLW	b'11100000'				; EndAddress = (EndAddress>0x1F) ? 0x1F : EndAddress
				BTFSC	STATUS,Z
				GOTO	DF0C53RCBLabel1
				MOVLW	0x1F
				MOVWF	EndAddress
DF0C53RCBLabel1
				MOVLW	CCCRStartLo8
				ADDWF	StartAddress,F			; Offset address to start of CCCR
				ADDWF	EndAddress,F			; Offset address to start of CCCR
				CALL	SendSpiRamBytes			; Send the bytes to the host
				GOTO	EndCMD53				; All done

DoFn0Cmd53FBR1Bytes
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoFn0Cmd53ReadFBR1Bytes	; It's a read operation.
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoFn0Cmd53ReadFBR1Bytes
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11100000'
				BTFSS	STATUS,Z				; IF (Address > 0x1F)
				GOTO	DoCmd53ReadStuffBytes	; THEN Send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				XORLW	CsaWindowOffset			; Is this a read from CSA?
				BTFSC	STATUS,Z
				GOTO	DoCmd53ReadCSA1Bytes	; Yes, this is a read from CSA
				MOVWF	EndAddress
				MOVF	ByteCount,W				; EndAddress = StartAddress + ByteCount
				ADDWF	EndAddress,F
				DECF	EndAddress,F
				MOVF	EndAddress,W
				ANDLW	b'11100000'				; EndAddress = (EndAddress>0x1F) ? 0x1F : EndAddress
				BTFSC	STATUS,Z
				GOTO	DF0C53RF1BLabel1
				MOVLW	0x1F
				MOVWF	EndAddress
DF0C53RF1BLabel1
				MOVLW	FBR1Start
				ADDWF	StartAddress,F			; Offset address to start of FBR1
				ADDWF	EndAddress,F			; Offset address to start of FBR1
				CALL	SendSpiRamBytes			; Send the bytes to the host
				GOTO	EndCMD53				; All done



DoFn0Cmd53CISBytes
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoFn0Cmd53ReadCISBytes	; It's a read operation.
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoFn0Cmd53ReadCISBytes
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11000000'
				BTFSS	STATUS,Z				; IF (Address > 0x3F)
				GOTO	DoCmd53ReadStuffBytes	; THEN Send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVWF	EndAddress
				MOVF	ByteCount,W				; EndAddress = StartAddress + ByteCount
				ADDWF	EndAddress,F
				DECF	EndAddress,F
				MOVF	EndAddress,W
				ANDLW	b'11000000'				; EndAddress = (EndAddress>0x3F) ? 0x3F : EndAddress
				BTFSC	STATUS,Z
				GOTO	DF0C53RCISBLabel1
				MOVLW	0x3F
				MOVWF	EndAddress
DF0C53RCISBLabel1
				CALL	SendSpiCISBytes			; Send the bytes to the host
				GOTO	EndCMD53				; All done

DoCmd53CSA1Bytes
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoCmd53ReadCSA1Bytes	; It's a read operation.
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoCmd53ReadCSA1Bytes
				BSF		STATUS,RP0				; Bank 1
#ifndef	READ_STIMULUS_FILE
				BTFSS	FBR1Start,CSA_Enable	; Is the CSA enabled?
				GOTO	DoCmd53ReadStuffBytes	; No, just return 0x00.
#endif
				CALL	SendSpiFlashBytes		; Send the bytes
				GOTO	EndCMD53				; All done


DoCmd53ReadStuffBytes
				CALL	SendSpiStuffBytes		; Send "Stuff Bytes" to the host
				GOTO	EndCMD53				; All done

DoCmd53IgnoreBytes
				CALL	ReadSpiIgnoreBytes		; Read and ignore the bytes that are sent!
				GOTO	EndCMD53				; All done

DoFn1Cmd53
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing the msb of Byte count
				ANDLW	MsbByteCountMask		; Mask all but bit 0.
				MOVWF	(ByteCount+1)			; Byte count is 9 bits
				MOVF	(SpiRcvBuffer+4),W		; Get the byte containing the LSB of ByteCount
				MOVWF	ByteCount				; Set ByteCount
				BTFSS	STATUS,Z				; IF (ByteCount != 0)
				GOTO	DF1C53RLabel1			; THEN continue
				MOVF	(ByteCount+1),W			; ELSE IF (ByteCount(msb) != 0)
				BTFSS	STATUS,Z
				GOTO	DF1C53RLabel1			; THEN continue
				BTFSC	(SpiRcvBuffer+1),BlockMode		; ELSE IF (BlockMode)
				GOTO	DoR5ParameterError		; THEN BlockMode Infinite Read not supported
				MOVLW	0x02					; ELSE ByteCount = 512
				MOVWF	(ByteCount+1)
DF1C53RLabel1				
				CLRF	DataByte
				CALL	DoR5Response			; Send the R5 Response

				; Is the address > b'00111111111111111'
				MOVF	(SpiRcvBuffer+1),W		; Get the byte containing bits [16:15] of Address
				ANDLW	0x03					; Test if bits 16 or 15 are set
				BTFSC	STATUS,Z
				GOTO	DF1C53Label2
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoCmd53ReadStuffBytes	; Send 0's if out of address range
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.
				
DF1C53Label2
				; Get the lower 15 bits of address in Address register
				MOVF	(SpiRcvBuffer+2),W		; Get the byte containing bits [14:7] of Address
				MOVWF	(Address+1)				; Move the high order address byte
				MOVF	(SpiRcvBuffer+3),W		; Get the byte containing bits [6:0] of Address
				ANDLW	b'11111110'				; Mask unwanted bit
				MOVWF	Address					; Move the low order address byte
				CLRF	STATUS
				RRF		(Address+1),F				; Shift right to byte align it
				BCF		(Address+1),7
				RRF		Address,F
				
				; Is this a valid address? (Address <= 0xFF)
				MOVF	(Address+1),W				; Test the high order byte
				BTFSC	STATUS,Z
				GOTO	DoFn1Cmd53ReadBytes		; This is a valid address
				
				; Else this is an unsupported address
				BTFSS	(SpiRcvBuffer+1),RWFlag	; Is this a read operation?
				GOTO	DoCmd53ReadStuffBytes	; Send 0's if out of address range
				GOTO	DoCmd53IgnoreBytes		; No, It's a write operation. Just ignore the data.

DoFn1Cmd53ReadBytes
				MOVF	Address,W				; Get low-order byte
				ANDLW	b'11000000'
				BTFSS	STATUS,Z				; IF (Address > 0x3F)
				GOTO	DoCmd53ReadStuffBytes	; THEN Send 0's if out of address range
				MOVF	Address,W				; Get low-order byte
				MOVWF	StartAddress
				MOVWF	EndAddress
				MOVF	ByteCount,W				; EndAddress = StartAddress + ByteCount
				ADDWF	EndAddress,F
				DECF	EndAddress,F
				MOVF	EndAddress,W
				ANDLW	b'11000000'				; EndAddress = (EndAddress>0x3F) ? 0x3F : EndAddress
				BTFSC	STATUS,Z
				GOTO	DF1C53RCBLabel1
				MOVLW	0x3F
				MOVWF	EndAddress
DF1C53RCBLabel1
				MOVLW	Fn1RegStartLo8
				ADDWF	StartAddress,F			; Offset start address to start of Function 1 register space
				ADDWF	EndAddress,F			; Offset end address to start of Function 1 register space
				CALL	SendSpiRamBytes			; Send the bytes to the host
				GOTO	EndCMD53				; All done




;--------------------------------------------------------------------------
; Relocatable Functions
;--------------------------------------------------------------------------

;----------------------------------------------------------------------
;
;	Function:	    SendSpiByte
;
;	Description:	This procedure outputs the contents of SpiTxData
;					register to the SPI port. The SPI output shift
;					register is on the CPLD.
;					Note: Ensure Bank Select registers configured
;					for Bank 0 before calling this function.
;
;----------------------------------------------------------------------
SendSpiByte

SSBLP1
#ifndef	READ_STIMULUS_FILE
				BTFSC	RdyN					; Wait for buffer not full
				GOTO	SSBLP1
#endif

				MOVF	SpiTxData,W				; Get the byte to be transmitted
				MOVWF	PORTD
				BCF		WriteN					; Set WriteN low				

SSBLP2
#ifndef	READ_STIMULUS_FILE
				BTFSC	CmdN					; Wait for Data Ack signal
				GOTO	SSBLP2
#endif

				BSF		WriteN					; Set WriteN high

				RETURN							; Return from subroutine



;----------------------------------------------------------------------
;
;	Function:	    SendSpiRamBytes
;
;	Description:	This procedure outputs a block of RAM data to the
;					SPI port. On entry, StartAddress, EndAddress, and
;					ByteCount should be set. 
;					If (EndAddress-StartAddress) < byteCount, then
;					the remaining bytes are sent as 0's (Stuff byte).
;					The OpCode bit in the SpiRcvBuffer is checked to
;					determine if this is a multi-byte read to fixed
;					address or multi-byte read to incrementing
;					address.
;
;----------------------------------------------------------------------
SendSpiRamBytes
				CLRF	STATUS					; Bank 0
				MOVLW	Cmd53StartOfBlock		; Send a CMD53 "Start of block" byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVF	StartAddress,W			; Get start address
				MOVWF	FSR						; Use indirect addressing
SSRBLoop1
				MOVF	FSR,W					; Get current address
				SUBWF	EndAddress,W			; 
				BTFSC	STATUS,C				; IF (FSR<=endAddress)
				GOTO	SSRBLabel1				; THEN continue sending bytes
				GOTO	SSRBLabel2				; ELSE send "Stuff Bytes"
SSRBLabel1
				MOVF	INDF,W					; Get byte at address FSR
				GOTO	SSRBLabel3
SSRBLabel2
				MOVLW	StuffByte				; Insert a stuff byte
SSRBLabel3
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				BTFSC	(SpiRcvBuffer+1),OpCode	; Is this a multi-byte read
				INCF	FSR,F					; to incrementing address?
				DECFSZ	ByteCount,F				; Are we done?
				GOTO	SSRBLoop1				; Not done
								
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				
				RETURN							; Return from subroutine


;----------------------------------------------------------------------
;
;	Function:	    SendSpiFlashBytes
;
;	Description:	This procedure reads flash memory and outputs a block
;					of Flash data to the SPI port. On entry, Fbr1CsaPtr
;					and ByteCount should be set. 
;
;----------------------------------------------------------------------
SendSpiFlashBytes
				CLRF	STATUS					; Bank 0
				
				CALL	WaitCsabusReady			; Wait for buffer not full
												; before we put something on csabus

				MOVLW	ABANK_2					; CPLD Address Bank #2
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr+2,W			; Get bits [23:16] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[16]
				BSF		PCLK					; Set PCLK high
				MOVLW	ABANK_1					; CPLD Address Bank #1
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr+1,W			; Get bits [15:8] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[15:8]
				BSF		PCLK					; Set PCLK high
				MOVLW	ABANK_0					; CPLD Address Bank #0
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr,W			; Get bits [7:0] of CSA Pointer
				CLRF	STATUS					; Bank 0
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[7:0]
				BSF		PCLK					; Set PCLK high
				BSF		STATUS,RP0				; Bank 1
				MOVF	ByteCount,W				; Get the ByteCount
				ADDWF	Fbr1CsaPtr,F			; Increment the CSA Ptr now, before we do the read
				BTFSS	STATUS,C
				GOTO	SSFBLabel1
				INCF	Fbr1CsaPtr+1,F
				BTFSS	STATUS,Z
				GOTO	SSFBLabel1
				INCF	Fbr1CsaPtr+2,F
SSFBLabel1								
				MOVF	(ByteCount+1),W			; Get the ByteCount MSB
				ADDWF	(Fbr1CsaPtr+1),F
				BTFSS	STATUS,C
				GOTO	SSFBLabel2
				INCF	Fbr1CsaPtr+2,F
SSFBLabel2								
				CLRF	STATUS					; Bank 0
				MOVLW	Cmd53StartOfBlock		; Send a CMD53 "Start of block" byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				
				CALL	WaitCsabusReady			; Wait for data to be latched
												; before we tri-state CSA Bus

				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				BCF		#OE						; Set #OE low on Flash

SSFBLoop1				
				CALL	WaitCsabusReady			; Wait for CSA Bus Ready

				BCF		WriteN					; Set WriteN low to send byte
				
				CALL	WaitDataAck


				BSF		WriteN					; Set WriteN high

				CALL	WaitCsabusReady			; Wait for CSA Bus Ready

				BSF		#OE						; Set #OE high on Flash
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				MOVLW	ABANK_NONE | ADDR_INC	; Configure CPLD
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write data
				BSF		PCLK					; Set PCLK high
				BCF		PCLK					; Set PCLK low to increment address
				BSF		PCLK					; Set PCLK high
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				BCF		#OE						; Set #OE low on Flash
				MOVF	ByteCount,W				; Decrement ByteCount
				BTFSC	STATUS,Z
				DECF	(ByteCount+1),F
				DECF	ByteCount,F
				BTFSS	STATUS,Z
				GOTO	SSFBLoop1
				MOVF	(ByteCount+1),W
				BTFSS	STATUS,Z				; Are we done?
				GOTO	SSFBLoop1				; No, send next byte

DoneCMD53ReadCSA

				CALL	WaitCsabusReady			; Wait for CSA Bus Ready

				BSF		#OE						; All done, set #OE high
				BSF		STATUS,RP0				; Bank 1
				BCF		STATUS,RP1				;
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				CLRF	STATUS					; Bank 0

				CALL	WaitCsabusReady			; Wait for CSA Bus Ready

				MOVLW	ABANK_NONE				; CPLD Address Bank #NONE, no ADDR_INC mode
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				BCF		PCLK					; Pulse PCLK to ignore address
				BSF		PCLK					;   data
				
				RETURN							; Return from subroutine



;----------------------------------------------------------------------
;
;	Function:	    SendSpiStuffBytes
;
;	Description:	This procedure outputs a block of "Stuff Bytes" to the
;					SPI port. On entry, ByteCount should be set. 
;
;----------------------------------------------------------------------
SendSpiStuffBytes
				CLRF	STATUS					; Bank 0
				MOVLW	Cmd53StartOfBlock		; Send a CMD53 "Start of block" byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	StuffByte				; Insert 8 stuff bits
				MOVWF	SpiTxData
SSSBLoop1
				CALL	SendSpiByte				; Send the byte to the host
				MOVF	ByteCount,W				; Decrement ByteCount
				BTFSC	STATUS,Z
				DECF	(ByteCount+1),F
				DECF	ByteCount,F
				BTFSS	STATUS,Z
				GOTO	SSSBLoop1
				MOVF	(ByteCount+1),W
				BTFSS	STATUS,Z				; Are we done?
				GOTO	SSSBLoop1				; No, send next byte
								
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	Cmd53CRCByte			; Send a dummy CRC byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				
				RETURN							; Return from subroutine


;----------------------------------------------------------------------
;
;	Function:	    ReadSpiIgnoreBytes
;
;	Description:	This procedure reads a block of "Ignored Bytes" from the
;					SPI port. On entry, ByteCount should be set. 
;
;----------------------------------------------------------------------
ReadSpiIgnoreBytes
				CLRF	STATUS					; Bank 0
				BCF		Led						; Turn the LED off
				CALL	ReadSpiByte				; Read an "ignore" byte
				CLRF	STATUS					; Bank 0
				MOVWF	(Fn1RegStartLo8+32)
				CALL	ReadSpiByte				; Read a CMD53 "Start of block" byte
				CLRF	STATUS					; Bank 0
				MOVWF	(Fn1RegStartLo8+33)
				
RSIBLoop1
				CALL	ReadSpiByte				; Read the data byte from the host
				CLRF	STATUS					; Bank 0
				MOVWF	(Fn1RegStartLo8+34)
				MOVF	ByteCount,W				; Decrement ByteCount
				BTFSC	STATUS,Z
				DECF	(ByteCount+1),F
				DECF	ByteCount,F
				BTFSS	STATUS,Z
				GOTO	RSIBLoop1
				MOVF	(ByteCount+1),W
				BTFSS	STATUS,Z				; Are we done?
				GOTO	RSIBLoop1				; No, read next byte
								
				CALL	ReadSpiByte				; Read a dummy CRC byte
				CLRF	STATUS					; Bank 0
				MOVWF	(Fn1RegStartLo8+35)
				CALL	ReadSpiByte				; Read a dummy CRC byte
				CLRF	STATUS					; Bank 0
				MOVWF	(Fn1RegStartLo8+36)

				MOVLW	0xFF					; Send a dummy byte
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				MOVLW	Cmd53DataAccepted		; Send a data accepted token
				MOVWF	SpiTxData
				CALL	SendSpiByte				; Send the byte to the host
				
				RETURN							; Return from subroutine


;----------------------------------------------------------------------
;
;	Function:	    ReadSpiByte
;
;	Description:	This procedure reads a byte from the
;					SPI port. Data that is read is in W on return.
;
;----------------------------------------------------------------------
; On entry, STATUS must be cleared to ensure
; addressing is correct. (Bank 1).
ReadSpiByte
				BSF		STATUS,RP0				; Bank 1
RSBSpiLoop
#ifdef	READ_STIMULUS_FILE
				GOTO	RSBInputBYTE
#endif
        		BTFSC   SSPSTAT,BF				; Is Data Received?
        		GOTO    RSBInputBYTE			; Yes, then process the byte
EndRSBSpiLoop
        		GOTO    RSBSpiLoop				; Else check again

; Process one byte of input on the SPI port.
; At this point, time is critical. We can assume that RP1 is cleared prior to calling this. 
RSBInputBYTE
				BCF		STATUS,RP0				; Bank 0
				MOVF	SSPBUF,W				; Get received data
        		
RSBEndInputBYTE
				RETURN							; Wait for next byte
				


;----------------------------------------------------------------------
;
;	Function:	    ProgramCPLDLoop
;
;	Description:	This procedure tri-states ports B,D and E on the PIC
;					controller so that the CPLD can be programmed
;					in circuit.
;
;----------------------------------------------------------------------
ProgramCPLDLoop		
				CLRF	STATUS
				BSF		STATUS,RP0				; Bank1
				MOVLW	B'11111111'				; Tri-state Ports B,D,E
				MOVWF	TRISB					;
				MOVWF	TRISD					;
				MOVWF	TRISE					;
				BCF		STATUS,RP0				; Bank0
CPLDEndlessLoop
				GOTO	CPLDEndlessLoop			; Just loop forever, we are programming CPLD
				

				
;----------------------------------------------------------------------
;
;	Function:	    WaitForKey
;
;	Description:	This procedure waits for the button on the
;					board (SW1) to be pressed and released.
;
;----------------------------------------------------------------------
WaitForKey
        		BTFSS   SwitchSW1				; See if SW1 pressed
        		GOTO    KeyDown					; Yes then wait for button release
EndWaitForKey
        		GOTO    WaitForKey				; Else check again
;
KeyDown
        		BTFSS   SwitchSW1				; Wait for key release
        		GOTO    KeyDown					; Not released, wait
EndKeyDown
        		RETURN							; Key released, so return



;----------------------------------------------------------------------
;
;	Function:	    WriteFlashByte
;
;	Description:	This procedure writes a single byte to flash.
;
;----------------------------------------------------------------------
WriteFlashByte
				CLRF	STATUS
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					; PORTD<7:0> = outputs
				BCF		STATUS,RP0				; Bank 0
				CLRF	(Address+2)				; Clear msb of Address
				
; Send first instruction to Flash
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0xAA
				MOVWF	(Address+1)
				MOVLW	0xAA
				MOVWF	DataByte
				CALL	SendFlashCommand
				
; Send second instruction to Flash
				MOVLW	0xAA					; Send instruction to Flash
				MOVWF	Address					; Address = 0x02AA
				MOVLW	0x02					; Data = 0x55
				MOVWF	(Address+1)
				MOVLW	0x55
				MOVWF	DataByte
				CALL	SendFlashCommand
				
; Send third instruction to Flash
				MOVLW	0x55					; Send instruction to Flash
				MOVWF	Address					; Address = 0x0555
				MOVLW	0x05					; Data = 0xA0
				MOVWF	(Address+1)
				MOVLW	0xA0
				MOVWF	DataByte
				CALL	SendFlashCommand
								
; Send fourth instruction (byte to program) to Flash
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaPtr,W			; Send Data to Flash
				CLRF	STATUS					; Bank 0
				MOVWF	Address					; Address = ByteAddress
				BSF		STATUS,RP0				; Bank 1
				MOVF	(Fbr1CsaPtr+1),W		; Data = DataByte
				CLRF	STATUS					; Bank 0
				MOVWF	(Address+1)
				BSF		STATUS,RP0				; Bank 1
				MOVF	(Fbr1CsaPtr+2),W
				CLRF	STATUS					; Bank 0
				MOVWF	(Address+2)
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaWindow,W
				CLRF	STATUS					; Bank 0
				MOVWF	DataByte
				CALL	SendFlashCommand
								
; Read back the status
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'11111111'				; Tri-state csabus
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0
				BCF		#OE						; Set #OE low
				BSF		STATUS,RP0				; Bank 1
				MOVF	Fbr1CsaWindow,W			; Get the data that was programmed
				ANDLW	b'10000000'				; Mask all but bit 7
				MOVWF	w1
				CLRF	STATUS					; Bank 0
CheckFlashStatus
				MOVF	PORTD,W					; Read data from Flash
				ANDLW	b'10000000'				; Mask all but bit 7
				XORWF	w1,W
				BTFSS	STATUS,Z				; IF !match
				GOTO	CheckFlashStatus		; THEN wait for a match
								
FlashComplete
				BSF		#OE						; Set #OE high
				BSF		STATUS,RP0				; Bank 1
				MOVLW	B'00000000'				; Enable csabus for output
				MOVWF	TRISD					;
				BCF		STATUS,RP0				; Bank 0

				RETURN
				
				

				
;----------------------------------------------------------------------
;
;	Function:	    SendFlashCommand
;
;	Description:	This procedure sends a command to flash. Commands
;					are issued by writing command data to specific
;					flash addresses. Upon entry, variables Address and
;					DataByte must be set.
;
;----------------------------------------------------------------------
SendFlashCommand
				CLRF	STATUS					; Bank 0
				MOVLW	ABANK_2					; CPLD Address Bank #2
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				
				MOVF	(Address+2),W			; Put Address hi-byte on csabus
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[16]
				BSF		PCLK					; Set PCLK high

				MOVLW	ABANK_1					; CPLD Address Bank #1
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				
				MOVF	(Address+1),W			; Put Address mid-byte on csabus
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[15:8]
				BSF		PCLK					; Set PCLK high

				MOVLW	ABANK_0					; CPLD Address Bank #0
				MOVWF	PORTD					; Put byte on csabus
				BCF		PCLK					; Set PCLK low to write byte to
				BSF		PCLK					;   Address Bank Select on CPLD
				
				MOVF	Address,W				; Put Address lo-byte on csabus
				MOVWF	PORTD
				BCF		PCLK					; Write data to FlashAddress[7:0]
				BSF		PCLK					; Set PCLK high
				
				MOVF	DataByte,W				; Put command on csabus
				MOVWF	PORTD				
				BCF		#WE						; Set #WE low
				BSF		#WE						; Set #WE high
				
				RETURN
				





;--------------------------------------------------------------------------
; CIS Data
;--------------------------------------------------------------------------

				ORG		0x1800					; Page 24 of Program Memory (256 byte pages)

;----------------------------------------------------------------------
;
;	Table:		    CIS_TABLE
;
;	Description:	This table contains Function 0 tuples. On exit, W
;					contains the byte at offset W (on entry).
;
;					All tuple data is stored in little-endian format.
;
;----------------------------------------------------------------------
CIS_TABLE		
				ADDWF	PCL,F					; Table lookup, W = offset
				RETLW	0x21					; TPL_CODE_CISTPL_FUNCID (0x21)
				RETLW	0x02					; TPL_LINK Link to next Tuple
				RETLW	0x0C					; TPLFID_FUNCTION Card Function Code (0x0C)
				RETLW	0x00					; TPL_FID_SYSINIT (0x00)
				RETLW	0x22					; TPL_CODE_CISTPL_FUNCE (0x22)
				RETLW	0x04					; TPL_LINK Link to next Tuple
				RETLW	0x00					; TPLFE_TYPE Type of extended data (0x00)
				RETLW	TPLFE_FN0_BLK_SIZE_LO	; TPLFE_FN0_BLK_SIZE 
				RETLW	TPLFE_FN0_BLK_SIZE_HI	; TPLFE_FN0_BLK_SIZE
				RETLW	TPLFE_MAX_TRAN_SPEED	; TPLFE_MAX_TRAN_SPEED
				RETLW	0x20					; TPL_CODE_CISTPL_MANFID (0x20)
				RETLW	0x04					; TPL_LINK Link to next Tuple
				RETLW	0x96					; TPLMID_MANF SDIO Card Manufacturer Code
				RETLW	0x02					; TPLMID_MANF SDIO Card Manufacturer Code
				RETLW	0x47					; TPLMID_CARD Minor Version Number
				RETLW	0x53					; TPLMID_CARD Major Version Number
				RETLW	0x15					; TPL_CODE_CISTPL_VERS_1 (0x15)
				RETLW	0x24					; TPL_LINK Link to next Tuple
				RETLW	0x07					; 
				RETLW	0x00					; 
				RETLW	0x50					; P
				RETLW	0x61					; a
				RETLW	0x6C					; l
				RETLW	0x6D					; m
				RETLW	0x2C					; ,
				RETLW	0x20					;  
				RETLW	0x49					; I
				RETLW	0x6E					; n
				RETLW	0x63					; c
				RETLW	0x2E					; .
				RETLW	0x00					;  
				RETLW	0x53					; S
				RETLW	0x44					; D
				RETLW	0x49					; I
				RETLW	0x4F					; O
				RETLW	0x20					;  
				RETLW	0x44					; D
				RETLW	0x65					; e
				RETLW	0x76					; v
				RETLW	0x65					; e
				RETLW	0x6C					; l
				RETLW	0x6F					; o
				RETLW	0x70					; p
				RETLW	0x6D					; m
				RETLW	0x65					; e
				RETLW	0x6E					; n
				RETLW	0x74					; t
				RETLW	0x20					; 
				RETLW	0x43					; C
				RETLW	0x61					; a
				RETLW	0x72					; r
				RETLW	0x64					; d
				RETLW	0x2E					; .
				RETLW	0x00					;  
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)

;----------------------------------------------------------------------
;
;	Table:		    FN1_CIS_TABLE
;
;	Description:	This table contains Function 1 tuples. On exit, W
;					contains the byte at offset W (on entry).
;
;----------------------------------------------------------------------
FN1_CIS_TABLE		
				ADDWF	PCL,F					; Table lookup, W = offset
				RETLW	0x21					; TPL_CODE_CISTPL_FUNCID (0x21)
				RETLW	0x02					; TPL_LINK Link to next Tuple
				RETLW	0x0C					; TPLFID_FUNCTION Card Function Code (0x0C)
				RETLW	0x00					; TPL_FID_SYSINIT (0x00)
				RETLW	0x22					; TPL_CODE_CISTPL_FUNCE (0x22)
				RETLW	0x1C					; TPL_LINK Link to next Tuple
				RETLW	0x01					; TPLFE_TYPE Type of extended data (0x01)
				RETLW	0x01					; TPLFE_FUNCTION_INFO (FN Wake-up support)
				RETLW	0x00					; TPLFE_STD_IO_REV
				RETLW	0x00					; TPLFE_CARD_PSN
				RETLW	0x00					; TPLFE_CARD_PSN
				RETLW	0x00					; TPLFE_CARD_PSN
				RETLW	0x00					; TPLFE_CARD_PSN
				RETLW	TPLFE_CSA_SIZE_LO		; TPLFE_CSA_SIZE (128k Flash RAM)
				RETLW	TPLFE_CSA_SIZE_MID1		; TPLFE_CSA_SIZE
				RETLW	TPLFE_CSA_SIZE_MID2		; TPLFE_CSA_SIZE
				RETLW	TPLFE_CSA_SIZE_HI		; TPLFE_CSA_SIZE
				RETLW	TPLFE_CSA_PROPERTY		; TPLFE_CSA_PROPERTY
				RETLW	TPLFE_MAX_BLK_SIZE_FN1_LO	; TPLFE_MAX_BLK_SIZE 
				RETLW	TPLFE_MAX_BLK_SIZE_FN1_HI	; TPLFE_MAX_BLK_SIZE
				RETLW	0x00					; TPLFE_OCR
				RETLW	0x01					; TPLFE_OCR
				RETLW	0xFF					; TPLFE_OCR
				RETLW	0x00					; TPLFE_OCR
				RETLW	0x08					; TPLFE_OP_MIN_PWR
				RETLW	0x0A					; TPLFE_OP_AVG_PWR
				RETLW	0x0F					; TPLFE_OP_MAX_PWR
				RETLW	0x01					; TPLFE_SB_MIN_PWR
				RETLW	0x01					; TPLFE_SB_AVG_PWR
				RETLW	0x01					; TPLFE_SB_MAX_PWR
				RETLW	0x00					; TPLFE_MIN_BW
				RETLW	0x00					; TPLFE_MIN_BW
				RETLW	0x00					; TPLFE_OPT_BW
				RETLW	0x00					; TPLFE_OPT_BW
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)
				RETLW	0xFF					; TPL_CODE_CISTPL_END (0xFF)

				
				END

