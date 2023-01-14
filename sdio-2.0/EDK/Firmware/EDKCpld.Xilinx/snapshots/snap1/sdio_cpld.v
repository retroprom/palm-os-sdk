////////////////////////////////////////////////////////////////////////
//	Filename:	SDIO_CPLD.v
//	Author:		Geoff Richmond
//	
//	Description:	CPLD implementation for interfacing PIC
//					microcontroller to SDIO controller.
//
////////////////////////////////////////////////////////////////////////
`define	FWIDTH		8		// Width of the FIFO - 8 bits

`define	AWIDTH		17		// Width of the output Address Bus
`define	DWIDTH		8		// Width of the Data In Bus to load ADDR
`define	BSWIDTH		3		// Bank Select Width (bits necessary to 
									// select each address controller register)

`define	SDEPTH		8		// Depth of the SPI shift register (bits)
`define	CWIDTH		4		// Width of SPI shift counter, 2^CWIDTH = SDEPTH


`include "fifo.v"
`include "addrctlr.v"
`include "spi_shift_out.v"
`include "state_ctlr.v"

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
input 				CmdN;		// Read Control  (currently unused)
input					DI;		// SD DI - Serial Data In
input					CD;		// SD CD - Card Select
input					ResetN;	// Reset signal

// Outputs
output				DO;		// SD DO - Serial Data Out
output [(`AWIDTH-1):0]		addr;		// Flash address bus
output				RdyN;		// Ready output

// I/O's
input [(`DWIDTH-1):0]			csabus;		// CSA Data Bus


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

// DO is tri-stated output
assign DO = (DO_OutputEnable == 1'b1) ? SPI_Shift_Out : 1'bZ;

assign	addr				= ADDRDataOut;

reg		SCLKInt;

always @(posedge ICLK or negedge ResetN)
begin
	if(!ResetN) begin
		SCLKInt	<= 0;
	end
	
	else begin
		// Handshaking with SPI Shift Register
		SCLKInt	<= SCLK;
	end
end		



// Module Instances

STATE_CTLR	state_ctlr( .SC_ResetN(ResetN),
					.SC_IClk(ICLK),
					.SC_PClk(PCLK),
					.SC_Fifo_Full(FIFO_Full),
					.SC_StrbN(StrbN),
					.SC_BSel(BSel),
					.SC_Addr_Inc(ADDRInc),
					.SC_RdyN(RdyN),
					.SC_Data_Bus(csabus),
					.SC_CmdN(CmdN),
					.SC_DO_OutputEnable(DO_OutputEnable)
				);

				
FIFO	fifo(	.FIFO_Clk(ICLK),
				.FIFO_RstN(ResetN),
				.FIFO_Data_In(csabus),
				.FIFO_WriteN(WriteN),
				.FIFO_RcvrRdyN(FIFO_SpiRdyN),
				.FIFO_RcvrLoadingN(FIFO_SpiLoadingN),
				.FIFO_CmdN(CmdN),
				.FIFO_Data_Out(FIFO_DataOut),
				.FIFO_Full(FIFO_Full),
				.FIFO_DataRdyN(FIFO_FifoRdyN)
			 );
						
SPI_SHIFT_OUT	spi_shift_out( .SPI_Clk(SCLKInt),
										.SPI_ResetN(ResetN),
										.SPI_Data_RdyN(FIFO_FifoRdyN),
										.SPI_Data_In(FIFO_DataOut),
										.SPI_Sdo(SPI_Shift_Out),
										.SPI_AlmostEmptyN(FIFO_SpiRdyN),
										.SPI_EmptyN(FIFO_SpiLoadingN)
									);

			
ADDR_CTLR	addr_ctlr(	.ACTL_StrbN(StrbN),
					.ACTL_RstN(ResetN),
					.ACTL_Data_In(csabus),
					.ACTL_Inc(ADDRInc),
					.ACTL_Addr_Out(ADDRDataOut),
					.ACTL_BSel(BSel),
					.ACTL_CmdN(CmdN)
				);

endmodule	// SDIO
