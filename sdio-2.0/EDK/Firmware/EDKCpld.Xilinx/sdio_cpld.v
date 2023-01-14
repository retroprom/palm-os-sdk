/////////////////////////////////////////////////////////////////////////////
//
// SDIO CPLD SDIO_CPLD.v Src File
//
// Copyright (c) 2001 Palm Inc. or its subsidiaries.
// All rights reserved.  This software may be copied and used solely for 
// developing products for the communicating with the Palm Inc. platform
// and for archival and backup purposes.  Except for the foregoing, no part 
// of this software may be reproduced or transmitted in any form or by any 
// means or used to make any derivative work (such as translation, 
// transformation or adaptation) without express written consent from Palm Inc.
//
// Palm Inc. reserves the right to revise this software and to make changes in 
// content from time to time without obligation on the part of Palm Inc. to 
// provide notification of such revision or changes.  PALM INC. MAKES NO 
// REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF ERRORS OR 
// THAT THE SOFTWARE IS SUITABLE FOR YOUR USE.  THE SOFTWARE IS PROVIDED ON
// AN "AS IS" BASIS.  PALM INC. MAKES NO WARRANTIES, TERMS OR CONDITIONS, EXPRESS
// OR IMPLIED, EITHER IN FACT OR BY OPERATION OF LAW, STATUTORY OR 
// OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
//
// TO THE FULL EXTENT ALLOWED BY LAW, PALM INC. ALSO EXCLUDES FOR ITSELF AND ITS
// SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING 
// NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
// PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
// BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING 
// OUT OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF PALM INC. HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES.
//
// HotSync, Palm Inc., and Graffiti are registered trademarks, 
// and Palm III and Palm OS are trademarks of Palm Inc. or its 
// subsidiaries.
//
// IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK, THE OTHER SOFTWARE AND 
// DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT 
// ACCOMPANYING THE COMPACT DISK.
// 
/////////////////////////////////////////////////////////////////////////////
//
// Revision History:
//
// Date         Name        Description
// ----------------------------------------------------------
// 12/20/01		GAR         initial release
//
/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	Filename:	SDIO_CPLD.v
//	Author:		Geoff Richmond
//	
//	Description:	CPLD implementation for interfacing PIC
//					microcontroller to SDIO controller.
//
////////////////////////////////////////////////////////////////////////


`define	AWIDTH		17		// Width of the output Address Bus
`define	DWIDTH		8		// Width of the Data In Bus to load ADDR
`define	BSWIDTH		3		// Bank Select Width (bits necessary to 
									// select each address controller register)


module SDIO(	csabus,
				ICLK,
				SCLK,
				PCLK,
				WriteN,
				RdyN,
				CmdN,
				DI,
				DO,
				CD,
				addr,
				ResetN );

// Inputs
input					ICLK;		// Instruction Clock
input					SCLK;		// SD (SPI) Clock
input 				PCLK;		// Parallel Load Clock
input 				WriteN;	// Write Control
input					DI;		// SD DI - Serial Data In
input					CD;		// SD CD - Card Select
input					ResetN;	// Reset signal

// Outputs
output				DO;		// SD DO - Serial Data Out
output [(`AWIDTH-1):0]		addr;		// Flash address bus
output				RdyN;		// Ready output
output 				CmdN;		// DataAck signal

// I/O's
inout [(`DWIDTH-1):0]			csabus;		// CSA Data Bus


// Combinational Logic

wire								FIFO_SpiRdyN;	// SPI Shift Register ready for data
wire								FIFO_SpiLoadingN;	// SPI Shift Register ready for data
wire								FIFO_Full;		// Fifo Full signal from FIFO
wire								FIFO_FifoRdyN;	// Fifo ready to transmit data
wire [(`DWIDTH-1):0]			FIFO_DataOut;	// Data Out from FIFO

wire								SPI_Shift_Out;	// Serial data out from SPI Shift Register
wire								DO_OutputEnable;	// DO Hi-Z control

wire [(`AWIDTH-1):0]			ADDRDataOut;	// Address Controller output
wire								StrbN;			// Strobe for Address Controller
wire [(`BSWIDTH-1):0]		BSel;				// Bank Select to Address Controller
wire								ADDRInc;		// Increment signal to Address Controller

wire [(`DWIDTH-1):0]			csabus_input;		// CSA Data Bus input

// DO is tri-stated output
assign DO = (CD == 1'b0) ? SPI_Shift_Out : 1'bZ;

// csabus is a bi-directional bus
assign	csabus = ResetN ? 8'bz : 8'b00010000 ;	// Output a version number during reset  high-order 4 bits are major version,
																// low-order 4 bits are minor version.

assign	csabus_input	= csabus;

assign	addr				= ADDRDataOut;



// Module Instances

STATE_CTLR	state_ctlr( .SC_ResetN(ResetN),
					.SC_IClk(ICLK),
					.SC_PClk(PCLK),
					.SC_StrbN(StrbN),
					.SC_BSel(BSel),
					.SC_Addr_Inc(ADDRInc),
					.SC_Data_Bus(csabus_input)
				);

				
SPI_SHIFT_OUT	spi_shift_out( .SPI_Clk(SCLK),
										.SPI_ResetN(ResetN),
										.SPI_Data_RdyN(WriteN),
										.SPI_Data_In(csabus_input),
										.SPI_Sdo(SPI_Shift_Out),
										.SPI_RdyN(RdyN),
										.SPI_DataAck(CmdN)
									);

			
ADDR_CTLR	addr_ctlr(	.ACTL_StrbN(StrbN),
					.ACTL_RstN(ResetN),
					.ACTL_Data_In(csabus_input),
					.ACTL_Inc(ADDRInc),
					.ACTL_Addr_Out(ADDRDataOut),
					.ACTL_BSel(BSel)
				);

endmodule	// SDIO
